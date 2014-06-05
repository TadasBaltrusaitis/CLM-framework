function [ SparsityMatrix ] = sparsity_grid( x, side, width, width_end)
%SIMILARITYNEIGHBOR Summary of this function goes here
%   Detailed explanation goes here

    % this assumes that the patch is laid out with first column, then second
    % column, ... final column (column major)

    SimilarityMatrix = zeros(side*side);
    for i=1:width
        SimilarityMatrix = (similarity_neighbor_grid_further(x, side, [1,2,3,4], i) | SimilarityMatrix);
    end
    
    SimilarityMatrix_end = zeros(side*side);
    for i=1:width_end
        SimilarityMatrix_end = (similarity_neighbor_grid_further(x, side, [1,2,3,4], i) | SimilarityMatrix_end);
    end
    
    SparsityMatrix = double(SimilarityMatrix_end & (~SimilarityMatrix));
    
    assert(isequal(SparsityMatrix, SparsityMatrix'));
end