function [ bbox, shapes ] = Detect_tree_based_zhu( image )
%DETECT_TREE_BASED Summary of this function goes here
%   Detailed explanation goes here

    % Convert the image to three channels as that is expected
    if(size(image,3) == 1)
       image = cat(3, image, image, image); 
    end

    map_frontal = [68, 17;
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

    addpath('../face_detection/face_detection_zhu/face-release1.0-basic/');
    addpath('../PDM_helpers');

    load('../face_detection/face_detection_zhu/face-release1.0-basic/p204-Wild.mat');
    % 5 levels for each octave
    model.interval = 5;
    % set up the threshold
    model.thresh = min(0, model.thresh);
    
    bs = detect(image, model, model.thresh);
    bs = clipboxes(image, bs);
    bs = nms_face(bs,0.3);    

    bbox = [];
    shapes = [];
    if(~isempty(bs))
        for b=1:numel(bs)
            xs = (bs(b).xy(:,1) + bs(b).xy(:,3))/2;
            ys = (bs(b).xy(:,2) + bs(b).xy(:,4))/2;

            lmark_det = [xs, ys];
            shape = zeros(66,2);

            shape(map_frontal(:,2),:) = lmark_det(map_frontal(:,1),:);
            
            minX = min(xs);
            minY = min(ys);
            maxX = max(xs);
            maxY = max(ys);

            shapes = cat(3, shapes, shape);
            
            bbox = cat(2, bbox, [minX, minY, maxX, maxY]');
            
        end
    end
end

