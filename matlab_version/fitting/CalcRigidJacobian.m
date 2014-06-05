function J = CalcRigidJacobian(M, V, p_local, p_global)

 	n = size(M, 1)/3;
  
	% Get the current 3D shape (not affected by global transform, as this
	% is how the Jacobian was derived (for derivation please see
	% ../derivations/orthoJacobian
	shape3D = GetShape3D(M, V, p_local);

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

