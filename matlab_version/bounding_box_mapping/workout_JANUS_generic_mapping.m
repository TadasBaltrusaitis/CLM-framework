clear
addpath('../PDM_helpers/');
load('../models/pdm/pdm_68_aligned_wild.mat');

janus_meta_csv = 'D:\JANUS_test\CS0\protocol/metadata.csv';
root_data_loc = 'D:\JANUS_test\CS0/';

meta_data = csvread(janus_meta_csv, 1, 6);
detections = meta_data(:,1:4);
landmarks = meta_data(:,5:10);
    
image_locs = readtable(janus_meta_csv, 'ReadVariableNames', false);
image_locs = table2cell(image_locs(2:end,3));
    
num_imgs = size(detections,1);

images = struct;
labels = zeros(num_imgs, 68, 2);
    
janus_inds_in_68 = [37, 40, 43, 46, 34];
janus_ind_x = [1, 1, 3, 3, 5];
janus_ind_y = [2, 2, 4, 4, 6];

for imgs = 1:num_imgs

    labels(imgs, janus_inds_in_68, 1) = landmarks(imgs, janus_ind_x);
    labels(imgs, janus_inds_in_68, 2) = landmarks(imgs, janus_ind_y);

end

% Find the desired bounding boxes

bboxes_gt = zeros(num_imgs, 4);

overlaps = zeros(size(labels,1), 1);
to_use = true(num_imgs,1);

view_ids = zeros(size(labels,1), 1);

for  imgs = 1:num_imgs
    
    init_rot = [0,0,0];
    if(labels(imgs,40,1) == 0 && labels(imgs,34,1) ~= 0 && labels(imgs,43,1) ~= 0)
        init_rot = [0,pi/2,0];
        view_ids(imgs) = 2;
        s_width = 0.3691; s_height = 0.5406; s_tx = 0.1153; s_ty = 0.4277;
        
        %         continue; 
    elseif(labels(imgs,43,1) == 0 && labels(imgs,34,1) ~= 0 && labels(imgs,40,1) ~= 0)
        init_rot = [ 0,-pi/2,0];
        view_ids(imgs) = 3;
        s_width = 0.3630; s_height = 0.5167; s_tx = 0.5709; s_ty = 0.4492;
        %         continue; 
    elseif(labels(imgs,43,1) ~= 0 && labels(imgs,40,1) ~= 0 && labels(imgs,34,1) ~= 0)
        view_ids(imgs) = 1;
        s_width = 0.6686; s_height = 0.5083; s_tx = 0.1375; s_ty = 0.4387;
