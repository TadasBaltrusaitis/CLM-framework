function [shape3D] = GetShape3D(M, V, p)

    shape3D = M + V * p;

    shape3D = reshape(shape3D, numel(shape3D) / 3, 3);
    
end