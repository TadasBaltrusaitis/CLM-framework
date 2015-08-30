% load('ocv.mat')
% Move into Matlab space
% bboxes = bboxes + 1;

load('matlab.mat')

% Find the actual bboxes
bboxes_detector = bboxes(:,3:6);
% add x offset
bboxes_detector(:,1) = bboxes(:,1) + bboxes_detector(:,1);

% add y offset
bboxes_detector(:,2) = bboxes(:,2) + bboxes_detector(:,2);

non_detected = bboxes(:,3) == 1;

% Find the width and height mappings
widths_gt = (max(gt_labels(:,:,1)') - min(gt_labels(:,:,1)'))';
widths_det = bboxes_detector(:,3);

bad_det = abs(1 - widths_gt ./ widths_det) > 0.5;

non_detected = non_detected | bad_det;

% if the width is quite different from detection then it failed

bboxes_detector = bboxes_detector(~non_detected,:);
gt_labels = gt_labels(~non_detected,:,:);
dets = dets(~non_detected,:);

%% some visualisations
% a = 1;
% plot(gt_labels(a,:,1), gt_labels(a,:,2), '.r');
% hold on;
% bbox = bboxes_detector(a,:);
% % bbox(2) = -bbox(2);
% rectangle('Position', bbox);
% hold off;
% axis equal;

% Want to find out what scaling and translation would lead to the smallest
% RMSE error between initialised landmarks and gt landmarks TODO

% Find the width and height mappings
widths_gt = (max(gt_labels(:,:,1)') - min(gt_labels(:,:,1)'))';
heights_gt = (max(gt_labels(:,:,2)') - min(gt_labels(:,:,2)'))';

widths_det = bboxes_detector(:,3);
heights_det = bboxes_detector(:,4);

s_width = widths_det \ widths_gt;
s_height = heights_det \ heights_gt;

tx_gt =  min(gt_labels(:,:,1)')';
ty_gt =  min(gt_labels(:,:,2)')';

tx_det = bboxes_detector(:,1);
ty_det = bboxes_detector(:,2);

s_tx = mean((tx_gt - tx_det) ./ widths_det);
s_ty = mean((ty_gt - ty_det) ./ heights_det);