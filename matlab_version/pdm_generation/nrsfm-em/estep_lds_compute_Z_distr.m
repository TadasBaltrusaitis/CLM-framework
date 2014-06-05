function [E_z, E_zz, y, M, xt_n, Pt_n, Ptt1_n, xt_t1, Pt_t1] = sfm_lds_Estep(P, S_bar, V, RR, Tr, phi, mu0, sigma0, Q, sigma_sq)
%[E_z, E_zz, y, M, xt_n, Pt_n, Ptt1_n, xt_t1, Pt_t1] = sfm_lds_Estep(P, S_bar, V, RR, Tr, phi, mu0, sigma0, Q, sigma_sq)

K = size(V,1)/3;
[T, J] = size(P);
T = T/2;

Pc = P - [Tr(:,1); Tr(:,2)]*ones(1,J);

M_t = zeros(2*J, K);
P_hat_t = zeros(2*J, 1);

E_z = zeros(K, T);
E_zz = zeros(T*K, K);

invSigmaSq_p = eye(2*J)./sigma_sq;

M = cell(1, T+1);
M{1} = [];
y = zeros(2*J, T+1);
y(:,1) = 0;
for t=1:T,
   Pt = [P(t,:) P(t+T,:)]';
   
   Rt = [RR(t,:); RR(t+T,:)];
   
   for kk = 1:K,      
      M_t(1:J, kk) = (Rt(1,:)*V(1+(kk-1)*3:kk*3, :))'; 
      M_t(J+1:end, kk) = (Rt(2,:)*V(1+(kk-1)*3:kk*3, :))';            
   end
   P_hat_t(1:J) = (Rt(1,:)*S_bar)'; 
   P_hat_t(J+1:end) = (Rt(2,:)*S_bar)';            
   
   y(:, t+1) = Pt - P_hat_t;
   
   M{t+1} = M_t;  
end

maxIter = 20;
[xt_n, Pt_n, Ptt1_n, xt_t1, Pt_t1] = kalmansmooth(y, M, maxIter, phi, mu0, sigma0, Q, sigma_sq, K, 2*J, T);

E_z = xt_n(:,2:end);
for t=1:T,
   E_zz((t-1)*K+1:t*K,:) = Pt_n{t+1} + E_z(:,t)*E_z(:,t)';
end

