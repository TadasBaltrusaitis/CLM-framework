function WriteOutFaceCheckersNNbinary(locationTxt, faceCheckers)

    addpath('..\PDM_helpers\');
    
    % use little-endian
    faceCheckerFile = fopen(locationTxt, 'w', 'l');        
    
    views = numel(faceCheckers);
    
    % Type 0 - linear SVR, 1 - feed forward neural net
    fwrite(faceCheckerFile, 1, 'uint'); % 4 bytes
        
    % Number of face checkers
    fwrite(faceCheckerFile, views, 'uint'); % 4 bytes
    
    % Matrices representing view orientations
    for i=1:views
        % this indicates that we're writing a 3x1 double matrix
        writeMatrixBin(faceCheckerFile, faceCheckers(i).centres', 6);
    end
    
    for i = 1:views
        
        % The normalisation models
        % Mean of images
        writeMatrixBin(faceCheckerFile, faceCheckers(i).mean_ex, 6);
                
        % Standard deviation of images
        writeMatrixBin(faceCheckerFile, faceCheckers(i).std_ex, 6);
                
        nn = faceCheckers(i).nn;
        
        num_depth_layers = numel(nn.size) - 1;
        % Get the number of layers
        fwrite(faceCheckerFile, num_depth_layers, 'uint'); % 4 bytes
        
        % The activation function used 0 - sigmoid, 1 - tanh, 2 - ReLu
        act_fun = 0;
        if(strcmp(nn.activation_function, 'tanh_opt'))
            act_fun = 1; 
        elseif(strcmp(nn.activation_function, 'ReLu'))
            act_fun = 2;
        end
        
        fwrite(faceCheckerFile, act_fun, 'uint'); % 4 bytes
        
        % The activation function used 0 - sigmoid, 1 - tanh, 2 - ReLu
        out_fun = 0;
        if(strcmp(nn.output, 'tanh_opt'))
            out_fun = 1; 
        elseif(strcmp(nn.output, 'ReLu'))
            out_fun = 2;
        end
                
        fwrite(faceCheckerFile, out_fun, 'uint'); % 4 bytes
        
        for layers=1:num_depth_layers
           
            W = nn.W{layers};
            writeMatrixBin(faceCheckerFile, W, 6);
        end
        

        % Piecewise affine warp
        
        nPix = faceCheckers(i).nPix;
        minX = faceCheckers(i).minX;
        minY = faceCheckers(i).minY;
               
        destination = reshape(faceCheckers(i).destination, numel(faceCheckers(i).destination), 1);         
        triangulation = faceCheckers(i).triangulation;
        triX = faceCheckers(i).triX;
        mask = faceCheckers(i).mask;
        alphas = faceCheckers(i).alphas;
        betas = faceCheckers(i).betas;
        
        fwrite(faceCheckerFile, nPix, 'uint'); % 4 bytes
        fwrite(faceCheckerFile, minX, 'float64'); % 8 bytes
        fwrite(faceCheckerFile, minY, 'float64'); % 8 bytes
        
        % Destination shape
        writeMatrixBin(faceCheckerFile, destination, 6);
        
        % Triangulation
        writeMatrixBin(faceCheckerFile, triangulation, 4);
        
        % Triangle map
        writeMatrixBin(faceCheckerFile, triX, 4);        

        % Mask
        writeMatrixBin(faceCheckerFile, mask, 4);        
        
        % Alphas
        writeMatrixBin(faceCheckerFile, alphas, 6);        

        % Betas
        writeMatrixBin(faceCheckerFile, betas, 6);        

    end
    
    fclose(faceCheckerFile);
    
end