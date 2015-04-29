data_dir = 'E:\datasets\yt_celeb\';

dirs = dir(data_dir);
dirs = dirs(3:end);

for i=1:numel(dirs)
    
    
    gt_labels = dir([data_dir dirs(i).name, '/*.pts']);
    curr = 0;
    labels = zeros(68, 2, numel(gt_labels));
    for g=1:numel(gt_labels)
        curr = curr+1;
        
        gt_landmarks = dlmread([data_dir dirs(i).name, '/', gt_labels(g).name], ' ', 'A4..B71');
       
        labels(:,:,curr) = gt_landmarks;
    end
    save([data_dir dirs(i).name], 'labels');
end