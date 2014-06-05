function [ alphas, betas, triX, mask, xmin, ymin, npix ] = InitialisePieceWiseAffine( triangulation, sourcePoints )
%INITIALISEPIECEWICEAFFINE Summary of this function goes here
%   Detailed explanation goes here

    triangulation = triangulation + 1;
    numPoints = size(sourcePoints, 1);

    numTris = size(triangulation, 1);
    
    alphas = zeros(size(triangulation, 1), 3);
    betas = zeros(size(triangulation, 1), 3);
    
    xs = sourcePoints(:,1);
    ys = sourcePoints(:,2);
    
	for i = 1:numTris
                	
        j = triangulation(i, 1);
        k = triangulation(i, 2);
        l = triangulation(i, 3);

        c1 = ys(l) - ys(j);
        c2 = xs(l) - xs(j);
        c4 = ys(k) - ys(j);
        c3 = xs(k) - xs(j);
        		
        c5 = c3*c1 - c2*c4;

        alphas(i, 1) = (ys(j) * c2 - xs(j) * c1) / c5;
        alphas(i, 2) = c1/c5;
        alphas(i, 3) = -c2/c5;

        betas(i, 1) = (xs(j) * c4 - ys(j) * c3)/c5;
        betas(i, 2) = -c4/c5;
        betas(i, 3) = c3/c5;
    end

    xmin = min(xs);
    ymin = min(ys);
    
    xmax = max(xs);
    ymax = max(ys);
    
	w = int32(xmax - xmin + 1);
    h = int32(ymax - ymin + 1);
    
    mask = zeros(h, w);
    triX = zeros(h, w);
    
    shape = [xs, ys];
    
    for i = 1:h
        for j = 1:w

            currTri = findTriangle(double([double(j)-1+xmin, double(i)-1+ymin])', triangulation, shape);
            if(currTri ~= -1)
                triX(i, j) =  currTri - 1;
                mask(i, j) = 1;
            else
                triX(i, j) = -1;
            end		
        end
    end
    npix = sum(sum(mask));
end

function [tri] = findTriangle(point, tris, controlPoints)
    
    numTris = size(tris, 1);
    tri = -1;
    
    for i=1:numTris
       
        if(PointInTriangle(point, controlPoints(tris(i,1),:)', controlPoints(tris(i,2),:)', controlPoints(tris(i,3),:)'))
           tri = i;
           break;
        end
        
    end

end
        
function inTriangle = PointInTriangle(point, v1, v2, v3)

    inTriangle = SameSide(point, v1,v2,v3) && SameSide(point, v2,v1,v3) && SameSide(point, v3,v1,v2);

end

function sameSide = SameSide(toTest,v1,v2,v3)

    x0 = toTest(1);
    y0 = toTest(2);
    
    x1 = v1(1);
    x2 = v2(1);
    x3 = v3(1);
    
    y1 = v1(2);
    y2 = v2(2);
    y3 = v3(2);

    x = (x3-x2)*(y0-y2) - (x0-x2)*(y3-y2);
    y = (x3-x2)*(y1-y2) - (x1-x2)*(y3-y2);
    if(x*y >= 0)
        sameSide = 1;
    else
        sameSide = 0;
    end
%     cross1 = cross( v3 - v2, toTest - v2);
%     cross2 = cross( v3 - v2, v1 - v2);
%     
%     sameSide = (cross1 * cross2') >= 0;
end