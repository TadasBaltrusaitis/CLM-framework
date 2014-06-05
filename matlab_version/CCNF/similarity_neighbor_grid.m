function [ SimilarityMatrix ] = similarity_neighbor_grid( x, side, types)
%SIMILARITYNEIGHBOR Summary of this function goes here
%   Detailed explanation goes here

    % this assumes that the patch is laid out with first column, then second
    % column, ... final column (column major)

    SimilarityMatrix = eye(side*side);

    % types - 1 - horizontal, 2 - vertical, 3 - diagonal (bl-tr), 4 -
    % diagonal (br - tl)
    for t=1:numel(types)

        if(types(t) == 1)
            
            % for horizontal we want to link both neighbours 
            % (which are offset from the points by height)
            i = 1:(side*side-side);
            % create the neighboring links for i
            SimilarityMatrix(sub2ind([side^2, side^2], i, i+side)) = 1;
            SimilarityMatrix(sub2ind([side^2, side^2], i+side, i)) = 1;            
                        
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
            i_to_rem = i(mod(i, side) == 0);
            i_both = setdiff(i, i_to_rem);
            % create the neighboring links for i
            SimilarityMatrix(sub2ind([side^2, side^2], i_both+1, i_both)) = 1;
            SimilarityMatrix(sub2ind([side^2, side^2], i_both, i_both+1)) = 1;            
                        
        end        
        if(types(t) == 3)
            
            % for diagonal to top right, and bottom left don't use right most column
            i = 1:(side^2)-side;
            i_to_rem = i(mod(i-1, side) == 0);
            i_both = setdiff(i, i_to_rem);
            % create the neighboring links for i
            SimilarityMatrix(sub2ind([side^2, side^2], i_both+side-1, i_both)) = 1;
            SimilarityMatrix(sub2ind([side^2, side^2], i_both, i_both+side-1)) = 1;                                    
        end
        if(types(t) == 4)
            
            % for diagonal to top left, and bottom right don't use right most column
            i = 1:(side^2)-side;
            i_to_rem = i(mod(i, side) == 0);
            i_both = setdiff(i, i_to_rem);
            % create the neighboring links for i
            SimilarityMatrix(sub2ind([side^2, side^2], i_both+side+1, i_both)) = 1;
            SimilarityMatrix(sub2ind([side^2, side^2], i_both, i_both+side+1)) = 1;                   
                        
        end
        
    end        
    assert(isequal(SimilarityMatrix, SimilarityMatrix'));
end