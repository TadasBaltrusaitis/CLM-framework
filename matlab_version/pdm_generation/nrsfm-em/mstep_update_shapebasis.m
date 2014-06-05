function [newS_bar, newV] = mstep_update_shapebasis(P, E_z, E_zz, RR, Tr, S_bar, V)
%[newS_bar, newV] = mstep_update_shapebasis(P, E_z, E_zz, RR, Tr, S_bar, V)

% Computes an improved version of S_bar and V (Eq 21)

% E_z is KxT, E_zz is a (F*K)xK matrix
% V is (3*K)xJ and S_bar is 3xJ

K = size(E_z,1);
[T, J] = size(P); T = T/2;

Uc = P(1:T, :) - Tr(:,1)*ones(1,J);
Vc = P(T+1:2*T, :) - Tr(:,2)*ones(1,J);

vecH_hat = computeH(Uc, Vc, E_z, E_zz, RR);

H_hat = reshape(vecH_hat, 3*J, K+1);

newS_bar = reshape(H_hat(:,1), 3, J);
newV = zeros(3*K, J);
for kk=1:K,
   newV((kk-1)*3+[1:3],:) = reshape(H_hat(:,kk+1), 3, J);
end
