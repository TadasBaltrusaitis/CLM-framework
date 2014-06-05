function [patches] = Load_Patch_Experts( col_patch_dir, col_patch_file, depth_patch_dir, depth_patch_file, clmParams)
%LOAD_PATCH_EXPERTS Summary of this function goes here
%   Detailed explanation goes here
   
    colourPatchFiles = dir([col_patch_dir col_patch_file]);
    
    % load all of the pathes
    for i=1:numel(colourPatchFiles)
        
        load([col_patch_dir, colourPatchFiles(i).name]);

        % determine patch type (slightly hacky but SVR patch experts don't
        % CCNF ratio variable)
        if(isfield(normalisationOptions, 'ccnf_ratio'))
            
            patch = struct;
            patch.centers = centers;
            patch.trainingScale = trainingScale;
            patch.visibilities = visiIndex; 
            patch.patch_experts = patch_experts.patch_experts;
            patch.correlations = patch_experts.correlations;
            patch.rms_errors = patch_experts.rms_errors;
            patch.modalities = patch_experts.types;
            patch.multi_modal_types = patch_experts.types;
            
            patch.type = 'CCNF';
            
            % Knowing what normalisation was performed during training is
            % important for fitting
            patch.normalisationOptionsCol = normalisationOptions;

            % As the similarity inverses will depend on the window size
            % and alphas and betas, but not actual data, precalculate
            % them here
            
            % create the similarity inverses
            window_sizes = unique(clmParams.window_size(:));
            
            for s=1:size(window_sizes,1)
                               
                for view=1:size(patch.patch_experts,1)
                    for lmk=1:size(patch.patch_experts,2)
                        if(visiIndex(view, lmk))
                            num_modalities = size(patch.patch_experts{view,lmk}.thetas,3);
                            
                            num_hls = size(patch.patch_experts{view,lmk}.thetas,1);
                            
                            patchSize = sqrt(size( patch.patch_experts{view,lmk}.thetas,2)-1);
                            patchSize = [patchSize, patchSize];

                            % normalisation so that patch expert can be
                            % applied using convolution
                            w = cell(num_hls, num_modalities);
                            norm_w = cell(num_hls, num_modalities);

                            for hl=1:num_hls
                                for p=1:num_modalities

                                    w_c = patch.patch_experts{view,lmk}.thetas(hl, 2:end, p);
                                    norm_w_c = norm(w_c);
                                    w_c = w_c/norm(w_c);
                                    w_c = reshape(w_c, patchSize);
                                    w{hl,p} = w_c;
                                    norm_w{hl,p} = norm_w_c;
                                end
                            end

                            patch.patch_experts{view,lmk}.w = w;
                            patch.patch_experts{view,lmk}.norm_w = norm_w;
                            
                            similarities = {};
                            response_side_length = window_sizes(s) - 11 + 1;
                            for st=1:size(patch.patch_experts{view,lmk}.similarity_types, 1)
                                type_sim = patch.patch_experts{view,lmk}.similarity_types{st};
                                neighFn = @(x) similarity_neighbor_grid(x, response_side_length(1), type_sim);
                                similarities = [similarities; {neighFn}];
                            end
                            
                            sparsities = {};

                            for st=1:size(patch.patch_experts{view,lmk}.sparsity_types, 1)
                                spFn = @(x) sparsity_grid(x, response_side_length(1), patch.patch_experts{view,lmk}.sparsity_types(st,1), patch.patch_experts{view,lmk}.sparsity_types(st,2));
                                sparsities = [sparsities; {spFn}];
                            end
                                                            
                            region_length = response_side_length^2;
                                
                            [ ~, ~, PrecalcQ2sFlat, ~ ] = CalculateSimilarities_sparsity( 1, {zeros(region_length,1)}, similarities, sparsities);

                            PrecalcQ2flat = PrecalcQ2sFlat{1};
                            
                            SigmaInv = CalcSigmaCCNFflat(patch.patch_experts{view,lmk}.alphas, patch.patch_experts{view,lmk}.betas, region_length, PrecalcQ2flat, eye(region_length), zeros(region_length));
                            if(s == 1)
                                patch.patch_experts{view,lmk}.Sigma = {inv(SigmaInv)};
                            else
                                patch.patch_experts{view,lmk}.Sigma = cat(1, patch.patch_experts{view,lmk}.Sigma, {inv(SigmaInv)});
                            end
                        end
                    end
                end
            end
            
            if(i==1)
                patches = patch;
            else
                patches = [patches; patch];
            end
            
            
        else
            % creating the struct
            patch = struct;            
            
            patch.centers = centers;
            patch.trainingScale = trainingScale;
            patch.visibilities = visiIndex;
            patch.type = 'SVR';
            
            % if the normalisation options present in the loaded patch use
            % them, if not we use default values
            if(exist('normalisationOptions', 'var'))
                patch.normalisationOptionsCol = normalisationOptions;
            else
                patch.normalisationOptionsCol = normalisationColour;
            end

            % default for depth uses normalised-cross-corr per each sample,
            % rather than in a broad area
            if(~isfield(patch.normalisationOptionsCol, 'useZeroMeanPerPatch'))
                patch.normalisationOptionsCol.zscore = 0;
                patch.normalisationOptionsCol.useNormalisedCrossCorr = 1;
                patch.normalisationOptionsCol.useZeroMeanPerPatch = 1;
            end

            % Multi-modal section
            patch.patch_experts = cell(size(visiIndex,1), size(visiIndex,2));

            for view=1:size(visiIndex,1)
                for landmark=1:size(visiIndex,2)
                    patch.patch_experts{view, landmark} = struct;
                    multi_modal_types = patch_experts.types;
                    for p=1:numel(patch_experts.types)
                        patch.patch_experts{view, landmark}(p).type = patch_experts.types(p);
                        patch.patch_experts{view, landmark}(p).correlations = patch_experts.correlations(p);
                        patch.patch_experts{view, landmark}(p).rms_errors = patch_experts.rms_errors(p);

                        patch.patch_experts{view,landmark}(p).scaling = patch_experts.patch_experts{p}(view, landmark, 1);
                        patch.patch_experts{view,landmark}(p).bias = patch_experts.patch_experts{p}(view, landmark, 2);

                        patch.patch_experts{view,landmark}(p).w = reshape(patch_experts.patch_experts{p}(view, landmark, 3:end),11,11);                        

                    end
                end
            end

            patch.multi_modal_types = multi_modal_types;
            
            if(i==1)
                patches = patch;
            else
                patches = [patches; patch];
            end
        
        end
        clear 'normalisationOptions, centers, trainingScale, visiIndex, correlations, rmsErrors';
    end

    
    if(~isempty(depth_patch_file))
        
    depthPatchFiles = dir([depth_patch_dir depth_patch_file]);
       
    % load all of the depth patches
    for i=1:numel(depthPatchFiles)
        
        load([depth_patch_dir, depthPatchFiles(i).name]);
        
        % assuming that same view seen in depth and intensity
            
        % if the normalisation options present in the loaded patch use
        % them, if not we use default values
        if(exist('normalisationOptions', 'var'))
            patches(i).normalisationOptionsDepth = normalisationOptions;
        else
            patches(i).normalisationOptionsDepth = normalisationDepth;
        end

        % Multi-modal section
        patches(i).patch_experts_depth = cell(size(visiIndex,1), size(visiIndex,2));

        for view=1:size(visiIndex,1)
            for landmark=1:size(visiIndex,2)
                patches(i).patch_experts_depth{view, landmark} = struct;
                if(exist('patch_m', 'var'))
                    multi_modal_types = patch_m.types;
                    for p=1:numel(patch_m.types)
                        patches(i).patch_experts_depth{view, landmark}(p).type = patch_m.types(p);
                        patches(i).patch_experts_depth{view, landmark}(p).correlations = patch_m.correlations(p);
                        patches(i).patch_experts_depth{view, landmark}(p).rms_errors = patch_m.rms_errors(p);

                        patches(i).patch_experts_depth{view,landmark}(p).scaling = patch_m.patch_experts{p}(view, landmark, 1);
                        patches(i).patch_experts_depth{view,landmark}(p).bias = patch_m.patch_experts{p}(view, landmark, 2);
                        patches(i).patch_experts_depth{view,landmark}(p).w = reshape(patch_m.patch_experts{p}(view, landmark, 3:end),11,11);                        
                    end
                else
                    multi_modal_types = {'reg'};
                    % for backward compatibility
                    patches(i).patch_experts_depth{view, landmark}(1).type = {'reg'};
                    patches(i).patch_experts_depth{view, landmark}(1).correlations = correlations;
                    patches(i).patch_experts_depth{view, landmark}(1).rms_errors = rmsErrors;

                    patches(i).patch_experts_depth{view,landmark}(1).scaling = patchExperts(view, landmark, 1);
                    patches(i).patch_experts_depth{view,landmark}(1).bias = patchExperts(view, landmark, 2);
                    patches(i).patch_experts_depth{view,landmark}(1).w = reshape(patchExperts(view, landmark, 3:end), 11,11);
                end
            end
        end

    clear 'normalisationOptions, centers, trainingScale, visiIndex, correlations, rmsErrors';
    end
    end
end

