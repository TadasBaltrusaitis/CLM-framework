clear

csv_loc = 'D:\JANUS_training\aflw\aflw_68_dev.csv';
csv_meta_loc = 'D:\JANUS_training\aflw/metadata_68_dev.csv';
root_loc = 'D:\Datasets\AFLW/';

[images, detections, labels] = Collect_imgs(csv_loc, csv_meta_loc, root_loc);

output_root = './mouth_fit/';
if(~exist(output_root, 'dir'))
    mkdir(output_root)
end
eye_inds = [43,44,45,46,47,48];
to_use = sum(labels(:,eye_inds,1)~=0,2) > 1;

load('results/results_dev_clnf_bbox_mv.mat');
shapes_no_h = experiments.shapes;

errors_normed_basic = compute_error( labels(to_use,eye_inds,:) - 0.5,  shapes_no_h(eye_inds,:,to_use)-0.5, detections);

load('results/results_dev_clnf_bbox_mv_hierarch.mat');
shapes = experiments.shapes;
errors_normed = compute_error( labels(to_use,eye_inds,:) - 0.5,  shapes(eye_inds,:,to_use)-0.5, detections);

errs_improv = errors_normed - errors_normed_basic;

[sorted_errs_improv, inds_improv] = sort(errs_improv);

% Create a cycle of points
rearange_inds_1 = 1:6;

f = figure;

%% Display most improved images
for i=inds_improv(1:40)'
   
    img = imread(images(i).img);
    shape_eye = experiments.shapes(eye_inds,:,i)+1;
    
    shape_eye_old = shapes_no_h(eye_inds,:,i)+1;
    
    [height_img, width_img,~] = size(img);

    img_min_x = max(int32(min(shape_eye(:,1))) - 10,1);
    img_max_x = min(int32(max(shape_eye(:,1))) + 10,width_img);

    img_min_y = max(int32(min(shape_eye(:,2))) - 10,1);
    img_max_y = min(int32(max(shape_eye(:,2))) + 10,height_img);

    shape_eye(:,1) = shape_eye(:,1) - double(img_min_x);
    shape_eye(:,2) = shape_eye(:,2) - double(img_min_y);

    shape_eye_old(:,1) = shape_eye_old(:,1) - double(img_min_x);
    shape_eye_old(:,2) = shape_eye_old(:,2) - double(img_min_y);

    img = img(img_min_y:img_max_y, img_min_x:img_max_x, :);    
    
    if(size(img,1) < 200)
        
        scale = 200.0 /  size(img,1);
        
        img = imresize(img, scale);
        shape_eye = (shape_eye - 0.5) * scale + 0.5;
        
        shape_eye_old = (shape_eye_old - 0.5) * scale + 0.5;
    end
    
    
    imshow(img, 'Border', 'tight');
    hold on;

    plot_curve(shape_eye(rearange_inds_1, :), 'b', 'LineWidth', 3);
     
    plot(shape_eye(:,1), shape_eye(:,2),'.y','MarkerSize',15);    

    print(f, '-dpng', sprintf('%s/mouth_%s%d_after.png', output_root, 'fit', i));
    hold off;

    % do the before
    imshow(img, 'Border', 'tight');
    hold on;
        
    plot_curve(shape_eye_old(rearange_inds_1, :), 'b', 'LineWidth', 3);
     
    plot(shape_eye_old(:,1), shape_eye_old(:,2),'.y','MarkerSize',15);      

    print(f, '-dpng', sprintf('%s/mouth_%s%d_before.png', output_root, 'fit', i));
    hold off;    
    
end