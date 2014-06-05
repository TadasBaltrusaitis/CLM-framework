function [xt_n, Pt_n, Ptt1_n, xt_t1, Pt_t1] = kalmansmooth(y, M, maxIter, phi, mu0, sigma0, Q, sigma_sq, p, q, n)

% Adapted from code written by Hrishi Deshpande

% Based on eqn (A3)-(A12) of Appendix A in Shumway, R.H. & Stoffer, D.S. (1982),
% "An approach to time series smoothing and forecasting using the EM algorithm", Journal of Time Series Analysis, 3, 253-264

%- - - - - - - - - - - - - - - -
% Forward steps. Eqns (A3)-(A7).

xt_t1 = zeros(p, n+1); % t = 0, 1, ..., n
Pt_t1 = cell (1, n+1); % t = 0, 1, ..., n
K     = cell (1, n+1); % t = 0, 1, ..., n
xt_t  = zeros(p, n+1); % t = 0, 1, ..., n
Pt_t  = cell (1, n+1); % t = 0, 1, ..., n

t = 0; tIdx = t+1;
xt_t(:,tIdx) = mu0;
Pt_t{tIdx} = sigma0;
for t = 1:n
   tIdx = t+1;
   
   xt_t1(:,tIdx) = phi*xt_t(:,tIdx-1);                                              % (A3)
   Pt_t1{tIdx}   = phi*Pt_t{tIdx-1}*phi' + Q;                                       % (A4)
   if 1,
      K{tIdx}       = Pt_t1{tIdx}*M{tIdx}' * inv(M{tIdx}*Pt_t1{tIdx}*M{tIdx}' + sigma_sq*eye(q));   % (A5)
   else
      % Using the Matrix Inversion Lemma 
      % (see http://www-2.cs.cmu.edu/afs/cs.cmu.edu/user/zoubin/www/SALD/week5b.pdf)
      invR = eye(q)./sigma_sq;   AA = inv(inv(Pt_t1{tIdx}) + M{tIdx}'*invR*M{tIdx}); BB = (invR - invR*M{tIdx}*AA*M{tIdx}'*invR);
      K{tIdx}       = Pt_t1{tIdx}*M{tIdx}' * BB;                                       % (A5)
   end
   xt_t(:,tIdx)  = xt_t1(:,tIdx) + K{tIdx}*(y(:,tIdx) - M{tIdx}*xt_t1(:,tIdx));     % (A6)
   Pt_t{tIdx}    = Pt_t1{tIdx} - K{tIdx}*M{tIdx}*Pt_t1{tIdx};                       % (A7)    
end


%- - - - - - - - - - - - - - - - 
% Backward steps. Eqns (A8)-(A10)

Jt   = cell (1, n+1); % t = 0, 1, ..., n
xt_n = zeros(p, n+1); % t = 0, 1, ..., n
Pt_n = cell (1, n+1); % t = 0, 1, ..., n

t=n; tIdx = t+1;
xt_n(:,tIdx) = xt_t(:,tIdx); % (A9)
Pt_n{tIdx} = Pt_t{tIdx};     % (A10)

for t=n:-1:1
   tIdx = t+1;
   
   Jt{tIdx-1}     = Pt_t{tIdx-1}*phi'*inv(Pt_t1{tIdx});                                    % (A8)
   xt_n(:,tIdx-1) = xt_t(:,tIdx-1) + Jt{tIdx-1}*(xt_n(:,tIdx) - phi*xt_t(:,tIdx-1));       % (A9)
   Pt_n{tIdx-1}   = Pt_t{tIdx-1} + Jt{tIdx-1} * (Pt_n{tIdx} - Pt_t1{tIdx}) * Jt{tIdx-1}';  % (A10)
end


%- - - - - - - - - - - - - - - -
% Backward steps. Eqns (A11)-(A12)

Ptt1_n = cell(1, n+1); % t = 0, 1, ..., n

t = n; tIdx = t+1;
Ptt1_n{tIdx} = (eye(p) - K{tIdx}*M{tIdx}) * phi * Pt_t{tIdx-1};         % (A12)
for t=n:-1:2
   tIdx = t+1;
   Ptt1_n{tIdx-1} = Pt_t{tIdx-1}*Jt{tIdx-2}' + ...
      Jt{tIdx-1}*(Ptt1_n{tIdx} - phi*Pt_t{tIdx-1})*Jt{tIdx-2}';       % (A11)
end
