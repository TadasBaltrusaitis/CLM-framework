% for easier readibility write them row by row
function writeMatrixBin(fileID, M, type)

    % 4 bytes each for the description
    fwrite(fileID, size(M,1), 'uint');
    fwrite(fileID, size(M,2), 'uint');
    fwrite(fileID, type, 'uint');
    
    % Convert the matrix to OpenCV format (row minor as opposed to column
    % minor)
    M = M';
    
    % type 0 - uint8, 1 - int8, 2 - uint16, 3 - int16, 4 - int, 5 -
    % float32, 6 - float64
    
    % Write out the matrix itself    
    
    switch type
        case 0
            type = 'uint8';
        case 1
            type = 'int8';
        case 2
            type = 'uint16';
        case 3
            type = 'int16';
        case 4
            type = 'int';
        case 5
            type = 'float32';
        case 6
            type = 'float64';            
        otherwise
            type = 'float32';
    end
    fwrite(fileID, M, type);
end