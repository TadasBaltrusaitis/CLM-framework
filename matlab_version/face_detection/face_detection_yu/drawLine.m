% Function: 
%   drawLine(A, visible, c, mark_c, marksize, linwid, style)
%
% Usage:
%   This function draws the landmark as markers and connect the landmarks 
%   with lines if set to. For example, if style is '.-', lines will
%   be drawn to connect landmarks that are defined to be connected.
%
% Params:
%   A - input landmark matrix (66x2), 
%
%   visible - an indicator vector showing whether certain landmark is visible,
%     - (66 x 1) 0/1 vector
%
%   c - the color of line in drawing
%       options: 'r','g','b','c','m','y','k','w',etc
%
%   mark_c - the color of marker (marking each landmark)
%       options: 'r','g','b','c','m','y','k','w',etc
%
%   marksize - the size of marker.
%
%   linwid - the width of line in drawing
%
%   style - marker style
%       options:
%               .     point              -     solid
%               o     circle             :     dotted
%               x     x-mark             -.    dashdot 
%               +     plus               --    dashed   
%               *     star             (none)  no line
%               s     square
%               d     diamond
%               v     triangle (down)
%               ^     triangle (up)
%               <     triangle (left)
%               >     triangle (right)
%               p     pentagram
%               h     hexagram
%
% Return:
%   []
%
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