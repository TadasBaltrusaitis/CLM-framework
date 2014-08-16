function WriteOutFaceCheckersCNNbinary(locationTxt, faceCheckers)

    addpath('..\PDM_helpers\');
    
    % use little-endian
    faceCheckerFile = fopen(locationTxt, 'w', 'l');        
    
    views = numel(faceCheckers);
    
    % Type 0 - linear SVR, 1 - feed forward neural net, 2 - CNN
    fwrite(faceCheckerFile, 2, 'uint'); % 4 bytes
        
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
                
        cnn = faceCheckers(i).cnn;
        
        num_depth_layers = size(cnn.layers,1);

        % Get the number of layers
        fwrite(faceCheckerFile, num_depth_layers, 'uint'); % 4 bytes
        
        for layers=2:num_depth_layers
           
            % write layer type: 0 - convolutional, 1 - subsampling, 2-
            % fully connected
            if(cnn.layers{layers}.type == 'c')
                
                % write the type (convolutional)
                fwrite(faceCheckerFile, 0, 'uint'); % 4 bytes
                
                num_in_map = size(cnn.layers{layers}.k,2);

                % write the number of input maps
                fwrite(faceCheckerFile, num_in_map, 'uint'); % 4 bytes

                num_out_kerns = cnn.layers{layers}.outputmaps;

                % write the number of kernels for each output map
                fwrite(faceCheckerFile, num_out_kerns, 'uint'); % 4 bytes
                    
                % Write output map bias terms
                for k2=1:num_out_kerns    
                    fwrite(faceCheckerFile, cnn.layers{layers}.b{k2}, 'float32'); % 4 bytes
                end
                    
                for k=1:num_in_map                                        
                    for k2=1:num_out_kerns
                        % Write out the bias term                                                
                        W = cnn.layers{layers}.k{k}{k2};
                        writeMatrixBin(faceCheckerFile, W, 5);                
                    end
                end
                
            else
                fwrite(faceCheckerFile, 1, 'uint'); % 4 bytes
                % size of scaling
                fwrite(faceCheckerFile, cnn.layers{layers}.scale, 'uint'); % 4 bytes
            end
            
        end
        
        % This is the fully connected layer
        fwrite(faceCheckerFile, 2, 'uint'); % 4 bytes
        
        % the bias term
        fwrite(faceCheckerFile, cnn.ffb, 'float32');
        % the weights
        writeMatrixBin(faceCheckerFile, cnn.ffW, 5);
        %sigm(net.ffW * net.fv + repmat(net.ffb, 1, size(net.fv, 2)));

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