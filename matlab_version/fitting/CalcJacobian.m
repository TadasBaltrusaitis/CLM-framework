% This calculates the combined rigid with non-rigid Jacobian (non-rigid can
% eiher be expression or identity one)
function J = CalcJacobian(M, V, p_local, p_global)

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
    J(:,1:6) = CalcRigidJacobian(M, V, p_local, p_global);
    
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
