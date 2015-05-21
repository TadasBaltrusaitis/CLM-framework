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
load('intraface_v1.1/bu.mat');
[bu_intraface_error, ~, ~, all_errors_bu_intraface] = calcBUerror(pose_results, buDir);

load('intraface_v1.1/bu20.mat');
[bu_intraface_error_20, ~, ~, all_errors_bu_intraface_20] = calcBUerror(pose_results, buDir);

%%
% Run the biwi test
biwi_dir = '/biwi pose/';
biwi_results_root = '/biwi pose results/';
load('intraface_v1.1/biwi.mat');
[biwi_intraface_error, ~, ~, ~, all_errors_biwi_intraface] = calcBiwiError(pose_results, [database_root biwi_dir]);

load('intraface_v1.1/biwi20.mat');
[biwi_intraface_error20, ~, ~, ~, all_errors_biwi_intraface20] = calcBiwiError(pose_results, [database_root biwi_dir]);

%% Run the CLM-Z and general ICT test
ict_dir = ['ict/'];
ict_results_root = ['ict results/'];
load('intraface_v1.1/ict.mat');
[ict_error_intraface, ~, ~, ~, all_errors_ict_intraface] = calcIctError(pose_results, [database_root ict_dir]);

load('intraface_v1.1/ict20.mat');
[ict_error_intraface20, ~, ~, ~, all_errors_ict_intraface20] = calcIctError(pose_results, [database_root ict_dir]);
