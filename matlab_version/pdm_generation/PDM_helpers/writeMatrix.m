% for easier readibility write them row by row
function writeMatrix(fileID, M, type)

    fprintf(fileID, '%d\r\n', size(M,1));
    fprintf(fileID, '%d\r\n', size(M,2));
    fprintf(fileID, '%d\r\n', type);
    
    for i=1:size(M,1)
        if(type == 4 || type == 0)
            fprintf(fileID, '%d ', M(i,:));
        else
            fprintf(fileID, '%.9f ', M(i,:));
        end
        fprintf(fileID, '\r\n');
    end
end