function [ error_per_image ] = compute_error_point_to_line_left_eye( ground_truth_all, detected_points_all, occluded )
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



right_eye_inds_from_68 = [37,38,39,40,41,42,37];
right_eye_inds_from_28 = [9,11,13,15,17,19];

num_of_images = size(ground_truth_all,3);

num_points_gt = size(ground_truth_all,1);

num_points_det = size(detected_points_all,1);

error_per_image = zeros(num_of_images,1);

for i =1:num_of_images
    
    if(num_points_det == 6)
        detected_points = detected_points_all(:,:,i);
    elseif(num_points_det == 68 || num_points_det == 66)
        detected_points = detected_points_all(right_eye_inds_from_68,:,i);        
    elseif(num_points_det == 28)
        detected_points = detected_points_all(right_eye_inds_from_28,:,i);        
    elseif(num_points_det == 49)
        
    end
    ground_truth_points  = ground_truth_all(:,:,i);
    
    if(num_points_gt == 66 || num_points_gt == 68)
        interocular_distance = norm(ground_truth_points(37,:)-ground_truth_points(46,:));
        ground_truth_points = ground_truth_points(right_eye_inds_from_68,:,:);
    else
        interocular_distance = norm(ground_truth_points(37-17,:)-ground_truth_points(46-17,:));
        ground_truth_points = ground_truth_points(right_eye_inds_from_68,:,:);
    end
    
    sum=0;
    for j=1:6
        
        if(j== 1 || j == 6)
            % eye corners should align perfectly
            sum = sum + norm(detected_points(j,:)-ground_truth_points(j,:));
        else
            % points between eye corners measured in distance to the two appropriate line
            % segments
            sum = sum + point_to_segments(detected_points(j,:), ground_truth_points(j-1:j+1,:));
        end
    end
    error_per_image(i) = sum/(6*interocular_distance);
end

error_per_image = error_per_image(~occluded);

end

function seg_dist = point_to_segments(point, segments)
    
    seg_dists = zeros(size(segments, 1)-1,1);

    for i=1:size(segments, 1)-1
       
        vec1 = point - segments(i,:);
        vec2 = segments(i+1,:) - segments(i,:);
        
        d = (vec1 * vec2') / (norm(vec2)^2);
        
        if(d < 0)
            seg_dists(i) = norm(vec1);
        elseif(d > 1)
            seg_dists(i) = norm(point - segments(i+1,:));
        else
            seg_dists(i) = sqrt( norm(vec1)^2 - norm(d * vec2)^2);
        end
    end
    seg_dist = min(seg_dists);
end