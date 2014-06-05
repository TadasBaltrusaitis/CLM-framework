% Shark Demo
%
% Copyright (c) by Lorenzo Torresani, Stanford University
% 
% A demo of Non-Rigid Structure From Motion on artificial shark sequence
%
%
% The 3D reconstruction technique is based on the following paper:
% 
%  Lorenzo Torresani, Aaron Hertzmann and Christoph Bregler, 
%     Learning Non-Rigid 3D Shape from 2D Motion, NIPS 16, 2003
%  http://cs.stanford.edu/~ltorresa/projects/learning-nr-shape/
% 
%
% Function em_sfm implements the algorithms "EM-Gaussian" and "EM-LDS" described
% in the paper
%
% I recommend that you try to compile the CMEX code for the function computeH:
% type 'mex computeH.c' in the Matlab Command Window ('mex computeH.c -l matlb' under Unix)
%

% loads the matrix P3_gt containing the ground thruth data: P3_gt([t t+T t+2*T],:) contains the 3D coordinates of the J points at time t
% (T is the number of frames, J is the number of points)
load('jaws.mat');
[T, J] = size(P3_gt); T = T/3;

% 2D motion resulting from orthographic projection (Eq (1))
p2_obs = P3_gt(1:2*T, :);

% runs the non-rigid structure from motion algorithm
use_lds = 1;
max_em_iter = 60;
tol = 0.0001;
K = 2; % number of deformation shapes
Zcoords_gt = P3_gt(2*T+1:3*T,:) - mean(P3_gt(2*T+1:3*T,:),2)*ones(1,J);
Zdist = max(Zcoords_gt,[],2) - min(Zcoords_gt,[],2); % size of the 3D shape along the Z axis for each time frame
MD = zeros(T,J);

[P3, S_hat, V, RO, Tr, Z] = em_sfm(p2_obs, MD, K, use_lds, tol, max_em_iter);

%% Compares it with ground truth. 
% Note that there are still 2 unresolvable ambiguities:
% 1. depth direction (i.e. the shape could be "flipped" along the Z axis) -> we test both possibilities
% 2. Z translation                                                        -> we subtract the mean of the Z coords to evaluate reconstruction results
Zcoords_em = P3(2*T+1:3*T,:) - mean(P3(2*T+1:3*T,:),2)*ones(1,J);

Zerror1 = mean( mean(abs(Zcoords_em - Zcoords_gt), 2)./Zdist );
Zerror2 = mean( mean(abs(-Zcoords_em - Zcoords_gt), 2)./Zdist );

if Zerror2 < Zerror1,
   avg_zerror = 100*Zerror2;
   P3(2*T+1:3*T,:) = -(P3(2*T+1:3*T,:) - mean(P3(2*T+1:3*T,:),2)*ones(1,J));
else
   avg_zerror = 100*Zerror1;
   P3(2*T+1:3*T,:) = P3(2*T+1:3*T,:) - mean(P3(2*T+1:3*T,:),2)*ones(1,J);
end
fprintf('Average reconstruction error in Z: %f%%\n', avg_zerror);

vis_reconstruction(P3_gt, P3);

