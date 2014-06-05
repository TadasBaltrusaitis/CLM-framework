function [ R, T ] = AlignShapesKabsch ( alignFrom, alignTo )
%ALIGN3DSHAPES Summary of this function goes here
%   Detailed explanation goes here

    dims = size(alignFrom, 2);

    alignFromMean = alignFrom - repmat(mean(alignFrom), size(alignFrom,1),1);
    alignToMean = alignTo - repmat(mean(alignTo), size(alignTo,1),1);

    [U, ~, V] = svd( alignFromMean' * alignToMean);

    % make sure no reflection is there
    d = sign(det(V*U'));
    corr = eye(dims);
    corr(end,end) = d;
    
    R = V*corr*U';

    T = mean(alignTo) - (R * mean(alignFrom)')';
    T = T';
end

