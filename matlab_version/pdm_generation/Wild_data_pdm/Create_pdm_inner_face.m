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

inner_ids = 18:68;

%%
[ normX, normY, normZ, meanShape, Transform ] = ProcrustesAnalysis3D(x,y,z, true);

normX = normX(:, inner_ids);
normY = normY(:, inner_ids);
normZ = normZ(:, inner_ids);
meanShape = meanShape(inner_ids,:);
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

% Now normalise it to have actual world scale
lPupil = [(M(20) + M(23))/2; (M(20 + 51) + M(23 + 51))/2];
rPupil = [(M(26) + M(29))/2; (M(26 + 51) + M(29 + 51))/2];

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
M_inner = M;

load('pdm_68_multi_pie.mat', 'M');
M_align = M;

n_points = numel(M)/3;
M_inner_full = reshape(M_align, n_points,3);
M_inner_full = M_inner_full(inner_ids, :);

n_points_mouth = numel(inner_ids);

M_inner = reshape(M_inner, n_points_mouth, 3);

% work out the translation and rotation needed
[ R, T ] = AlignShapesKabsch(M_inner, M_inner_full);

% Transform the wild one to be in the same reference as the Multi-PIE one
M_aligned = (R * M_inner')';

M_aligned = M_aligned(:);
V_aligned = V;

% need to align the principal components as well
for i=1:size(V,2)
    
    V_aligned_pie_curr = (R * reshape(V(:,i), n_points_mouth, 3)')';
    V_aligned(:,i) = V_aligned_pie_curr(:);    
    
end

V = V_aligned;
M = M_aligned;

save('pdm_51_inner.mat', 'E', 'M', 'V');

writePDM(V, E, M, 'pdm_51_inner.txt');

% also save this to model location
if(exist('../../models/pdm/', 'file'))
    save('../../models/pdm/pdm_51_inner.mat', 'E', 'M', 'V');
end
