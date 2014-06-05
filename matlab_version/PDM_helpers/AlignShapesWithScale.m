function [ A, T, error, alignedShape ] = AlignShapesWithScale( alignFrom, alignTo )
%ALIGNSHAPESWITHSCALE Summary of this function goes here
%   Detailed explanation goes here

    numPoints = size(alignFrom,1);

    meanFrom = mean(alignFrom);    
    meanTo = mean(alignTo);
    
    alignFromMeanNormed = bsxfun(@minus, alignFrom, meanFrom);
    alignToMeanNormed = bsxfun(@minus, alignTo, meanTo);

    % scale now
    sFrom = sqrt(sum(alignFromMeanNormed(:).^2)/numPoints);
    sTo = sqrt(sum(alignToMeanNormed(:).^2)/numPoints);
    
    s = sTo / sFrom;
    
    alignFromMeanNormed = alignFromMeanNormed/sFrom;
    alignToMeanNormed = alignToMeanNormed/sTo;
    
    [R, t] = AlignShapesKabsch(alignFromMeanNormed, alignToMeanNormed);
    
    A = s * R;
    aligned = (A * alignFrom')';
    T = mean(alignTo - aligned);
    alignedShape = bsxfun(@plus, aligned, T);
    error = sqrt(mean(sum((alignedShape - alignTo).^2,2)));
    
end