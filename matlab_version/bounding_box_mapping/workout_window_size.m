%% Work out the eye scaling first using the PDM
addpath('../PDM_helpers/');
% also work out a size for each window
% Replace this with the location of in 300 faces in the wild data
if(exist([getenv('USERPROFILE') '/Dropbox/AAM/test data/'], 'file'))
    root_test_data = [getenv('USERPROFILE') '/Dropbox/AAM/test data/'];    
else
    root_test_data = 'F:/Dropbox/Dropbox/AAM/test data/';
end

[~, detections, labels] = Collect_wild_imgs(root_test_data, true, true, true, true);

% load('../models/pdm/pdm_68_multi_pie');
load('../models/pdm/pdm_68_aligned_wild');

%%
shapes = zeros(size(labels));
num_points = 68;

for i=1:size(labels,1)

    [ a_orig, R, ~, ~, ~, ~] = fit_PDM_ortho_proj_to_2D(M, E, V, squeeze(labels(i,:,:)));
    view_actual = Rot2Euler(R);
    
    views = [0,0,0; 0,-30,0; -30,0,0; 0,30,0; 30,0,0];
    views = views * pi/180;   

    [~,view_id] = min(sum(abs(bsxfun(@plus, views, -view_actual)), 2));
    
    rot = Euler2Rot(views(view_id,:));   
    
    rot_m = rot * reshape(M, num_points, 3)';
    width_model = max(rot_m(1,:)) - min(rot_m(1,:));
    height_model = max(rot_m(2,:)) - min(rot_m(2,:));

    bounding_box = detections(i,:);

    a = (((bounding_box(3) - bounding_box(1)) / width_model) + ((bounding_box(4) - bounding_box(2))/ height_model)) / 2;

    tx = (bounding_box(3) + bounding_box(1))/2;
    ty = (bounding_box(4) + bounding_box(2))/2;

    % correct it so that the bounding box is just around the minimum
    % and maximum point in the initialised face
    tx = tx - a*(min(rot_m(1,:)) + max(rot_m(1,:)))/2;
    ty = ty - a*(min(rot_m(2,:)) + max(rot_m(2,:)))/2;

    % visualisation of the initial state
    %hold off;imshow(Image);hold on;plot(a*rot_m(1,:)+tx, a*rot_m(2,:)+ty,'.r');hold on;rectangle('Position', [bounding_box(1), bounding_box(2), bounding_box(3)-bounding_box(1), bounding_box(4)-bounding_box(2)]);
    global_params = [a, 0, 0, 0, tx, ty]';
    global_params(2:4) = views(view_id);

    local_params = zeros(numel(E), 1);

    shape = GetShapeOrtho(M, V, local_params, global_params);
    shape = shape(:,1:2);
    shapes(i,:,:) = shape;

    shapes(i,:,:) = shapes(i,:,:) / a_orig;
    labels(i,:,:) = labels(i,:,:) / a_orig;
    
end

%%
errs_img = sort(squeeze(mean(mean(abs(labels - shapes), 2), 3)));

errs_lmk = squeeze(mean(mean(abs(labels - shapes), 1), 3));

errs_img_95 = errs_img(round(0.95*end));

scales = [0.25, 0.35, 0.5];

windows = floor(errs_img_95 * 2 * scales + 11) + 1