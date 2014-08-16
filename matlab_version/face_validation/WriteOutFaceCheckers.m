function WriteOutFaceCheckers(locationTxt, locationMlab, faceCheckers)

    addpath('..\PDM_helpers\');
    
    faceCheckerFile = fopen(locationTxt, 'w');        
    
    views = numel(faceCheckers);
    
    fprintf(faceCheckerFile, '# Number of face checkers\r\n%d\r\n', views);

    fprintf(faceCheckerFile, '# Face checker centres\r\n');
    
    for i=1:views
        % this indicates that we're writing a 3x1 double matrix
        writeMatrix(faceCheckerFile, faceCheckers(i).centres', 6);
    end
    
    for i = 1:views
        
        b = faceCheckers(i).b;
        w = faceCheckers(i).w;
        
        fprintf(faceCheckerFile, '# Principal Components %d\r\n', i);
        writeMatrix(faceCheckerFile, faceCheckers(i).principal_components, 6);
               
        fprintf(faceCheckerFile, '# Mean of images %d\r\n', i);
        writeMatrix(faceCheckerFile, faceCheckers(i).mean_ex, 6);
                
        fprintf(faceCheckerFile, '# Standard deviation of images %d\r\n', i);
        writeMatrix(faceCheckerFile, faceCheckers(i).std_ex, 6);
        
        fprintf(faceCheckerFile, '# Classifier %d\r\n', i);

        fprintf(faceCheckerFile, '# Linear SVR\r\n');
        fprintf(faceCheckerFile, '%f\r\n', b);
        writeMatrix(faceCheckerFile, w, 6);
        
        fprintf(faceCheckerFile, '# Piecewise affine warp \r\n');
        
        nPix = faceCheckers(i).nPix;
        minX = faceCheckers(i).minX;
        minY = faceCheckers(i).minY;
        
        source = reshape(faceCheckers(i).source, numel(faceCheckers(i).source), 1);        
        triangulation = faceCheckers(i).triangulation;
        triX = faceCheckers(i).triX;
        mask = faceCheckers(i).mask;
        alphas = faceCheckers(i).alphas;
        betas = faceCheckers(i).betas;
        
        fprintf(faceCheckerFile, '%d\r\n%f\r\n%f\r\n', nPix, minX, minY);
        
        fprintf(faceCheckerFile, '# source shape \r\n');
        writeMatrix(faceCheckerFile, source, 6);
        
        fprintf(faceCheckerFile, '# triangulation \r\n');
        writeMatrix(faceCheckerFile, triangulation, 4);
        
        fprintf(faceCheckerFile, '# triangle map \r\n');      
        writeMatrix(faceCheckerFile, triX, 4);        

        fprintf(faceCheckerFile, '# mask \r\n');      
        writeMatrix(faceCheckerFile, mask, 4);        
        
        fprintf(faceCheckerFile, '# alphas \r\n');      
        writeMatrix(faceCheckerFile, alphas, 6);        

        fprintf(faceCheckerFile, '# betas \r\n');      
        writeMatrix(faceCheckerFile, betas, 6);        

    end
    
    fclose(faceCheckerFile);
    
end