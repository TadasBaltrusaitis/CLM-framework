function [newRO, newRR] = mstep_update_rotation(P, S_bar, V, E_z, E_zz, RO, Tr)
%[newRO, newRR] = mstep_update_rotation(P, S_bar, V, E_z, E_zz, RO, Tr)

% Linearizes the expression in Eq 24 using exponential maps and 
% solves for an improved rotation

% update step
tw_step = 0.3;

[K, T] = size(E_z);
J = size(S_bar, 2);

Pc = P - Tr(:)*ones(1,J);

newRR = zeros(2*T,3);
newRO = RO;

for iter=1:1,   
   for t = 1:T,    
      A = zeros(3);
      B = zeros(2,3);
      
      zz_hat_t = [1 E_z(:,t)'; E_z(:,t) E_zz((t-1)*K+1:t*K,:)];
      
      for j=1:J,
         H_j = [S_bar(:,j) reshape(V(:,j), 3, K)];
         
         A = A + H_j*zz_hat_t*H_j';
         
         B = B + ([Pc(t,j); Pc(t+T,j)] * [1 E_z(:,t)'] * H_j');
      end
      
      oldRO_t = RO{t};
      
      C = oldRO_t*A;
      D = B - oldRO_t(1:2,:)*A;
      
      % now we solve the system: [1 0 0; 0 1 0]*twist*C = D  
      CC = [0   C(3,1) -C(2,1)
         -C(3,1) 0 C(1,1)
         0   C(3,2) -C(2,2)
         -C(3,2) 0 C(1,2)
         0   C(3,3) -C(2,3)
         -C(3,3) 0 C(1,3)];
      
      DD = D(:);
      
      % twist optimization   
      twist_vect = tw_step*pinv(CC)*DD;
      
      twh = [0      -twist_vect(3) twist_vect(2)
         twist_vect(3)  0      -twist_vect(1)
         -twist_vect(2) twist_vect(1)  0     ];
      dR = expm(twh);
      newRO_t = dR*oldRO_t;
      
      newRO{t} = newRO_t;
      newRR(t,:) = newRO_t(1,:);
      newRR(t+T,:) = newRO_t(2,:);     
   end
   
   RO = newRO;
end