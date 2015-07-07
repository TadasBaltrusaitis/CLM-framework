function [ error ] = Mirror_error( Image, bounding_box, PDM, patchExperts, clmParams, shape_prev, varargin)
%MIRROR_ERROR Summary of this function goes here
%   Detailed explanation goes here

    Image = fliplr(Image);

    [img_height, img_width, ~] = size(Image);
    
    % flip the bounding box
    bounding_box_mirror = [img_width - bounding_box(3) + 1, bounding_box(2), img_width - bounding_box(1) + 1, bounding_box(4)];
    
    if(any(strcmp(varargin,'orientation')))
        orientation = varargin{find(strcmp(varargin, 'orientation'))+1};        
        orientation(2) = -orientation(2);
        varargin = {'orientation', orientation};
    else
        varargin = {};

    end
    
    
%     bounding_box_mirror = bounding_box_mirror + 2;
        
    % flip the bounding box, if using orientations flip those as well    
    shape2D = Fitting_from_bb( Image, [], bounding_box_mirror, PDM, patchExperts, clmParams, varargin{:});

    % mirror the shape now
    if(size(shape2D,1) == 66)
        mirror_inds = [1,17;2,16;3,15;4,14;5,13;6,12;7,11;8,10;18,27;19,26;20,25;21,24;22,23;...
              32,36;33,35;37,46;38,45;39,44;40,43;41,48;42,47;49,55;50,54;51,53;60,56;59,57;...
              61,63;66,64];            
    elseif(size(shape2D,1) == 68)
        mirror_inds = [1,17;2,16;3,15;4,14;5,13;6,12;7,11;8,10;18,27;19,26;20,25;21,24;22,23;...
                      32,36;33,35;37,46;38,45;39,44;40,43;41,48;42,47;49,55;50,54;51,53;60,56;59,57;...
                      61,65;62,64;68,66];    
    end
    
    shape2D(:,1) = img_width - shape2D(:,1) + 1;

%     hold on;plot(shape2D(:,1), shape2D(:,2), '.g');hold off;
    
    tmp1 = shape2D(mirror_inds(:,1),:);
    tmp2 = shape2D(mirror_inds(:,2),:);            
    shape2D(mirror_inds(:,2),:) = tmp1;
    shape2D(mirror_inds(:,1),:) = tmp2;   

    error = mean(sqrt(sum((shape_prev - shape2D).^2,2)));
end

