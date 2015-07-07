function Vanilla_CLNF_on_CS0()

addpath('../PDM_helpers/');
addpath('../fitting/normxcorr2_mex_ALL');
addpath('../fitting/');
addpath('../CCNF/');
addpath('../models/');

[images, detections, labels] = Collect_CS0_imgs();

%% loading the patch experts
   
clmParams = struct;

clmParams.window_size = [25,25; 23,23; 21,21;];

clmParams.numPatchIters = size(clmParams.window_size,1);

[patches] = Load_Patch_Experts( '../models/general/', 'ccnf_patches_*_general.mat', [], [], clmParams);

%% Fitting the model to the provided image

verbose = false; % set to true to visualise the fitting
output_root = './wild_fit_clnf/';

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

shapes_all = zeros(68,size(labels,3), size(labels,1));
labels_all = zeros(size(labels,2),size(labels,3), size(labels,1));
lhoods = zeros(numel(images),1);
all_lmark_lhoods = zeros(num_points, numel(images));
all_views_used = zeros(numel(images),1);

tic
for i=1:numel(images)

    image = imread(images(i).img);
    image_orig = image;
    
    if(size(image,3) == 3)
        image = rgb2gray(image);
    end              

    bbox = detections(i,:);
        
    bbox(3) = bbox(1) + bbox(3);
    bbox(4) = bbox(2) + bbox(4);
    
    [shape,global_params,~,lhood,lmark_lhood,view_used] = Fitting_from_bb(image, [], bbox, pdm, patches, clmParams);

    all_lmark_lhoods(:,i) = lmark_lhood;
    all_views_used(i) = view_used;
    
%     [~,view] = min(sum((patches(1).centers * pi/180 - repmat(global_params(2:4)', size(patches(1).centers,1), 1)).^2,2));visibilities = logical(patches(1).visibilities(view,:))';imshow(image);hold on;plot(shape(visibilities,1), shape(visibilities,2), '.r');hold off;

    % shape correction for matlab format
    shapes_all(:,:,i) = shape - 0.5; % centering the pixel
    labels_all(:,:,i) = labels(i,:,:);

    if(mod(i, 100)==0)
        fprintf('%d done\n', i );
    end

    lhoods(i) = lhood;
    if(verbose)
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
experiment.errors = compute_error_JANUS(labels_all(:,:,1:i), shapes_all(:,:,1:i));
% save the experiment
if(~exist('experiments', 'var'))
    experiments = experiment;
else
    experiments = cat(1, experiments, experiment);
end

%%
output_results = 'results/results_janus_clnf.mat';
save(output_results, 'experiments');
    
end
