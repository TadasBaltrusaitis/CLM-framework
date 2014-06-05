function [phiIn, mu0In, sigma0In, QIn] = iit_lds(P, S_bar, V, RR, Tr, sigma_sq)

K = size(V,1)/3;
[T, J] = size(P);
T = T/2;

y = zeros(2*J, T);
P_hat_t = zeros(2*J, 1);
M_t = zeros(2*J, K);

for t=1:T,
   Pt = [P(t,:) P(t+T,:)]';
      
   Rt = [RR(t,:); RR(t+T,:)];
   
   P_hat_t(1:J,1) = (Rt(1,:)*S_bar)'; 
   P_hat_t(J+1:end,1) = (Rt(2,:)*S_bar)';            
   
      
   for kk = 1:K,      
      M_t(1:J, kk) = (Rt(1,:)*V(1+(kk-1)*3:kk*3, :))'; 
      M_t(J+1:end, kk) = (Rt(2,:)*V(1+(kk-1)*3:kk*3, :))';            
   end  
   M{t} = M_t;
      
   y(:, t) = Pt - P_hat_t;  
end

A = eye(2*J)./sigma_sq;
for t=1:T,
   temp1 = A*M{t};
   temp2 = A - temp1*inv(eye(K)+M{t}'*temp1)*temp1';
   temp1b(t,:) = y(:,t)'*temp2*M{t};
end
mu0In = mean(temp1b)';
Q = cov(temp1b);
QIn = Q;
t1 = temp1b(1:T-1,:);
t2 = temp1b(2:T,:);      
phiIn = inv(t1'*t1+Q)*t1'*t2;     
sigma0In = QIn;
