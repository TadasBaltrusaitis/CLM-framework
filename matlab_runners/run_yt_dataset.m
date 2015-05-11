clm_exe = '"../Release/SimpleCLM.exe"';

output = 'yt_features/';

if(~exist(output, 'file'))
    mkdir(output)
end
    
% Run the BU test with CLM
if(exist([getenv('USERPROFILE') '/Dropbox/AAM/test data/'], 'file'))
    database_root = [getenv('USERPROFILE') '/Dropbox/AAM/test data/'];    
else
    database_root = 'F:/Dropbox/Dropbox/AAM/test data/';
end

database_root = [database_root, '/ytceleb_annotations_CVPR2014/'];

in_vids = dir([database_root '/*.avi']);

command = clm_exe;

% add all videos to single argument list (so as not to load the model anew
% for every video)
for i=1:numel(in_vids)
    
    [~, name, ~] = fileparts(in_vids(i).name);
    
    % where to output tracking results
    outputFile_fp = [output name '_fp.txt'];
    in_file_name = [database_root, '/', in_vids(i).name];        
    
    command = cat(2, command, [' -f "' in_file_name '" -of "' outputFile_fp '"']);                     
end

dos(command);

%% evaluating yt datasets
d_loc = 'yt_features/';

files_yt = dir([d_loc, '/*.txt']);
preds_all_eye = [];
gts_all = [];
for i = 1:numel(files_yt)
    [~, name, ~] = fileparts(files_yt(i).name);
    pred_landmarks = dlmread([d_loc, files_yt(i).name], ' ');
    pred_landmarks = pred_landmarks(:,3:end);
    
    xs = pred_landmarks(:, 1:end/2);
    ys = pred_landmarks(:, end/2+1:end);
    pred_landmarks = zeros([size(xs,2), 2, size(xs,1)]);
    pred_landmarks(:,1,:) = xs';
    pred_landmarks(:,2,:) = ys';
    
    load([database_root, name(1:end-3), '.mat']);
    preds_all_eye = cat(3, preds_all_eye, pred_landmarks);
    gts_all = cat(3, gts_all, labels);
end

%% evaluating yt datasets
d_loc = 'yt_features_no_eye/';

files_yt = dir([d_loc, '/*.txt']);
preds_all = [];
gts_all = [];
for i = 1:numel(files_yt)
    [~, name, ~] = fileparts(files_yt(i).name);
    pred_landmarks = dlmread([d_loc, files_yt(i).name], ' ');
    pred_landmarks = pred_landmarks(:,3:end);
    
    xs = pred_landmarks(:, 1:2:end/2);
    ys = pred_landmarks(:, end/2+1:2:end);
    pred_landmarks = zeros([size(xs,2), 2, size(xs,1)]);
    pred_landmarks(:,1,:) = xs';
    pred_landmarks(:,2,:) = ys';
    
    load([database_root, name(1:end-3), '.mat']);
    preds_all = cat(3, preds_all, pred_landmarks);
    gts_all = cat(3, gts_all, labels);
end

%%
clnf_error = compute_error( gts_all - 1.5,  preds_all);
clnf_error_eye = compute_error( gts_all - 1.5,  preds_all_eye);