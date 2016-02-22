clear;

% fitting parameters more suitable for ccnf

%%
% Run the BU test with ccnf
if exist('D:/Datasets/HeadPose', 'file')
    database_root = 'D:/Datasets/HeadPose/';    
elseif(exist([getenv('USERPROFILE') '/Dropbox/AAM/test data/'], 'file'))
    database_root = [getenv('USERPROFILE') '/Dropbox/AAM/test data/'];    
else
    database_root = 'F:/Dropbox/Dropbox/AAM/test data/';
end

buDir = [database_root, '/bu/uniform-light/'];

% The fast and accurate ccnf
%%
v = 3;
[fps_bu_OF, resFolderBU_OF] = run_bu_experiment(buDir, false, v, 'model', 'model/main_ccnf_general.txt');
[bu_error_OF, pred_hp_bu, gt_hp_bu, all_errors_bu_OF, rels_bu] = calcBUerror(resFolderBU_OF, buDir);

%%
% Run the Biwi test
biwi_dir = '/biwi pose/';
biwi_results_root = '/biwi pose results/';

% Intensity
v = 4;
[fps_biwi_OF, res_folder_OF] = run_biwi_experiment(database_root, biwi_dir, biwi_results_root, false, false, v, 'model', 'model/main_ccnf_general.txt');
% Calculate the resulting errors
[biwi_error_OF, pred_hp_biwi, gt_hp_biwi, ~, all_errors_biwi_OF, rels_biwi] = calcBiwiError([database_root res_folder_OF], [database_root biwi_dir]);

%% Run the ICT test
ict_dir = ['ict/'];
ict_results_root = ['ict results/'];

v = 4;
% Intensity
[fps_ict_OF, res_folder_ict_OF] = run_ict_experiment(database_root, ict_dir, ict_results_root, false, false, v, 'model', 'model/main_ccnf_general.txt');
% Calculate the resulting errors
[ict_error_OF, pred_hp_ict, gt_hp_ict, ~, all_errors_ict_OF, rel_ict] = calcIctError([database_root res_folder_ict_OF], [database_root ict_dir]);

%% Save the results
filename = 'results/Pose_OF';
save(filename);

% Also save them in a reasonable .txt format for easy comparison
f = fopen('results/Pose_OF.txt', 'w');
fprintf(f, 'Dataset and model,        pitch,  yaw,  roll,  mean,  median\n');
fprintf(f, 'biwi error:  %.3f,   %.3f, %.3f,  %.3f,  %.3f\n', biwi_error_OF, mean(all_errors_biwi_OF(:)), median(all_errors_biwi_OF(:)));
fprintf(f, 'bu error:    %.3f,   %.3f, %.3f,  %.3f,  %.3f\n', bu_error_OF, mean(all_errors_bu_OF(:)), median(all_errors_bu_OF(:)));
fprintf(f, 'ict error:   %.3f,   %.3f, %.3f,  %.3f,  %.3f\n', ict_error_OF, mean(all_errors_ict_OF(:)), median(all_errors_ict_OF(:)));

fclose(f);
clear 'f'