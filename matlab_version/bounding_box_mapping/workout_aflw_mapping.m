clear
addpath('../PDM_helpers/');
load('../models/pdm/pdm_68_aligned_wild.mat');

aflw_landmark_csv = 'D:\JANUS_training\aflw/aflw_68.csv';
root_data_loc = 'D:\Datasets\AFLW/';
aflw_face_det = 'D:\JANUS_training\aflw/metadata_68.csv';

landmarks = csvread(aflw_landmark_csv, 0, 1);

meta_data = csvread(aflw_face_det, 0, 5);
rotations = meta_data(:,1:3);
yaw = rotations(:,3);
roll = -rotations(:,1);
pitch = -rotations(:,2);
rotations = cat(2, pitch, yaw, roll);

detections = meta_data(:,5:8);
    
image_locs = readtable(aflw_landmark_csv, 'ReadVariableNames', false);
image_locs = table2cell(image_locs(:,1));
    
num_imgs = size(detections,1);

images = struct;
labels = zeros(num_imgs, 68, 2);
    
for imgs = 1:num_imgs

    labels(imgs, :, 1) = landmarks(imgs, 1:2:end);
    labels(imgs, :, 2) = landmarks(imgs, 2:2:end);

end

% Find the desired bounding boxes

bboxes_gt = zeros(num_imgs, 4);

overlaps = zeros(size(labels,1), 1);

for  imgs = 1:num_imgs
    
    if(sum(squeeze(labels(imgs,:,1))~=0) > 5)
        [ ~, ~, ~, ~, ~, err, shapeOrtho] = fit_PDM_ortho_proj_to_2D_rot(M, E, V, squeeze(labels(imgs,:,:)), rotations(imgs,:));
    else
        continue;
    end
    tlx_gt = min(shapeOrtho(:,1)')';
    tly_gt = min(shapeOrtho(:,2)')';

    blx_gt = max(shapeOrtho(:,1)')';
    bly_gt = max(shapeOrtho(:,2)')';

    visible_points = sum(labels(imgs,:,1)~=0);
    
    bboxes_gt(imgs,:) = [tlx_gt, tly_gt, blx_gt, bly_gt];
    
%     imshow(imread([root_data_loc,  image_locs{imgs}]));
%     hold on;
%     plot(labels(imgs,:,1), labels(imgs,:,2), '.g');
%     plot(shapeOrtho(:,1), shapeOrtho(:,2), '.r');
%     rectangle('Position', detections(imgs,:));
%     hold off;
%     
    % Working out the initial overlap
    det = detections(imgs,:);
    det(3) = det(1) + det(3);
    det(4) = det(2) + det(4);
    overlaps(imgs) = overlap(bboxes_gt(imgs,:), det);
    
end

to_use = overlaps > 0.3;
overlaps = overlaps(to_use);
detections = detections(to_use,:);
detections(:,3) = detections(:,1) + detections(:,3);
detections(:,4) = detections(:,2) + detections(:,4);

bboxes_gt = bboxes_gt(to_use,:);
rotations = rotations(to_use,:);

%% The default mapping
[s_width, s_height, s_tx, s_ty, over, n_over] = workout_mapping(bboxes_gt, detections);

% Orientation aware mapping
views = [ 0, 0, 0;   0, -30, 0; 0, -60, 0; 0, -90,  0; 0, 30, 0; 0, 60, 0; 0, 90, 0;
         20, 0, 0; -20,   0, 0; 0,   0, 30; 0,  0, -30] * pi/180; %mean normed error 0.090 median normed error 0.0755
     
s_widths = zeros(size(views,1),1);     
s_heights = zeros(size(views,1),1);     
s_txs = zeros(size(views,1),1);     
s_tys = zeros(size(views,1),1);     

% need to find the closest rotation indices for each view
dists = zeros(size(rotations, 1), size(views,1));
for j=1:size(views,1)
   dists(:,j) =  sum((bsxfun(@plus, rotations, -views(j,:))).^2,2);
end
[~, indices] = min(dists');
indices = indices';

for i = 1:size(views, 1)


    [s_widths(i), s_heights(i), s_txs(i), s_tys(i), over, n_over] = workout_mapping(bboxes_gt(indices == i,:), detections(indices == i,:));

    fprintf('From %.2f, to %.2f, view (%.f,%.1f,%.1f), num images %d\n', mean(over), mean(n_over), views(i,1)*180/pi, views(i,2)*180/pi, views(i,3)*180/pi, sum(indices==i)); 
end
     
save('mappings', 'views', 's_widths', 's_heights', 's_txs', 's_tys');
