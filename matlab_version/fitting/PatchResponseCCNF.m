function [ responses ] = PatchResponseCCNF(patches, patch_experts_class, visibilities, patchExperts, window_size)
%PATCHRESPONSESVM Summary of this function goes here
%   Detailed explanation goes here

    normalisationOptions = patchExperts.normalisationOptionsCol;
    patchSize = normalisationOptions.patchSize;
                  
    responses = cell(size(patches, 1), 1);
    empty = zeros(window_size(1)-patchSize(1)+1, window_size(2)-patchSize(2)+1);
    
    for i = 1:numel(patches(:,1))
        responses{i} = empty;
        if visibilities(i)
            
            col_norm = normalisationOptions.useNormalisedCrossCorr == 1;

            b = zeros(numel(empty), 1);

            num_hl = size(patch_experts_class{i}.thetas,1);
            
            smallRegionVec = patches(i,:);
            smallRegion = reshape(smallRegionVec, window_size(1), window_size(2));

            for hls = 1:num_hl

                w = patch_experts_class{i}.w{hls};
                
                % normalisation needed per each response
                norm_w = patch_experts_class{i}.norm_w{hls};

                response = -norm_w * SVMresponse(smallRegion, w, col_norm, patchSize) - patch_experts_class{i}.thetas(hls, 1);

                % here we include the bias term as well, as it wasn't added
                % during the response calculation
                h1 = 1./(1 + exp(response(:)));
                b = b + (2 * patch_experts_class{i}.alphas(hls) * h1);

            end
            
            % Sigma will be dependent on the size of the patch find the
            % needed precomputed Sigma
            rel_sigma = 1;
            if(numel(patch_experts_class{i}.Sigma) > 1)
                for sig=1:numel(patch_experts_class{i}.Sigma)
                    if(size(patch_experts_class{i}.Sigma{sig},2)==numel(b))
                        rel_sigma = sig;
                        break;
                    end
                end
            end

            response = patch_experts_class{i}.Sigma{rel_sigma} * b;      
            
            % make sure we have no negative responses
            response = response - min(response);
            
            responses{i}(:) = response;
            
        end
    end
    
end

function response = SVMresponse(region, patchExpert, normalise_x_corr,patchSize)

    if(normalise_x_corr)
        
        % the fast mex convolution
        [response] = normxcorr2_mex(patchExpert, region);

        response = response(patchSize(1):end-patchSize(1)+1,patchSize(2):end-patchSize(2)+1);       
    else
        % this assumes that the patch is already normed
        template = rot90(patchExpert,2);
        response = conv2(region, template, 'valid');  
    end
end
