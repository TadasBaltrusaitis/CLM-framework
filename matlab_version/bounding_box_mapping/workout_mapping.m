function [ s_width, s_height, s_tx, s_ty, original_overlap, new_overlap ] = workout_mapping( gt_detections, detections )
%WORKOUT_MAPPING Summary of this function goes here
%   Detailed explanation goes here

    % Find the width and height mappings
    widths_gt = gt_detections(:,3)- gt_detections(:,1);
    heights_gt = gt_detections(:,4)- gt_detections(:,2);

    widths_det = detections(:,3) - detections(:,1);
    heights_det = detections(:,4) - detections(:,2);

%     s_width = widths_det \ widths_gt;
%     s_height = heights_det \ heights_gt;

    s_width = median(widths_gt ./ widths_det);
    s_height = median(heights_gt ./ heights_det);

    tx_gt =  gt_detections(:,1);
    ty_gt =  gt_detections(:,2);

    tx_det = detections(:,1);
    ty_det = detections(:,2);

%     s_tx = mean((tx_gt - tx_det) ./ widths_det);
%     s_ty = mean((ty_gt - ty_det) ./ heights_det);
    s_tx = median((tx_gt - tx_det) ./ widths_det);
    s_ty = median((ty_gt - ty_det) ./ heights_det);

    % newbbox
    new_widths = widths_det * s_width;
    new_heights = heights_det * s_height;
    new_tx = widths_det * s_tx + tx_det;
    new_ty = heights_det * s_ty + ty_det;

    bbox_det_tp_new = [new_tx, new_ty, new_tx + new_widths, new_ty + new_heights];

    new_overlap = zeros(size(bbox_det_tp_new, 1), 1);
    original_overlap = zeros(size(bbox_det_tp_new, 1), 1);
    for gt_ind = 1:size(bbox_det_tp_new, 1)
        new_overlap(gt_ind) = overlap(gt_detections(gt_ind,:), bbox_det_tp_new(gt_ind,:));
        original_overlap(gt_ind) = overlap(gt_detections(gt_ind,:), detections(gt_ind,:));
    end

end

