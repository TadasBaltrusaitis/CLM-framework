clear

csv_loc = 'D:\JANUS_training\aflw\aflw_68_dev.csv';
csv_meta_loc = 'D:\JANUS_training\aflw/metadata_68_dev.csv';
root_loc = 'D:\Datasets\AFLW/';

[~, bboxes, labels] = Collect_imgs(csv_loc, csv_meta_loc, root_loc);

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

load('results/results_dev_clnf_jpeg.mat');
shapes = experiments.shapes;

clnf_error = compute_error( labels,  shapes(:,:,:,1)-0.5, bboxes);

[error_x, error_y] = cummErrorCurve(clnf_error);

plot(error_x, error_y, 'Color',[ 0.8500    0.3250    0.0980],'DisplayName', 'JPEG - 1', 'LineWidth',line_width);

load('results/results_dev_clnf_jpeg.mat');
shapes = experiments.shapes;

clnf_error = compute_error( labels,  shapes(:,:,:,2)-0.5, bboxes);

[error_x, error_y] = cummErrorCurve(clnf_error);

plot(error_x, error_y, 'DisplayName', 'JPEG - 10','Color', [0,0.447,0.741], 'LineWidth',line_width);

load('results/results_dev_clnf_jpeg.mat');
shapes = experiments.shapes;

clnf_error = compute_error( labels,  shapes(:,:,:,3)-0.5, bboxes);

[error_x, error_y] = cummErrorCurve(clnf_error);

plot(error_x, error_y, 'DisplayName', 'JPEG - 20', 'LineWidth',line_width);

load('results/results_dev_clnf_jpeg.mat');
shapes = experiments.shapes;

clnf_error = compute_error( labels,  shapes(:,:,:,4)-0.5, bboxes);

[error_x, error_y] = cummErrorCurve(clnf_error);

plot(error_x, error_y, 'DisplayName', 'JPEG - 30', 'Color', 'k', 'LineWidth',line_width);

% load('results/results_dev_clm.mat');
% shapes = experiments.shapes;
% 
% clnf_error = compute_error( labels,  shapes-0.5, bboxes);
% 
% [error_x, error_y] = cummErrorCurve(clnf_error);
% 
% plot(error_x, error_y, 'b','DisplayName', 'CLM basic', 'LineWidth',line_width);

% load('results/results_dev_clnf_bbox.mat');
% shapes = experiments.shapes;
% 
% clnf_error = compute_error( labels,  shapes-0.5, bboxes);
% 
% [error_x, error_y] = cummErrorCurve(clnf_error);
% 
% plot(error_x, error_y, 'Color',[0.929,0.694,0.125],'DisplayName', 'CLNF refined ROI', 'LineWidth',line_width);

% load('results/results_dev_clnf_bbox_mv_janus.mat');
% shapes = experiments.shapes;
% 
% clnf_error = compute_error( labels,  shapes-0.5, bboxes);
% 
% [error_x, error_y] = cummErrorCurve(clnf_error);
% 
% plot(error_x, error_y, 'g','DisplayName', 'CLNF met', 'LineWidth',line_width);

% load('results/results_dev_clnf_ideal_bbox.mat');
% shapes = experiments.shapes;
% 
% clnf_error = compute_error( labels,  shapes-0.5, bboxes);
% 
% [error_x, error_y] = cummErrorCurve(clnf_error);
% 
% plot(error_x, error_y, 'k','DisplayName', 'CLNF manual ROI multiview', 'LineWidth',line_width);
% 
% load('results/results_dev_clnf_bbox_mv.mat');
% shapes = experiments.shapes;
% 
% clnf_error = compute_error( labels,  shapes-0.5, bboxes);
% 
% [error_x, error_y] = cummErrorCurve(clnf_error);
% 
% plot(error_x, error_y, 'r','DisplayName', 'CLNF automatic ROI multiview', 'LineWidth',line_width);

set(gca,'xtick',[0:0.03:0.12])
xlim([0,0.12]);
xlabel('Size normalised shape RMS error','FontName','Helvetica');
ylabel('Proportion of images','FontName','Helvetica');
grid on
title('AFLW accuracy','FontSize',40,'FontName','Helvetica');

legend('show', 'Location', 'SouthEast');

print -dpdf results/dev_data_res_jpeg.pdf