function [ decision ] = face_check_wild( img, shape, global_params )
%FACE_CHECK_WILD Summary of this function goes here
%   Detailed explanation goes here

load '../models/pdm/pdm_68_multi_pie.mat';
load '../models/tri_68.mat';
load '../face_validation_svr/trained/face_check_general_68.mat';

% first need to determine the view

centres = cat(1, faceCheckers.centres);

dists = centres*pi/180 - repmat(global_params(2:4)',size(centres,1),1);
[~,view_id] = min(sum(dists.^2,2));
%     shape = faceCheckers(view_id).source;

img_crop = Crop(img, shape, faceCheckers(view_id).triangulation,...
    faceCheckers(view_id).triX, faceCheckers(view_id).mask,...
    faceCheckers(view_id).alphas, faceCheckers(view_id).betas,...
    faceCheckers(view_id).nPix, faceCheckers(view_id).minX, ...
    faceCheckers(view_id).minY);

img_crop = reshape(img_crop(logical(faceCheckers(view_id).mask)), 1, faceCheckers(view_id).nPix);
img_crop(isnan(img_crop)) = 0;
% normalisation
img_crop = (img_crop - mean(img_crop));
norms = std(img_crop);
if(norms==0)            
    norms = 1;
end
img_crop = img_crop / norms;

% Projection onto principal components
img_crop = img_crop - faceCheckers(view_id).mean_ex;
img_crop = img_crop ./ faceCheckers(view_id).std_ex;

img_crop = img_crop * faceCheckers(view_id).principal_components;

decision = faceCheckers(view_id).b + faceCheckers(view_id).w' * img_crop';