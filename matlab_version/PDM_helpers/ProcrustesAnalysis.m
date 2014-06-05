function [normX, normY, meanShape, Transform] = ProcrustesAnalysis(x, y, options)

% Translate all elements to origin and scale to 1
normX = zeros(size(x));
normY = zeros(size(y));

for i = 1:size(x,1)
    
    offsetX = mean(x(i,:));
    offsetY = mean(y(i,:));

    Transform.offsetX(i) = offsetX;
    Transform.offsetY(i) = offsetY;
    
    normX(i,:) = x(i,:) - offsetX;
    normY(i,:) = y(i,:) - offsetY;

    % Get the Frobenius norm, to scale the shapes to unit size
    scale = norm([normX(i,:) normY(i,:)], 'fro');
    
    Transform.scale(i) = scale;
    
    normX(i,:) = normX(i,:)/scale;
    normY(i,:) = normY(i,:)/scale;
    
end

% Rotate elements untill all of them have the same orientation

% the initial estimate of rotation would be the first element
% if change is less than 1% stop (shouldn't take more than 2 steps)
change = 0.1;

meanShape = [ normX(1,:); normY(1,:) ]';

Transform.Rotation = zeros(size(x,1),1);

for i = 1:30
    
    % align all of the shapes to the mean shape
    
    % remember all orientations to get the mean one
    orientations = zeros(size(normX,1),1);
    
    for j = 1:size(x,1)
                
        % do SVD of mean * X'
        currentShape = [ normX(j,:); normY(j,:) ]';
        [U, ~, V] = svd( meanShape' * currentShape);
        rot = V*U';
        
        if(asin(rot(2,1)) > 0)
           orientations(j) = real(acos(rot(1,1)));
        else
           orientations(j) = real(-acos(rot(1,1)));
        end
        
        Transform.Rotation(j) = Transform.Rotation(j) + orientations(j);
        
        currentShape = currentShape * rot;                
        
        normX(j,:) = currentShape(:,1)';
        normY(j,:) = currentShape(:,2)';
        
    end
    
    % recalculate the mean shape;
    oldMean = meanShape;
    meanShape = [mean(normX); mean(normY)]';
    
    % rotate the mean shape to mean rotation
    meanOrientation = mean(orientations);
    
    % Do this only the first time
    if(i==1)
        
        rotM = [ cos(-meanOrientation) -sin(-meanOrientation); sin(-meanOrientation) cos(-meanOrientation) ]; 
        meanShape = meanShape * rotM;
    end
    % scale mean shape to unit
    meanScale = norm(meanShape, 'fro');
    meanShape = meanShape*(1/meanScale);
    
    % find frobenious norm
    diff = norm(oldMean - meanShape, 'fro');
    
    if(diff/norm(oldMean,'fro') < change)
        break;
    end
    
end

% transform to tangent space to preserve linearities

% get the scaling factors for each shape
if(options.TangentSpaceTransform)
    scaling = [ normX normY ] * [ meanShape(:,1)' meanShape(:,2)']';
    for i=1:size(x,1)
        normX(i,:) = normX(i,:) * (1 / scaling(i));
        normY(i,:) = normY(i,:) * (1 / scaling(i));
    end
end

