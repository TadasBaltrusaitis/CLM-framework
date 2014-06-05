% Function: 
%   demo
%   
% Usage:
%   This function demonstrates how to call the functions we provided to
%   detect facial landmarks and get pose information. In the demo version, 
%   we only choose the largest face ROI detected to further localize its 
%   landmraks. And the current version is only suitable for windows 
%   platform. The matlab version is recommended above R2009a and the C++ 
%   compiler above MS. Visual C++ 9.0. Those versions before are not tested.
%
% Params:
%   None
%
% Return: None
%
% Author: 
%   Xiang Yu, yuxiang03@gmail.com
% 
% Citation:
%   X. Yu, J. Huang, S. Zhang, W. Yan and D.N. Metaxas, Pose-free Facial
%   Landmark Fitting via Optimized Part Mixures and Cascaded Deformable
%   Shape Model. In ICCV, 2013. 
%
% Creation Date: 10/12/2013
%
function demo()
clear all;
close all;
clc;
addpath('.\model');
S = load('model_param.mat');
Model = S.Model;
pc_version = computer();
if(strcmp(pc_version,'PCWIN')) % currently the code just supports windows OS
    addpath('.\face_detect_32');
    addpath('.\mex_32');
elseif(strcmp(pc_version, 'PCWIN64'))
    addpath('.\face_detect_64');
    addpath('.\mex_64');
end

Model.frontalL = @(X) Select(X, Model.frontal_landmark);
Model.leftL = @(X) Select(X, Model.left_landmark);
Model.rightL = @(X) Select(X, Model.right_landmark);


% change your image folder and image name here
img_fold = '.\test\';
img_name = '4.jpg';
img = imread([img_fold,img_name]);

%------------------------------------------------------------------
% 3 types of face alignment input
% (1) no initial landmarks
% [shape, pglobal, visible] = faceAlign(img, Model, []);
%
% (2) initial landmarks
%  l_shape = [xxx...x yyy...yy] input should be set by user
% [shape, pglobal, visible] = faceAlign(img, Model, l_shape);

load('test.mat');
% img = image;
[shape, pglobal, visible] = faceAlign(img, Model, []);

figure,
% imshow(imread([img_fold,img_name]));
imshow(img);
hold on;
if(~isempty(shape))
    % input: shape, visible, line_color, marker color, marker size, line width, style
    drawLine(reshape(shape,Model.nPts,2), visible, 'b', 'g', 5, 2, '.'); 
    hold off;
end

function Y = Select(X, rows) % x,y,z,x,y,z,...
    Y1 = X(3*(rows-1)+1, :);
    Y2 = X(3*(rows-1)+2, :);
    Y3 = X(3*(rows-1)+3, :);
    Y = [Y1; Y2; Y3];