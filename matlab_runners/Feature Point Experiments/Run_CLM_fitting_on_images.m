function [ err_outline, err_no_outline ] = Run_CLM_fitting_on_images(output_loc, database_root, varargin)
%RUN_CLM_FITTING_ON_IMAGES Summary of this function goes here
%   Detailed explanation goes here

dataset_dirs = {};

if(any(strcmp(varargin, 'use_afw')))
    afw_loc = [database_root '/AFW/'];
    dataset_dirs = cat(1, dataset_dirs, afw_loc); 
end
if(any(strcmp(varargin, 'use_lfpw')))
    lfpw_loc = [database_root 'lfpw/testset/'];
    dataset_dirs = cat(1, dataset_dirs, lfpw_loc); 
end
if(any(strcmp(varargin, 'use_ibug')))
    ibug_loc = [database_root 'ibug/'];
    dataset_dirs = cat(1, dataset_dirs, ibug_loc); 
end
if(any(strcmp(varargin, 'use_helen')))
    helen_loc = [database_root '/helen/testset/'];
    dataset_dirs = cat(1, dataset_dirs, helen_loc); 
end

if(any(strcmp(varargin, 'reg_factors')))
    regFactors = varargin{find(strcmp(varargin, 'reg_factors')) + 1};
else
    % the default reg factor to use
    regFactors = [25];
end               
    
if(any(strcmp(varargin, 'mean_shift_band')))
    meanShiftBand = varargin{find(strcmp(varargin, 'mean_shift_band')) + 1};
else
    % the default mean shift band factor to use
    meanShiftBand = [1.5];
end

if(any(strcmp(varargin, 'verbose')))
    verbose = true;
else
    verbose = false;
end

if(any(strcmp(varargin, 'tikhonov_factors')))
    tikhonov_factors = varargin{find(strcmp(varargin, 'tikhonov_factors')) + 1};
else
    % the default mean shift band factor to use
    if(any(strcmp(varargin, 'use_depth')))
        tikhonov_factors = 0;
    else
        tikhonov_factors = 5;
    end
end
      
command = '"../../x64/Release/FaceLandmarkImg.exe" ';

if(any(strcmp(varargin, 'model')))
    model = varargin{find(strcmp(varargin, 'model')) + 1};
else
    % the default model is the 68 point in the wild one
    model = '"model/main_wild.txt"';
end

if(any(strcmp(varargin, 'multi_view')))
    multi_view = varargin{find(strcmp(varargin, 'multi_view')) + 1};
else
    multi_view = 0;
end

command = cat(2, command, [' -mloc ' model ' ']);
command = cat(2, command, [' -multi_view ' num2str(multi_view) ' ']);

sigma = ['-clm_sigma ', num2str(meanShiftBand), ' '];
command = cat(2, command, sigma);

reg_f = ['-reg ', num2str(regFactors), ' '];
command = cat(2, command, reg_f);

tikh_w = ['-w_reg ', num2str(tikhonov_factors), ' '];
command = cat(2, command, tikh_w);
   
tic
for i=1:numel(dataset_dirs)

    input_loc = ['-fdir "', dataset_dirs{i}, '" '];
    command_c = cat(2, command, input_loc);
        
    out_loc = ['-ofdir "', output_loc, '" '];
    command_c = cat(2, command_c, out_loc);
 
    if(verbose)
        out_im_loc = ['-oidir "', output_loc, '" '];
        command_c = cat(2, command_c, out_im_loc);
    end
    
    command_c = cat(2, command_c, ' -wild ');
    
    dos(command_c);

end
toc

%%

% Extract the error sizes
dirs = {[database_root '/AFW/'];
    [database_root '/ibug/'];
    [database_root '/helen/testset/'];
    [database_root 'lfpw/testset/'];};

landmark_dets = dir([output_loc '/*.pts']);

landmark_det_dir = [output_loc '/'];

num_imgs = size(landmark_dets,1);

labels = zeros(68,2,num_imgs);

shapes = zeros(68,2,num_imgs);

curr = 0;

for i=1:numel(dirs)
    
    
    gt_labels = dir([dirs{i}, '*.pts']);
    
    for g=1:numel(gt_labels)
        curr = curr+1;
        
        gt_landmarks = dlmread([dirs{i}, gt_labels(g).name], ' ', 'A4..B71');
       
        % find the corresponding detection       
        landmark_det = dlmread([landmark_det_dir, gt_labels(g).name], ' ', 'A4..B71');
        
        labels(:,:,curr) = gt_landmarks;
            
        if(size(landmark_det,1) == 66)
            inds_66 = [[1:60],[62:64],[66:68]];
            shapes(inds_66,:,curr) = landmark_det;
        else
            shapes(:,:,curr) = landmark_det;
        end
    end
    
end
         
% Convert to correct format, so as to have same feature points in ground
% truth and detections
if(size(shapes,2) == 66 && size(labels,2) == 68)
    inds_66 = [[1:60],[62:64],[66:68]];
    
    labels = labels(inds_66,:,:);
    shapes = shapes(inds_66,:,:);
end

% Center the pixel
labels = labels - 0.5;

err_outline = compute_error(labels, shapes);

labels_no_out = labels(18:end,:,:);
shapes_no_out = shapes(18:end,:,:);

err_no_outline = compute_error(labels_no_out, shapes_no_out);

%%

save([output_loc, 'res.mat'], 'labels', 'shapes', 'err_outline', 'err_no_outline');
    
end