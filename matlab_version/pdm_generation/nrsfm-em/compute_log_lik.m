function loglik = compute_log_lik(P, S_bar, V, E_z, E_zz, RO, Tr, sigma_sq)

[K, T] = size(E_z);
J = size(S_bar, 2);

M_t = zeros(2*J, K);

loglik = - 0.5*T * (2*J*log(sigma_sq));
for t = 1:T,   
   R_t = RO{t};
   
   Sdef = S_bar;
   for kk = 1:K,
      Sdef = Sdef + E_z(kk,t)*V((kk-1)*3+[1:3],:);
      
      M_t(1:J, kk) = (R_t(1,:)*V((kk-1)*3+[1:3],:))'; 
      M_t(J+1:end, kk) = (R_t(2,:)*V((kk-1)*3+[1:3], :))';
   end;
   
   invSigmaSq_p = eye(2*J)./sigma_sq;
   
   f_bar_t = R_t(1:2,:)*S_bar;
   f_bar_t = [f_bar_t(1,:) f_bar_t(2,:)]';
   
   f_t = [P(t, :) P(t+T, :)]';
   t_vect_t = [Tr(t,1)*ones(J,1); Tr(t,2)*ones(J,1)];
      
   covZ_t = E_zz((t-1)*K+1:t*K,:) - E_z(:,t)*E_z(:,t)';      
   loglik = loglik - 0.5*(((f_t-f_bar_t-t_vect_t)./sigma_sq)'*(f_t-f_bar_t-t_vect_t)) + (((f_t-f_bar_t-t_vect_t)'./sigma_sq)*M_t*E_z(:,t)) ...
      - 0.5*trace(((M_t./sigma_sq)'*M_t) * E_zz((t-1)*K+1:t*K,:)) - 0.5*log(det(covZ_t));
end