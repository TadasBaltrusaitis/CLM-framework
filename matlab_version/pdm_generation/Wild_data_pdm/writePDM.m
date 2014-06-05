function writePDM( V, E, M, outputFile, Vmorph, Emorph )
%WRITEPDM Summary of this function goes here
%   Detailed explanation goes here

    fId = fopen(outputFile,'w');

    % number of elements
    
    % Comment
    fprintf(fId, '# The mean values of the components (in mm)\n');
    writeMatrix(fId, M, 6);
    
    fprintf(fId, '# The principal components (eigenvectors) of identity or combined identity and expression model\n');
    writeMatrix(fId, V, 6);
    
    fprintf(fId, '# The variances of the components (eigenvalues) of identity or combined identity and expression model\n');
    writeMatrix(fId, E', 6);

    if(nargin > 4)
        fprintf(fId, '# The principal components (eigenvectors) of expression\n');
        writeMatrix(fId, Vmorph, 6);

        fprintf(fId, '# The variances of the components (eigenvalues) of expression\n');
        writeMatrix(fId, Emorph', 6);
    end
    
    fclose(fId);
end

% for easier readibility write them row by row
function writeMatrix(fileID, M, type)

    fprintf(fileID, '%d\n', size(M,1));
    fprintf(fileID, '%d\n', size(M,2));
    fprintf(fileID, '%d\n', type);
    
    for i=1:size(M,1)
        fprintf(fileID, '%f ', M(i,:));
        fprintf(fileID, '\n');
    end
end