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

shapes_all = zeros(size(labels,2),size(labels,3), size(labels,1));
labels_all = zeros(size(labels,2),size(labels,3), size(labels,1));

errors_left_eye = zeros(numel(images),1);
errors_right_eye = zeros(numel(images),1);

tic
for i=1:numel(images)

    labels_curr = squeeze(labels(i,:,:));
    
    [ a, R, T, ~, l_params, err, shapeOrtho] = fit_PDM_ortho_proj_to_2D_no_reg(pdm.M, pdm.E, pdm.V, labels_curr);

    shapes_all(:,:,i) = shapeOrtho;
    labels_all(:,:,i) = labels_curr;

    if(mod(i, 100)==0)
        fprintf('%d done\n', i );
    end

    actualShape = squeeze(labels(i,:,:));

    errors_left_eye(i) = compute_error_point_to_line_left_eye(actualShape, shapeOrtho, [0]);
    errors_right_eye(i) = compute_error_point_to_line_right_eye(actualShape, shapeOrtho, [0]);

    if(errors_left_eye(i) > 0.02 || errors_right_eye(i) > 0.02)
        plot(shapeOrtho(:,1), -shapeOrtho(:,2), 'r.'); hold on;
        axis equal;
        plot(labels_curr(:,1), -labels_curr(:,2), 'g.'); hold off;
    end

end
  
save('Errors_PDM_eyes.mat', 'errors_left_eye', 'errors_right_eye');

end
