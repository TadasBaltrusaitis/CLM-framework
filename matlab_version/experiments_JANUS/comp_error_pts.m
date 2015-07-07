%gts

dirs = {'../../test data/AFW/';
    '../../test data/ibug/';
    '../../test data/helen/testset/';
    '../../test data/lfpw/testset/';};

landmark_dets = dir('out_clnf_cpp\*.pts');

landmark_det_dir = 'out_clnf_cpp\';

num_imgs = size(landmark_dets,1);

labels = zeros(68,2,num_imgs);
shapes = zeros(68,2,num_imgs);

landmark_gt = dir(['../../test data/AFW/*.pts']);

curr = 0;

for i=1:numel(dirs)
    
    curr = curr+1;
    
    gt_labels = dir([dirs{i}, '*.pts']);
    
    for g=1:numel(gt_labels)
           gt_landmarks = importdata([dirs{i}, gt_labels(g).name], ' ', 3);
           gt_landmarks = gt_landmarks.data;
 
           % find the corresponding detection
           
           landmark_det = importdata([landmark_det_dir, gt_labels(g).name], ' ', 3);
           landmark_det = landmark_det.data;
           
           labels(:,:,curr) = gt_landmarks;
           shapes(:,:,curr) = landmark_det;
           
    end
    
end




