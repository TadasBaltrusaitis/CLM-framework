function [ a, R, T, T3D, params, error, shapeOrtho ] = fit_PDM_ortho_proj_to_2D( M, E, V, shape2D, f, cx, cy)
%FITPDMTO2DSHAPE Summary of this function goes here
%   Detailed explanation goes here

    params = zeros(size(E));

    hidden = false;

    % if some of the points are unavailable modify M, V, and shape2D (can
    % later infer the actual shape from this)
    if(sum(shape2D(:)==0) > 0)        

        hidden = true;
        % which indices to remove
        inds_to_rem = shape2D(:,1) == 0 | shape2D(:,2) == 0;

        shape2D = shape2D(~inds_to_rem,:);

        inds_to_rem = repmat(inds_to_rem, 3, 1);

        M_old = M;
        V_old = V;

        M = M(~inds_to_rem);
        V = V(~inds_to_rem,:);

    end
    
    num_points = numel(M) / 3;
    
    m = reshape(M, num_points, 3)';
    width_model = max(m(1,:)) - min(m(1,:));
    height_model = max(m(2,:)) - min(m(2,:));

    bounding_box = [min(shape2D(:,1)), min(shape2D(:,2)),...
                    max(shape2D(:,1)), max(shape2D(:,2))];
    
    a = (((bounding_box(3) - bounding_box(1)) / width_model) + ((bounding_box(4) - bounding_box(2))/ height_model)) / 2;
        
    tx = (bounding_box(3) + bounding_box(1))/2;
    ty = (bounding_box(4) + bounding_box(2))/2;
    
    % correct it so that the bounding box is just around the minimum
    % and maximum point in the initialised face
    tx = tx - a*(min(m(1,:)) + max(m(1,:)))/2;
    ty = ty - a*(min(m(2,:)) + max(m(2,:)))/2;    
    
    R = eye(3); 
    T = [tx; ty];
    
    currShape = getShapeOrtho(M, V, params, R, T, a);
    
    currError = getRMSerror(currShape, shape2D);
    
    reg_rigid = zeros(6,1);
    regFactor = 20;
    regularisations = [reg_rigid; regFactor ./ E]; % the above version, however, does not perform as well
    regularisations = diag(regularisations)*diag(regularisations);
    
    red_in_a_row = 0;
    
    for i=1:1000
                      
        shape3D = M + V * params;
        shape3D = reshape(shape3D, numel(shape3D) / 3, 3);
                
        % Now find the current residual error        
        currShape = a * R(1:2,:)*shape3D' + repmat(T, 1, numel(M)/3); 
        currShape = currShape'; 
        
        error_res = shape2D - currShape;
        
        eul = Rot2Euler(R);
        
        p_global = [a; eul'; T];
        
        % get the Jacobians
        J = CalcJacobian(M, V, params, p_global);
        
        % RLMS style update
        p_delta = (J'*J + regularisations) \ (J'*error_res(:) - regularisations*[p_global;params]);
                                
        [params, p_global] = CalcReferenceUpdate(p_delta, params, p_global);
        
        a = p_global(1);
        R = Euler2Rot(p_global(2:4));
        T = p_global(5:6);
        
        shape3D = M + V * params;
        shape3D = reshape(shape3D, numel(shape3D) / 3, 3);
        currShape = a * R(1:2,:)*shape3D' + repmat(T, 1, numel(M)/3); 
        currShape = currShape'; 
             
        error = getRMSerror(currShape, shape2D);
        
        if(0.999 * currError < error)
            red_in_a_row = red_in_a_row + 1;
            if(red_in_a_row == 5)
                break;
            end
        end
        
        currError = error;
        
    end    
    
    if(hidden)
        shapeOrtho = getShapeOrtho(M_old, V_old, params, R, T, a);
    else
        shapeOrtho = currShape;
    end
    if(nargin == 7)
    
        Zavg = f / a;
        Xavg = (T(1) - cx) / a;
        Yavg = (T(2) - cy) / a;

        T3D = [Xavg;Yavg;Zavg];
    else
        T3D = [0;0;0];
    end
    
end

function [shape2D] = getShapeOrtho(M, V, p, R, T, a)

    % M - mean shape vector
    % V - eigenvectors
    % p - parameters of non-rigid shape
    % R - rotation matrix
    % T - translation vector (tx, ty)
    shape3D = getShape3D(M, V, p);
    shape2D = a * R(1:2,:)*shape3D' + repmat(T, 1, numel(M)/3);
    shape2D = shape2D';
end

function [shape2D] = getShapeOrthoFull(M, V, p, R, T, a)

    % M - mean shape vector
    % V - eigenvectors
    % p - parameters of non-rigid shape
    % R - rotation matrix
    % T - translation vector (tx, ty)    
    T = [T; 0];
    shape3D = getShape3D(M, V, p);
    shape2D = a * R*shape3D' + repmat(T, 1, numel(M)/3);
    shape2D = shape2D';
end

function [shape3D] = getShape3D(M, V, params)

    shape3D = M + V * params;
    shape3D = reshape(shape3D, numel(shape3D) / 3, 3);
    
end

function [error] = getRMSerror(shape2Dv1, shape2Dv2)

    error = sqrt(mean(reshape(shape2Dv1 - shape2Dv2, numel(shape2Dv1), 1).^2));

end

% This calculates the combined rigid with non-rigid Jacobian
function J = CalcJacobian(M, V, p, p_global)

    n = size(M, 1)/3;
    
    non_rigid_modes = size(V,2);
    
    J = zeros(n*2, 6 + non_rigid_modes);
    
    
    % now the layour is
    % ---------- Rigid part -------------------|----Non rigid part--------|
    % dx_1/ds, dx_1/dr1, ... dx_1/dtx, dx_1/dty dx_1/dp_1 ... dx_1/dp_m
    % dx_2/ds, dx_2/dr1, ... dx_2/dtx, dx_2/dty dx_2/dp_1 ... dx_2/dp_m
    % ...
    % dx_n/ds, dx_n/dr1, ... dx_n/dtx, dx_n/dty dx_n/dp_1 ... dx_n/dp_m
    % dy_1/ds, dy_1/dr1, ... dy_1/dtx, dy_1/dty dy_1/dp_1 ... dy_1/dp_m
    % ...
    % dy_n/ds, dy_n/dr1, ... dy_n/dtx, dy_n/dty dy_n/dp_1 ... dy_n/dp_m
    
    % getting the rigid part
    J(:,1:6) = CalcRigidJacobian(M, V, p, p_global);
    
    % constructing the non-rigid part
    R = Euler2Rot(p_global(2:4));
    s = p_global(1);
    
    % 'rotate' and 'scale' the principal components
    
    % First reshape to 3D
    V_X = V(1:n,:);
    V_Y = V(n+1:2*n,:);
    V_Z = V(2*n+1:end,:);
    
    J_x_non_rigid = s*(R(1,1)*V_X + R(1,2)*V_Y + R(1,3)*V_Z);
    J_y_non_rigid = s*(R(2,1)*V_X + R(2,2)*V_Y + R(2,3)*V_Z);
    
    J(1:n, 7:end) = J_x_non_rigid;
    J(n+1:end, 7:end) = J_y_non_rigid;
    
end

function J = CalcRigidJacobian(M, V, p, p_global)

 	n = size(M, 1)/3;
  
	% Get the current 3D shape (not affected by global transform, as this
	% is how the Jacobian was derived (for derivation please see
	% ../derivations/orthoJacobian
	shape3D = GetShape3D(M, V, p);

	% Get the rotation matrix corresponding to current global orientation
	R = Euler2Rot(p_global(2:4));
	s = p_global(1);
    
    % Rigid Jacobian is laid out as follows
    % dx_1/ds, dx_1/dr1, dx_1/dr2, dx_1/dr3, dx_1/dtx, dx_1/dty
    % dx_2/ds, dx_2/dr1, dx_2/dr2, dx_2/dr3, dx_2/dtx, dx_2/dty
    % ...
    % dx_n/ds, dx_n/dr1, dx_n/dr2, dx_n/dr3, dx_n/dtx, dx_n/dty
    % dy_1/ds, dy_1/dr1, dy_1/dr2, dy_1/dr3, dy_1/dtx, dy_1/dty
    % ...
    % dy_n/ds, dy_n/dr1, dy_n/dr2, dy_n/dr3, dy_n/dtx, dy_n/dty
    
    J = zeros(n*2, 6);
    
    % dx/ds = X * r11  + Y * r12 + Z * r13
    % dx/dr1 =  s*(r13 * Y - r12 * Z)
    % dx/dr2 = -s*(r13 * X - r11 * Z)
    % dx/dr3 =  s*(r12 * X - r11 * Y)
    % dx/dtx = 1
    % dx/dty = 0
    
    % dy/ds = X * r21  + Y * r22 + Z * r23
    % dy/dr1 =  s * (r23 * Y - r22 * Z)
    % dy/dr2 = -s * (r23 * X - r21 * Z)
    % dy/dr3 =  s * (r22 * X - r21 * Y)
    % dy/dtx = 0
    % dy/dty = 1
        
    % set the Jacobian for x's
    
    % with respect to scaling factor
    J(1:n,1) = shape3D * R(1,:)';
    
    % with respect to angular rotation around x, y, and z axes
    
    % Change of x with respect to change in axis angle rotation
	dxdR = [      0,  R(1,3), -R(1,2);
            -R(1,3),       0,  R(1,1);
             R(1,2), -R(1,1),      0];

    J(1:n,2:4) = s*(dxdR * shape3D')';
         
    % with respect to translation
    J(1:n,5) = 1;
    J(1:n,6) = 0;
    
    % set the Jacobian for y's

    % with respect to scaling factor
    J(n+1:end,1) = shape3D * R(2,:)';

     % with respect to angular rotation around x, y, and z axes
    
    % Change of y with respect to change in axis angle rotation
    dydR = [      0,  R(2,3), -R(2,2);
            -R(2,3),       0,  R(2,1);
             R(2,2), -R(2,1),      0];
         
     J(n+1:end,2:4) = s*(dydR * shape3D')';

     % with respect to translation
     J(n+1:end,5) = 0;
     J(n+1:end,6) = 1;

end

% This updates the parameters based on the updates from the RLMS
function [non_rigid, rigid] = CalcReferenceUpdate(params_delta, current_non_rigid, current_global)


    rigid = zeros(6, 1);
    % Same goes for scaling and translation parameters
    rigid(1) = current_global(1) + params_delta(1);
    rigid(5) = current_global(5) + params_delta(5);
    rigid(6) = current_global(6) + params_delta(6);
    
    % for rotation however, we want to make sure that the rotation matrix
    % approximation we have 
	% R' = [1, -wz, wy
	%       wz, 1, -wx
	%       -wy, wx, 1]	
    % is a legal rotation matrix, and then we combine it with current
    % rotation (through matrix multiplication) to acquire the new rotation
    
	R = Euler2Rot(current_global(2:4));

    wx = params_delta(2);
    wy = params_delta(3);
    wz = params_delta(4);
    
    R_delta = [1, -wz, wy;
               wz, 1, -wx;
               -wy, wx, 1];
	
	% Make sure R_delta is orthonormal
	R_delta = OrthonormaliseRotation(R_delta);
	
    % Combine rotations
	R_final = R * R_delta;

	% Extract euler angle
	euler = Rot2Euler(R_final);	
	
	rigid(2:4) = euler;
    
    if(length(params_delta) > 6)
        % non-rigid parameters can just be added together
        non_rigid = params_delta(7:end) +  current_non_rigid;
    else
        non_rigid = current_non_rigid;
    end
    
end

function R_ortho = OrthonormaliseRotation(R)
  
    % U * V' is basically what we want, as it's guaranteed to be
    % orthonormal
    [U, ~, V] = svd(R);

    % We also want to make sure no reflection happened
    
    % get the orthogonal matrix from the initial rotation matrix
    X = U*V';

    % This makes sure that the handedness is preserved and no reflection happened
    % by making sure the determinant is 1 and not -1
    W = eye(3);
    W(3,3) = det(X);
    R_ortho = U*W*V';
end