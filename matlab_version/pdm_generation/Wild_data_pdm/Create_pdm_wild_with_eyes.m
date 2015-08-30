clear;

addpath('../PDM_helpers/');

%% Create the PDM using PCA, from the recovered 3D data
clear
load('Torr_wild');

% need to still perform procrustes though

x = P3(1:end/3,:);
y = P3(end/3+1:2*end/3,:);

% To make sure that PDM faces the right way (positive Z towards the screen)
z = -P3(2*end/3+1:end,:);

%% Replacing and augmenting eye models with synth
num_samples = size(x, 1);
to_augment_ratio = 0.2;
rng(0);
% Take half of the samples and replace either their left or right eye with
% a synthetic one from an eye PDM but with same alignment and scaling
to_augment = randperm(num_samples);
to_augment = to_augment(1:round(end * to_augment_ratio));

left_eye_inds = [37,38,39,40,41,42];
right_eye_inds = [43,44,45,46,47,48];

load('pdm_l_eye.mat');
M_l_eye = M;
V_l_eye = V;
E_l_eye = E;

load('pdm_r_eye.mat');
M_r_eye = M;
V_r_eye = V;
E_r_eye = E;
num_points = numel(M) / 3;

for i = to_augment

    shape_3D = cat(1, x(i,:), y(i,:), z(i,:))';
    
    eye_shape_l = shape_3D(left_eye_inds, :);
    eye_shape_r = shape_3D(right_eye_inds, :);
    
    [ R_l, T_l ] = AlignShapesWithScale ( reshape(M_l_eye, num_points, 3), eye_shape_l );
    [ R_r, T_r ] = AlignShapesWithScale ( reshape(M_r_eye, num_points, 3), eye_shape_r );
    
    params_left = randn(size(E)) .* sqrt(E);
    params_right = params_left;
    % Third of samples with left and right eye done independently
    if(mod(i,3) == 0)
        params_right = randn(size(E)) .* sqrt(E);
    end
    shape_l_unaligned = M_l_eye + V_l_eye * params_left;
    shape_r_unaligned = M_r_eye + V_r_eye * params_right;
    
    shape_l = (R_l * reshape(shape_l_unaligned, num_points, 3)' + repmat(T_l', 1, num_points))';
    shape_r = (R_r * reshape(shape_r_unaligned, num_points, 3)' + repmat(T_r', 1, num_points))';
    
    shape_3D(left_eye_inds, :) = shape_l;
    shape_3D(right_eye_inds, :) = shape_r;

    x(i,:) = shape_3D(:,1);
    y(i,:) = shape_3D(:,2);
    z(i,:) = shape_3D(:,3);
end

%%

[ normX, normY, normZ, meanShape, Transform ] = ProcrustesAnalysis3D(x,y,z, true);
observations = [normX normY normZ];

[princComp, score, eigenVals] = princomp(observations,'econ');
% Keeping most variation
totalSum = sum(eigenVals);
count = numel(eigenVals);
for i=1:numel(eigenVals)
   if ((sum(eigenVals(1:i)) / totalSum) >= 0.999)
      count = i;
      break;
   end
end

V = princComp(:,1:count);
E = eigenVals(1:count);
M = meanShape(:);

% Now normalise it to have actual world scale
lPupil = [(M(37) + M(40))/2; (M(37 + 68) + M(40 + 68))/2];
rPupil = [(M(43) + M(46))/2; (M(43 + 68) + M(46 + 68))/2];

dist = norm(lPupil - rPupil,2);

% average human interocular distance is 65mm
scaling = 65 / dist;

% normalise the mean values as well
M(1:end/3) = M(1:end/3) - mean(M(1:end/3));
M(end/3+1:2*end/3) = M(end/3+1:2*end/3) - mean(M(end/3+1:2*end/3));
M(2*end/3+1:end) = M(2*end/3+1:end) - mean(M(2*end/3+1:end));

M = M * scaling;
E = E * scaling .^ 2;

% orthonormalise V
scalingV = sqrt(sum(V.^2));
V = V ./ repmat(scalingV, numel(M),1);

E = E .* (scalingV') .^ 2;

%% Also align the model to a Multi-PIE one for accurate head orientation
% also align the mean shape model (an aligned 68 point PDM from Multi-PIE dataset)
M_wild = M;

load('pdm_68_multi_pie.mat', 'M');
M_align = M;

n_points = numel(M)/3;

M_wild = reshape(M_wild, n_points, 3);

% work out the translation and rotation needed
[ R, T ] = AlignShapesKabsch(M_wild, reshape(M_align, n_points,3));

% Transform the wild one to be in the same reference as the Multi-PIE one
M_aligned = (R * M_wild')';

M_aligned(:,1) = M_aligned(:,1) + T(1);
M_aligned(:,2) = M_aligned(:,2) + T(2);
M_aligned(:,3) = M_aligned(:,3) + T(3);

M_aligned = M_aligned(:);
V_aligned = V;

% need to align the principal components as well
for i=1:size(V,2)
    
    V_aligned_pie_curr = (R * reshape(V(:,i), n_points, 3)')';
    V_aligned(:,i) = V_aligned_pie_curr(:);    
    
end

V = V_aligned;
M = M_aligned;

save('pdm_68_aligned_wild_eyes.mat', 'E', 'M', 'V');

writePDM(V, E, M, 'pdm_68_aligned_wild_eyes.txt');

% also save this to model location
if(exist('../../models/pdm/', 'file'))
    save('../../models/pdm/pdm_68_aligned_wild_eyes.mat', 'E', 'M', 'V');
end
