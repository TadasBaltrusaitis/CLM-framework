function [ images, detections, labels, inds, bboxes_kang ] = Collect_CS0_imgs()
%COLLECT_C0_IMGS Summary of this function goes here
%   Detailed explanation goes here

    janus_meta_csv = 'D:\JANUS_test\CS0\protocol/metadata.csv';
    root_data_loc = 'D:\JANUS_test\CS0/';
    % aflw_face_det = 'D:\JANUS_training\aflw/metadata_68.csv';

    meta_data = csvread(janus_meta_csv, 1, 6);
    detections = meta_data(:,1:4);
    landmarks = meta_data(:,5:10);

    % meta_data = csvread(aflw_face_det, 0, 5);
    % rotations = meta_data(:,1:3);
    % yaw = rotations(:,3);
    % roll = -rotations(:,1);
    % pitch = -rotations(:,2);
    % rotations = cat(2, pitch, yaw, roll);

    image_locs = readtable(janus_meta_csv, 'ReadVariableNames', false);
    image_locs = table2cell(image_locs(2:end,3));

    num_imgs = size(detections,1);

    labels = zeros(num_imgs, 3, 2);
    labels(:,1,:) = meta_data(:, 7:8);
    labels(:,2,:) = meta_data(:, 5:6);
    labels(:,3,:) = meta_data(:, 9:10);
%     janus_inds_in_68 = [37, 40, 43, 46, 34];
%     janus_ind_x = [1, 1, 3, 3, 5];
%     janus_ind_y = [2, 2, 4, 4, 6];
    
    images = struct;

    for imgs = 1:num_imgs

%         new_widths = detections(imgs,3) * 0.6323;
%         new_heights = detections(imgs,4) * 0.5109;
%         new_tx = detections(imgs,3) * 0.1815 + detections(imgs,1);
%         new_ty = detections(imgs,4) * 0.4297 + detections(imgs,2);
% 
%         bbox_det_new = [new_tx, new_ty, new_widths, new_heights];
%         detections(imgs, :) = bbox_det_new;
        
%         labels(imgs, janus_inds_in_68, 1) = landmarks(imgs, janus_ind_x);
%         labels(imgs, janus_inds_in_68, 2) = landmarks(imgs, janus_ind_y);
        images(imgs).img = [root_data_loc,  image_locs{imgs}];
    end
    
    % remove images with no detections, no labels and no eyes labels
    both_eyes = sum(sum(labels(:,1:2,:),3),2) ~= 0;
    two_points = sum(sum(labels(:,[1,3],:),3),2) ~= 0 | sum(sum(labels(:,[2,3],:),3),2) ~= 0;
    has_nose = sum(labels(:,3,:),3) ~= 0;
    to_use = sum(detections,2) ~= 0 & sum(sum(labels,3),2) ~= 0 & both_eyes & two_points & has_nose;
    
    images = images(to_use);
    detections = detections(to_use,:);
    labels = labels(to_use,:,:);
    
    inds = zeros(size(labels,1),1);
    inds(sum(labels(:,1,:),3) ~= 0 & sum(labels(:,2,:),3) ~= 0) = 1;
   
    left_eye = labels(:,1,:);
    right_eye = labels(:,2,:);
    both_eyes = left_eye(:,:,1) ~= 0 & right_eye(:,:,1)~=0;
    nose = labels(:,3,:);
    right_profile = right_eye(:,1,1) == 0 & left_eye(:,1,1) ~= 0 & nose(:,:,1)~=0;
    left_profile = left_eye(:,1,1) == 0 & right_eye(:,1,1) ~= 0 & nose(:,:,1)~=0;
    
    bboxes_kang = zeros(size(detections));
    
    bboxes_kang(both_eyes,:) = [right_eye(both_eyes,:,1), 0.5*(left_eye(both_eyes,:,2) + right_eye(both_eyes,:,2)),...
        left_eye(both_eyes,:,1) - right_eye(both_eyes,:,1),  0.85 * (left_eye(both_eyes,:,1) - right_eye(both_eyes,:,1))];

    bboxes_kang(right_profile,:) = [2*min(left_eye(right_profile,:,1),nose(right_profile,:,1)) - max(left_eye(right_profile,:,1),nose(right_profile,:,1)), left_eye(right_profile,:,2),...
                                    2 * abs(nose(right_profile,:,1) - left_eye(right_profile,:,1)), nose(right_profile,:,2) - left_eye(right_profile,:,2)];
    
    bboxes_kang(left_profile,:) = [min(nose(left_profile,:,1), right_eye(left_profile,:,1)), right_eye(left_profile,:,2),...
                                    2 * abs(right_eye(left_profile,:,1) - nose(left_profile,:,1)), nose(left_profile,:,2) - right_eye(left_profile,:,2)];
    
                                
%     for i=1:size(bboxes_kang,1)
%        
%         imshow(imread(images(i).img));hold on;
%         plot(labels(i,:,1), labels(i,:,2), '.');        
%         rectangle('Position', bboxes_kang(i,:));
%         hold off;
%     end
%     bboxes_kang    
    
%     detections(:,3) = detections(:,1) + detections(:,3);
%     detections(:,4) = detections(:,2) + detections(:,4);
end

