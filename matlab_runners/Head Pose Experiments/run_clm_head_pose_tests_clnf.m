clear;

% fitting parameters more suitable for ccnf

%%
% Run the BU test with ccnf
if(exist([getenv('USERPROFILE') '/Dropbox/AAM/test data/'], 'file'))
    database_root = [getenv('USERPROFILE') '/Dropbox/AAM/test data/'];    
else
    database_root = 'F:/Dropbox/Dropbox/AAM/test data/';
end

buDir = [database_root, '/bu/uniform-light/'];

% The fast and accurate ccnf
%%
v = 3;
[fps_bu_general, resFolderBUccnf_general] = run_bu_experiment_clm(buDir, false, v, 'model', 'model/main_ccnf_general.txt');
[bu_error_ccnf_ccnf_general, pred_hp_bu, gt_hp_bu, all_errors_bu_ccnf_general, rels_bu] = calcBUerror(resFolderBUccnf_general, buDir);

%%
% Run the Biwi test
biwi_dir = '/biwi pose/';
biwi_results_root = '/biwi pose results/';

% Intensity
v = 4;
[fps_biwi_ccnf_general, res_folder_ccnf_general] = run_biwi_experiment_clm(database_root, biwi_dir, biwi_results_root, false, false, v, 'model', 'model/main_ccnf_general.txt');
% Calculate the resulting errors
[biwi_error_ccnf_general, pred_hp_biwi, gt_hp_biwi, ~, all_errors_biwi_ccnf_general, rels_biwi] = calcBiwiError([database_root res_folder_ccnf_general], [database_root biwi_dir]);

%% Run the ICT test
ict_dir = ['ict/'];
ict_results_root = ['ict results/'];

v = 4;
% Intensity
[fps_ict_ccnf_general, res_folder_ict_ccnf_general] = run_ict_experiment_clm(database_root, ict_dir, ict_results_root, false, false, v, 'model', 'model/main_ccnf_general.txt');
% Calculate the resulting errors
[ict_error_ccnf_general, pred_hp_ict, gt_hp_ict, ~, all_errors_ict_ccnf_general, rel_ict] = calcIctError([database_root res_folder_ict_ccnf_general], [database_root ict_dir]);

%% Save the results
v = 1;
filename = [sprintf('results/Pose_clm_ccnf_v%s', num2str(v))];
save(filename);

% Also save them in a reasonable .txt format for easy comparison
f = fopen(sprintf('results/Pose_clm_ccnf_v%s.txt', num2str(v)), 'w');
fprintf(f, 'Dataset and model,        pitch,  yaw,  roll,  mean,  median\n');
fprintf(f, 'biwi error ccnf general:  %.3f,   %.3f, %.3f,  %.3f,  %.3f\n', biwi_error_ccnf_general, mean(all_errors_biwi_ccnf_general(:)), median(all_errors_biwi_ccnf_general(:)));
fprintf(f, 'bu error ccnf general:    %.3f,   %.3f, %.3f,  %.3f,  %.3f\n', bu_error_ccnf_ccnf_general, mean(all_errors_bu_ccnf_general(:)), median(all_errors_bu_ccnf_general(:)));
fprintf(f, 'ict error ccnf general:   %.3f,   %.3f, %.3f,  %.3f,  %.3f\n', ict_error_ccnf_general, mean(all_errors_ict_ccnf_general(:)), median(all_errors_ict_ccnf_general(:)));

fclose(f);
clear 'f'