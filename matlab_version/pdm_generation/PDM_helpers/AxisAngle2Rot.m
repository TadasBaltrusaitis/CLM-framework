function [Rot] = AxisAngle2Rot(axisAngle)

    theta = norm(axisAngle, 2);
    
    nx = axisAngle / theta;
    
    nx = [    0  -nx(3)  nx(2);
           nx(3)     0  -nx(1);
          -nx(2)  nx(1)     0 ];
    
    Rot = eye(3) + sin(theta) * nx + (1-cos(theta))*nx^2;
