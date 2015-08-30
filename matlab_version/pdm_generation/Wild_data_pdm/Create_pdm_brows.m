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

brow_ids = 18:27;

%%
[ normX, normY, normZ, meanShape, Transform ] = ProcrustesAnalysis3D(x,y,z, true);

normX = normX(:, brow_ids);
normY = normY(:, brow_ids);
normZ = normZ(:, brow_ids);
meanShape = meanShape(brow_ids,:);
% make it zero centered now?
offsets = mean(meanShape);

normX = normX - offsets(1);
normY = normY - offsets(1);
normZ = normZ - offsets(1);

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

load('pdm_68_multi_pie.mat', 'M');
M_full = M;

V = princComp(:,1:count);
E = eigenVals(1:count);
M = meanShape(:);

left_corner = [M(1); M(1+10)];
right_corner = [M(10); M(10+10)];

dist_norm = norm(left_corner - right_corner,2);

left_corner_full = [M_full(18); M_full(18+68)];
right_corner_full = [M_full(27); M_full(27+68)];

dist_norm_full = norm(left_corner_full - right_corner_full,2);

% average human interocular distance is 65mm
scaling = dist_norm_full / dist_norm;

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
M_mouth = M;

load('pdm_68_multi_pie.mat', 'M');
M_align = M;

n_points = numel(M)/3;
M_mouth_full = reshape(M_align, n_points,3);
M_mouth_full = M_mouth_full(brow_ids, :);

n_points_mouth = numel(brow_ids);

M_mouth = reshape(M_mouth, n_points_mouth, 3);

% work out the translation and rotation needed
[ R, T ] = AlignShapesKabsch(M_mouth, M_mouth_full);

% Transform the wild one to be in the same reference as the Multi-PIE one
M_aligned = (R * M_mouth')';

M_aligned = M_aligned(:);
V_aligned = V;

% need to align the principal components as well
for i=1:size(V,2)
    
    V_aligned_pie_curr = (R * reshape(V(:,i), n_points_mouth, 3)')';
    V_aligned(:,i) = V_aligned_pie_curr(:);    
    
end

V = V_aligned;
M = M_aligned;

save('pdm_10_brows.mat', 'E', 'M', 'V');

writePDM(V, E, M, 'pdm_10_brows.txt');

% also save this to model location
if(exist('../../models/pdm/', 'file'))
    save('../../models/pdm/pdm_10_brows.mat', 'E', 'M', 'V');
end
