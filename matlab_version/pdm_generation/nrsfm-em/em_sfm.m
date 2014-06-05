function [P3, S_bar, V, RO, Tr, Z, sigma_sq, phi, Q, mu0, sigma0] = em_sfm(P, MD, K, use_lds, tol, max_em_iter)

%  Non-Rigid Structure From Motion with Gaussian/LDS Deformation Model
%  Copyright (c) by Lorenzo Torresani, Stanford University
% 
%  Based on the following paper:
% 
%  Lorenzo Torresani, Aaron Hertzmann and Christoph Bregler, 
%     Learning Non-Rigid 3D Shape from 2D Motion, NIPS 16, 2003
%  http://cs.stanford.edu/~ltorresa/projects/learning-nr-shape/
%
%  Please refer to this publication if you use this program for 
%  research or for technical applications. 
%
%
%  INPUT:
%
%  P           - (2*T) x J tracking matrix:          P([t t+T],:) contains the 2D projections of the J points at time t
%  MD          - T x J missing data binary matrix:   MD(t, j)=1 if no valid data is available for point j at time t, 0 otherwise
%  K           - number of deformation basis
%  use_lds     - set to 1 to model deformations using a linear dynamical system; set to 0 otherwise
%  tol         - termination tolerance (proportional change in likelihood)
%  max_em_iter - maximum number of EM iterations 
%
%
%  OUTPUT:
%
%  P3          - (3*T) x J 3D-motion matrix:                    ( P3([t t+T t+2*T],:) contains the 3D coordinates of the J points at time t )
%  S_bar       - shape average:            3 x J matrix
%  V           - deformation shapes:       (3*K) x J matrix     ( V((n-1)*3+[1:3],:) contains the n-th deformation basis )
%  RO          - rotation:                 cell array           ( RO{t} gives the rotation matrix at time t )
%  Tr          - translation:              T x 2 matrix
%  Z           - deformation weights:      T x K matrix
%  sigma_sq    - variance of the noise in feature position
%  phi         - LDS transition matrix
%  Q           - LDS state noise matrix
%  mu0         - initial state mean
%  sigma0      - initial state variance

if mod(size(P,1), 1) ~= 0,
   fprintf('Error: size(P) must be (2*T)xJ\n');
   return;
end
if (size(P,1)/2 ~= size(MD,1)) | (size(P,2) ~= size(MD,2))
   fprintf('Error: Size incompatibility between P and MD\n');
   return;
end
if mod(K, 1) ~= 0,
   fprintf('Error: K must be an integer value\n');
   return;
end

[T, J] = size(MD);
r = 3*(K + 1); % motion rank

P_hat = P; % if any of the points are missing, P_hat will be updated during the M-step

% uses rank 3 factorization to get a first initialization for rotation and S_bar
[R_init, Trvect, S_bar] = rigidfac(P_hat, MD);

Tr(:,1) = Trvect(1:T);
Tr(:,2) = Trvect(T+1:2*T);

R = zeros(2*T, 3);
% enforces rotation constraints
for t = 1:T,
   Ru = R_init(t,:);
   Rv = R_init(T+t,:);
   Rz = cross(Ru,Rv); if det([Ru;Rv;Rz])<0, Rz = -Rz; end;
   RO_approx = apprRot([Ru;Rv;Rz]);
   RO{t} = RO_approx;
   R(t,:) = RO_approx(1,:);
   R(t+T,:) = RO_approx(2,:);
end;

% given the initial estimates of rotation, translation and shape average, it initializes 
% deformation shapes and weights through LSQ minimization of the reprojection error
[V, Z] = init_SB(P_hat, Tr, R, S_bar, K);

% initializes sigma_sq
E_zz_init = cov(Z);
E_zz_init = repmat(E_zz_init, T, 1);
sigma_sq = mstep_update_noisevar(P_hat, S_bar, V, Z', E_zz_init, RO, Tr);

if use_lds,
   [phi, mu0, sigma0, Q] = init_lds(P_hat, S_bar, V, R, Tr, sigma_sq);
else
   phi = [];
   mu0 = [];
   sigma0 = [];
   Q = [];
end
   
loglik = 0;
annealing_const = 60;
max_anneal_iter = round(max_em_iter/2);

for em_iter=1:max_em_iter,   
   if use_lds,
      [E_z, E_zz, y, M, xt_n, Pt_n, Ptt1_n, xt_t1, Pt_t1] = estep_lds_compute_Z_distr(P_hat, S_bar, V, R, Tr, phi, mu0, sigma0, Q, sigma_sq);
      
      [phi, Q, sigma_sq, mu0, sigma0] = mstep_lds_update(y, M, xt_n, Pt_n, Ptt1_n);
   else
      % computes the hidden variables distributions
      [E_z, E_zz] = estep_compute_Z_distr(P_hat, S_bar, V, R, Tr, sigma_sq);     % (Eq 17-18)
   end
   Z = E_z';
      
   % updates shape basis
   [S_bar, V] = mstep_update_shapebasis(P_hat, E_z, E_zz, R, Tr, S_bar, V);   % (Eq 21)
      
   % fills in missing points
   if sum(MD(:))>0,
      P_hat = mstep_update_missingdata(P_hat, MD, S_bar, V, E_z, RO, Tr);     % (Eq 25)
   end
      
   % updates rotation
   [RO, R] = mstep_update_rotation(P_hat, S_bar, V, E_z, E_zz, RO, Tr);       % (Eq 24)
      
   % updates translation
   Tr = mstep_update_transl(P_hat, S_bar, V, E_z, RO);                        % (Eq 23)
      
   if ~use_lds,
      % updates noise variance
      sigma_sq = mstep_update_noisevar(P_hat, S_bar, V, E_z, E_zz, RO, Tr);      % (Eq 22)   
      if em_iter < max_anneal_iter,
         sigma_sq = sigma_sq * (1 + annealing_const*(1 - em_iter/max_anneal_iter));
      end
      
      oldloglik = loglik;
      % computes log likelihood
      loglik = compute_log_lik(P_hat, S_bar, V, E_z, E_zz, RO, Tr, sigma_sq);   
      
      fprintf('LogLik(%d): %f\n', em_iter, loglik);   
      
      if (em_iter <= 2),
         loglikbase = loglik;
      elseif (loglik < oldloglik)
         fprintf('Violation');
%          keyboard;
      elseif 0 & ((loglik-loglikbase)<(1 + tol)*(oldloglik-loglikbase)),
         fprintf('\n');
         break;
      end   
   else
      fprintf('Iteration %d/%d\n', em_iter, max_em_iter);
   end
end

P3 = zeros(3*T, J);
for t = 1:T,
   z_t = Z(t,:);
   Rf = [R(t,:); R(t+T,:)];
   S = S_bar;
   for kk = 1:K,
      S = S+z_t(kk)*V((kk-1)*3+[1:3],:);
   end;
   S = RO{t}*S;
   
   P3([t t+T t+2*T], :) = S + [Tr(t, [1 2]) -mean(S(3,:))]'*ones(1,J);
end
