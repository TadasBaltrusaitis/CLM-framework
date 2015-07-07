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

load('results/results_janus_clnf_better_ROI_mv_3.mat');
shapes = experiments.shapes;

widths = detections(:,3) * 0.7;
widths(widths> 170) = 170;
[bin_sizes, edges, bin] = histcounts(widths,30);

clnf_error = compute_error_JANUS( experiments.labels,  shapes);

error_per_bin = zeros(numel(bin_sizes)-1,1);
xs = zeros(numel(bin_sizes)-1,1);
for b=1:numel(bin_sizes)
%     error_per_bin(b) = sum(clnf_error(bin==b) < 0.2) / sum(bin==b);
    error_per_bin(b) = median(clnf_error(bin==b));
    xs(b) = (edges(b) + edges(b+1)) / 2;
end

% [error_x, error_y] = cummErrorCurve(clnf_error);

plot(xs(2:end), smooth(error_per_bin(2:end)), 'Color', [0,0.447,0.741], 'LineWidth',line_width);
% plot(xs(2:end), error_per_bin(2:end), 'Color', [0,0.447,0.741], 'LineWidth',line_width);
set(gca,'xtick',15:15:150)
xlim([xs(2),xs(end)]);
xlabel('Face width in pixels','FontName','Helvetica');
ylabel('Median error','FontName','Helvetica');
grid on
title('Error rates vs face size','FontSize',40,'FontName','Helvetica');

% legend('show', 'Location', 'SouthEast');

print -dpdf results/JANUS_data_res_size.pdf