function [ decision ] = face_check_cnn( img, shape, global_params, cnns )
%FACE_CHECK_CNN Summary of this function goes here
%   Detailed explanation goes here

%%
if(size(img,3) == 3)
    img = rgb2gray(img);
end
% first need to determine the view
centres = cat(1, cnns.centres);

dists = centres*pi/180 - repmat(global_params(2:4)',size(centres,1),1);
[~,view_id] = min(sum(dists.^2,2));

mask_small = cnns(view_id).mask(1:size(cnns(view_id).triX,1), 1:size(cnns(view_id).triX,2));

if(size(cnns(view_id).destination,1) == 66 && size(shape,1) == 68)
    label_inds = [1:60,62:64,66:68];
    shape = shape(label_inds,:);
end
img_crop = Crop(img, shape, cnns(view_id).triangulation,...
    cnns(view_id).triX, mask_small,...
    cnns(view_id).alphas, cnns(view_id).betas,...
    cnns(view_id).nPix, cnns(view_id).minX, ...
    cnns(view_id).minY);

%%
img_crop = reshape(img_crop(logical(cnns(view_id).mask)), 1, cnns(view_id).nPix);
img_crop(isnan(img_crop)) = 0;

% normalisation (local)
img_crop = (img_crop - mean(img_crop));
norms = std(img_crop);
if(norms==0)            
    norms = 1;
end
img_crop = img_crop / norms;

% normalisation (global)
img_crop = img_crop - cnns(view_id).mean_ex;
img_crop = img_crop ./ cnns(view_id).std_ex;

mask = cnns(view_id).mask;

% Normalisation to 0-1
% img(mask) = img_crop / 255;
img = zeros(size(mask));
img(mask) = img_crop;
% img = cat(3, img, img);
 % [er, bad] = nntest(nn, test_x, test_y);

 cnn = cnns(view_id).cnn;
%%
cnn = cnnff(cnn, img);

decision =   cnn.o(1);
       
% normalise decision from ~ 0, 1 to [0,3]
decision = decision * 3;