function [P3, S_bar, V, Z, RO, Tr] = random_nr_motion(T, J, K, state)
% [P3, S_bar, V, Z, RO, Tr] = random_nr_motion(T, J, K, state)
%
%  INPUT:
%
%  T           - number of frames
%  J           - number of points
%  K           - number of deformation basis
%
%  OUTPUT:
%
%  P3          - (3*T) x J 3D-motion matrix:    P3([t t+T t+2*T],:) contains the 3D coordinates of the J points at time t
%  S_bar       - shape average:                 3 x J matrix
%  V           - deformation shapes:            (3*K) x J matrix     ( V((n-1)*3+[1:3],:) contains the n-th deformation basis )
%  Z           - deformation weights:           T x K matrix
%  RO          - rotation:                      cell array           ( RO{t} gives the rotation matrix at time t )
%  Tr          - translation:                   T x 2 matrix

rand('state', state);
randn('state', state);

S_bar = rand(3, J);

[q,r] = qr(rand(3*J));

V = zeros(3*K, J);
for kk=1:K,
   V(1+(kk-1)*3:3*kk, :) = reshape(q(:,kk), 3, J);
end

Z = randn(T,K);
Tr = randn(T,2);

a = (rand(T,1)-0.5)*2*pi;
b = (rand(T,1)-0.5)*2*pi;
c = (rand(T,1)-0.5)*2*pi;
P3 = zeros(3*T, J);
for t=1:T,
   R1 = [1 0 0; 0 cos(a(t)) -sin(a(t)); 0 sin(a(t)) cos(a(t))];
   R2 = [cos(b(t)) 0 sin(b(t)); 0 1 0; -sin(b(t)) 0 cos(b(t))];
   R3 = [cos(c(t)) -sin(c(t)) 0; sin(c(t)) cos(c(t)) 0; 0 0 1];
   
   RO{t} = R1*R2*R3;
   
   Sdef = S_bar;
   for kk = 1:K,
      Sdef = Sdef+Z(t,kk)*V((kk-1)*3+[1:3],:);
   end;   
   Sdef = RO{t}*Sdef +[Tr(t,:)'; 0]*ones(1, J);
   
   P3([t t+T t+2*T], :) = Sdef;
end
