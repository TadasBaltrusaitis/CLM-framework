clear
     
curr_dir = cd('.');

% Replace this with your downloaded 300-W train data
if(exist([getenv('USERPROFILE') '/Dropbox/AAM/test data/'], 'file'))
    database_root = [getenv('USERPROFILE') '/Dropbox/AAM/test data/'];    
else
    database_root = 'F:/Dropbox/Dropbox/AAM/test data/';
end

%% Run using CLNF in the wild model
out_clnf = [curr_dir '/out_wild_clnf_wild/'];
if(~exist(out_clnf, 'file'))
   mkdir(out_clnf); 
end

[err_clnf_wild, err_no_out_clnf_wild] = Run_CLM_fitting_on_images(out_clnf, database_root, 'use_afw', 'use_lfpw', 'use_ibug', 'use_helen', 'verbose', 'model', 'model/main_ccnf_wild.txt', 'multi_view', 1);

%% Run using SVR model
out_svr = [curr_dir '/out_wild_svr_wild/'];
if(~exist(out_svr, 'file'))
   mkdir(out_svr); 
end

[err_svr_wild, err_no_out_svr_wild] = Run_CLM_fitting_on_images(out_svr, database_root, 'use_afw', 'use_lfpw', 'use_ibug', 'use_helen', 'verbose', 'model', 'model/main_svr_wild.txt', 'multi_view', 1);                

%% Run using general CLNF model
out_clnf = [curr_dir '/out_wild_clnf/'];
if(~exist(out_clnf, 'file'))
   mkdir(out_clnf); 
end

[err_clnf, err_no_out_clnf] = Run_CLM_fitting_on_images(out_clnf, database_root, 'use_afw', 'use_lfpw', 'use_ibug', 'use_helen', 'verbose', 'model', 'model/main_ccnf_general.txt', 'multi_view', 1);
                      
%% Run using SVR model
out_svr = [curr_dir '/out_wild_svr/'];
if(~exist(out_svr, 'file'))
   mkdir(out_svr); 
end

[err_svr, err_no_out_svr] = Run_CLM_fitting_on_images(out_svr, database_root, 'use_afw', 'use_lfpw', 'use_ibug', 'use_helen', 'verbose', 'model', 'model/main_svr_general.txt', 'multi_view', 1);                

%%
save('results/landmark_detections.mat');

f = fopen('results/landmark_detections.txt', 'w');
fprintf(f, 'Type, mean, median\n');
fprintf(f, 'err clnf: %f, %f\n', mean(err_clnf), median(err_clnf));
fprintf(f, 'err clnf wild: %f, %f\n', mean(err_clnf_wild), median(err_clnf_wild));

fprintf(f, 'err svr: %f, %f\n', mean(err_svr), median(err_svr));
fprintf(f, 'err svr wild: %f, %f\n', mean(err_svr_wild), median(err_svr_wild));

fprintf(f, 'err clnf no out: %f, %f\n', mean(err_no_out_clnf), median(err_no_out_clnf));
fprintf(f, 'err clnf wild no out: %f, %f\n', mean(err_no_out_clnf_wild), median(err_no_out_clnf_wild));

fprintf(f, 'err svr no out: %f, %f\n', mean(err_no_out_svr), median(err_no_out_svr));
fprintf(f, 'err svr wild no out: %f, %f\n', mean(err_no_out_svr_wild), median(err_no_out_svr_wild));

fclose(f);

%% Draw the corresponding error graphs comparing CLNF and SVR

% set up the canvas
line_width = 6;
scrsz = get(0,'ScreenSize');
figure1 = figure('Position',[20 50 3*scrsz(3)/4 0.9*scrsz(4)]);

set(figure1,'Units','Inches');
pos = get(figure1,'Position');
set(figure1,'PaperPositionMode','Auto','PaperUnits','Inches','PaperSize',[pos(3), pos(4)])
% Create axes
axes1 = axes('Parent',figure1,'FontSize',40,'FontName','Helvetica');

% load intraface errors
load('landmark_det_baselines/intraface_wild.mat');
labels = labels_all(18:end,:,:);
shapes = shapes_all(18:end,:,:);

intraface_error = compute_error( labels - 0.5,  shapes);

% removing faces that were not detected by intraface for fairness
detected = intraface_error < 1;

inds_in_cpp = [];

load('landmark_det_baselines/zhu_wild.mat');
load('out_wild_clnf_wild/res.mat');    
for i=1:size(labels,3)
    
    diffs = squeeze(sum(sum(bsxfun(@plus, labels_all(18:end,:,:)-0.5, - labels([18:60,62:64,66:end],:,i)),1),2));
    inds_in_cpp = cat(1, inds_in_cpp, find(diffs == 0));

end
detected_cpp = detected(inds_in_cpp);

% load intraface errors
load('landmark_det_baselines/intraface_wild.mat');
labels = labels_all(18:end,:,detected);
shapes = shapes_all(18:end,:,detected);

intraface_error = compute_error( labels - 0.5,  shapes);
[error_x, error_y] = cummErrorCurve(intraface_error);
plot(error_x, error_y, 'g--','DisplayName', 'SDM', 'LineWidth',line_width);
hold on;

% load clnf errors
load('out_wild_clnf_wild/res.mat');
labels = labels([1:60,62:64,66:end],:, detected_cpp);
shapes = shapes([1:60,62:64,66:end],:, detected_cpp);
labels = labels(18:end,:,:);
shapes = shapes(18:end,:,:);

clnf_error_cpp = compute_error( labels - 0.5,  shapes);
[error_x, error_y] = cummErrorCurve(clnf_error_cpp);
plot(error_x, error_y, 'r','DisplayName', 'CLM+CLNF', 'LineWidth',line_width);
hold on;

% load svr errors
load('out_wild_svr_wild/res.mat');
labels = labels([1:60,62:64,66:end],:, detected_cpp);
shapes = shapes([1:60,62:64,66:end],:, detected_cpp);
labels = labels(18:end,:,:);
shapes = shapes(18:end,:,:);

svr_error_cpp = compute_error( labels - 0.5,  shapes);
[error_x, error_y] = cummErrorCurve(svr_error_cpp);
plot(error_x, error_y, 'b-.','DisplayName', 'CLM+SVR', 'LineWidth',line_width);

load('landmark_det_baselines/zhu_wild.mat');
labels = labels_all(18:end,:, detected);
shapes = shapes_all(18:end,:, detected);

zhu_error = compute_error( labels,  shapes);
[error_x, error_y] = cummErrorCurve(zhu_error);
plot(error_x, error_y, 'c:','DisplayName', 'Tree based', 'LineWidth',line_width);
hold on;

load('landmark_det_baselines/drmf_wild.mat');
labels = labels_all(18:end,:, detected);
shapes = shapes_all(18:end,:, detected);

drmf_error = compute_error( labels,  shapes);
[error_x, error_y] = cummErrorCurve(drmf_error);
plot(error_x, error_y, 'kx','DisplayName', 'DRMF', 'LineWidth',line_width);
hold on;

% Make it look nice and print to a pdf
set(gca,'xtick',[0:0.05:0.15])
xlim([0,0.15]);
xlabel('Size normalised shape RMS error','FontName','Helvetica');
ylabel('Proportion of images','FontName','Helvetica');
grid on
legend('show', 'Location', 'SouthEast');
print -dpdf in-the-wild-res-no-outline.pdf
