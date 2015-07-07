clear

csv_loc = 'D:\JANUS_training\aflw\aflw_68_dev.csv';
csv_meta_loc = 'D:\JANUS_training\aflw/metadata_68_dev.csv';
root_loc = 'D:\Datasets\AFLW/';

[images, detections, labels, inds] = Collect_CS0_imgs();

%% 
scrsz = get(0,'ScreenSize');
figure1 = figure('Position',[20 50 3*scrsz(3)/4 0.9*scrsz(4)]);

set(figure1,'Units','Inches');
pos = get(figure1,'Position');
set(figure1,'PaperPositionMode','Auto','PaperUnits','Inches','PaperSize',[pos(3), pos(4)])

% Create axes
axes1 = axes('Parent',figure1,'FontSize',40,'FontName','Helvetica');

line_width = 5;
hold on;

load('results/results_janus_clnf.mat');
shapes = experiments.shapes;

clnf_error = compute_error_JANUS( experiments.labels(:,:,inds ==1),  shapes(:,:,inds ==1));

[error_x, error_y] = cummErrorCurve(clnf_error);

plot(error_x, error_y, 'c','DisplayName', 'CLNF basic', 'LineWidth',line_width);

load('results/results_janus_clnf_better_ROI.mat');
shapes = experiments.shapes;

clnf_error = compute_error_JANUS( experiments.labels(:,:,inds ==1),  shapes(:,:,inds ==1));

[error_x, error_y] = cummErrorCurve(clnf_error);

plot(error_x, error_y, 'b','DisplayName', 'CLNF refined ROI', 'LineWidth',line_width);

load('results/results_janus_clnf_perfect_ROI.mat');
shapes = experiments.shapes;

clnf_error = compute_error_JANUS( experiments.labels(:,:,inds ==1),  shapes(:,:,inds ==1));

[error_x, error_y] = cummErrorCurve(clnf_error);

plot(error_x, error_y, 'k','DisplayName', 'CLNF refined ROI', 'LineWidth',line_width);

load('results/results_janus_clnf_better_ROI_mv_3.mat');
shapes = experiments.shapes;

clnf_error = compute_error_JANUS( experiments.labels(:,:,inds ==1),  shapes(:,:,inds ==1));

[error_x, error_y] = cummErrorCurve(clnf_error);

plot(error_x, error_y, 'r','DisplayName', 'CLNF refined ROI multi view', 'LineWidth',line_width);

% load('results/results_dev_clnf_bbox_mv.mat');
% shapes = experiments.shapes;
% 
% clnf_error = compute_error( labels,  shapes-0.5, bboxes);
% 
% [error_x, error_y] = cummErrorCurve(clnf_error);

% plot(error_x, error_y, 'k','DisplayName', 'CLNF refined ROI multiview', 'LineWidth',line_width);

set(gca,'xtick',[0:0.05:0.2])
xlim([0,0.2]);
xlabel('Size normalised shape RMS error for "frontal"','FontName','Helvetica');
ylabel('Proportion of images','FontName','Helvetica');
grid on
% title('Fitting in the wild without outline','FontSize',60,'FontName','Helvetica');

legend('show', 'Location', 'SouthEast');

print -dpdf results/JANUS_data_res_frontal.pdf