%         continue; 
    else
        to_use(imgs) = 0;
        continue;
    end
    
    if((labels(imgs,40,1) == 0 && labels(imgs,43,1) == 0) || sum(detections(imgs,:)) == 0)
        to_use(imgs) = 0;
        continue; 
    end
    [ ~, ~, ~, err, shapeOrtho] = fit_PDM_ortho_janus(M, squeeze(labels(imgs,:,:)), init_rot);
    
    tlx_gt = min(shapeOrtho(:,1)')';
    tly_gt = min(shapeOrtho(:,2)')';

    blx_gt = max(shapeOrtho(:,1)')';
    bly_gt = max(shapeOrtho(:,2)')';

    visible_points = sum(labels(imgs,:,1)~=0);
    
    bboxes_gt(imgs,:) = [tlx_gt, tly_gt, blx_gt, bly_gt];
    
%     new_widths = detections(imgs,3) * s_width;
%     new_heights = detections(imgs,4) * s_height;
%     new_tx = detections(imgs,3) * s_tx + detections(imgs,1);
%     new_ty = detections(imgs,4) * s_ty + detections(imgs,2);
% 
%     bbox_det_new = [new_tx, new_ty, new_widths, new_heights];
%     
%     imshow(imread([root_data_loc,  image_locs{imgs}]));
%     hold on;
%     plot(labels(imgs,:,1), labels(imgs,:,2), '.g');
%     plot(shapeOrtho(:,1), shapeOrtho(:,2), '.r');
%     rectangle('Position', detections(imgs,:));
%     rectangle('Position', bbox_det_new, 'EdgeColor', 'red');
%     hold off;
%     
    % Working out the initial overlap
    det = detections(imgs,:);
    det(3) = det(1) + det(3);
    det(4) = det(2) + det(4);
    overlaps(imgs) = overlap(bboxes_gt(imgs,:), det);
    
end

% to_use = to_use;% & overlaps > 0.1;
overlaps = overlaps(to_use);
detections = detections(to_use,:);
bboxes_gt = bboxes_gt(to_use,:);
image_locs = image_locs(to_use);

detections(:,3) = detections(:,1) + detections(:,3);
detections(:,4) = detections(:,2) + detections(:,4);
view_ids = view_ids(to_use);

[s_width, s_height, s_tx, s_ty, over, n_over] = workout_mapping(bboxes_gt, detections);

[s_width_f, s_height_f, s_tx_f, s_ty_f, over_f, n_over_f] = workout_mapping(bboxes_gt(view_ids==1 & overlaps > 0.1,:), detections(view_ids==1 & overlaps > 0.1,:));
[s_width_l, s_height_l, s_tx_l, s_ty_l, over_l, n_over_l] = workout_mapping(bboxes_gt(view_ids==2 & overlaps > 0.1,:), detections(view_ids==2 & overlaps > 0.1,:));
[s_width_r, s_height_r, s_tx_r, s_ty_r, over_r, n_over_r] = workout_mapping(bboxes_gt(view_ids==3 & overlaps > 0.1,:), detections(view_ids==3 & overlaps > 0.1,:));

% Capturing profile views, rolled views, slightly bigger views, up-down
% views
views = [0,0,0; 0,0,0; 0, -30, 0; 0, -60, 0; 0, -90, 0; 0, 30, 0; 0, 60, 0; 0, 90, 0; 0, -30, 0; 0, -60, 0; 0, -90, 0; 0, 30, 0; 0, 60, 0; 0, 90, 0; 0,0,30; 0,0,-30; -20,0,0; 20,0,0] * pi/180;

s_width_l = interp1([0,1], [s_width_f, s_width_l], [0.33, 0.66, 1]);
s_height_l = interp1([0,1], [s_height_f, s_height_l], [0.33, 0.66, 1]);
s_tx_l = interp1([0,1], [s_tx_f, s_tx_l], [0.33, 0.66, 1]);
s_ty_l = interp1([0,1], [s_ty_f, s_ty_l], [0.33, 0.66, 1]);

s_width_r = interp1([0,1], [s_width_f, s_width_r], [0.33, 0.66, 1]);
s_height_r = interp1([0,1], [s_height_f, s_height_r], [0.33, 0.66, 1]);
s_tx_r = interp1([0,1], [s_tx_f, s_tx_r], [0.33, 0.66, 1]);
s_ty_r = interp1([0,1], [s_ty_f, s_ty_r], [0.33, 0.66, 1]);

s_widths = [s_width_f, 1.2*s_width_f, s_width_r, s_width_l, 1.2*s_width_r, 1.2*s_width_l, s_width_f, s_width_f, s_width_f, s_width_f];
s_heights = [s_height_f, 1.2*s_height_f, s_height_r, s_height_l,  1.2*s_height_r, 1.2*s_height_l, s_height_f, s_height_f, s_height_f, s_height_f];
s_txs = [s_tx_f, 0.83*s_tx_f, s_tx_r, s_tx_l, 0.83 * s_tx_r, 0.83 * s_tx_l, s_tx_f, s_tx_f, s_tx_f, s_tx_f];
s_tys = [s_ty_f, 0.83*s_ty_f, s_ty_r, s_ty_l, 0.83 * s_ty_r, 0.83 * s_ty_l, s_ty_f, s_ty_f, 0.8*s_ty_f, 1.2*s_ty_f];

save('mappings_CS0', 'views', 's_widths', 's_heights', 's_txs', 's_tys');

bounding_boxes_all = struct;
for i=1:size(views,1)
    
    roi_view = detections;
    
    roi_view(:,3) = roi_view(:,3) - roi_view(:,1);
    roi_view(:,4) = roi_view(:,4) - roi_view(:,2);
    
    new_widths = roi_view(:,3) * s_widths(i);
    new_heights = roi_view(:,4) * s_heights(i);
    new_txs = roi_view(:,3) * s_txs(i) + roi_view(:,1);
    new_tys = roi_view(:,4) * s_tys(i) + roi_view(:,2);
    
    roi_view(:,1) = new_txs;
    roi_view(:,2) = new_tys;
    roi_view(:,3) = new_widths;
    roi_view(:,4) = new_heights;
    bounding_boxes_all(i).view = views(i,:);
    bounding_boxes_all(i).bounding_boxes = roi_view;
end
bounding_boxes = zeros(size(overlaps,1),4);
load('../experiments_JANUS/results/results_janus_clnf_better_ROI_mv_3.mat');
views_used = experiments.all_views_used;
all_views = zeros(size(overlaps,1), 3);
for i=1:size(overlaps,1)
    bounding_boxes(i,:) = bounding_boxes_all(views_used(i)).bounding_boxes(i,:);
    all_views(i,:) = views(views_used(i),:);
%     imshow(imread([root_data_loc,  image_locs{i}]));
%     hold on;
%     rectangle('Position', bounding_boxes(i,:));
%     hold off;
end
save('Janus_refined_bounding_boxes', 'bounding_boxes', 'image_locs', 'all_views');