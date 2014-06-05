function [E_z, E_zz] = estep_compute_Z_distr(P, S_bar, V, RR, Tr, sigma_sq)
%[E_z, E_zz] = estep_compute_Z_distr(P, S_bar, V, RR, Tr, sigma_sq)

% Computes the distribution over Z given the current parameter estimates (see Eq 17-18)

K = size(V,1)/3;
[T, J] = size(P);
T = T/2;

Pc = P - Tr(:)*ones(1,J);

M_t = zeros(2*J, K);
P_hat_t = zeros(2*J, 1);

E_z = zeros(K, T);
E_zz = zeros(T*K, K);

invSigmaSq_p = eye(2*J)./sigma_sq;
for t=1:T,
   R_t = [RR(t,:); RR(t+T,:)];
   
   for kk = 1:K,      
      M_t(1:J, kk) = (R_t(1,:)*V(1+(kk-1)*3:kk*3, :))'; 
      M_t(J+1:end, kk) = (R_t(2,:)*V(1+(kk-1)*3:kk*3, :))';            
   end
   P_hat_t(1:J) = (R_t(1,:)*S_bar)'; 
   P_hat_t(J+1:end) = (R_t(2,:)*S_bar)';            
   
   %beta_t = M_t' * inv(M_t*M_t' + sigma_sq*eye(2*J));                      % (Eq 16)
   % Can be computed much more efficiently using the matrix inversion lemma:   
   AA = M_t./sigma_sq; 
   beta_t = M_t'*(invSigmaSq_p - AA*inv(eye(K) + M_t'*M_t./sigma_sq)*AA');   % (Eq 16)
   
   E_z(:, t) = beta_t*([Pc(t, :) Pc(t+T, :)]' - P_hat_t);                    % (Eq 17)
   E_zz((t-1)*K+1:t*K,:) = eye(K) - beta_t*M_t + E_z(:,t)*E_z(:,t)';         % (Eq 18)
end

