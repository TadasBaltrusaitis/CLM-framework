function [ error_per_image ] = compute_error( ground_truth_all, detected_points_all, bboxes )
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


num_of_images = size(ground_truth_all,1);
num_of_points = size(ground_truth_all,2);

error_per_image = zeros(num_of_images,1);

for i =1:num_of_images
    detected_points      = detected_points_all(:,:,i);
    ground_truth_points  = squeeze(ground_truth_all(i,:,:));
    normalisation = (bboxes(i, 3) + bboxes(i, 4))/2;
    
    sum=0;
    points_used = 0;
    for j=1:num_of_points
        if(ground_truth_points(j,:) ~= 0)
            sum = sum+norm(detected_points(j,:)-ground_truth_points(j,:));
            points_used = points_used + 1;
        end
    end
    error_per_image(i) = sum/(points_used*normalisation);
end

end
