function Script_PDM_eyes()

addpath('../PDM_helpers/');
addpath('../fitting/normxcorr2_mex_ALL');
addpath('../fitting/');
addpath('../CCNF/');
addpath('../models/');

% Replace this with the location of in 300 faces in the wild data
if(exist([getenv('USERPROFILE') '/Dropbox/AAM/test data/'], 'file'))
    root_test_data = [getenv('USERPROFILE') '/Dropbox/AAM/test data/'];    
else
    root_test_data = 'F:/Dropbox/Dropbox/AAM/test data/';
end

[images, detections, labels] = Collect_wild_imgs(root_test_data);

%% Fitting the model to the provided image

% the default PDM to use
pdmLoc = ['../models/pdm/pdm_68_aligned_wild_eyes.mat'];

load(pdmLoc);

pdm = struct;
pdm.M = double(M);
pdm.E = double(E);
pdm.V = double(V);

num_points = numel(M)/3;

errors = zeros(numel(images),1);
shapes_all = zeros(size(labels,2),size(labels,3), size(labels,1));
labels_all = zeros(size(labels,2),size(labels,3), size(labels,1));
errors_normed = zeros(numel(images),1);

errors_left_eye = zeros(numel(images),1);
errors_right_eye = zeros(numel(images),1);

tic
for i=1:numel(images)

    image = imread(images(i).img);
    image_orig = image;
    
    if(size(image,3) == 3)
        image = rgb2gray(image);
    end              
    
    labels_curr = squeeze(labels(i,:,:));
    
    [ a, R, T, ~, l_params, err, shapeOrtho] = fit_PDM_ortho_proj_to_2D(pdm.M, pdm.E, pdm.V, labels_curr);

    shape = shapeOrtho;
    shapes_all(:,:,i) = shapeOrtho;
    labels_all(:,:,i) = labels_curr;

    if(mod(i, 200)==0)
        fprintf('%d done\n', i );
    end

    valid_points =  sum(squeeze(labels(i,:,:)),2) > 0;
    valid_points(1:17) = 0;

    actualShape = squeeze(labels(i,:,:));
    errors(i) = sqrt(mean(sum((actualShape(valid_points,:) - shape(valid_points,:)).^2,2)));      
    width = ((max(actualShape(valid_points,1)) - min(actualShape(valid_points,1)))+(max(actualShape(valid_points,2)) - min(actualShape(valid_points,2))))/2;
    errors_normed(i) = errors(i)/width;     

    errors_left_eye(i) = compute_error_point_to_line_left_eye(actualShape, shapeOrtho, [0]);
    errors_right_eye(i) = compute_error_point_to_line_right_eye(actualShape, shapeOrtho, [0]);

    if(errors_normed(i) > 0.035 || errors_left_eye(i) > 0.035 || errors_right_eye(i) > 0.035)
        imshow(image);hold on; plot(shape(:,1), shape(:,2), '.g'); hold off;
    end
end
  
save('Errors_PDM_eyes.mat', 'errors_normed', 'errors_left_eye', 'errors_right_eye');

end
