function [ mapX, mapY ] = WarpRegion( xmin, ymin, mask, triX, coeffs )
%WARPREGION Summary of this function goes here
%   Detailed explanation goes here

    %%
    [h, w] = size(mask);
    mapX = zeros(size(mask));
    mapY = zeros(size(mask));
 
    ys = [1:h]' * ones(1, w) + ymin - 1;
    xs = ([1:w]' * ones(1, h))' + xmin - 1;
    
    for t=0:size(coeffs,1)-1
       
        trimap = triX == t;
        
        a = coeffs(t+1,:); 
                
        xo = a(1) + a(2) * xs + a(3) * ys;       
        
        mapX(trimap) = xo(trimap);

        yo = a(4) + a(5) * xs + a(6) * ys;
        mapY(trimap) = yo(trimap);
        
    end
    
    mapX(~mask) = -1;
    mapY(~mask) = -1;
     
    %%
%     [h, w] = size(mask);
%     mapX_2 = zeros(size(mask));
%     mapY_2 = zeros(size(mask));
%  
%     ys = [1:h]' * ones(1, w) + ymin - 1;
%     xs = ([1:w]' * ones(1, h))' + xmin - 1;
%     
%     ys = ys(:);
%     xs = xs(:);
%     
%     xos = coeffs(1,:) + bsxfun(@times, coeffs(2,:), xs) + bsxfun(@times, coeffs(3,:), ys);    
%     yos = coeffs(4,:) + bsxfun(@times, coeffs(5,:), xs) + bsxfun(@times, coeffs(6,:), ys);
%     
%     maps = repmat(trimap(:),1, size(coeffs,1));
%     maps = repmat
    
end

