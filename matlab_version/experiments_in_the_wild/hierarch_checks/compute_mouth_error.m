function [ error_per_image ] = compute_mouth_error( ground_truth_all, detected_points_all, occluded )
%compute_error
%   compute the average point-to-point Euclidean error of right eye normalized by the
%   inter-ocular distance (measured as the Euclidean distance between the
%   outer corners of the eyes)
%
%   Inputs:
%          grounth_truth_all, size: num_of_points x 2 x num_of_images
%          detected_points_all, size: num_of_points x 2 x num_of_images
%   Output:
%          error_per_image, size: num_of_images x 1

mouth_inds_from_68 = 49:68;
mouth_inds_from_49 = 30:49;

mouth_inds_from_66 = 49:66;

mouth_inds_from_20 = [1:12, 14:16, 18:20];

num_of_images = size(ground_truth_all,3);

num_points_gt = size(ground_truth_all,1);

num_points_det = size(detected_points_all,1);

error_per_image = zeros(num_of_images,1);

for i =1:num_of_images
    
    if(num_points_det == 20)
        detected_points = detected_points_all(:,:,i);
    elseif(num_points_det == 68)
        detected_points = detected_points_all(mouth_inds_from_68,:,i);           
    elseif(num_points_det == 66)
        detected_points = detected_points_all(mouth_inds_from_66,:,i);           
    elseif(num_points_det == 49)
        detected_points = detected_points_all(mouth_inds_from_49,:,i);        
    end
    
    ground_truth_points  = ground_truth_all(:,:,i);
    
    if(num_points_gt == 66 || num_points_gt == 68)
        interocular_distance = norm(ground_truth_points(37,:)-ground_truth_points(46,:));
        ground_truth_points = ground_truth_points(mouth_inds_from_68,:,:);
    else
        interocular_distance = norm(ground_truth_points(37-17,:)-ground_truth_points(46-17,:));
        ground_truth_points = ground_truth_points(mouth_inds_from_68,:,:);
    end
    
    if(size(detected_points,1) == 18 && size(ground_truth_points,1) == 20)
        ground_truth_points = ground_truth_points(mouth_inds_from_20,:);
    end
    sum=0;
    
    for j=1:size(detected_points,1)
        sum = sum+norm(detected_points(j,:)-ground_truth_points(j,:));
    end
    
    error_per_image(i) = sum/(size(detected_points,1)*interocular_distance);
end

if(nargin > 2)
    error_per_image = error_per_image(~occluded);
end

end
