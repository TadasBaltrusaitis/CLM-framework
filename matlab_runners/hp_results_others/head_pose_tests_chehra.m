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
load('chehra/bu_pose.mat');
[bu_chehra_error, ~, ~, all_errors_bu_chehra] = calcBUerror_chehra(pose_results, buDir);

%%
% Run the biwi test
biwi_dir = '/biwi pose/';
biwi_results_root = '/biwi pose results/';
load('chehra/biwi_pose.mat');
[biwi_chehra_error, ~, ~, ~, all_errors_biwi_chehra] = calcBiwiError_chehra(pose_results, [database_root biwi_dir]);

%% Run the general ICT test
ict_dir = ['ict/'];
ict_results_root = ['ict results/'];
load('chehra/ict_pose.mat');
[ict_error_chehra, ~, ~, ~, all_errors_ict_chehra] = calcIctError_chehra(pose_results, [database_root ict_dir]);