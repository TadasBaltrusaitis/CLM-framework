clear
addpath('../PDM_helpers/');
addpath(genpath('../fitting/'));
addpath('../models/');
addpath(genpath('../face_detection'));
addpath('../CCNF/');

%% loading the patch experts
   
[clmParams, pdm] = Load_CLM_params_wild();

[clmParams_eye, pdm_eye] = Load_CLM_params_eye();

% An accurate CCNF (or CLNF) model
% [patches] = Load_Patch_Experts( '../models/general/', 'ccnf_patches_*_general.mat', [], [], clmParams);
% A simpler (but less accurate SVR)
[patches] = Load_Patch_Experts( '../models/general/', 'svr_patches_*_general.mat', [], [], clmParams);

[patches_eye] = Load_Patch_Experts( 'C:\Users\Tadas\Dropbox\AAM\patch_experts_eyes\svr_training\trained/', 'svr_patches_*_synth.mat', [], [], clmParams);

clmParams.multi_modal_types  = patches(1).multi_modal_types;

clmParams_eye.multi_modal_types  = patches_eye(1).multi_modal_types;

%%
root_dir = 'C:\Users\Tadas\Dropbox\AAM\test data\gaze_original\p00/';
images = dir([root_dir, '*.jpg']);

verbose = true;

for img=1:numel(images)
    image_orig = imread([root_dir images(img).name]);

    % First attempt to use the Matlab one (fastest but not as accurate, if not present use yu et al.)
%     [bboxs, det_shapes] = detect_faces(image_orig, {'cascade', 'yu'});
    % Zhu and Ramanan and Yu et al. are slower, but also more accurate 
    % and can be used when vision toolbox is unavailable
    % [bboxs, det_shapes] = detect_faces(image_orig, {'yu', 'zhu'});
    
    % The complete set that tries all three detectors starting with fastest
    % and moving onto slower ones if fastest can't detect anything
    [bboxs, det_shapes] = detect_faces(image_orig, {'cascade', 'yu', 'zhu'});
    
    if(size(image_orig,3) == 3)
        image = rgb2gray(image_orig);
    end              

    %%

    if(verbose)
        f = figure;    
        if(max(image(:)) > 1)
            imshow(double(image_orig)/255, 'Border', 'tight');
        else
            imshow(double(image_orig), 'Border', 'tight');
        end
        axis equal;
        hold on;
    end

    for i=1:size(bboxs,2)

        % Convert from the initial detected shape to CLM model parameters,
        % if shape is available
        
        bbox = bboxs(:,i);
        
        if(~isempty(det_shapes))
            shape = det_shapes(:,:,i);
            inds = [1:60,62:64,66:68];
            M = pdm.M([inds, inds+68, inds+68*2]);
            E = pdm.E;
            V = pdm.V([inds, inds+68, inds+68*2],:);
            [ a, R, T, ~, params, err, shapeOrtho] = fit_PDM_ortho_proj_to_2D(M, E, V, shape);
            g_param = [a; Rot2Euler(R)'; T];
            l_param = params;

            % Use the initial global and local params for clm fitting in the image
            [shape,~,~,lhood,lmark_lhood,view_used] = Fitting_from_bb(image, [], bbox, pdm, patches, clmParams, 'gparam', g_param, 'lparam', l_param);
        else
            [shape,~,~,lhood,lmark_lhood,view_used] = Fitting_from_bb(image, [], bbox, pdm, patches, clmParams);
        end
        
        % shape correction for matlab format
        shape = shape + 1;

        if(verbose)

            % valid points to draw (not to draw self-occluded ones)
            v_points = logical(patches(1).visibilities(view_used,:));

            try

            plot(shape(v_points,1), shape(v_points',2),'.r','MarkerSize',20);
            plot(shape(v_points,1), shape(v_points',2),'.b','MarkerSize',10);

            catch warn

            end
        end
        
        % Map from detected landmarks to eye params
        shape_r_eye = zeros(20,2);
        shape_r_eye([9,11,13,15,17,19],:) = shape([43,44,45,46,47,48], :);
        
        [ a, R, T, ~, params, err, shapeOrtho] = fit_PDM_ortho_proj_to_2D(pdm_eye.M, pdm_eye.E, pdm_eye.V, shape_r_eye);
            
        g_param = [a; Rot2Euler(R)'; T];
        l_param = params;

        % Use the initial global and local params for clm fitting in the image
        patches_eye(1).visibilities(1:8) = 0;
        patches_eye(2).visibilities(1:8) = 0;
        patches_eye(3).visibilities(1:8) = 0;
        [shape_eye,~,~,lhood,lmark_lhood,view_used] = Fitting_from_bb(image, [], bbox, pdm_eye, patches_eye, clmParams_eye, 'gparam', g_param, 'lparam', l_param);

        plot(shape_eye(:,1), shape_eye(:,2), '.g', 'MarkerSize',15);
%         % Now do the eyes
%         min_x = shape(43,1);
%         max_x = shape(43,1);
%         bbox_eye = shape(43,1)
        
    end
    hold off;
    
end