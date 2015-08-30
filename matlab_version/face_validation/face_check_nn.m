function [ decision ] = face_check_nn( img, shape, global_params, nns )
%FACE_CHECK_NN Summary of this function goes here
%   Detailed explanation goes here

if(size(img,3) == 3)
    img = rgb2gray(img);
end
% first need to determine the view
centres = cat(1, nns.centres);

dists = centres*pi/180 - repmat(global_params(2:4)',size(centres,1),1);
[~,view_id] = min(sum(dists.^2,2));

img_crop = Crop(img, shape, nns(view_id).triangulation,...
    nns(view_id).triX, nns(view_id).mask,...
    nns(view_id).alphas, nns(view_id).betas,...
    nns(view_id).nPix, nns(view_id).minX, ...
    nns(view_id).minY);

%%
img_crop = reshape(img_crop(logical(nns(view_id).mask)), 1, nns(view_id).nPix);
img_crop(isnan(img_crop)) = 0;

%%
% normalisation (local)
img_crop = (img_crop - mean(img_crop));
norms = std(img_crop);
if(norms==0)            
    norms = 1;
end
img_crop = img_crop / norms;

% normalisation (global)
img_crop = img_crop - nns(view_id).mean_ex;
img_crop = img_crop ./ nns(view_id).std_ex;
nn = nns(view_id).nn;
 % [er, bad] = nntest(nn, test_x, test_y);
nn = nnff(nn, img_crop, zeros(size(img_crop,1), nn.size(end)));

decision =  nn.a{end};  
       
%%
% normalise decision from ~ 0, 1 to [0,3]
decision = decision * 3;