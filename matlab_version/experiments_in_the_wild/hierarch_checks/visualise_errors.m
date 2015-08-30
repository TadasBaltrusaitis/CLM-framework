% Visualising the errors

%% First draw the initialisation errors
if(exist([getenv('USERPROFILE') '/Dropbox/AAM/test data/'], 'file'))
    root_test_data = [getenv('USERPROFILE') '/Dropbox/AAM/test data/'];    
else
    root_test_data = 'F:/Dropbox/Dropbox/AAM/test data/';
end

[images, detections, labels] = Collect_wild_imgs(root_test_data);

pdmLoc = ['../models/pdm/pdm_68_aligned_wild.mat'];
labels = labels - 0.5;
num_points = size(labels,2);

load(pdmLoc);
pdm = struct; pdm.M = double(M); pdm.E = double(E); pdm.V = double(V);

shapes_init_all = zeros(num_points, 2, size(detections,1));
errors_init_all = zeros(size(detections,1), 1);
views_used = zeros(size(detections,1), 1);
labels_all = zeros(num_points, 2, size(detections,1));

for i=1:size(detections,1)
    labels_all(:,:,i) = labels(i,:,:);
    % Find the best orientation
    views = [0,0,0; 0,-20,0; 0,20,0; 30,0,0];
    views = views * pi/180;    
        
    shapes_init = zeros(num_points, 2, size(views,1));
    error_init = zeros(size(views,1),1);
    
    for v = 1:size(views,1)                                                                                 

        bbox = detections(i,:);  
        bbox(2) = bbox(2) + 0.025 * (bbox(4) - bbox(2));
        rot = Euler2Rot(views(v,:));  
        rot_m = rot * reshape(M, num_points, 3)';
        width_model = max(rot_m(1,:)) - min(rot_m(1,:));
        height_model = max(rot_m(2,:)) - min(rot_m(2,:));

        a = (((bbox(3) - bbox(1)) / width_model) + ((bbox(4) - bbox(2))/ height_model)) / 2;

        tx = (bbox(3) + bbox(1))/2;
        ty = (bbox(4) + bbox(2))/2;

        % correct it so that the bounding box is just around the minimum
        % and maximum point in the initialised face
        tx = tx - a*(min(rot_m(1,:)) + max(rot_m(1,:)))/2;
        ty = ty - a*(min(rot_m(2,:)) + max(rot_m(2,:)))/2;

        % visualisation of the initial state
        %hold off;imshow(Image);hold on;plot(a*rot_m(1,:)+tx, a*rot_m(2,:)+ty,'.r');hold on;rectangle('Position', [bounding_box(1), bounding_box(2), bounding_box(3)-bounding_box(1), bounding_box(4)-bounding_box(2)]);
        global_params = [a, 0, 0, 0, tx, ty]';
        global_params(2:4) = views(v,:);
        local_params = zeros(size(E));
        [shape2D] = GetShapeOrtho(M, V, local_params, global_params);
        shapes_init(:,:,v) = shape2D(:,1:2);        
        error_init(v) = compute_error(squeeze(labels(i,:,:)), shapes_init(:,:,v));
    end
    
    [~,ind] = min(error_init);
    views_used(i) = ind;
    shapes_init_all(:,:,i) = shapes_init(:,:,ind);
    errors_init_all(i) = error_init(ind);
end
[~,~,errs_pp] = compute_error(labels_all, shapes_init_all);
views_used(errors_init_all > 0.5) = -1;

%%
load('results/results_wild_clnf_general_hierarch.mat');
[~,~,errs_pp] = compute_error(labels_all, experiments.shapes);
% for v=1:size(views,1)
% % Want to scatter them around mean of each landmark
%     figure;
%     set(gca,'YDir','reverse');
%     rot = Euler2Rot(views(v,:));  
%     rot_m = rot * reshape(M, num_points, 3)';
% 
%     plot(rot_m(1,:), +rot_m(2,:), '.r');
%     hold on;
%     for p=1:num_points
%         errors_x = squeeze(errs_pp(views_used==v,p,1)) * 30;
%         errors_y = squeeze(errs_pp(views_used==v,p,2)) * 30;
% %         scatter(bsxfun(@plus, rot_m(1,p), errors_x), bsxfun(@plus, -rot_m(2,p), -errors_y));
%         sig = cov(errors_x, errors_y);
%         plotcov2([rot_m(1,p) + mean(errors_x); rot_m(2,p) + mean(errors_y)], sig, 'conf', 0.5);
%     end
%    
% %     plot(rot_m(1,:) + errors(:,1)'*65, -rot_m(2,:) - errors(:,2)'*65, '.b');
%     axis equal;
%     hold off;
% end