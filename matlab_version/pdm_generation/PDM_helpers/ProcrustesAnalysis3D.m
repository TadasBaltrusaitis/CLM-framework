function [ normX, normY, normZ, meanShape, Transform ] = ProcrustesAnalysis3D( x, y, z, tangentSpace, meanShape )
%PROCRUSTESANALYSIS3D Summary of this function goes here
%   Detailed explanation goes here

meanProvided = false;

if(nargin > 4)
    meanProvided = true;
end

% Translate all elements to origin
normX = zeros(size(x));
normY = zeros(size(y));
normZ = zeros(size(z));

for i = 1:size(x,1)
    
    offsetX = mean(x(i,:));
    offsetY = mean(y(i,:));
    offsetZ = mean(z(i,:));

    Transform.offsetX(i) = offsetX;
    Transform.offsetY(i) = offsetY;
    Transform.offsetZ(i) = offsetZ;
    
    normX(i,:) = x(i,:) - offsetX;
    normY(i,:) = y(i,:) - offsetY;
    normZ(i,:) = z(i,:) - offsetZ;
    
end

% Rotate elements untill all of them have the same orientation

% the initial estimate of rotation would be the first element
% if change is less than 1% stop (shouldn't take more than 2 steps)
change = 0.1;

if(~meanProvided)
    meanShape = [ mean(normX); mean(normY); mean(normZ) ]';
end
% scale all the shapes to mean shape

% Get the Frobenius norm, to scale the shapes to mean size (still want to
% retain mm)
meanScale = norm(meanShape, 'fro');    
    
for i = 1:size(x,1)
    
    scale = norm([normX(i,:) normY(i,:) normZ(i,:)], 'fro')/meanScale;
    
    normX(i,:) = normX(i,:)/scale;
    normY(i,:) = normY(i,:)/scale;
    normZ(i,:) = normZ(i,:)/scale;
    
end

Transform.RotationX = zeros(size(x,1),1);
Transform.RotationY = zeros(size(x,1),1);
Transform.RotationZ = zeros(size(x,1),1);

for i = 1:30
    
    % align all of the shapes to the mean shape
    
    % remember all orientations to get the mean one (in euler angle form, pitch, yaw roll)
    orientationsX = zeros(size(normX,1),1);
    orientationsY = zeros(size(normX,1),1);
    orientationsZ = zeros(size(normX,1),1);
    
    for j = 1:size(x,1)
                
        currentShape = [normX(j,:); normY(j,:); normZ(j,:)]';
        % we want to align the current shape to the mean one
        [ R, T ] = AlignShapesKabsch(currentShape, meanShape);
        
        eulers = Rot2Euler(R);

        orientationsX(j) = eulers(1);
        orientationsY(j) = eulers(2);
        orientationsZ(j) = eulers(3);

        Transform.RotationX(j) = eulers(1);
        Transform.RotationY(j) = eulers(2);
        Transform.RotationZ(j) = eulers(3);
        
        currentShape = R * currentShape';                
        
        normX(j,:) = currentShape(1,:);
        normY(j,:) = currentShape(2,:);
        normZ(j,:) = currentShape(3,:);
        
    end
    
    % recalculate the mean shape
%     if(~meanProvided)
        oldMean = meanShape;
        meanShape = [mean(normX); mean(normY); mean(normZ)]';
        meanScale = norm(meanShape, 'fro');  
%     end
    
    for j = 1:size(x,1)
    
        scale = norm([normX(j,:) normY(j,:) normZ(j,:)], 'fro')/meanScale;

        normX(j,:) = normX(j,:)/scale;
        normY(j,:) = normY(j,:)/scale;
        normZ(j,:) = normZ(j,:)/scale;

    end

    if(i==1 && ~meanProvided)
        
        % rotate the mean shape to mean rotation
        meanOrientationX = mean(orientationsX);
        meanOrientationY = mean(orientationsY);
        meanOrientationZ = mean(orientationsZ);

        R = Euler2Rot([meanOrientationX, meanOrientationY, meanOrientationZ]);
        meanShape = (R * meanShape')';
    end
    
    % find frobenious norm
    diff = norm(oldMean - meanShape, 'fro');
    
    if(diff/norm(oldMean,'fro') < change)
        break;
    end
    
end

% transform to tangent space to preserve linearities

% get the scaling factors for each shape
if(tangentSpace)
    [ normX, normY, normZ] = TangentSpaceTransform(normX, normY, normZ, meanShape);
end

end

