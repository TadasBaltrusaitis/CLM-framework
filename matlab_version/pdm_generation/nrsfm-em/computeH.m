function vecH_hat = computeH(Xc, Yc, E_z, E_zz, RR)
%vecH_hat = computeH(Xc, Yc, E_z, E_zz, RR)

fprintf('You are running the Matlab version of function ''computeH''. This program will run very slowly.... \nI recommend that you try to compile the CMEX code on your platform using command ''mex computeH.c'' (''mex computeH.c -l matlb'' under Unix)\n\n');

K = size(E_z,1);
J = size(Xc,2);
T = size(Xc,1);

KK2 = zeros(3*J*(K+1), 3*J*(K+1));
KK3 = zeros(3*J, K+1);

for t=1:T
   P_t = [Xc(t,:); Yc(t,:)];
        
   zz_hat_t = [1 E_z(:,t)'; E_z(:,t) E_zz((t-1)*K+1:t*K,:)];
   
   R_t = [RR(t, :); RR(t+T, :)];
      
   KK1 = kron(speye(J), R_t);
   
   KK2 = KK2 + kron(zz_hat_t', KK1'*KK1);
   
   KK3 = KK3 + KK1'* P_t(:) * [1 E_z(:,t)'];
end

vecH_hat = pinv(KK2) * KK3(:);
