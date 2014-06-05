function sigma_sq = mstep_update_noisevar_md(P, S_bar, V, E_z, E_zz, RO, Tr)
%sigma_sq = mstep_update_noisevar(P, S_bar, V, E_z, E_zz, RO, Tr)

% Updates noise variance (Eq 22)

[K, T] = size(E_z);
J = size(S_bar, 2);

M_t = zeros(2*J, K);

sigma_sq = 0;
for t = 1:T,
   R_t = RO{t};
   
   Sdef = S_bar;
   for kk = 1:K,
      Sdef = Sdef + E_z(kk,t)*V((kk-1)*3+[1:3],:);
      
      M_t(1:J, kk) = (R_t(1,:)*V((kk-1)*3+[1:3],:))'; 
      M_t(J+1:end, kk) = (R_t(2,:)*V((kk-1)*3+[1:3], :))';
   end;
   
   f_bar_t = R_t(1:2,:)*S_bar;
   f_bar_t = [f_bar_t(1,:) f_bar_t(2,:)]';
   
   f_t = [P(t, :) P(t+T, :)]';
   t_vect_t = [Tr(t,1)*ones(J,1); Tr(t,2)*ones(J,1)];
   
   s1 = (f_t - f_bar_t - t_vect_t)'*(f_t - f_bar_t - t_vect_t);
   
   s2 = 2*(f_t - f_bar_t - t_vect_t)'*M_t*E_z(:,t);
   
   s3 = trace(M_t'*M_t*E_zz((t-1)*K+1:t*K,:));
   
   sigma_sq = sigma_sq + (s1 - s2 + s3);
end

sigma_sq = sigma_sq/(2*J*T);

