clear

csv_loc = 'D:\JANUS_training\aflw\aflw_68_dev.csv';
csv_meta_loc = 'D:\JANUS_training\aflw/metadata_68_dev.csv';
root_loc = 'D:\Datasets\AFLW/';

[~, bboxes, labels] = Collect_imgs(csv_loc, csv_meta_loc, root_loc);

%% 

mouth_inds = 49:68;

scrsz = get(0,'ScreenSize');
figure1 = figure('Position',[20 50 3*scrsz(3)/4 0.9*scrsz(4)]);

set(figure1,'Units','Inches');
pos = get(figure1,'Position');
set(figure1,'PaperPositionMode','Auto','PaperUnits','Inches','PaperSize',[pos(3), pos(4)])

% Create axes
axes1 = axes('Parent',figure1,'FontSize',40,'FontName','Helvetica');

line_width = 5;
hold on;

load('results/results_dev_clnf_bbox_mv.mat');
shapes = experiments.shapes;
to_use = sum(labels(:,mouth_inds,1)~=0,2) > 1;

clnf_error = compute_error( labels(to_use,mouth_inds,:),  shapes(mouth_inds,:,to_use)-0.5, bboxes);

[error_x, error_y] = cummErrorCurve(clnf_error);

plot(error_x, error_y, 'Color', [0, 0.447, 0.741],'DisplayName', 'CLNF', 'LineWidth',line_width);

load('results/results_dev_clnf_bbox_mv_hierarch.mat');
shapes = experiments.shapes;

clnf_error_hierarch = compute_error( labels(to_use,mouth_inds,:),  shapes(mouth_inds,:,to_use)-0.5, bboxes);

[error_x, error_y] = cummErrorCurve(clnf_error_hierarch);

plot(error_x, error_y, 'Color', [0.85, 0.325, 0.098], 'DisplayName', 'CLNF hierarchical', 'LineWidth',line_width);

set(gca,'xtick',[0:0.01:0.06])
xlim([0,0.06]);
xlabel('Size normalised shape RMS error','FontName','Helvetica');
ylabel('Proportion of images','FontName','Helvetica');
grid on
title('Mouth error - AFLW','FontSize',40,'FontName','Helvetica');

legend('show', 'Location', 'SouthEast');

print -dpdf results/dev_data_res_mouth.pdf

%% 

eye_inds = [43,44,45,46,47,48, 37,38,39,40,41,42];

scrsz = get(0,'ScreenSize');
figure1 = figure('Position',[20 50 3*scrsz(3)/4 0.9*scrsz(4)]);

set(figure1,'Units','Inches');
pos = get(figure1,'Position');
set(figure1,'PaperPositionMode','Auto','PaperUnits','Inches','PaperSize',[pos(3), pos(4)])

% Create axes
axes1 = axes('Parent',figure1,'FontSize',40,'FontName','Helvetica');

line_width = 5;
hold on;

load('results/results_dev_clnf_bbox_mv.mat');
shapes = experiments.shapes;
to_use = sum(labels(:,eye_inds,1)~=0,2) > 2;

clnf_error = compute_error( labels(to_use,eye_inds,:),  shapes(eye_inds,:,to_use)-0.5, bboxes);

[error_x, error_y] = cummErrorCurve(clnf_error);

plot(error_x, error_y, 'Color', [0, 0.447, 0.741],'DisplayName', 'CLNF', 'LineWidth',line_width);

load('results/results_dev_clnf_bbox_mv_hierarch.mat');
shapes = experiments.shapes;

clnf_error_hierarch = compute_error( labels(to_use,eye_inds,:),  shapes(eye_inds,:,to_use)-0.5, bboxes);

[error_x, error_y] = cummErrorCurve(clnf_error_hierarch);

plot(error_x, error_y, 'Color', [0.85, 0.325, 0.098], 'DisplayName', 'CLNF hierarchical', 'LineWidth',line_width);

set(gca,'xtick',[0:0.01:0.075])
xlim([0,0.075]);
xlabel('Size normalised shape RMS error','FontName','Helvetica');
ylabel('Proportion of images','FontName','Helvetica');
grid on
title('Eyelid error - AFLW','FontSize',40,'FontName','Helvetica');

legend('show', 'Location', 'SouthEast');

print -dpdf results/dev_data_res_eyes.pdf