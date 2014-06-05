root_test_data = '../../test data/';
[~,dets,gt_labels] = Collect_wild_imgs(root_test_data);

%% Analysing the bounding boxes and errors

detection_root = './openCV_haarcascade_frontalface_alt/';

%%
afw_loc = [detection_root, 'out_haar_afw/'];
afw_dets = dir([afw_loc '*.pts']);

bboxes = [];

bboxes = cat(1, bboxes, zeros(numel(afw_dets), 6));

count = 0;
for i=1:numel(afw_dets)
    
    count = count + 1;
    
    bboxes(count,:) = csvread([afw_loc, afw_dets(i).name]);
    
end
%%
lfpw_loc = [detection_root, 'out_lfpw_trainset/'];
lfpw_dets = dir([lfpw_loc '*.pts']);

bboxes = cat(1, bboxes, zeros(numel(lfpw_dets), 6));

for i=1:numel(lfpw_dets)
    
    count = count + 1;
    
    bboxes(count,:) = csvread([lfpw_loc, lfpw_dets(i).name]);
    
end

%%
ibug_loc = [detection_root, 'out_ibug/'];
ibug_dets = dir([ibug_loc '*.pts']);

bboxes = cat(1, bboxes, zeros(numel(ibug_dets), 6));

for i=1:numel(ibug_dets)
    
    count = count + 1;
    
    bboxes(count,:) = csvread([ibug_loc, ibug_dets(i).name]);
    
end
%%
helen_loc = [detection_root, 'out_helen_trainset/'];
helen_dets = dir([helen_loc '*.pts']);

bboxes = cat(1, bboxes, zeros(numel(helen_dets), 6));

for i=1:numel(helen_dets)
    
    count = count + 1;
    
%     f = fopen([afw_loc, afw_dets(i).name]);
    bboxes(count,:) = csvread([helen_loc, helen_dets(i).name]);
%     fclose(f);
%     bboxes(count,:) = bbox;
    
end

%%

save('ocv.mat', 'bboxes', 'dets', 'gt_labels');
