% Random Shapes Demo
%
% Copyright (c) by Lorenzo Torresani, Stanford University
% 
% A demo of Non-Rigid Structure From Motion on artificial random 
% shapes. 
%
%
% The 3D reconstruction technique is based on the following paper:
% 
%  Lorenzo Torresani, Aaron Hertzmann and Christoph Bregler, 
%     Learning Non-Rigid 3D Shape from 2D Motion, NIPS 16, 2003
%  http://cs.stanford.edu/~ltorresa/projects/learning-nr-shape/
% 
%
% Function em_sfm implements the algorithm "EM-Gaussian" and "EM-LDS" described
% in the paper
%
% I recommend that you try to compile the CMEX code for the function computeH:
% type 'mex computeH.c' in the Matlab Command Window ('mex computeH.c -l matlb' under Unix)
%

T = 100;        % number of frames
J = 60;         % number of points
K = 5;          % number of deformation shapes

state = 1000;       % random generator state

% generates a random sequence of non-rigid 3D motion according to
% the deformation model described by Eq (2) and (3)
[P3_gt, S_bar_gt, V_gt, Z_gt, RO_gt, Tr_gt] = random_nr_motion(T, J, K, state);

% 2D motion resulting from orthographic projection (Eq (1))
p2_obs = P3_gt(1:2*T, :);

% removes 40% of the data
missingdata_rate = 0.4;
if missingdata_rate>0,
   rp = randperm(T*J);
   ind_md = rp(1:round(T*J*missingdata_rate));
   MD = zeros(T, J);
   MD(ind_md) = 1;
   [i_md, j_md] = ind2sub([T J], ind_md);
   p2_obs(sub2ind([2*T J], [i_md i_md+T], [j_md j_md])) = 0; % set to 0 the values corresponding to missing data
else
   MD = zeros(T, J);
end

% runs the non-rigid structure from motion algorithm with different number of deformation shapes
max_em_iter = 50;
use_lds = 0; % doesn't use LDS since the data was generated at random, w/o  temporal smoothness
tol = 0.001;
k_values = [K-1:K+1];
Zcoords_gt = P3_gt(2*T+1:3*T,:) - mean(P3_gt(2*T+1:3*T,:),2)*ones(1,J);
Zdist = max(Zcoords_gt,[],2) - min(Zcoords_gt,[],2); % size of the 3D shape along the Z axis for each time frame
ze = [];
fprintf('3D reconstruction with %f missing data...\n', missingdata_rate*100);
for kk=k_values,
   [P3, S_hat, V, RO, Tr, Z] = em_sfm(p2_obs, MD, kk, use_lds, tol, max_em_iter);
      
   % Compares it with ground truth. 
   % Note that there are still 2 ambiguities that cannot be resolved:
   % 1. depth direction (i.e. the shape could be "flipped" along the Z axis) -> we test both possibilities
   % 2. Z translation                                                        -> we subtract the mean of the Z coords to evaluate reconstruction results
   Zcoords_em = P3(2*T+1:3*T,:) - mean(P3(2*T+1:3*T,:),2)*ones(1,J);
   
   Zerror1 = mean( mean(abs(Zcoords_em - Zcoords_gt), 2)./Zdist );
   Zerror2 = mean( mean(abs(-Zcoords_em - Zcoords_gt), 2)./Zdist );
   
   avg_zerror = 100*min(Zerror1, Zerror2);
   
   ze = [ze avg_zerror];
   
   hold off;
   plot(k_values(1:length(ze)), ze, '-o');
   title(['3D reconstruction with ' num2str(missingdata_rate*100) '% missing data'], 'fontweight', 'bold');
   xlabel('K (number of deformation shapes)');
   ylabel('% Z error');
   grid on;
   drawnow;
end


