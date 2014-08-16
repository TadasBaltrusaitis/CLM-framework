function [warpedImage] = Crop(inputImage, destinationPoints, triangulation, triX, mask, alphas, betas, nPix, minX, minY)

    coeffs = CalculateCoefficients(alphas, betas, triangulation, destinationPoints);
    [ mapX, mapY ] = WarpRegion( minX, minY, mask, triX, coeffs );
    if(size(inputImage,3) == 1)
       warpedImage = Remap(inputImage, mapX, mapY);
    else
       
        warpedImage = [];
        for i=1:size(inputImage,3)
           warpedImage = cat(3, warpedImage,  Remap(inputImage(:,:,i), mapX, mapY));
        end
        
    end
end