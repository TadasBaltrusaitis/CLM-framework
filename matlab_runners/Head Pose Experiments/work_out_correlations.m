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
resFolderBUclnf_general = [database_root, '/bu/uniform-light/CLMr3/'];
[~, pred_hp_bu, gt_hp_bu, ~, rels_bu] = calcBUerror(resFolderBUclnf_general, buDir);

biwi_dir = '/biwi pose/';
biwi_results_root = '/biwi pose results/';
res_folder_clnf_general = '/biwi pose results//CLMr4/';
[~, pred_hp_biwi, gt_hp_biwi, ~, ~, rels_biwi] = calcBiwiError([database_root res_folder_clnf_general], [database_root biwi_dir]);

ict_dir = ['ict/'];
ict_results_root = ['ict results/'];
res_folder_ict_clnf_general = 'ict results//CLMr4/';
[~, pred_hp_ict, gt_hp_ict, ~, ~, rel_ict] = calcIctError([database_root res_folder_ict_clnf_general], [database_root ict_dir]);

resFolderBUCLM_general = [database_root, '/bu/uniform-light/CLMr1/'];
[~, pred_hp_bu_clm, pred_gt_bu_clm, all_errors_bu_svr_general, rels_bu_clm] = calcBUerror(resFolderBUCLM_general, buDir);

biwi_dir = '/biwi pose/';
res_folder_clnf_general = '/biwi pose results//CLMr1/';
[~, pred_hp_biwi_clm, gt_hp_biwi_clm, ~, ~, rels_biwi_clm] = calcBiwiError([database_root res_folder_clnf_general], [database_root biwi_dir]);


biwi_dir = '/biwi pose/';
res_folder_clnf_general = '/biwi pose results//CLMr2_depth/';
[~, pred_hp_biwi_clmz, gt_hp_biwi_clmz, ~, ~, rels_biwi_clmz] = calcBiwiError([database_root res_folder_clnf_general], [database_root biwi_dir]);

res_folder_ict_clnf_general = 'ict results//CLMr1/';
[~, pred_hp_ict_clm, gt_hp_ict_clm, ~, ~, rel_ict_clm] = calcIctError([database_root res_folder_ict_clnf_general], [database_root ict_dir]);

ict_results_root = ['ict results/'];
res_folder_ict_clnf_general = 'ict results//CLMr2_depth/';
[~, pred_hp_ict_clmz, gt_hp_ict_clmz, ~, ~, rel_ict_clmz] = calcIctError([database_root res_folder_ict_clnf_general], [database_root ict_dir]);

%%
%all_hps = cat(1, pred_hp_bu, pred_hp_biwi, pred_hp_ict, pred_hp_bu_clm, pred_hp_biwi_clm, pred_hp_biwi_clmz, pred_hp_ict_clm, pred_hp_ict_clmz);
%all_gts = cat(1, gt_hp_bu, gt_hp_biwi, gt_hp_ict, pred_gt_bu_clm, gt_hp_biwi_clm, gt_hp_biwi_clmz, gt_hp_ict_clm, gt_hp_ict_clmz);
% all_rels = cat(1, rels_bu, rels_biwi, rel_ict, rels_bu_clm, rels_biwi_clm, rels_biwi_clmz, rel_ict_clm, rel_ict_clmz);

all_hps = cat(1, pred_hp_bu, pred_hp_biwi, pred_hp_ict);
all_gts = cat(1, gt_hp_bu, gt_hp_biwi, gt_hp_ict);
all_rels = cat(1, rels_bu, rels_biwi, rel_ict);

rel_frames = all_rels > 0.8;

all_err = mean(abs(all_gts - all_hps), 2);

corr(all_hps, all_gts)
corr(all_hps(rel_frames, :), all_gts(rel_frames, :))

centres_all = [0     0     0;
               0   -20     0;
               0   -45     0;
               0   -70     0;
               0    20     0;
               0    45     0;
               0    70     0];       
           
ids = zeros(size(all_err,1),1);
mins = [];

for i=1:size(centres_all, 1)                
    mins = cat(2, mins, mean(abs(bsxfun(@plus, all_hps, -centres_all(i,:))), 2));
end

% find the center id for each of the frames
[~, ids] = min(mins');
ids = ids';

%%
% pitch_err = mean(abs(all_hps(rel_frames,1) - all_gts(rel_frames,1)))
% yaw_err = mean(abs(all_hps(rel_frames,2) - all_gts(rel_frames,2)))
% roll_err = mean(abs(all_hps(rel_frames,3) - all_gts(rel_frames,3)))

%% draw errors properly
yaw_bins  = [-50, -40, -30, -20, -10, 0, 10, 20, 30, 40, 50];
err_yaw_bin = zeros(size(yaw_bins));
std_yaw_bin = zeros(size(yaw_bins));

yaw_bin = bsxfun(@plus, all_gts(:,2), yaw_bins);
[~, ids] = min(abs(yaw_bin'));
ids = ids';

for i=1:numel(yaw_bins)
   
    rel_bins = ids == i & rel_frames;
    
    err_bin(i) = mean(abs(all_hps(rel_bins,2) - all_gts(rel_bins,2)));
    std_bin(i) = std(abs(all_hps(rel_bins,2) - all_gts(rel_bins,2)));
end
