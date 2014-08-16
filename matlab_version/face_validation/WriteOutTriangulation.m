function WriteOutTriangulation(locationTxt, locationMlab, faceCheckers)

    addpath('..\PDM_helpers\');
    
    triangulationFile = fopen(locationTxt, 'w');        
    
    views = numel(faceCheckers);
    
    fprintf(triangulationFile, '# Number of triangulations\r\n%d\r\n', views);

    for i = 1:views
        
        fprintf(triangulationFile, '# Triangulation %d\r\n', i);
  
        triangulation = faceCheckers(i).triangulation;

        % swap around for clockwise convention in openGL
        tmp = triangulation(:,2);
        triangulation(:,2) = triangulation(:,1);
        triangulation(:,1) = tmp;
                
        fprintf(triangulationFile, '# triangulation \r\n');
        writeMatrix(triangulationFile, triangulation, 4);        

    end
    
    fclose(triangulationFile);
    
    save(locationMlab);
end