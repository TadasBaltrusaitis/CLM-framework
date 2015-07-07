function Vanilla_CLNF_jpeg_errors()

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

% load the images to detect landmarks of

csv_loc = 'D:\JANUS_training\aflw\aflw_68_dev.csv';
csv_meta_loc = 'D:\JANUS_training\aflw/metadata_68_dev.csv';
jpeg_root = 'D:\Datasets\AFLW\Janus_Validation_Set\total/';

[images, detections, labels, ~, jpeg_quals] = Collect_AFLW_imgs_jpeg(csv_loc, csv_meta_loc, jpeg_root);

%% loading the patch experts
   
num_levels = numel(jpeg_quals);

clmParams = struct;

clmParams.window_size = [25,25; 23,23; 21,21;];

clmParams.numPatchIters = size(clmParams.window_size,1);

[patches] = Load_Patch_Experts( '../models/general/', 'ccnf_patches_*_general.mat', [], [], clmParams);
% [patches] = Load_Patch_Experts( '../models/janus/', 'ccnf_patches_*_janus.mat', [], [], clmParams);
%% Fitting the model to the provided image

verbose = true; % set to true to visualise the fitting
output_root = './dev_fit_jpeg/';

% the default PDM to use
pdmLoc = ['../models/pdm/pdm_68_aligned_wild.mat'];

load(pdmLoc);

pdm = struct;
pdm.M = double(M);
pdm.E = double(E);
pdm.V = double(V);

% the default model parameters to use
clmParams.regFactor = 25;               
clmParams.sigmaMeanShift = 2;
clmParams.tikhonov_factor = 5;

clmParams.startScale = 1;
clmParams.num_RLMS_iter = 10;
clmParams.fTol = 0.01;
clmParams.useMultiScale = true;
clmParams.use_multi_modal = 1;
clmParams.multi_modal_types  = patches(1).multi_modal_types;
   
% for recording purposes
experiment.params = clmParams;

num_points = numel(M)/3;

shapes_all = zeros(size(labels,2),size(labels,3), size(labels,1), num_levels);
labels_all = zeros(size(labels,2),size(labels,3), size(labels,1), num_levels);
lhoods = zeros(numel(images),num_levels);
all_lmark_lhoods = zeros(num_points, numel(images), num_levels);
all_views_used = zeros(numel(images),num_levels);

% Use the multi-hypothesis model, as bounding box tells nothing about
% orientation
load('../bounding_box_mapping/mappings.mat');

tic
for i=1:size(images,1)
        
    for l=1:num_levels
        image = imread(images{i,l});
        image_orig = image;

        if(size(image,3) == 3)
            image = rgb2gray(image);
        end      
        
        % view variable is loaded in from the mappings
        shapes = zeros(num_points, 2, size(views,1));
        ls = zeros(size(views,1),1);
        lmark_lhoods = zeros(num_points,size(views,1));
        views_used = zeros(num_points,size(views,1));
        global_params_all = zeros(6, size(views,1));

        % Find the best orientation
        for v = 1:size(views,1)

            s_width = s_widths(v);
            s_height = s_heights(v);
            s_tx = s_txs(v);
            s_ty = s_tys(v);

            bbox = detections(i,:);
            bbox_orig = bbox;

            % Correct the widths
            bbox(3) = bbox(:,3) * s_width;
            bbox(4) = bbox(:,4) * s_height;

            % Correct the location
            bbox(1) = bbox(1) + bbox_orig(3) * s_tx;
            bbox(2) = bbox(2) + bbox_orig(4) * s_ty;            

            bbox(3) = bbox(1) + bbox(3);
            bbox(4) = bbox(2) + bbox(4);            

            bbox = bbox + 1;

            [shapes(:,:,v),global_params_all(:,v),~,ls(v),lmark_lhoods(:,v),views_used(v)] = Fitting_from_bb(image, [], bbox, pdm, patches, clmParams, 'orientation', views(v,:));

        end

        [lhood, v_ind] = max(ls);

        lmark_lhood = lmark_lhoods(:,v_ind);

        shape = shapes(:,:,v_ind);
        view_used = v_ind;
        global_params = global_params_all(:,v_ind);

        all_lmark_lhoods(:,i, l) = lmark_lhood;
        all_views_used(i, l) = view_used;

    %     [~,view] = min(sum((patches(1).centers * pi/180 - repmat(global_params(2:4)', size(patches(1).centers,1), 1)).^2,2));visibilities = logical(patches(1).visibilities(view,:))';imshow(image);hold on;plot(shape(visibilities,1), shape(visibilities,2), '.r');hold off;

        % shape correction for matlab format
        shapes_all(:,:,i, l) = shape;
        labels_all(:,:,i) = labels(i,:,:);

        if(mod(i, 200)==0)
            fprintf('%d done\n', i );
        end

        lhoods(i, l) = lhood;
        if(verbose)
            [height_img, width_img,~] = size(image_orig);
            width = max(shape(:,1)) - min(shape(:,1));
            height = max(shape(:,2)) - min(shape(:,2));

            img_min_x = max(int32(min(shape(:,1))) - width/3,1);
            img_max_x = min(int32(max(shape(:,1))) + width/3,width_img);

            img_min_y = max(int32(min(shape(:,2))) - height/3,1);
            img_max_y = min(int32(max(shape(:,2))) + height/3,height_img);

            shape(:,1) = shape(:,1) - double(img_min_x);
            shape(:,2) = shape(:,2) - double(img_min_y);

            image_orig = image_orig(img_min_y:img_max_y, img_min_x:img_max_x, :);    

            % valid points to draw (not to draw
            % occluded ones)
            [~,view] = min(sum((patches(1).centers * pi/180 - repmat(global_params(2:4)', size(patches(1).centers,1), 1)).^2,2));
            visibilities = logical(patches(1).visibilities(view,:))';        

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

            plot(shape(visibilities,1), shape(visibilities,2),'.r','MarkerSize',20);
            plot(shape(visibilities,1), shape(visibilities,2),'.b','MarkerSize',10);
    %                                         print(f, '-r80', '-dpng', sprintf('%s/%s%d.png', output_root, 'fit', i));
            print(f, '-djpeg', sprintf('%s/%s%d_level_%d.jpg', output_root, 'fit', i, jpeg_quals(l)));
    %                                         close(f);
            hold off;
            close(f);
            catch warn

            end
        end
    end
end
toc
experiment.lhoods = lhoods;
experiment.shapes = shapes_all;
experiment.labels = labels_all;
experiment.aflw_error = zeros(size(images,1), num_levels);
for i=1:num_levels
    experiment.aflw_error(:,i) = compute_error(labels, squeeze(shapes_all(:,:,:,i)) - 0.5, detections);
    fprintf('Error jpeg %d -  mean: %.3f median %.4f\n', ...
    jpeg_quals(i), mean(experiment.aflw_error(:,i)), median(experiment.aflw_error(:,i)));
end
experiment.all_lmark_lhoods = all_lmark_lhoods;
experiment.all_views_used = all_views_used;
% save the experiment
if(~exist('experiments', 'var'))
    experiments = experiment;
else
    experiments = cat(1, experiments, experiment);
end

%%
output_results = 'results/results_dev_clnf_jpeg.mat';
save(output_results, 'experiments');
    
end
