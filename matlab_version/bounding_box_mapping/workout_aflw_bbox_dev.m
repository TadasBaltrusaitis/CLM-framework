clear
addpath('../PDM_helpers/');
load('../models/pdm/pdm_68_aligned_wild.mat');

aflw_landmark_csv = 'D:\JANUS_training\aflw/aflw_68_dev.csv';
root_data_loc = 'D:\Datasets\AFLW/';
aflw_face_det = 'D:\JANUS_training\aflw/metadata_68_dev.csv';

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

for  imgs = 1:num_imgs
    
    if(sum(squeeze(labels(imgs,:,1))~=0) > 3)
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
    
end

save('AFLW_gt_bbox.mat', 'bboxes_gt');