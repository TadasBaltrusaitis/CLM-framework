clear;

%%
% Run the BU test with CLM
if(exist([getenv('USERPROFILE') '/Dropbox/AAM/test data/'], 'file'))
    database_root = [getenv('USERPROFILE') '/Dropbox/AAM/test data/'];    
else
    database_root = 'F:/Dropbox/Dropbox/AAM/test data/';
end

buDir = [database_root, '/bu/uniform-light/'];

% The fast and accurate single light models
%%
v = 1;
[fps_bu_general, resFolderBUCLM_general] = run_bu_experiment(buDir, false, v, 'model', 'model/main_clm_general.txt');
[bu_error_clm_svr_general, ~, ~, all_errors_bu_svr_general] = calcBUerror(resFolderBUCLM_general, buDir);

%%
% Run the CLM-Z and general Biwi test
biwi_dir = '/biwi pose/';
biwi_results_root = '/biwi pose results/';

% Intensity
v = 1;
[fps_biwi_clm, res_folder_clm_biwi] = run_biwi_experiment(database_root, biwi_dir, biwi_results_root, false, false, v, 'model', 'model/main_clm-z.txt');
% Calculate the resulting errors
[biwi_error_clm, ~, ~, ~, all_errors_biwi_clm] = calcBiwiError([database_root res_folder_clm_biwi], [database_root biwi_dir]);

% Intensity with depth
v = 2;
[fps_biwi_clmz, res_folder_clmz_biwi] = run_biwi_experiment(database_root, biwi_dir, biwi_results_root, false, true, v, 'model', 'model/main_clm-z.txt');
% Calculate the resulting errors
[biwi_error_clmz, ~, ~, ~, all_errors_biwi_clm_z] = calcBiwiError([database_root res_folder_clmz_biwi], [database_root biwi_dir]);

%% Run the CLM-Z and general ICT test
ict_dir = ['ict/'];
ict_results_root = ['ict results/'];

v = 1;
% Intensity
[fps_ict_clm, res_folder_ict_clm] = run_ict_experiment(database_root, ict_dir, ict_results_root, false, false, v, 'model', 'model/main_clm-z.txt');
[ict_error_clm, ~, ~, ~, all_errors_ict_clm] = calcIctError([database_root res_folder_ict_clm], [database_root ict_dir]);

v = 2;
% Intensity and depth
[fps_ict_clmz, res_folder_ict_clmz] = run_ict_experiment(database_root, ict_dir, ict_results_root, false, true, v, 'model', 'model/main_clm-z.txt');
[ict_error_clmz, ~, ~, ~, all_errors_ict_clm_z] = calcIctError([database_root res_folder_ict_clmz], [database_root ict_dir]);

%% Save the results
v = 1;
filename = 'results/Pose_clm';
save(filename);
% 
% Also save them in a reasonable .txt format for easy comparison
f = fopen('results/Pose_clm.txt', 'w');
fprintf(f, 'Dataset and model,        pitch,  yaw,  roll,  mean,  median\n');
fprintf(f, 'biwi error clm:           %.3f,   %.3f, %.3f,  %.3f,  %.3f\n', biwi_error_clm, mean(all_errors_biwi_clm(:)), median(all_errors_biwi_clm(:)));
fprintf(f, 'biwi error clm-z:         %.3f,   %.3f, %.3f,  %.3f,  %.3f\n', biwi_error_clmz, mean(all_errors_biwi_clm_z(:)), median(all_errors_biwi_clm_z(:)));
fprintf(f, 'bu error clm general:     %.3f,   %.3f, %.3f,  %.3f,  %.3f\n', bu_error_clm_svr_general, mean(all_errors_bu_svr_general(:)), median(all_errors_bu_svr_general(:)));
fprintf(f, 'ict error clm:            %.3f,   %.3f, %.3f,  %.3f,  %.3f\n', ict_error_clm, mean(all_errors_ict_clm(:)), median(all_errors_ict_clm(:)));
fprintf(f, 'ict error clm-z:          %.3f,   %.3f, %.3f,  %.3f,  %.3f\n', ict_error_clmz, mean(all_errors_ict_clm_z(:)), median(all_errors_ict_clm_z(:)));

fclose(f);
clear 'f'