clear
load('results/results_dev_clnf_exps.mat')

all_params = cat(1, experiments(:).params);

all_mean_errs = mean(cat(2,experiments(:).aflw_error));
all_med_errs = median(cat(2,experiments(:).aflw_error));
all_geom_err = sqrt(all_med_errs .* all_mean_errs);

all_mean_errs_front = mean(cat(2,experiments(:).aflw_error_frontal));
all_med_errs_front = median(cat(2,experiments(:).aflw_error_frontal));
all_geom_err_front = sqrt(all_med_errs_front .* all_mean_errs_front);

all_mean_errs_ex = mean(cat(2,experiments(:).aflw_error_extreme));
all_med_errs_ex = median(cat(2,experiments(:).aflw_error_extreme));
all_geom_err_ex = sqrt(all_med_errs_ex .* all_mean_errs_ex);

[~,best_med] = min(all_med_errs');
[~,best_med_f] = min(all_med_errs_front');
[~,best_med_ex] = min(all_med_errs_ex');

[~,best_med_g] = min(all_geom_err');
[~,best_med_f_g] = min(all_geom_err_front');
[~,best_med_ex_g] = min(all_geom_err_ex');