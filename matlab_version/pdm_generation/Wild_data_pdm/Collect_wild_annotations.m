% collect the training data annotations (from the 300 faces in the wild
% challenge)

test_data_loc = '../../../';

dataset_locs = {[test_data_loc, '/test data/helen/trainset/'];
                [test_data_loc, '/test data/lfpw/trainset/']};

all_pts = [];
for i=1:numel(dataset_locs)
    landmarkLabels = dir([dataset_locs{i} '\*.pts']);

    for p=1:numel(landmarkLabels)
        landmarks = importdata([dataset_locs{i}, landmarkLabels(p).name], ' ', 3);
        landmarks = landmarks.data;
        all_pts = cat(3, all_pts, landmarks);
    end
end

%%

xs = squeeze(all_pts(:,1,:));
ys = squeeze(all_pts(:,2,:));

all_pts = cat(2,xs,ys)';

save('wild_68_pts', 'all_pts');
