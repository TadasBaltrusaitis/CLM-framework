function [V, Z] = init_SB(P, Tr, RR, S_bar, K)
%[V, Z] = init_SB(P, Tr, RR, S_bar, K)

[T, J] = size(P);
T = T/2;

V = zeros(3*K, J);
Z = zeros(T, K);

W_tilda = P - RR*S_bar - [Tr(:,1); Tr(:,2)]*ones(1,J);
for kk=1:K, % iterates over deformations
   V_kk = zeros(T, 3*J);
   for t=1:T,
       try
      V_kk_t = pinv(RR([t t+T], :))*W_tilda([t t+T], :);
       catch err
          fprintf('Wrong at %d\n', t); 
       end
      V_kk(t,:) = V_kk_t(:)';
   end
   
   [a,b,c] = svd(V_kk, 0);
   sqrtb = sqrt(b(1,1));
   
   Z(:, kk) = a(:,1) * sqrtb;
   new_V_kk = sqrtb * c(:,1)';
   
   V((kk-1)*3+1:kk*3, :) = reshape(new_V_kk, 3, J);
   
   for t = 1:T,       
      W_tilda([t t+T], :) = W_tilda([t t+T], :) - RR([t t+T], :)*(Z(t,kk)*V((kk-1)*3+1:kk*3, :));
   end   
end
