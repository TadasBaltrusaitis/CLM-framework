% Function: 
%   [faceAnchor, scale] = anchorDetect(img, Model)
%
% Usage:
%   This function uses Optimized Part Mixture model to detect anchor points for
%   facial landmark initialization
% Params:
%   img - input facial image, 
%
%   Model - the same structure as in demo.m
%
% Output:
%
%   faceAnchor: structure {s,c,xy,level} s-scale, c-cluster (1,2,...13), 
%               xy-coordinate structure (x,y,width, height), level: feature pyramid level (not used here)
%
%   scale: the image is automatically scaled to best fitted size. To get original anchor points, apply faceAnchor(i).xy / scale
%
%
% Authors: 
%   Xiang Yu, yuxiang03@gmail.com
%
% Citation: 
%   X. Yu, J. Huang, S. Zhang, W. Yan and D.N. Metaxas, Pose-free Facial
%   Landmark Fitting via Optimized Part Mixures and Cascaded Deformable
%   Shape Model. In ICCV, 2013. 
%
% Creation Date: 01/17/2014
%