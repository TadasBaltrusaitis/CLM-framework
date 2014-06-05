function P_hat = mstep_update_missingdata(P, MD, S_bar, V, E_z, RO, Tr)
% P_hat = mstep_update_missingdata(P, MD, S_bar, V, E_z, RO, Tr)

% Fills in missing data using Eq 25

[K, T] = size(E_z);
J = size(S_bar, 2);

ind = find(sum(MD')>0);

P_hat = P;
for kk = 1:length(ind),
   
   t = ind(kk);
   
   missingpoints_t = find(MD(t, :));
      
   for s=1:length(missingpoints_t),
      j = missingpoints_t(s);
      
      H_j = [S_bar(:,j) reshape(V(:,j), 3, K)]; % H_j is 3x(K+1)
      
      S_tj = H_j*[1; E_z(:,t)]; % S_tj is 3x1
      
      newf_t = RO{t}(1:2,:) * S_tj + Tr(t,:)';
      
      P_hat([t t+T], j) = newf_t;
   end
end

