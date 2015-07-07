function [ error_per_image, err_pp, err_pp_dim ] = compute_error( ground_truth_all, detected_points_all )
%compute_error
%   compute the average point-to-point Euclidean error normalized by the
%   inter-ocular distance (measured as the Euclidean distance between the
%   outer corners of the eyes)
%
%   Inputs:
%          grounth_truth_all, size: num_of_points x 2 x num_of_images
%          detected_points_all, size: num_of_points x 2 x num_of_images
%   Output:
%          error_per_image, size: num_of_images x 1


num_of_images = size(ground_truth_all,3);
num_of_points = size(ground_truth_all,1);

error_per_image = zeros(num_of_images,1);
err_pp = zeros(num_of_images, num_of_points);
err_pp_dim = zeros(num_of_images, num_of_points, 2);

for i =1:num_of_images
    detected_points      = detected_points_all(:,:,i);
    ground_truth_points  = ground_truth_all(:,:,i);
    if(num_of_points == 66 || num_of_points == 68)
        interocular_distance = norm(ground_truth_points(37,:)-ground_truth_points(46,:));
    else
        interocular_distance = norm(ground_truth_points(37-17,:)-ground_truth_points(46-17,:));
    end
    sum=0;
    for j=1:num_of_points
        sum = sum+norm(detected_points(j,:)-ground_truth_points(j,:));
        err_pp(i,j) = norm(detected_points(j,:)-ground_truth_points(j,:));
        err_pp_dim(i,j,1) = detected_points(j,1)-ground_truth_points(j,1);
        err_pp_dim(i,j,2) = detected_points(j,2)-ground_truth_points(j,2);
    end
    error_per_image(i) = sum/(num_of_points*interocular_distance);
    err_pp(i,:) = err_pp(i,:) ./ interocular_distance;
    err_pp_dim(i,:) = err_pp_dim(i,:) ./ interocular_distance;
end

end
