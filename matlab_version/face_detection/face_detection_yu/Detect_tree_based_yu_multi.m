function [bboxs, shapes] = Detect_tree_based_yu_multi(image)

addpath('../../PDM_helpers');

    map_frontal_17 = [1, 34;
                    2,32;
                    3,36;
                    4,28;
                    5,40;
                    6,37;
                    7,43;
                    8,46;
                    9,52;
                    10,49;
                    11,55;
                    12,58;
                    13,9;
                    14,5;
                    15,1;
                    16,13;
                    17,17 ];

    map_frontal_39_right = [1,36; % nose
                            2,34;
                            3,31;
                            4,30;
                            5,29;
                            6,28;
                            7,48;% left eye
                            8,46;
                            9,46;
                           10,44;
                           11,48;
                           12,23;%brows
                           13,24;
                           14,25;
                           15,26;
                           17,53;% lips
                           18,53;
                           19,55;
                           26,52;
                           30,9; % face outline
                           32,10;
                           33,11;
                           34,12;
                           35,13;
                           36,14;
                           37,15;
                           38,16;
                           39,17;];
                           
    map_frontal_39_left = [1,32; % nose
                            2,34;
                            3,31;
                            4,30;
                            5,29;
                            6,28;
                            7,41;% right eye
                            8,37;
                            9,37;
                           10,39;
                           11,41;
                           12,22;%brows
                           13,21;
                           14,20;
                           15,19;
                           17,51;% lips
                           18,51;
                           19,49;
                           26,52;
                           30,9; % face outline
                           32,8;
                           33,7;
                           34,6;
                           35,5;
                           36,4;
                           37,3;
                           38,2;
                           39,1;];                           
                           
    map_frontal_68 = [68, 17;
               67, 16;
               66, 15;
               65, 14;
               64, 13;
               63, 12;
               62, 11;
               61, 10;
               60, 1;
               59, 2;
               58, 3;
               57, 4;
               56, 5;
               55, 6;
               54, 7;
               53, 8;
               52, 9;% face outline
               16, 18;
               17, 19;
               18, 20;
               19, 21;
               20, 22; % left brow
               31, 23;
               30, 24;
               29, 25;
               28, 26;
               27, 27;% right brow
               9, 28;
               8, 29;
               7, 30;
               6, 31;
               3, 32;
               2, 33;
               1, 34;
               4, 35;
               5, 36;% nose
               15, 37;
               14, 38;
               13, 39;
               10, 40;
               11, 41;
               12, 42; % left eye
               21, 43;
               24, 44;
               25, 45;
               26, 46;
               23, 47;
               22, 48; % right eye
               35, 49;
               34, 50;
               33, 51;
               32, 52;
               39, 53;
               40, 54;
               41, 55;
               44, 56;
               46, 57;
               51, 58;
               48, 59;
               50, 60; % outer lips
               37, 61;
               38, 62;
               43, 63;
               45, 64;
               47, 65;
               49, 66;
               ];
           
S = load('../face_detection_yu/model/model_param.mat');
Model = S.Model;
pc_version = computer();
if(strcmp(pc_version,'PCWIN')) % currently the code just supports windows OS
    addpath('../face_detection_yu/face_detect_32');
    addpath('../face_detection_yu/mex_32');
elseif(strcmp(pc_version, 'PCWIN64'))
    addpath('../face_detection_yu/face_detect_64');
    addpath('../face_detection_yu/mex_64');
end

Model.frontalL = @(X) Select(X, Model.frontal_landmark);
Model.leftL = @(X) Select(X, Model.left_landmark);
Model.rightL = @(X) Select(X, Model.right_landmark);

%%
od = cd('../face_detection_yu/');
[face_shapes, scales] = anchorDetect(image, Model);
cd(od);

shapes = [];
bboxs = [];
for b=1:numel(face_shapes)
    xs = (face_shapes(b).xy(:,1) + face_shapes(b).xy(:,3))/2;
    ys = (face_shapes(b).xy(:,2) + face_shapes(b).xy(:,4))/2;
    
    xs = xs/scales;
    ys = ys/scales;
%     plot(xs, ys, '.b');
     shape = [];
    % Convert this to a 66 point model
    if(numel(xs) == 17)
        shape_sparse = [xs, ys];

        % Map the 2D shape to the PDM
        shape = zeros(66,2);
        shape(map_frontal_17(:,2),:) = shape_sparse(map_frontal_17(:,1),:);

        % The detected sparse Values
        [ ~, ~, ~, ~, ~, ~, shape] = fit_PDM_ortho_proj_to_2D(Model.M, Model.E', Model.V, shape);
    elseif(numel(xs) == 68)
        shape_sparse = [xs, ys];
        shape = zeros(66,2);
        shape(map_frontal_68(:,2),:) = shape_sparse(map_frontal_68(:,1),:);
    elseif(numel(xs) == 39)
        
        shape_sparse = [xs, ys];
        shape = zeros(66,2);
        
        % the face is turned left (from camera point of view) and right
        % from person view
        if(xs(1) < xs(end))            
            shape(map_frontal_39_right(:,2),:) = shape_sparse(map_frontal_39_right(:,1),:);
        else
            shape(map_frontal_39_right(:,2),:) = shape_sparse(map_frontal_39_left(:,1),:);
        end
        [ ~, ~, ~, ~, ~, ~, shape] = fit_PDM_ortho_proj_to_2D(Model.M, Model.E', Model.V, shape);
    end
% TODO mapping for 39 points
    
    if(~isempty(shape))
        bboxs = cat(2, bboxs, [min(shape(:,1)), min(shape(:,2)), max(shape(:,1)), max(shape(:,2))]');
    end
    
    shapes = cat(3, shapes, shape);
%     plot(shape(:,1), shape(:,2), '.r');
    
end


