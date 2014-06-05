function [ axisAngle ] = Rot2AxisAngle( Rot )
%ROT2AXISANGLE Summary of this function goes here
%   Detailed explanation goes here

    theta = acos((trace(Rot) - 1) / 2);

    vec = 1.0/(2*sin(theta));
    vec = vec * [Rot(3,2) - Rot(2,3), Rot(1,3) - Rot(3,1), Rot(2,1) - Rot(1,2)];
    axisAngle = vec * theta;
end

