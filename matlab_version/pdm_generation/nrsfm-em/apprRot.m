function R = apprRot(Ra)
%R = apprRot(Ra)

i1 = 0.5; i2 = 0.5;
U = Ra(1,:);
V = Ra(2,:);
un = norm(U);
vn = norm(V);
Un = U/un;
Vn = V/vn;

vp = Un*Vn';
up = Vn*Un';

Vc = Vn-vp*Un;  Vc = Vc/norm(Vc);
Uc = Un-up*Vn;  Uc = Uc/norm(Uc);

Ua = i1*Un+i2*Uc; Ua = Ua/norm(Ua); 
Va = i1*Vn+i2*Vc; Va = Va/norm(Va);



R = [Ua;Va;cross(Ua,Va)];
if det(R)<0, R(3,:) = -R(3,:); end;


