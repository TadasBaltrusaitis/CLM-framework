function [ transformedX, transformedY, transformedZ ] = TangentSpaceTransform( x, y, z, meanShape )
%TANGENTSPACETRANSFORM Summary of this function goes here
%   Detailed explanation goes here

    scaling = [ x y z] * [ meanShape(:,1)' meanShape(:,2)' meanShape(:,3)']';
    for i=1:size(x,1)
        x(i,:) = x(i,:) * (1 / scaling(i));
        y(i,:) = y(i,:) * (1 / scaling(i));
        z(i,:) = z(i,:) * (1 / scaling(i));
    end
    
    transformedX = x * mean(scaling);
    transformedY = y * mean(scaling);
    transformedZ = z * mean(scaling);
    
end

