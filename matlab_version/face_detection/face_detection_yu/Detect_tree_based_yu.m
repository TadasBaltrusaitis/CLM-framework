function [bboxs, shapes] = Detect_tree_based_yu(image)

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
[shape, pglobal, visible] = faceAlign(image, Model, []);

if(~isempty(shape))
    shapes = [shape(1:end/2),shape(end/2+1:end)];
    bboxs = [min(shapes(:,1)), min(shapes(:,2)), max(shapes(:,1)), max(shapes(:,2))]';
else
    shapes = [];
    bboxs = [];
end
