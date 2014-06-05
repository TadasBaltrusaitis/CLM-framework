function [ responses ] = PatchResponseSVM_multi_modal( patches, patch_experts, visibilities, normalisationOptions, clmParameters, window_size)
%PATCHRESPONSESVM Summary of this function goes here
%   Detailed explanation goes here
    patchSize = normalisationOptions.patchSize;
                      
    responses = cell(size(patches, 1), 1);
    empty = zeros(window_size(1)-patchSize(1)+1, window_size(2)-patchSize(2)+1);
    

    % prepare the patches through either turning them to gradients or 
    if(clmParameters.use_multi_modal)
        
        patches_to_use = cell(numel(clmParameters.multi_modal_types),1);

        for t=1:numel(clmParameters.multi_modal_types)
            if(strcmp(clmParameters.multi_modal_types{t}, 'reg'))
                patches_reg = patches;
                if(normalisationOptions.zscore)
                    meanCurr = mean(patches, 2);
                    stdCurr = std(double(patches), 0, 2);

                    stdCurr(stdCurr == 0) = 1;

                    patches_reg = bsxfun(@minus, patches, meanCurr);

                    patches_reg = bsxfun(@rdivide, patches_reg, stdCurr);
                end                
                patches_to_use{t} = patches_reg;                
            elseif(strcmp(clmParameters.multi_modal_types{t}, 'grad'))
                v = [1];
                h = [-1 0 1];
                grad_patches = zeros(size(patches));
                for i = 1:numel(patches(:,1))
                    if visibilities(i)
                        currSample = reshape(patches(i,:), window_size(1), window_size(2));
                        edgeX = conv2(conv2(currSample, v, 'same'), h, 'same');
                        edgeY = conv2(conv2(currSample, v', 'same'), h', 'same');
                        grad = edgeX.^2 + edgeY.^2;
                        grad(1,:) = 0;
                        grad(:,1) = 0;
                        grad(end,:) = 0;
                        grad(:,end) = 0;
                        grad_patches(i,:) = reshape(grad, window_size(1) * window_size(1),1);
                    end
                end
                patches_to_use{t}  = grad_patches;
            end        
        end
    else
        patches_reg = patches;
        if(normalisationOptions.zscore)
            
            if(normalisationOptions.ignoreInvalidInMeanStd)
                
                % invalid data represented with 0, ignore it when computing
                % mean and standard deviation (useful for depth)
                for i = 1:size(patches,1)
                    mask = patches(i,:) ~= 0;
                    
                    meanCurr = mean(patches(i,mask));
                    stdCurr = std(patches(i,mask));
                    
                    patches(i,mask) = patches(i, mask) - meanCurr;
                    if(stdCurr ~= 0)
                        patches(i, mask) = patches(i, mask) ./ meanCurr;
                    end
                    
                    patches(i, ~mask) = normalisationOptions.setIllegalToPost;
                    
                end
                patches_reg = patches;
            else
                meanCurr = mean(patches, 2);
                stdCurr = std(double(patches), 0, 2);

                stdCurr(stdCurr == 0) = 1;

                patches_reg = bsxfun(@minus, patches, meanCurr);

                patches_reg = bsxfun(@rdivide, patches_reg, stdCurr);

            end
            
        end
        patches_to_use = {patches_reg};
    end
        
    for i = 1:numel(patches(:,1))
        responses{i} = empty;
        if visibilities(i)
%             responses{i} = ones(size(empty));
            colNorm = normalisationOptions.useNormalisedCrossCorr == 1;
            
            for p=1:numel(patches_to_use)
                
                smallRegionVec = patches_to_use{p}(i,:);
                smallRegion = reshape(smallRegionVec, window_size(1), window_size(2));                

                % get the patch response
                response = SVMresponse(smallRegion, patch_experts{i}(p).w, colNorm, patchSize);   

                response = (exp(-(patch_experts{i}(p).scaling*response+patch_experts{i}(p).bias))+1).^-1;    
                
                if(p==1)
                    responses{i} = response;
                else    
                    responses{i} = responses{i} .* response;
                end     
            end
                                       
            normOp = (sum(responses{i}(:)));
            if(normOp ~= 0)
                  responses{i} = responses{i} ./ normOp;
            end
        end
    end
    
end

function response = SVMresponse(region, patchExpert, normalise_x_corr,patchSize)

    if(normalise_x_corr)
        % the much faster mex version
        [response] = normxcorr2_mex(patchExpert, region);

        response = response(patchSize(1):end-patchSize(1)+1,patchSize(2):end-patchSize(2)+1);       
    else        
        % this assumes that the patch is already normed
        template = rot90(patchExpert,2);
        response = conv2(region, template, 'valid');        
    end
end
