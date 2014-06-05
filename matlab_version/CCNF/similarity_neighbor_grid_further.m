function [ SimilarityMatrix ] = similarity_neighbor_grid_further( x, side, types, dist)
%SIMILARITYNEIGHBOR Summary of this function goes here
%   Detailed explanation goes here

    % this assumes that the patch is laid out with first column, then second
    % column, ... final column (column major)

%     dist = 2;
    SimilarityMatrix = eye(side*side);

    % types - 1 - horizontal, 2 - vertical, 3 - diagonal (bl-tr), 4 -
    % diagonal (br - tl)
    for t=1:numel(types)

        if(types(t) == 1)
            
            % for horizontal we want to link both neighbours 
            % (which are offset from the points by height)
            i = 1:(side*side-side*dist);
            % create the neighboring links for i
            SimilarityMatrix(sub2ind([side^2, side^2], i, i+side*dist)) = 1;
            SimilarityMatrix(sub2ind([side^2, side^2], i+side*dist, i)) = 1;            
                        
            % visualise
            % vis = zeros(height, width)
            % [i, j] = ind2sub([sz, sz], find(SimilarityMatrix(:)==1));
            % i_2 = mod(i-1, width)+1;
            % j_2 = floor((j-1)/height)+1;
            % vis(sub2ind([width, height], i_2, j_2) = 1;
            % imagesc(vis);
        end        
        if(types(t) == 2)
            
            % for vertical we want to link both neighbours except at edge
            % cases which are mod(y_loc,side) = 0 as they are at the edges
            i = 1:side*side;
            i_to_rem =[];
            for s=1:dist
                i_to_rem = union(i_to_rem, i(mod(i+s-1, side) == 0));
            end
            i_both = setdiff(i, i_to_rem);
            % create the neighboring links for i
            SimilarityMatrix(sub2ind([side^2, side^2], i_both+dist, i_both)) = 1;
            SimilarityMatrix(sub2ind([side^2, side^2], i_both, i_both+dist)) = 1;
                        
        end        
        if(types(t) == 3)
            
            % for diagonal to top right, and bottom left don't use right most column
            i = 1:(side^2)-dist * side;
            i_to_rem = [];
            for s=1:dist
                i_to_rem = union(i_to_rem,i(mod(i-s, side) == 0));
            end
            i_both = setdiff(i, i_to_rem);
            % create the neighboring links for i
            SimilarityMatrix(sub2ind([side^2, side^2], i_both+dist*side-dist, i_both)) = 1;
            SimilarityMatrix(sub2ind([side^2, side^2], i_both, i_both+dist*side-dist)) = 1;                                    
        end
        if(types(t) == 4)
            
            % for diagonal to top left, and bottom right don't use right most column
            i = 1:(side^2)-dist*side;
            i_to_rem = [];
            for s=1:dist
                i_to_rem = union(i_to_rem, i(mod(i+s-1, side) == 0));
            end
            i_both = setdiff(i, i_to_rem);
            % create the neighboring links for i
            SimilarityMatrix(sub2ind([side^2, side^2], i_both+dist*side+dist, i_both)) = 1;
            SimilarityMatrix(sub2ind([side^2, side^2], i_both, i_both+dist*side+ dist)) = 1;                   
                        
        end
        
    end        
    assert(isequal(SimilarityMatrix, SimilarityMatrix'));
end