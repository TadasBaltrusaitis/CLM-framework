clear

%% 
inds = 1:337;
scrsz = get(0,'ScreenSize');
figure1 = figure('Position',[20 50 3*scrsz(3)/4 0.9*scrsz(4)]);

set(figure1,'Units','Inches');
pos = get(figure1,'Position');
set(figure1,'PaperPositionMode','Auto','PaperUnits','Inches','PaperSize',[pos(3), pos(4)])

% Create axes
axes1 = axes('Parent',figure1,'FontSize',40,'FontName','Helvetica');

line_width = 6;
hold on;

load('results/results_wild_clnf.mat');
labels = experiments.labels([1:60,62:64,66:end],:,:);
shapes = experiments.shapes([1:60,62:64,66:end],:,:);
labels = labels(18:end,:,:);
shapes = shapes(18:end,:,:);

clnf_error = compute_error( labels(:,:,inds),  shapes(:,:,inds));

[error_x, error_y] = cummErrorCurve(clnf_error);
hold on;

plot(error_x, error_y, 'r','DisplayName', 'CLM + CCNF', 'LineWidth',line_width);

load('results/intraface_wild_resize.mat');
labels_all = labels_all(18:end,:,:);
shapes_all = shapes_all(18:end,:,:);

intraface_wild_error = compute_error(labels_all(:,:,inds), shapes_all(:,:,inds));

[error_x, error_y] = cummErrorCurve(intraface_wild_error);

plot(error_x, error_y, '.-g','DisplayName', 'SDM [32]', 'LineWidth',line_width);

load('results/zhu_wild.mat');
labels_all = labels_all(18:end,:,:);
shapes_all = shapes_all(18:end,:,:);

zhu_wild_error = compute_error(labels_all(:,:,inds), shapes_all(:,:,inds));

[error_x, error_y] = cummErrorCurve(zhu_wild_error);

plot(error_x, error_y, '.-c','DisplayName', 'Tree based (p204) [28]', 'LineWidth',line_width);

load('results/results_wild_clm.mat');
labels = experiments.labels([1:60,62:64,66:end],:,:);
shapes = experiments.shapes([1:60,62:64,66:end],:,:);
labels = labels(18:end,:,:);
shapes = shapes(18:end,:,:);

clm_error = compute_error( labels(:,:,inds),  shapes(:,:,inds));

[error_x, error_y] = cummErrorCurve(clm_error);

plot(error_x, error_y, '--b','DisplayName', 'CLM + SVR', 'LineWidth',line_width);

load('results/drmf_wild.mat');
labels_all = labels_all(18:end,:,:);
shapes_all = shapes_all(18:end,:,:);

drmf_error = compute_error(labels_all(:,:,inds), shapes_all(:,:,inds));

[error_x, error_y] = cummErrorCurve(drmf_error);

plot(error_x, error_y, '-.k','DisplayName', 'DRMF [27]', 'LineWidth',line_width);

set(gca,'xtick',[0:0.05:0.15])
xlim([0,0.15]);
xlabel('Size normalised shape RMS error','FontName','Helvetica');
ylabel('Proportion of images','FontName','Helvetica');
grid on
% title('Fitting in the wild without outline','FontSize',60,'FontName','Helvetica');

legend('show', 'Location', 'SouthEast');

print -dpdf results/in-the-wild-clnf-no-outline_afw.pdf

%% 
inds = 562:696;

scrsz = get(0,'ScreenSize');
figure1 = figure('Position',[20 50 3*scrsz(3)/4 0.9*scrsz(4)]);

set(figure1,'Units','Inches');
pos = get(figure1,'Position');
set(figure1,'PaperPositionMode','Auto','PaperUnits','Inches','PaperSize',[pos(3), pos(4)])

% Create axes
axes1 = axes('Parent',figure1,'FontSize',40,'FontName','Helvetica');

line_width = 6;
hold on;

load('results/results_wild_clnf.mat');
labels = experiments.labels([1:60,62:64,66:end],:,:);
shapes = experiments.shapes([1:60,62:64,66:end],:,:);
labels = labels(18:end,:,:);
shapes = shapes(18:end,:,:);

clnf_error = compute_error( labels(:,:,inds),  shapes(:,:,inds));

[error_x, error_y] = cummErrorCurve(clnf_error);
hold on;

plot(error_x, error_y, 'r','DisplayName', 'CLM + CCNF', 'LineWidth',line_width);

load('results/intraface_wild_resize.mat');
labels_all = labels_all(18:end,:,:);
shapes_all = shapes_all(18:end,:,:);

intraface_wild_error = compute_error(labels_all(:,:,inds), shapes_all(:,:,inds));

[error_x, error_y] = cummErrorCurve(intraface_wild_error);

plot(error_x, error_y, '.-g','DisplayName', 'SDM [32]', 'LineWidth',line_width);

load('results/zhu_wild.mat');
labels_all = labels_all(18:end,:,:);
shapes_all = shapes_all(18:end,:,:);

