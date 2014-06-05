function [euler] = Rot2Euler(R)

    q0 = sqrt( 1 + R(1,1) + R(2,2) + R(3,3) ) / 2;
    q1 = (R(3,2) - R(2,3)) / (4*q0) ;
    q2 = (R(1,3) - R(3,1)) / (4*q0) ;
    q3 = (R(2,1) - R(1,2)) / (4*q0) ;

    yaw  = asin(2*(q0*q2 + q1*q3));
    pitch= atan2(2*(q0*q1-q2*q3), q0*q0-q1*q1-q2*q2+q3*q3); 
    roll = atan2(2*(q0*q3-q1*q2), q0*q0+q1*q1-q2*q2-q3*q3);
    
    euler = [pitch, yaw, roll];
