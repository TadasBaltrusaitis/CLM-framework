function [Rot] = Euler2Rot(euler)

	rx = euler(1);
	ry = euler(2);
	rz = euler(3);

	Rx = [1 0 0; 0 cos(rx) -sin(rx); 0 sin(rx) cos(rx)];
	Ry = [cos(ry) 0 sin(ry); 0 1 0; -sin(ry) 0 cos(ry)];
	Rz = [cos(rz) -sin(rz) 0; sin(rz) cos(rz) 0; 0 0 1];
	
	Rot = Rx * Ry * Rz;
