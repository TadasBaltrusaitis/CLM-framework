function [ outputTexture ] = Remap( inputTexture, mapX, mapY )
%REMAP Summary of this function goes here
%   Detailed explanation goes here

    outputTexture = zeros(size(mapX));

    [X,Y] = meshgrid(0:size(inputTexture,2)-1,0:size(inputTexture,1)-1);
        
    inds = find(mapX ~= -1);
    
    xSources = mapX(inds);
    ySources = mapY(inds);    
    
    Z = interp2(X, Y, double(inputTexture), xSources, ySources);
    outputTexture(inds) = Z;
end

