clear
load('scales.mat');
% first ex
scale_names = {'madrs_totalscore', 'panss_pos_total', 'panss_neg_total', 'panss_gen_total',...
               'anxiety_reported_apprehens', 'depression_include_sadness', 'suicidality_expressed_desi',...
               'guilt_overconcern_or_remor', 'hostility_animosity_contem', ...
               'elevated_mood_a_pervasive', 'grandiosity_exaggerated_se', 'suspiciousness_expressed_o', ...
               'blunted_affect_restricted', 'emotional_withdrawal_defic', 'tension_observable_physica', ...
               'unco_operativeness_resista', 'excitement_heightened_emot', 'distractibility_degree_to',...
               'motor_hyperactivity_increa', 'mannerisms_and_posturing_u', 'BPRS_total'};


videos = {'150604_MS0010', '150618_MS0012', '150619_MS0013', '150624_MS0014',...
          '150624_MS0015', '150626_MS0016', '150629_MS0017', '150630_MS0019',...
          '150701_MS0020', '150707_MS0021', '150709_MS0022', '150713_MS0023',...
          '150713_MS0024', '150714_MS0025', '150715_MS0026', '150715_MS0027',...
          '150727_MS0028', '150728_MS0029'};

feature_dir = 'C:\hms\output_vids/';
features = [];
feature_names = {'pose_x', 'pose_y', 'pose_z', 'rot_x', 'rot_y', 'rot_z',...
                 'AU01_r', 'AU04_r', 'AU06_r', 'AU10_r', 'AU12_r', 'AU14_r',...
                 'AU17_r', 'AU25_r', 'AU02_r', 'AU05_r', 'AU09_r', 'AU15_r',...
                 'AU07_c', 'AU17_c', 'AU23_c', 'AU28_c', 'AU15_c', 'AU45_c'};

prefixes = {'pat_mean_', 'doc_mean_', 'pat_std_', 'doc_std_'};            
% prefixes = {'doc_mean_', 'doc_std_'};            

feature_names_all = {};

for p=prefixes
    for f=feature_names
        feature_name = cat(2, p{1}, f{1});
        feature_names_all = cat(1, feature_names_all, feature_name);
    end
end
             
for v=1:numel(videos)
    doc_name = dir([feature_dir, videos{v}, '*doc.txt']);
    pat_name = dir([feature_dir, videos{v}, '*pat.txt']);

    feats_doc = csvread([feature_dir, doc_name(1).name], 1, 0);
    success_doc = feats_doc(:,1) > 0;
    
    feats_doc_mean = mean(feats_doc(success_doc,2:end));
    feats_doc_std = std(feats_doc(success_doc,2:end));
    
    feats_pat = csvread([feature_dir, pat_name(1).name], 1, 0);
    success_pat = feats_pat(:,1) > 0;
    feats_pat_mean = mean(feats_pat(success_pat,2:end));
    feats_pat_std = std(feats_pat(success_pat,2:end));
    
    feats_all = cat(2, feats_pat_mean, feats_doc_mean, feats_pat_std, feats_doc_std);
%     feats_all = cat(2, feats_doc_mean, feats_doc_std);
    
    features = cat(1, features, feats_all);
end

for scale=1:size(scales,2)
   
    sc = scales(:,scale);
    c_scale = corr(features, sc);
    
    c_scale_sq = c_scale .^2;
    
    [best_c, ind_best] = max(c_scale_sq);
    
    fprintf('%s, best %s, corr - %.3f\n', scale_names{scale}, feature_names_all{ind_best}, c_scale(ind_best));
    
end
features = zscore(features);
% Do the actual prediction
preds_all = zeros(size(scales));
for test_fold = 1:size(scales,1)
   
    % cross-validate the parameters
    test_data = features(test_fold, :);
    
    train_fold = setdiff(1:size(scales,1), test_fold);
    
    train_data = features(train_fold,:);

    test_data = cat(2, ones(size(test_data,1),1), test_data);
    train_data = cat(2, ones(size(train_data,1),1), train_data);
    
    scales_test = scales(test_fold,:);
    scales_train = scales(train_fold, :);
    
    for scale = 1:size(scales,2)
    
        reg_vals = train_data \ scales_train(:, scale);
%         reg_vals = scales_train(:, scale) \ train_data;
%         p = regress(scales_train(:,scale), train_data);
%         p = polyfit(train_data, scales_train, 1);
        pred = test_data * reg_vals;
    
        preds_all(test_fold, scale) = pred;
    end
end

mean(abs(preds_all - scales));
fprintf('----------------------------\n');
for scale=1:size(scales,2)
   
    c_scale = corr(preds_all(:,scale), scales(:,scale));
    
    fprintf('%s, corr - %.3f\n', scale_names{scale}, c_scale);
    
end