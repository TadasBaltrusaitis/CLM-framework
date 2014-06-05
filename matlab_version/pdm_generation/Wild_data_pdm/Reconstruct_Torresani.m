clear

load('wild_68_pts');

%% Perform NRSFM by Torresani 
addpath('../nrsfm-em');
% (T is the number of frames, J is the number of points)

J = size(all_pts,2);
T = size(all_pts,1)/2;

use_lds = 0; % not modeling a linear dynamic system here
max_em_iter = 100;
tol = 0.001;
K = 25; % number of deformation shapes

MD = zeros(T,J);

[P3, S_hat, V, RO, Tr, Z] = em_sfm(all_pts, MD, K, use_lds, tol, max_em_iter);

save('Torr_wild', 'P3', 'S_hat', 'V', 'RO', 'Tr', 'Z');