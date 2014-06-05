clear;
close all;
clc;

addpath('.\model');
S = load('model_param.mat');
Model = S.Model;
pc_version = computer();
if(strcmp(pc_version,'PCWIN')) % currently the code just supports windows OS
    addpath('.\face_detect_32');
    addpath('.\mex_32');
elseif(strcmp(pc_version, 'PCWIN64'))
    addpath('.\face_detect_64');
    addpath('.\mex_64');
end

verbose_out = true;
output_root = '../out_yu/';

Model.frontalL = @(X) Select(X, Model.frontal_landmark);
Model.leftL = @(X) Select(X, Model.left_landmark);
Model.rightL = @(X) Select(X, Model.right_landmark);

%%
load('../../../Patch training regression/data prep/bu_to_use_training.mat');
load('../../../Patch training regression/data prep/mpie_to_use_training.mat');
imgs_used = cat(1, bu_users_to_use, mpie_users_to_use);

od = cd('../');
[images, detections, labels, depth_images] = Create_face_image_test_ibug(imgs_used,'use_afw', 'use_lfpw', 'use_ibug', 'use_helen');
% addpath(od);
cd(od);

%%
lmark_dets_all = zeros(66, 2, numel(images));
% do not care about face outline
valid_points = [18:66];
errors = zeros(size(labels,1),1);
errors_normed = zeros(size(labels,1),1);

shapes_all = zeros(66, 2, size(labels,1));
labels_all = zeros(66, 2, size(labels,1));

%%
for imgs = 1:size(images,1)
            
    landmark = squeeze(labels(imgs,:,:));
    img = imread(['../', images(imgs).img]);
    orig_image = img;
    
    [height_img, width_img, ~] = size(img);
        
    valid_points = landmark(:,1) > 0 & landmark(:,2) > 0;
    valid_points(1:17) = 0;    
    
    width = max(landmark(:,1)) - min(landmark(:,1));
    height = max(landmark(:,2)) - min(landmark(:,2));
    
    img_min_x = max(int32(min(landmark(:,1))) - width/3,1);
    img_max_x = min(int32(max(landmark(:,1))) + width/3,width_img);
 
    img_min_y = max(int32(min(landmark(:,2))) - height/3,1);
    img_max_y = min(int32(max(landmark(:,2))) + height/3,height_img);
    
    % make sure the image is even sized
    if(mod(img_max_x - img_min_x,2) == 0)
        img_max_x = img_max_x - 1;
    end
    if(mod(img_max_y - img_min_y,2) == 0)
        img_max_y = img_max_y - 1;
    end
    
    labels_all(:,:, imgs) = landmark;    
    
    landmark(:,1) = landmark(:,1) - double(img_min_x);
    landmark(:,2) = landmark(:,2) - double(img_min_y);
 
    img = img(img_min_y:img_max_y, img_min_x:img_max_x, :);    
    
    if(size(img,3) == 1)
       img = cat(3, img, img, img); 
    end
    
    tic
    [shape, pglobal, visible] = faceAlign(img, Model, []);
    toc
%     imshow(img);
%     hold on;
    if(~isempty(shape))
        % input: shape, visible, line_color, marker color, marker size, line width, style
%         drawLine(reshape(shape,Model.nPts,2), visible, 'b', 'g', 5, 2, '.'); 
%         hold off;
        
        xs = shape(1:end/2);        
        ys = shape(end/2+1:end);

    %     showboxes(image, bs,posemap),title('All detections above the threshold');

        lmark_dets = [xs, ys];
%         lmark_dets = zeros(66,2);

%         lmark_dets(map_frontal(:,2),:) = lmark_det(map_frontal(:,1),:);

        errors(imgs) = sqrt(mean(sum((lmark_dets(valid_points,:) - landmark(valid_points,:)).^2,2)));

        width = ((max(landmark(valid_points,1)) - min(landmark(valid_points,1)))+(max(landmark(valid_points,2)) - min(landmark(valid_points,2))))/2;

        % lmark in orig img
        lmark_dets_in_orig = lmark_dets;
        lmark_dets_in_orig(:,1) = lmark_dets(:,1) +  double(img_min_x);
        lmark_dets_in_orig(:,2) = lmark_dets(:,2) +  double(img_min_y);
        lmark_dets_all(:,:,imgs) = lmark_dets_in_orig;

        shapes_all(:,:, imgs) = lmark_dets_in_orig;

        % the size normalised error
        err_normed = errors(imgs) / width;    
        errors_normed(imgs) = err_normed;
    else
        
        lmark_dets = zeros(66,2);
        lmark_dets_in_orig = zeros(66,2);
        
        errors(imgs) = 20;
        errors_normed(imgs) = 1;
    end
    
    if(verbose_out)
        v_points = sum(landmark,2) > 0;
        f = figure('visible','off');
        if(max(img(:)) > 1)
            imshow(double(img)/255, 'Border', 'tight');
        else
            imshow(double(img), 'Border', 'tight');
        end
        axis equal;
        hold on;
        plot(lmark_dets(v_points,1), lmark_dets(v_points,2),'.r','MarkerSize',20);
        plot(lmark_dets(v_points,1), lmark_dets(v_points,2),'.b','MarkerSize',10);
        %                                         print(f, '-r80', '-dpng', sprintf('%s/%s%d.png', output_root, 'fit', i));
        print(f, '-djpeg', sprintf('%s/%s%d.jpg', output_root, 'fit', imgs));
        close(f);
    end
    
end

save('../results/yu_wild.mat', 'errors', 'errors_normed', 'lmark_dets_all');

