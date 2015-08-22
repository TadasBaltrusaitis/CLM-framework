clear

%% 
scrsz = get(0,'ScreenSize');
figure1 = figure('Position',[20 50 3*scrsz(3)/4 0.9*scrsz(4)]);

set(figure1,'Units','Inches');
pos = get(figure1,'Position');
set(figure1,'PaperPositionMode','Auto','PaperUnits','Inches','PaperSize',[pos(3), pos(4)])

% Create axes
axes1 = axes('Parent',figure1,'FontSize',40,'FontName','Helvetica');

line_width = 6;
hold on;

load('results/results_wild_clnf_general_final_inner.mat');
labels = experiments.labels - 0.5;
shapes = experiments.shapes;
labels(1:17,:,:) = [];
shapes(1:17,:,:) = [];

[clnf_error, err_pp] = compute_error( labels,  shapes);

[error_x, error_y] = cummErrorCurve(clnf_error);
hold on;

plot(error_x, error_y, 'b','DisplayName', 'CLNF inner', 'LineWidth',line_width);

load('results/results_wild_clnf_general.mat');
labels = experiments.labels - 0.5;
shapes = experiments.shapes;
labels(1:17,:,:) = [];
shapes(1:17,:,:) = [];

[clnf_error, err_pp] = compute_error( labels,  shapes);

[error_x, error_y] = cummErrorCurve(clnf_error);
hold on;

plot(error_x, error_y, 'g','DisplayName', 'CLNF', 'LineWidth',line_width);

load('results/results_wild_clnf_general_hierarch.mat');
labels = experiments.labels - 0.5;
shapes = experiments.shapes;

labels(1:17,:,:) = [];
shapes(1:17,:,:) = [];
[clnf_error, err_pp_h] = compute_error( labels,  shapes);

[error_x, error_y] = cummErrorCurve(clnf_error);
hold on;

plot(error_x, error_y, 'r','DisplayName', 'CLNF hierarch', 'LineWidth',line_width);

set(gca,'xtick',[0:0.025:0.1])
xlim([0,0.1]);
xlabel('Size normalised shape RMS error','FontName','Helvetica');
ylabel('Proportion of images','FontName','Helvetica');
grid on
title('Fitting in the wild','FontSize',60,'FontName','Helvetica');

legend('show', 'Location', 'SouthEast');

%% 
scrsz = get(0,'ScreenSize');
figure1 = figure('Position',[20 50 3*scrsz(3)/4 0.9*scrsz(4)]);

set(figure1,'Units','Inches');
pos = get(figure1,'Position');
set(figure1,'PaperPositionMode','Auto','PaperUnits','Inches','PaperSize',[pos(3), pos(4)])

% Create axes
axes1 = axes('Parent',figure1,'FontSize',40,'FontName','Helvetica');

line_width = 6;
hold on;

load('results/results_wild_clnf_general_final_inner.mat');
labels = experiments.labels - 0.5;
shapes = experiments.shapes;

clnf_error_left = compute_error_point_to_line_left_eye( labels,  shapes);
clnf_error_right = compute_error_point_to_line_right_eye( labels,  shapes);
clnf_error = cat(1, clnf_error_left, clnf_error_right);

[error_x, error_y] = cummErrorCurve(clnf_error);
hold on;

plot(error_x, error_y, 'b','DisplayName', 'CLNF', 'LineWidth',line_width);

load('results/results_wild_clnf_general_hierarch.mat');
labels = experiments.labels - 0.5;
shapes = experiments.shapes;

clnf_error_left = compute_error_point_to_line_left_eye( labels,  shapes);
clnf_error_right = compute_error_point_to_line_right_eye( labels,  shapes);
clnf_error = cat(1, clnf_error_left, clnf_error_right);

[error_x, error_y] = cummErrorCurve(clnf_error);
hold on;

plot(error_x, error_y, 'r','DisplayName', 'CLNF hierarch', 'LineWidth',line_width);

set(gca,'xtick',[0:0.01:0.05])
xlim([0,0.05]);
xlabel('Size normalised eyelid RMS error','FontName','Helvetica');
ylabel('Proportion of images','FontName','Helvetica');
grid on
title('Fitting in the wild','FontSize',60,'FontName','Helvetica');

legend('show', 'Location', 'SouthEast');

%% 
scrsz = get(0,'ScreenSize');
figure1 = figure('Position',[20 50 3*scrsz(3)/4 0.9*scrsz(4)]);

set(figure1,'Units','Inches');
pos = get(figure1,'Position');
set(figure1,'PaperPositionMode','Auto','PaperUnits','Inches','PaperSize',[pos(3), pos(4)])

% Create axes
axes1 = axes('Parent',figure1,'FontSize',40,'FontName','Helvetica');

line_width = 6;
hold on;

load('results/results_wild_clnf_general_final_inner.mat');
labels = experiments.labels - 0.5;
shapes = experiments.shapes;

clnf_error = compute_mouth_error( labels,  shapes);

[error_x, error_y] = cummErrorCurve(clnf_error);
hold on;

plot(error_x, error_y, 'b','DisplayName', 'CLNF', 'LineWidth',line_width);

load('results/results_wild_clnf_general_hierarch.mat');
labels = experiments.labels - 0.5;
shapes = experiments.shapes;

clnf_error = compute_mouth_error( labels,  shapes);

[error_x, error_y] = cummErrorCurve(clnf_error);
hold on;

plot(error_x, error_y, 'r','DisplayName', 'CLNF hierarch', 'LineWidth',line_width);

set(gca,'xtick',[0:0.02:0.08])
xlim([0,0.08]);
xlabel('Size normalised mouth RMS error','FontName','Helvetica');
ylabel('Proportion of images','FontName','Helvetica');
grid on
title('Fitting in the wild','FontSize',60,'FontName','Helvetica');

legend('show', 'Location', 'SouthEast');

%% 
scrsz = get(0,'ScreenSize');
figure1 = figure('Position',[20 50 3*scrsz(3)/4 0.9*scrsz(4)]);

set(figure1,'Units','Inches');
pos = get(figure1,'Position');
set(figure1,'PaperPositionMode','Auto','PaperUnits','Inches','PaperSize',[pos(3), pos(4)])

% Create axes
axes1 = axes('Parent',figure1,'FontSize',40,'FontName','Helvetica');

line_width = 6;
hold on;

load('results/results_wild_clnf_general_final_inner.mat');
labels = experiments.labels - 0.5;
shapes = experiments.shapes;

clnf_error = compute_brow_error_to_line( labels,  shapes);

[error_x, error_y] = cummErrorCurve(clnf_error);
hold on;

plot(error_x, error_y, 'b','DisplayName', 'CLNF', 'LineWidth',line_width);

load('results/results_wild_clnf_general_hierarch.mat');
labels = experiments.labels - 0.5;
shapes = experiments.shapes;

clnf_error = compute_brow_error_to_line( labels,  shapes);

[error_x, error_y] = cummErrorCurve(clnf_error);
hold on;

plot(error_x, error_y, 'r','DisplayName', 'CLNF hierarch', 'LineWidth',line_width);

set(gca,'xtick',[0:0.02:0.08])
xlim([0,0.08]);
xlabel('Brow error','FontName','Helvetica');
ylabel('Proportion of images','FontName','Helvetica');
grid on
title('Fitting in the wild','FontSize',60,'FontName','Helvetica');

legend('show', 'Location', 'SouthEast');