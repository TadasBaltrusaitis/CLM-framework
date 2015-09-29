function Script_CLNF_general_hierarch()

addpath('../PDM_helpers/');
addpath('../fitting/normxcorr2_mex_ALL');
addpath('../fitting/');
addpath('../CCNF/');
addpath('../models/');

% Replace this with the location of in 300 faces in the wild data
if(exist([getenv('USERPROFILE') '/Dropbox/AAM/test data/'], 'file'))
    root_test_data = [getenv('USERPROFILE') '/Dropbox/AAM/test data/'];    
else
    root_test_data = 'F:/Dropbox/Dropbox/AAM/test data/';
end

[images, detections, labels] = Collect_wild_imgs(root_test_data);

%% loading the patch experts and PDMs
   
clmParams = struct;

clmParams.window_size = [25,25; 23,23; 21,21;];
clmParams.numPatchIters = size(clmParams.window_size,1);

[patches] = Load_Patch_Experts( '../models/general/', 'ccnf_patches_*_general.mat', [], [], clmParams);

verbose = true; % set to true to visualise the fitting
output_root = './wild_fit_clnf_hierarch/';

% the default PDM to use
pdmLoc = ['../models/pdm/pdm_68_aligned_wild.mat'];

load(pdmLoc);
pdm = struct; pdm.M = double(M); pdm.E = double(E); pdm.V = double(V);

% the default full face model parameters to use
clmParams.regFactor = [35, 27, 20];
clmParams.sigmaMeanShift = [1.25, 1.375, 1.5]; 
clmParams.tikhonov_factor = [2.5, 5, 7.5];

clmParams.startScale = 1; clmParams.num_RLMS_iter = 10; clmParams.fTol = 0.01;
clmParams.useMultiScale = true; clmParams.use_multi_modal = 1;
clmParams.multi_modal_types  = patches(1).multi_modal_types;

% Loading eye PDM and patch experts
[clmParams_eye, pdm_right_eye, pdm_left_eye] = Load_CLM_params_eye();
[patches_right_eye] = Load_Patch_Experts( '../models/hierarch/', 'ccnf_patches_*_combined.mat', [], [], clmParams_eye);
[patches_left_eye] = Load_Patch_Experts( '../models/hierarch/', 'left_ccnf_patches_*_combined.mat', [], [], clmParams_eye);
clmParams_eye.multi_modal_types  = patches_right_eye(1).multi_modal_types;
right_eye_inds = [43,44,45,46,47,48];
left_eye_inds = [37,38,39,40,41,42];

% Loading mouth PDM and patch experts
[clmParams_mouth, pdm_mouth] = Load_CLM_params_mouth();
[patches_mouth] = Load_Patch_Experts( '../models/hierarch/', 'ccnf_patches_*_mouth_mv.mat', [], [], clmParams_mouth);
clmParams_mouth.multi_modal_types  = patches_mouth(1).multi_modal_types;
mouth_inds = 49:68;

% Loading brow PDM and patch experts
[clmParams_brow, pdm_brow] = Load_CLM_params_brows();
[patches_brow] = Load_Patch_Experts( '../models/hierarch/', 'ccnf_patches_*_brow.mat', [], [], clmParams_brow);
clmParams_mouth.multi_modal_types  = patches_brow(1).multi_modal_types;
brow_inds = 18:27;

%% for recording purposes
experiment.params = clmParams;

num_points = numel(M)/3;

shapes_all = zeros(size(labels,2),size(labels,3), size(labels,1));
labels_all = zeros(size(labels,2),size(labels,3), size(labels,1));
lhoods = zeros(numel(images),1);
all_lmark_lhoods = zeros(num_points, numel(images));
all_views_used = zeros(numel(images),1);

% Use the multi-hypothesis model, as bounding box tells nothing about
% orientation
multi_view = true;