zhu_wild_error = compute_error(labels_all(:,:,inds), shapes_all(:,:,inds));

[error_x, error_y] = cummErrorCurve(zhu_wild_error);

plot(error_x, error_y, '.-c','DisplayName', 'Tree based (p204) [28]', 'LineWidth',line_width);

load('results/results_wild_clm.mat');
labels = experiments.labels([1:60,62:64,66:end],:,:);
shapes = experiments.shapes([1:60,62:64,66:end],:,:);
labels = labels(18:end,:,:);
shapes = shapes(18:end,:,:);

clm_error = compute_error( labels(:,:,inds),  shapes(:,:,inds));

[error_x, error_y] = cummErrorCurve(clm_error);

plot(error_x, error_y, '--b','DisplayName', 'CLM + SVR', 'LineWidth',line_width);

load('results/drmf_wild.mat');
labels_all = labels_all(18:end,:,:);
shapes_all = shapes_all(18:end,:,:);

drmf_error = compute_error(labels_all(:,:,inds), shapes_all(:,:,inds));

[error_x, error_y] = cummErrorCurve(drmf_error);

plot(error_x, error_y, '-.k','DisplayName', 'DRMF [27]', 'LineWidth',line_width);

set(gca,'xtick',[0:0.05:0.15])
xlim([0,0.15]);
xlabel('Size normalised shape RMS error','FontName','Helvetica');
ylabel('Proportion of images','FontName','Helvetica');
grid on
% title('Fitting in the wild without outline','FontSize',60,'FontName','Helvetica');

legend('show', 'Location', 'SouthEast');

print -dpdf results/in-the-wild-clnf-no-outline_ibug.pdf

%% 
inds = [338:561,697:1026];

scrsz = get(0,'ScreenSize');
figure1 = figure('Position',[20 50 3*scrsz(3)/4 0.9*scrsz(4)]);

set(figure1,'Units','Inches');
pos = get(figure1,'Position');
set(figure1,'PaperPositionMode','Auto','PaperUnits','Inches','PaperSize',[pos(3), pos(4)])

% Create axes
axes1 = axes('Parent',figure1,'FontSize',40,'FontName','Helvetica');

line_width = 6;
hold on;

load('results/results_wild_clnf.mat');
labels = experiments.labels([1:60,62:64,66:end],:,:);
shapes = experiments.shapes([1:60,62:64,66:end],:,:);
labels = labels(18:end,:,:);
shapes = shapes(18:end,:,:);

clnf_error = compute_error( labels(:,:,inds),  shapes(:,:,inds));

[error_x, error_y] = cummErrorCurve(clnf_error);
hold on;

plot(error_x, error_y, 'r','DisplayName', 'CLM + CCNF', 'LineWidth',line_width);

load('results/intraface_wild_resize.mat');
labels_all = labels_all(18:end,:,:);
shapes_all = shapes_all(18:end,:,:);

intraface_wild_error = compute_error(labels_all(:,:,inds), shapes_all(:,:,inds));

[error_x, error_y] = cummErrorCurve(intraface_wild_error);

plot(error_x, error_y, '.-g','DisplayName', 'SDM [32]', 'LineWidth',line_width);

load('results/zhu_wild.mat');
labels_all = labels_all(18:end,:,:);
shapes_all = shapes_all(18:end,:,:);

zhu_wild_error = compute_error(labels_all(:,:,inds), shapes_all(:,:,inds));

[error_x, error_y] = cummErrorCurve(zhu_wild_error);

plot(error_x, error_y, '.-c','DisplayName', 'Tree based (p204) [28]', 'LineWidth',line_width);

load('results/results_wild_clm.mat');
labels = experiments.labels([1:60,62:64,66:end],:,:);
shapes = experiments.shapes([1:60,62:64,66:end],:,:);
labels = labels(18:end,:,:);
shapes = shapes(18:end,:,:);

clm_error = compute_error( labels(:,:,inds),  shapes(:,:,inds));

[error_x, error_y] = cummErrorCurve(clm_error);

plot(error_x, error_y, '--b','DisplayName', 'CLM + SVR', 'LineWidth',line_width);

load('results/drmf_wild.mat');
labels_all = labels_all(18:end,:,:);
shapes_all = shapes_all(18:end,:,:);

drmf_error = compute_error(labels_all(:,:,inds), shapes_all(:,:,inds));

[error_x, error_y] = cummErrorCurve(drmf_error);

plot(error_x, error_y, '-.k','DisplayName', 'DRMF [27]', 'LineWidth',line_width);

set(gca,'xtick',[0:0.05:0.15])
xlim([0,0.15]);
xlabel('Size normalised shape RMS error','FontName','Helvetica');
ylabel('Proportion of images','FontName','Helvetica');
grid on
% title('Fitting in the wild without outline','FontSize',60,'FontName','Helvetica');

legend('show', 'Location', 'SouthEast');

print -dpdf results/in-the-wild-clnf-no-outline_lfpw_helen.pdf
