function [phi, Q, sigma_sq, mu0, sigma0] = mstep_lds_update(y, M, xt_n, Pt_n, Ptt1_n)

% Adapted from code written by Hrishi Deshpande

% M-step as described in Shumway, R.H. & Stoffer, D.S. (1982), "An approach to time series smoothing and forecasting using the EM algorithm", 
% Journal of Time Series Analysis, 3, 253-264

q = size(y, 1); 
n = size(y, 2)-1; 
p = size(M{2}, 2);

%- - - - - - - - - -
% Eqns (9)-(11)
A = zeros(p); B = zeros(p); C = zeros(p);
for t=1:n
   tIdx = t+1;
   A = A + Pt_n{tIdx-1} + xt_n(:,tIdx-1) * xt_n(:,tIdx-1)';
   B = B + Ptt1_n{tIdx} + xt_n(:,tIdx) * xt_n(:,tIdx-1)';
   C = C + Pt_n{tIdx} + xt_n(:,tIdx) * xt_n(:,tIdx)';
end

%- - - - - - - - - -
% Eqns (12)-(14)
invA = inv(A);
phi = B * invA;
Q = (C - B*invA*B') / n;
R = zeros(q);
for t=1:n
   tIdx = t+1;
   R = R + (y(:,tIdx)-M{tIdx}*xt_n(:,tIdx)) * (y(:,tIdx)-M{tIdx}*xt_n(:,tIdx))' + ...
      M{tIdx}*Pt_n{tIdx}*M{tIdx}';
end
R = R / n;
R = eye(q)*mean(diag(R)); 
sigma_sq = R(1,1);
mu0 = xt_n(:, 1);            % (Eq 22, Ghahramani 1996)
sigma0 = Pt_n{1};            % (Eq 24, Ghahramani 1996)