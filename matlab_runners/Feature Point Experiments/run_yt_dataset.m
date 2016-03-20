clear

clm_exe = '"../../x64/Release/FaceTrackingVid.exe"';

output = 'yt_features/';

if(~exist(output, 'file'))
    mkdir(output)
end
    
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

%%
output = 'yt_features_clm/';

if(~exist(output, 'file'))
    mkdir(output)
end
    
command = clm_exe;
command = cat(2, command, ' -mloc model/main_clm_general.txt ');

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
d_loc_clm = 'yt_features_clm/';

files_yt = dir([d_loc, '/*.txt']);
preds_all = [];
preds_all_clm = [];
gts_all = [];
for i = 1:numel(files_yt)
    [~, name, ~] = fileparts(files_yt(i).name);
    pred_landmarks = dlmread([d_loc, files_yt(i).name], ',', 1, 0);
    pred_landmarks = pred_landmarks(:,5:end);
    
    xs = pred_landmarks(:, 1:end/2);
    ys = pred_landmarks(:, end/2+1:end);
    pred_landmarks = zeros([size(xs,2), 2, size(xs,1)]);
    pred_landmarks(:,1,:) = xs';
    pred_landmarks(:,2,:) = ys';
    
    pred_landmarks_clm = dlmread([d_loc_clm, files_yt(i).name], ',', 1, 0);
    pred_landmarks_clm = pred_landmarks_clm(:,5:end);
    
    xs = pred_landmarks_clm(:, 1:end/2);
    ys = pred_landmarks_clm(:, end/2+1:end);
    pred_landmarks_clm = zeros([size(xs,2), 2, size(xs,1)]);
    pred_landmarks_clm(:,1,:) = xs';
    pred_landmarks_clm(:,2,:) = ys';    
    
    load([database_root, name(1:end-3), '.mat']);
    preds_all = cat(3, preds_all, pred_landmarks);
    preds_all_clm = cat(3, preds_all_clm, pred_landmarks_clm);
    gts_all = cat(3, gts_all, labels);
end

%%
[clnf_error, err_pp_clnf] = compute_error( gts_all - 1.5,  preds_all);
[clm_error, err_pp_clm] = compute_error( gts_all - 1.5,  preds_all_clm);

filename = sprintf('results/fps_yt');
save(filename);

% Also save them in a reasonable .txt format for easy comparison
f = fopen('results/fps_yt.txt', 'w');
fprintf(f, 'Model, mean,  median\n');
fprintf(f, 'OpenFace (CLNF):  %.4f,   %.4f\n', mean(clnf_error), median(clnf_error));
fprintf(f, 'CLM:   %.4f,   %.4f\n', mean(clm_error), median(clm_error));

fclose(f);
clear 'f'