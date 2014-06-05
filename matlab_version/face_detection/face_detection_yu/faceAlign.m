% Function: 
%   [shape, pglobal, visible] = faceAlign(img, Model, l_shape)
%
% Usage:
%   This function localizes facial landmarks in a facial image or tracks
%   the facial landmarks in an image sequence. For tracking, each frame of
%   a video is sent as an input image and the tracked facial landmarks in
%   the previous frame is sent as initial landmarks for the current frame.
%   The outputs are well-aligned landmark coordinates, head pose
%   information and the visibility of each landmark.
%
% Params:
%   img - input image matrix, 
%   Model - pre-trained face model and parameters,
%   l_shape - initial landmark positions
%       - (132 x 1) matrix of which the first 66 are x coordinates 
%       and the following 66 are y coordinates.
%       - if l_shape = [], the function calls our initialization procedure,
%           which takes up the most time.
%       - if l_shape ~= [], the function directly calls fitting procedure which
%           is guaranteed real-time even with matlab. Thus tracking is promising
%           with this code.
%
% Return:
%   shape - predicted landmarks 
%       - (132 x 1) matrix of which the first 66 are x coordinates 
%       and the following 66 are y coordinates.
%       - [] if no face detected
%   pglobal - head pose information (1x6)
%       (1) - scale
%       (2) - pitch
%       (3) - yaw
%       (4) - roll
%       (5) - shift of x coordinates from reference shape
%       (6) - shift of y coordinates from reference shape
%   visible - indicator vector for each landmarks' visibility
%       - (66 x 1) 0/1 elements
% Authors: 
%   Xiang Yu, yuxiang03@gmail.com
%
% Citation: 
%   X. Yu, J. Huang, S. Zhang, W. Yan and D.N. Metaxas, Pose-free Facial
%   Landmark Fitting via Optimized Part Mixures and Cascaded Deformable
%   Shape Model. In ICCV, 2013. 
%
% Creation Date: 10/12/2013
%