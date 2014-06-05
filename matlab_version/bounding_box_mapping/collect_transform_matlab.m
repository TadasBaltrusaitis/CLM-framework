root_test_data = '../../test data/';
[images,dets,gt_labels] = Collect_wild_imgs(root_test_data);

%%

% The offsets and detector bounding boxes
bboxes = zeros(numel(images), 6);

for i=1:numel(images)
   
    image = imread(images(i).img);
    
    [rows, cols, ~] = size(image);
    
    % zone in on a smaller version of the image
    
    zoom_bbox = dets(i,:);

    width = zoom_bbox(3) - zoom_bbox(1);
    height = zoom_bbox(4) - zoom_bbox(2);
    
    zoom_bbox(1) = zoom_bbox(1) - width/2;
    zoom_bbox(2) = zoom_bbox(2) - height/2;
    
    zoom_bbox(3) = zoom_bbox(1) + 2 * width;
    zoom_bbox(4) = zoom_bbox(2) + 2 * height;
    
    zoom_bbox(zoom_bbox < 1) = 1;
    if(zoom_bbox(3) > cols)
        zoom_bbox(3) = cols;
    end
    if(zoom_bbox(4) > rows)
        zoom_bbox(4) = rows;
    end
        
    zoom_bbox = round(zoom_bbox);
    image_zoom = image(zoom_bbox(2):zoom_bbox(4), zoom_bbox(1):zoom_bbox(3),:);

    % The actual face detection
    face_detector = vision.CascadeObjectDetector();

    bbox = step(face_detector, image_zoom);    
    
    if(~isempty(bbox))
        bboxes(i,:) = [zoom_bbox(1), zoom_bbox(2), bbox(1), bbox(2), bbox(3), bbox(4)];
    end
    release(face_detector);
end

save('matlab.mat', 'bboxes', 'dets', 'gt_labels');