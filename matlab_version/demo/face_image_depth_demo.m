clear
addpath('../PDM_helpers/');
addpath(genpath('../fitting/'));
addpath('../models/');
addpath(genpath('../face_detection'));
addpath('../CCNF/');

%% loading the patch experts
   
[clmParams, pdm] = Load_CLM_params_66();

% A CLM-Z model trained on Multi-PIE and BU-4DFE
[patches] = Load_Patch_Experts( '../models/clmz/', 'svr_patches_multi_pie_*.mat', '../models/clmz/', 'svr_depth_patches_*.mat', clmParams);

clmParams.multi_modal_types  = patches(1).multi_modal_types;

%%

images = {'sample_depth_imgs/1.jpg', 'sample_depth_imgs/2.jpg', 'sample_depth_imgs/3.jpg', 'sample_depth_imgs/4.jpg', 'sample_depth_imgs/5.jpg'};
images_depth = {'sample_depth_imgs/1d.png', 'sample_depth_imgs/2d.png', 'sample_depth_imgs/3d.png', 'sample_depth_imgs/4d.png', 'sample_depth_imgs/5d.png'};
verbose = true;

for img=1:numel(images)

    image_orig = imread(images{img});           
    
    image_depth = imread(images_depth{img});
            
    % Need to convert from the disparity to depth values, and threshold
    image_depth = 10000./(image_depth);
    image_depth(image_depth > 300) = 0;

    % First attempt to use the Matlab one (fastest but not as accurate, if not present use yu et al.)
    [bboxs] = detect_faces(image_orig, {'cascade', 'zhu'});

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

        % Convert from the initial detected shape to CLM model parameters
        bbox = bboxs(:,i);

        % Use the initial global and local params for clm fitting in the image
        [shape,~,~,lhood,lmark_lhood,view_used] = Fitting_from_bb(image, image_depth, bbox, pdm, patches, clmParams);

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

    end
    hold off;
    
end