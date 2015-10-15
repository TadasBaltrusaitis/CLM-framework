% Working out corrections for head pose and model correlations
clear
%%
% first need to run run_clm_head_pose_tests_clnf
if(exist([getenv('USERPROFILE') '/Dropbox/AAM/test data/'], 'file'))
    database_root = [getenv('USERPROFILE') '/Dropbox/AAM/test data/'];    
else
    database_root = 'F:/Dropbox/Dropbox/AAM/test data/';
end
buDir = [database_root, '/bu/uniform-light/'];
resFolderBUccnf_general = [database_root, '/bu/uniform-light/CLMr3/'];
[~, pred_hp_bu, gt_hp_bu, ~, rels_bu] = calcBUerror(resFolderBUccnf_general, buDir);

biwi_dir = '/biwi pose/';
biwi_results_root = '/biwi pose results/';
res_folder_ccnf_general = '/biwi pose results//CLMr4/';
[~, pred_hp_biwi, gt_hp_biwi, ~, ~, rels_biwi] = calcBiwiError([database_root res_folder_ccnf_general], [database_root biwi_dir]);

ict_dir = ['ict/'];
ict_results_root = ['ict results/'];
res_folder_ict_ccnf_general = 'ict results//CLMr4/';
[~, pred_hp_ict, gt_hp_ict, ~, ~, rel_ict] = calcIctError([database_root res_folder_ict_ccnf_general], [database_root ict_dir]);

% resFolderBUCLM_general = [database_root, '/bu/uniform-light/CLMr1/'];
% [~, pred_hp_bu_clm, pred_gt_bu_clm, all_errors_bu_svr_general, rels_bu_clm] = calcBUerror(resFolderBUCLM_general, buDir);
% 
% biwi_dir = '/biwi pose/';
% res_folder_ccnf_general = '/biwi pose results//CLMr1/';
% [~, pred_hp_biwi_clm, gt_hp_biwi_clm, ~, ~, rels_biwi_clm] = calcBiwiError([database_root res_folder_ccnf_general], [database_root biwi_dir]);
% 
% biwi_dir = '/biwi pose/';
% res_folder_ccnf_general = '/biwi pose results//CLMr2_depth/';
% [~, pred_hp_biwi_clmz, gt_hp_biwi_clmz, ~, ~, rels_biwi_clmz] = calcBiwiError([database_root res_folder_ccnf_general], [database_root biwi_dir]);
% 
% res_folder_ict_ccnf_general = 'ict results//CLMr1/';
% [~, pred_hp_ict_clm, gt_hp_ict_clm, ~, ~, rel_ict_clm] = calcIctError([database_root res_folder_ict_ccnf_general], [database_root ict_dir]);
% 
% ict_results_root = ['ict results/'];
% res_folder_ict_ccnf_general = 'ict results//CLMr2_depth/';
% [~, pred_hp_ict_clmz, gt_hp_ict_clmz, ~, ~, rel_ict_clmz] = calcIctError([database_root res_folder_ict_ccnf_general], [database_root ict_dir]);

%%
all_hps = cat(1, pred_hp_bu, pred_hp_biwi, pred_hp_ict);
all_gts = cat(1, gt_hp_bu, gt_hp_biwi, gt_hp_ict);
all_rels = cat(1, rels_bu, rels_biwi, rel_ict);

rel_cutoff = 0.8;

rel_frames = all_rels > rel_cutoff;
fprintf('Proportion of reliable frames: %.2f\n', sum(rel_frames)/numel(rel_frames));

err_bu = abs(pred_hp_bu(rels_bu > rel_cutoff,:) - gt_hp_bu(rels_bu > rel_cutoff,:));
err_biwi = abs(pred_hp_biwi(rels_biwi > rel_cutoff,:) - gt_hp_biwi(rels_biwi > rel_cutoff,:));
err_ict = abs(pred_hp_ict(rel_ict > rel_cutoff,:) - gt_hp_ict(rel_ict > rel_cutoff,:));

all_err = mean(abs(all_gts - all_hps), 2);

% corr(all_hps, all_gts)
corr(all_hps(rel_frames, :), all_gts(rel_frames, :))