%% Fitting the model to the provided images
tic
for i=1:numel(images)

    image = imread(images(i).img);
    image_orig = image;
    
    if(size(image,3) == 3)
        image = rgb2gray(image);
    end              

    bbox = detections(i,:);                  
    
    % have a multi-view version
    if(multi_view)

        views = [0,0,0; 0,-30,0; -30,0,0; 0,30,0; 30,0,0];
        views = views * pi/180;                                                                                     

        shapes = zeros(num_points, 2, size(views,1));
        ls = zeros(size(views,1),1);
        lmark_lhoods = zeros(num_points,size(views,1));
        views_used = zeros(num_points,size(views,1));

        % Find the best orientation
        for v = 1:size(views,1)
            [shapes(:,:,v),~,~,ls(v),lmark_lhoods(:,v),views_used(v)] = Fitting_from_bb(image, [], bbox, pdm, patches, clmParams, 'orientation', views(v,:));                                            
        end

        [lhood, v_ind] = max(ls);
        lmark_lhood = lmark_lhoods(:,v_ind);

        shape = shapes(:,:,v_ind);
        view_used = views_used(v);

    else
        [shape,~,~,lhood,lmark_lhood,view_used] = Fitting_from_bb(image, [], bbox, pdm, patches, clmParams);
    end

    all_lmark_lhoods(:,i) = lmark_lhood;
    all_views_used(i) = view_used;

    %% now fit the hierarchical models (eyes and mouth)
                            
    % Perform eye fitting now
    shape_r_eye = shape(right_eye_inds, :);

    [ a, R, T, ~, l_params] = fit_PDM_ortho_proj_to_2D_no_reg(pdm_right_eye.M, pdm_right_eye.E, pdm_right_eye.V, shape_r_eye);

    bbox = [min(shape_r_eye(:,1)), min(shape_r_eye(:,2)), max(shape_r_eye(:,1)), max(shape_r_eye(:,2))];

    g_param = [a; Rot2Euler(R)'; T];

    [shape_r_eye] = Fitting_from_bb(image, [], bbox, pdm_right_eye, patches_right_eye, clmParams_eye, 'gparam', g_param, 'lparam', l_params);

    % Perform eye fitting now 
    shape_l_eye = shape(left_eye_inds, :);

    [ a, R, T, ~, l_params] = fit_PDM_ortho_proj_to_2D_no_reg(pdm_left_eye.M, pdm_left_eye.E, pdm_left_eye.V, shape_l_eye);

    bbox = [min(shape_l_eye(:,1)), min(shape_l_eye(:,2)), max(shape_l_eye(:,1)), max(shape_l_eye(:,2))];

    g_param = [a; Rot2Euler(R)'; T];

    [shape_l_eye] = Fitting_from_bb(image, [], bbox, pdm_left_eye, patches_left_eye, clmParams_eye, 'gparam', g_param, 'lparam', l_params);

    % Perform mouth fitting now 
    shape_mouth = shape(mouth_inds, :);

    [ a, R, T, ~, l_params] = fit_PDM_ortho_proj_to_2D_no_reg(pdm_mouth.M, pdm_mouth.E, pdm_mouth.V, shape_mouth);

    bbox = [min(shape_mouth(:,1)), min(shape_mouth(:,2)), max(shape_mouth(:,1)), max(shape_mouth(:,2))];

    g_param = [a; Rot2Euler(R)'; T];

    [shape_mouth] = Fitting_from_bb(image, [], bbox, pdm_mouth, patches_mouth, clmParams_mouth, 'gparam', g_param, 'lparam', l_params);

    % Perform brow fitting now 
    shape_brow = shape(brow_inds, :);

    [ a, R, T, ~, l_params] = fit_PDM_ortho_proj_to_2D_no_reg(pdm_brow.M, pdm_brow.E, pdm_brow.V, shape_brow);
    g_param = [a; Rot2Euler(R)'; T];

    bbox = [min(shape_brow(:,1)), min(shape_brow(:,2)), max(shape_brow(:,1)), max(shape_brow(:,2))];

    [shape_brow] = Fitting_from_bb(image, [], bbox, pdm_brow, patches_brow, clmParams_brow, 'gparam', g_param, 'lparam', l_params);
    
    % Now after detections incorporate the eyes back
    % into the face model

    shape(left_eye_inds, :) = shape_l_eye;
    shape(right_eye_inds, :) = shape_r_eye;
    shape(mouth_inds, :) = shape_mouth;
    shape(brow_inds, :) = shape_brow;
    
    [ ~, ~, ~, ~, ~, ~, shape_fit] = fit_PDM_ortho_proj_to_2D_no_reg(pdm.M, pdm.E, pdm.V, shape);
    
    %% Incorporate the hierarchical models back into the joint PDM
    
    shapes_all(:,:,i) = shape_fit;
    labels_all(:,:,i) = labels(i,:,:);

    if(mod(i, 200)==0)
        fprintf('%d done\n', i );
    end

    lhoods(i) = lhood;
    if(verbose)
        actualShape = squeeze(labels(i,:,:));
        [height_img, width_img,~] = size(image_orig);
        width = max(actualShape(:,1)) - min(actualShape(:,1));
        height = max(actualShape(:,2)) - min(actualShape(:,2));

        img_min_x = max(int32(min(actualShape(:,1))) - width/3,1);
        img_max_x = min(int32(max(actualShape(:,1))) + width/3,width_img);

        img_min_y = max(int32(min(actualShape(:,2))) - height/3,1);
        img_max_y = min(int32(max(actualShape(:,2))) + height/3,height_img);

        shape(:,1) = shape(:,1) - double(img_min_x);
        shape(:,2) = shape(:,2) - double(img_min_y);

        image_orig = image_orig(img_min_y:img_max_y, img_min_x:img_max_x, :);    

        % valid points to draw (not to draw
        % occluded ones)
        v_points = sum(squeeze(labels(i,:,:)),2) > 0;

        f = figure('visible','off');
        %f = figure;
        try
        if(max(image_orig(:)) > 1)
            imshow(double(image_orig)/255, 'Border', 'tight');
        else
            imshow(double(image_orig), 'Border', 'tight');
        end
        axis equal;
        hold on;
        
        plot(shape(v_points,1), shape(v_points,2),'.r','MarkerSize',20);
        plot(shape(v_points,1), shape(v_points,2),'.b','MarkerSize',10);
%                                         print(f, '-r80', '-dpng', sprintf('%s/%s%d.png', output_root, 'fit', i));
        print(f, '-djpeg', sprintf('%s/%s%d.jpg', output_root, 'fit', i));
%                                         close(f);
        hold off;
        close(f);
        catch warn

        end
    end

end
toc
experiment.lhoods = lhoods;
experiment.shapes = shapes_all;
experiment.labels = labels_all;
experiment.errors_normed = compute_error(labels_all - 0.5, shapes_all);
experiment.all_lmark_lhoods = all_lmark_lhoods;
experiment.all_views_used = all_views_used;
% save the experiment
if(~exist('experiments', 'var'))
    experiments = experiment;
else
    experiments = cat(1, experiments, experiment);
end
fprintf('experiment %d done: mean normed error %.3f median normed error %.4f\n', ...
    numel(experiments), mean(experiment.errors_normed), median(experiment.errors_normed));

%%
output_results = 'results/results_wild_clnf_general_hierarch.mat';
save(output_results, 'experiments');
    
end
