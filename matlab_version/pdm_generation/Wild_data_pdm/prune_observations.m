function [ observations ] = prune_observations( observations, percentage_to_keep )
%PRUNE_OBSERVATIONS Summary of this function goes here
%   Detailed explanation goes here

    distances = pdist(observations, @euclid_dist);
 
    distances = squareform(distances);
    
    m = max(distances(:));
    
    distances(logical(eye(size(distances)))) = m;

    to_rem = false(size(observations,1),1);
    
    % need to get rid of the smallest distances
    for i=size(observations,1):-1:round(percentage_to_keep * size(observations,1))
       
        [~, ind] = min(distances(:));
        [row, col] = ind2sub(size(distances),ind);
        
        % always remove the row?
        
        to_rem(row) = true;

        distances(row,:) = m;
        distances(:,row) = m;
    end

    observations = observations(~to_rem,:);
    
end

function [dist] = euclid_dist(XI, XJ)

    x_dist = bsxfun(@plus, XJ(:, 1:end/3), -XI(1:end/3)).^2;
    y_dist = bsxfun(@plus, XJ(:, end/3+1:2*end/3), -XI(end/3+1:2*end/3)).^2;
    z_dist = bsxfun(@plus, XJ(:, 2*end/3+1:end), - XI(2*end/3+1:end)).^2;
    
    dist = mean(sqrt(x_dist + y_dist + z_dist),2);

end