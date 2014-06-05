function [shape2D] = GetShapeOrtho(M, V, p, global_params)

    % M - mean shape vector
    % V - eigenvectors
    % p - parameters of non-rigid shape
    % V_exp
    % p_exp
    % global_params includes scale, euler rotation, translation, 
    % R - rotation matrix
    % T - translation vector (tx, ty)    
    
    R = Euler2Rot(global_params(2:4));    
    T = [global_params(5:6); 0];
    a = global_params(1);
    
    shape3D = GetShape3D(M, V, p);
    
    shape2D = bsxfun(@plus, a * R*shape3D', T);
    shape2D = shape2D';
end