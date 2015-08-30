function [ shape2D, global_params, local_params, final_lhood, landmark_lhoods, view_used ] = Fitting_from_bb( Image, DepthImage, bounding_box, PDM, patchExperts, clmParams, varargin)
%FITTING Summary of this function goes here
%   Detailed explanation goes here

    % the bounding box format is [minX, minY, maxX, maxY];   

    % the mean model shape
    M = PDM.M;         

    num_points = numel(M) / 3;
    
    if(any(strcmp(varargin,'orientation')))
        orientation = varargin{find(strcmp(varargin, 'orientation'))+1};        
        rot = Euler2Rot(orientation);        
    else
        rot = eye(3);
        orientation = [0;0;0];
    end
    
    rot_m = rot * reshape(M, num_points, 3)';
    width_model = max(rot_m(1,:)) - min(rot_m(1,:));
    height_model = max(rot_m(2,:)) - min(rot_m(2,:));

    a = (((bounding_box(3) - bounding_box(1)) / width_model) + ((bounding_box(4) - bounding_box(2))/ height_model)) / 2;
        
    tx = (bounding_box(3) + bounding_box(1))/2;
    ty = (bounding_box(4) + bounding_box(2))/2;
    
    % correct it so that the bounding box is just around the minimum
    % and maximum point in the initialised face
    tx = tx - a*(min(rot_m(1,:)) + max(rot_m(1,:)))/2;
    ty = ty - a*(min(rot_m(2,:)) + max(rot_m(2,:)))/2;

    % visualisation of the initial state
    %hold off;imshow(Image);hold on;plot(a*rot_m(1,:)+tx, a*rot_m(2,:)+ty,'.r');hold on;rectangle('Position', [bounding_box(1), bounding_box(2), bounding_box(3)-bounding_box(1), bounding_box(4)-bounding_box(2)]);
    global_params = [a, 0, 0, 0, tx, ty]';
    global_params(2:4) = orientation;

    local_params = zeros(numel(PDM.E), 1);
    
    if(any(strcmp(varargin,'gparam')))
        global_params = varargin{find(strcmp(varargin, 'gparam'))+1};
    end
    
    if(any(strcmp(varargin,'lparam')))
        local_params = varargin{find(strcmp(varargin, 'lparam'))+1};
    end
    
    scale = clmParams.startScale;              
            
    if(size(Image, 3) == 1)
        GrayImage = Image;
    else
        GrayImage = rgb2gray(Image);
    end
    
    [heightImg, widthImg] = size(GrayImage);

    % Some predefinitions for faster patch extraction
    [xi, yi] = meshgrid(0:widthImg-1,0:heightImg-1);
    xi = double(xi);
    yi = double(yi);
    
    GrayImageDb = double(GrayImage);
    
    clmParams_old = clmParams;
    
    % multi iteration refinement using NU-RLMS in each one
    for i=1:clmParams.numPatchIters
      
        current_patch_scaling = patchExperts(scale).trainingScale;
        visibilities = patchExperts(scale).visibilities;
        
        view = GetView(patchExperts(scale).centers, global_params(2:4));  
       
        % The shape fitting is performed in the reference frame of the
        % patch training scale
        refGlobal = [current_patch_scaling, 0, 0, 0, 0, 0]';

        % the reference shape
        refShape = GetShapeOrtho(M, PDM.V, local_params, refGlobal);

        % shape around which the patch experts will be evaluated in the original image
        [shape2D] = GetShapeOrtho(M, PDM.V, local_params, global_params);
        shape2D_img = shape2D(:,1:2);
        
        % Create transform using a slightly modified version of Kabsch that
        % takes scaling into account as well, in essence we get a
        % similarity transform from current estimate to reference shape
        [A_img2ref, T_img2ref, ~, ~] = AlignShapesWithScale(shape2D_img(:,1:2),refShape(:,1:2));

        % Create a transform, from shape in image to reference shape
        T = maketform('affine', [A_img2ref;T_img2ref]);
                
        shape_2D_ref = tformfwd(T, shape2D_img);
        
        % transform the current shape to the reference one, so we can
        % interpolate
        shape2D_in_ref = (A_img2ref * shape2D_img')';
        
        sideSizeX = (clmParams.window_size(i,1) - 1)/2;
        sideSizeY = (clmParams.window_size(i,2) - 1)/2;
        
        patches = zeros(size(shape2D_in_ref,1), clmParams.window_size(i,1) * clmParams.window_size(i,2));

        Ainv = inv(A_img2ref);
        
        % extract patches on which patch experts will be evaluted
        for l=1:size(shape2D_in_ref,1)      
            if(visibilities(view,l))

                xs = (shape2D_in_ref(l,1)-sideSizeX):(shape2D_in_ref(l,1)+sideSizeX);
                ys = (shape2D_in_ref(l,2)-sideSizeY):(shape2D_in_ref(l,2)+sideSizeY);                
                
                [xs, ys] = meshgrid(xs, ys);

                pairs = [xs(:), ys(:)];
                
                actualLocs = (Ainv * pairs')';
                
                actualLocs(actualLocs(:,1) < 0,1) = 0;
                actualLocs(actualLocs(:,2) < 0,2) = 0;
                actualLocs(actualLocs(:,1) > widthImg - 1,1) = widthImg - 1;
                actualLocs(actualLocs(:,2) > heightImg - 1,2) = heightImg - 1;
                
                [t_patch] = interp2_mine(xi, yi, GrayImageDb, actualLocs(:,1), actualLocs(:,2), 'bilinear');
                t_patch = reshape(t_patch, size(xs));

                patches(l,:) = t_patch(:);
                
            end
        end
        
        % Calculate patch responses, either SVR or CCNF
        if(strcmp(patchExperts(scale).type, 'SVR'))            
            responses = PatchResponseSVM_multi_modal( patches, patchExperts(scale).patch_experts(view,:), visibilities(view,:), patchExperts(scale).normalisationOptionsCol, clmParams, clmParams.window_size(i,:));
        elseif(strcmp(patchExperts(scale).type, 'CCNF'))                        
            responses = PatchResponseCCNF( patches, patchExperts(scale).patch_experts(view,:), visibilities(view,:), patchExperts(scale), clmParams.window_size(i,:));
        end
        
        % If a depth image is provided compute patch experts around it as
        % well (unless it's the final iteration)
        if(~isempty(DepthImage) && (i ~= clmParams.numPatchIters))
            
            % Extracting the depth patches here
            patches_depth = zeros(size(shape2D_in_ref,1), clmParams.window_size(i,1) * clmParams.window_size(i,2));

            % extract patches on which patch experts will be evaluted
            for l=1:size(shape2D_in_ref,1)      
                if(visibilities(view,l))

                    xs = (shape2D_in_ref(l,1)-sideSizeX):(shape2D_in_ref(l,1)+sideSizeX);
                    ys = (shape2D_in_ref(l,2)-sideSizeY):(shape2D_in_ref(l,2)+sideSizeY);                

                    [xs, ys] = meshgrid(xs, ys);

                    pairs = [xs(:), ys(:)];

                    actualLocs = (Ainv * pairs')';

                    actualLocs(actualLocs(:,1) < 1,1) = 1;
                    actualLocs(actualLocs(:,2) < 1,2) = 1;
                    actualLocs(actualLocs(:,1) > widthImg,1) = widthImg;
                    actualLocs(actualLocs(:,2) > heightImg,2) = heightImg;

                    % use nearest neighbour interpolation as bilinear would
                    % produce artefacts in depth image (when missing data
                    % is there)
                    [t_patch] = interp2_mine(xi, yi, DepthImage, actualLocs(:,1), actualLocs(:,2), 'nearest');
                    t_patch = reshape(t_patch, size(xs));

                    patches_depth(l,:) = t_patch(:);

                end
            end            

            old_mm = clmParams.use_multi_modal;
            clmParams.use_multi_modal = 0;
            responses_depth = PatchResponseSVM_multi_modal( patches_depth, patchExperts(scale).patch_experts_depth(view,:), visibilities(view,:), patchExperts(scale).normalisationOptionsDepth, clmParams, clmParams.window_size(i,:));
            clmParams.use_multi_modal = old_mm;
            
            % Combining the patch responses from different channels here
            for l=1:size(shape2D_in_ref,1)  
                responses{l} = responses{l} + responses_depth{l};
            end
        end
                
        % the better the correlation in training the more reliable the feature
        % the reliabilities are independent for every modality in SVR, so
        % combine them (also correlation is inverse to variance)
        if(strcmp(patchExperts(scale).type, 'SVR'))
            if(clmParams.use_multi_modal)
                reliabilities = patchExperts(scale).patch_experts{1,1}(end).correlations{1};
            else
                reliabilities = patchExperts(scale).patch_experts{1,1}(1).correlations{1};
            end
        else
            % for CCNF the modalities work together
            reliabilities = patchExperts(scale).correlations;
        end
        reliabilities = reliabilities(view,:);        
               
        % deal with the fact that params might be different for different
        % scales
        if(numel(clmParams_old.regFactor) > 1)
            clmParams.regFactor = clmParams_old.regFactor(i);
        end
        if(numel(clmParams_old.sigmaMeanShift) > 1)
            clmParams.sigmaMeanShift = clmParams_old.sigmaMeanShift(i);
        end
        if(numel(clmParams_old.tikhonov_factor) > 1)
            clmParams.tikhonov_factor = clmParams_old.tikhonov_factor(i);
        end
        
        % The actual NU-RLMS step
        
        % first the rigid transform
        [global_params, local_params] = NU_RLMS(global_params, local_params, PDM, responses, visibilities, view, reliabilities, shape2D_img, T, true, clmParams, []);
        
        % second the combined transform
        [global_params, local_params, final_lhood, landmark_lhoods] = ...
            NU_RLMS(global_params, local_params, PDM, responses, visibilities, view, reliabilities, shape2D_img, T, false, clmParams, []);            

        % Clamp orientation and make sure it doesn't get out of hand
        orientation = global_params(2:4);
        orientation(orientation < -pi/2) = -pi/2;
        orientation(orientation > pi/2) = pi/2;
        global_params(2:4) = orientation;
        
        % move up a scale if possible
        if(clmParams.useMultiScale && scale ~= numel(patchExperts))
            
            % only go up a scale if we don't need to upsample
            if(0.9 * patchExperts(scale+1).trainingScale < global_params(1))
                scale = scale + 1;
            else
                break;
            end
        end
    end
    
    % the view in last iteration
    view_used = view;
    
    % See how good the tracking was in the end
    [shape2D] = GetShapeOrtho(M, PDM.V, local_params, global_params);
    
    % Moving to matlab format
    shape2D = shape2D(:,1:2) + 1;
    
end


function [id] = GetView(centers, rotation)

    [~,id] = min(sum((centers * pi/180 - repmat(rotation', size(centers,1), 1)).^2,2));

end