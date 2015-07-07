function [ error_per_image ] = compute_error_JANUS( ground_truth_all, detected_points_all )
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

detected_points_all = 0.5 * (detected_points_all([43,37,34],:,:) + detected_points_all([46,40,34],:,:));

num_of_images = size(ground_truth_all,3);
error_per_image = zeros(num_of_images,1);

for i =1:num_of_images
    detected_points      = detected_points_all(:,:,i);
    ground_truth_points  = ground_truth_all(:,:,i);
    
    sum=0;
    if ground_truth_points(1,1)==0
        interocular_distance = norm(ground_truth_points(2,:)-ground_truth_points(3,:));
        sum = sum+norm(detected_points(2,:)-ground_truth_points(2,:));
        sum = sum+norm(detected_points(3,:)-ground_truth_points(3,:));
        error_per_image(i) = sum/(2*interocular_distance);
    elseif ground_truth_points(2,1)==0
        interocular_distance = norm(ground_truth_points(1,:)-ground_truth_points(3,:));
        sum = sum+norm(detected_points(1,:)-ground_truth_points(1,:));
        sum = sum+norm(detected_points(3,:)-ground_truth_points(3,:));
        error_per_image(i) = sum/(2*interocular_distance);
    else
        interocular_distance = norm(ground_truth_points(1,:)-ground_truth_points(2,:));
        %interocular_distance = norm(0.5 * (ground_truth_points(1,:) + ground_truth_points(2,:)) -ground_truth_points(3,:));
        for j=1:3
            sum = sum+norm(detected_points(j,:)-ground_truth_points(j,:));
        end
        error_per_image(i) = sum/(3*interocular_distance);
    end
end

end
