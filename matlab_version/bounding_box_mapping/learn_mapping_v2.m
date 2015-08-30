% load('ocv.mat')
% Move into Matlab space
% bboxes = bboxes + 1;

use_afw = false;
use_lfpw = true;
use_helen = true;
use_ibug = false;
root_test_data = 'F:\Dropbox\Dropbox\AAM\test data/';
% this loads the ground truths
% [gt_images, gt_detections, gt_labels] = Collect_wild_imgs(root_test_data, use_afw, use_lfpw, use_helen, use_ibug);

%% Find the width and height mappings
tlx_gt = min(gt_labels(:,:,1)')' - 1;
tly_gt = min(gt_labels(:,:,2)')' - 1;

blx_gt = max(gt_labels(:,:,1)')' - 1;
bly_gt = max(gt_labels(:,:,2)')' - 1;

bboxes_gt = [tlx_gt, tly_gt, blx_gt, bly_gt];

%% Extract the detector bounding boxes as well
f = fopen('F:\dlib_baseline\helen.txt');
det_file = textscan(f, '%s %f %f %f %f %f');
fclose(f);

f = fopen('F:\dlib_baseline\lfpw.txt');
det_file_2 = textscan(f, '%s %f %f %f %f %f');
fclose(f);

det_images = cat(1, det_file{1}, det_file_2{1});

scores_det = cat(1, det_file{2}, det_file_2{2});

bboxes_det = [det_file{3}, det_file{4}, det_file{5}, det_file{6}];

bboxes_det_2 = [det_file_2{3}, det_file_2{4}, det_file_2{5}, det_file_2{6}];

bboxes_det = cat(1, bboxes_det, bboxes_det_2);

%% Go through ground truth picking out the best detections
bbox_gt_with_tp = []; 
bbox_det_tp = [];
scores_tp = [];
overlaps_tp = [];

tps_all = zeros(size(bboxes_det, 1), 1);

for gt_ind = 1:size(bboxes_gt, 1)
   
    [~, fname, ~] = fileparts(gt_images(gt_ind).img);
    
    det_inds = find(strcmp(det_images, fname))';
    
    bbox_gt = bboxes_gt(gt_ind,:);
    bbox_det = [];
    
    max_overlap = 0;
    max_ind = 0;
    for det_ind=det_inds
       
        curr_ov = overlap(bbox_gt, bboxes_det(det_ind,:));
        
        if(curr_ov > max_overlap)
            max_overlap = curr_ov;
            bbox_det = bboxes_det(det_ind,:);
            score = scores_det(det_ind);
            max_ind = det_ind;
        end
        
    end
    
    if(max_overlap > 0.4)
                   
        tps_all(max_ind) = 1;
        
        bbox_gt_with_tp = cat(1, bbox_gt_with_tp, bbox_gt); 
        bbox_det_tp = cat(1, bbox_det_tp, bbox_det); 
        scores_tp = cat(1, scores_tp, score);
        overlaps_tp = cat(1, overlaps_tp, max_overlap);
    end
    
end

%% Work out the mapping and a suitable threshold, with EER?

%% some visualisations
% Want to find out what scaling and translation would lead to the smallest
% RMSE error between initialised landmarks and gt landmarks TODO

% Find the width and height mappings
widths_gt = bbox_gt_with_tp(:,3) - bbox_gt_with_tp(:,1);
heights_gt = bbox_gt_with_tp(:,4) - bbox_gt_with_tp(:,2);

widths_det = bbox_det_tp(:,3) - bbox_det_tp(:,1);
heights_det = bbox_det_tp(:,4) - bbox_det_tp(:,2);

s_width = widths_det \ widths_gt;
s_height = heights_det \ heights_gt;

tx_gt =  bbox_gt_with_tp(:,1);
ty_gt =  bbox_gt_with_tp(:,2);

tx_det = bbox_det_tp(:,1);
ty_det = bbox_det_tp(:,2);

s_tx = mean((tx_gt - tx_det) ./ widths_det);
s_ty = mean((ty_gt - ty_det) ./ heights_det);

% newbbox
new_widths = widths_det * s_width;
new_heights = heights_det * s_height;
new_tx = widths_det * s_tx + tx_det;
new_ty = heights_det * s_ty + ty_det;

bbox_det_tp_new = [new_tx, new_ty, new_tx + new_widths, new_ty + new_heights];

new_overlaps = zeros(size(bbox_det_tp_new, 1), 1);

for gt_ind = 1:size(bbox_det_tp_new, 1)
   
    new_overlaps(gt_ind) = overlap(bbox_gt_with_tp(gt_ind,:), bbox_det_tp_new(gt_ind,:));

end

%% Draw an ROC curve

score_bands = min(scores_det):(max(scores_det)-min(scores_det))/50:max(scores_det);
recalls = zeros(numel(score_bands), 1);
precisions = zeros(numel(score_bands), 1);

for i=1:numel(score_bands)
   
    precisions(i) = sum(tps_all(scores_det >= score_bands(i))) / sum(scores_det >= score_bands(i));
    recalls(i) = sum(tps_all(scores_det >= score_bands(i))) / size(gt_labels,1);
end

% tp

% bbox_det_new = bbox