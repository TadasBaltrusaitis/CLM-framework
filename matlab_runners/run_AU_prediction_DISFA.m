clear
DISFA_dir = 'D:/Datasets/DISFA/Videos_LeftCamera/';
clm_exe = '"../Release/FeatureExtraction.exe"';

videos = dir([DISFA_dir, '*.avi']);

output = 'AU_preds/';
if(~exist(output, 'file'))
    mkdir(output);
end

% Do it in parrallel for speed (replace the parfor with for if no parallel
% toolbox is available)
parfor v = 1:numel(videos)
   
    vid_file = [DISFA_dir, videos(v).name];
    
    [~, name, ~] = fileparts(vid_file);
    
    % where to output tracking results
    output_file = [output name '_au.txt'];
    command = [clm_exe ' -f "' vid_file '" -oaus "' output_file '" -q'];
        
    dos(command);
    
end

%% Now evaluate the predictions

% Note that DISFA was used in training, this is not meant for experimental
% results but rather to show how to do AU prediction and how to interpret
% the results

Label_dir = 'D:/Datasets/DISFA/ActionUnit_Labels/';
prediction_dir = 'AU_preds/';

label_folders = dir([Label_dir, 'SN*']);

AUs_disfa = [1,2,4,5,6,9,12,15,17,20,25,26];
labels_all = [];
label_ids = [];
for i=1:numel(label_folders)

    labels = [];
    for au = AUs_disfa
        in_file = sprintf('%s/%s/%s_au%d.txt', Label_dir, label_folders(i).name, label_folders(i).name, au);
        A = dlmread(in_file, ',');
        labels = cat(2, labels, A(:,2));
    end
    
    labels_all = cat(1, labels_all, labels);
    user_id = str2num(label_folders(i).name(3:end));
    label_ids = cat(1, label_ids, repmat(user_id, size(labels,1),1));
end

preds_files = dir([prediction_dir, '*SN*.txt']);

tab = readtable([prediction_dir, preds_files(1).name]);
column_names = tab.Properties.VariableNames;
aus_pred_int = [];
for c=3:numel(column_names)
    if(strfind(column_names{c}, '_r') > 0)
        aus_pred_int = cat(1, aus_pred_int, int32(str2num(column_names{c}(3:end-2))));
    end
end
    
[rel_preds,~,inds_au] = intersect(AUs_disfa, aus_pred_int);
preds_all = zeros(size(labels_all,1), numel(rel_preds));

for i=1:numel(preds_files)
   
    preds = dlmread([prediction_dir, preds_files(i).name], ',', 1, 0);
    preds = preds(:,4:4+numel(aus_pred_int)-1);

    user_id = str2num(preds_files(i).name(end - 14:end-12));
    rel_ids = label_ids == user_id;
    preds_all(rel_ids,:) = preds(:,inds_au);
    
end

%% now do the actual evaluation that the collection has been done
for au = 1:numel(rel_preds)
   [ accuracies, F1s, corrs, ccc, rms, classes ] = evaluate_au_prediction_results( preds_all(:,au), labels_all(:,au));
   fprintf('AU%d results - corr %.3f, ccc - %.3f\n', rel_preds(au), corrs, ccc);
end