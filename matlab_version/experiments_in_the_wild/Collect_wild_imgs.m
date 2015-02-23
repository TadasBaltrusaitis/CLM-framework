function [images, detections, labels] = Collect_wild_imgs(root_test_data)
       
    use_afw = true;
    use_lfpw = true;
    use_helen = true;
    use_ibug = true;
    
    use_68 = true;           
    
    images = [];
    labels = [];
    detections = [];
    
    if(use_afw)
        [img, det, lbl] = Collect_AFW(root_test_data, use_68);
        images = cat(1, images, img');
        detections = cat(1, detections, det);
        labels = cat(1, labels, lbl);        
    end
    
    if(use_lfpw)
        [img, det, lbl] = Collect_LFPW(root_test_data, use_68);
        images = cat(1, images, img');
        detections = cat(1, detections, det);
        labels = cat(1, labels, lbl);        
    end    
    
    if(use_ibug)
        [img, det, lbl] = Collect_ibug(root_test_data, use_68);
        images = cat(1, images, img');
        detections = cat(1, detections, det);
        labels = cat(1, labels, lbl);        
    end
    
    if(use_helen)
        [img, det, lbl] = Collect_helen(root_test_data, use_68);
        images = cat(1, images, img');
        detections = cat(1, detections, det);
        labels = cat(1, labels, lbl);        
    end    

    % convert to format expected by the Fitting method
    detections(:,3) = detections(:,1) + detections(:,3);
    detections(:,4) = detections(:,2) + detections(:,4);
    

end


function [images, detections, labels] = Collect_AFW(root_test_data, use_68)
    
dataset_loc = [root_test_data, '/AFW/'];

landmarkLabels = dir([dataset_loc '\*.pts']);

num_imgs = size(landmarkLabels,1);

images = struct;
if(use_68)
    labels = zeros(num_imgs, 68, 2);    
else
    labels = zeros(num_imgs, 66, 2);
end

detections = zeros(num_imgs, 4);

load([root_test_data, '/Bounding Boxes/bounding_boxes_afw.mat']);
num_landmarks = 68;

for imgs = 1:num_imgs

    [~,name,~] = fileparts(landmarkLabels(imgs).name);
        
    landmarks = dlmread([dataset_loc, landmarkLabels(imgs).name], ' ', [3,0,num_landmarks+2,1]);
    
    if(~use_68)
        inds_frontal = [1:60,62:64,66:68];
        landmarks = landmarks(inds_frontal,:);
    end
    
    images(imgs).img = [dataset_loc,  name '.jpg'];
    labels(imgs,:,:) = landmarks;
                   
    detections(imgs,:) = bounding_boxes{imgs}.bb_detector;
    
end

detections(:,3) = detections(:,3) - detections(:,1);
detections(:,4) = detections(:,4) - detections(:,2);

end

function [images, detections, labels] = Collect_LFPW(root_test_data, use_68)
    
dataset_loc = [root_test_data, '/lfpw/testset/'];

landmarkLabels = dir([dataset_loc '\*.pts']);

num_imgs = size(landmarkLabels,1);

images = struct;
if(use_68)
    labels = zeros(num_imgs, 68, 2);    
else
    labels = zeros(num_imgs, 66, 2);
end

detections = zeros(num_imgs, 4);

load([root_test_data, '/Bounding Boxes/bounding_boxes_lfpw_testset.mat']);

num_landmarks = 68;

for imgs = 1:num_imgs

    [~,name,~] = fileparts(landmarkLabels(imgs).name);
        
    landmarks = dlmread([dataset_loc, landmarkLabels(imgs).name], ' ', [3,0,num_landmarks+2,1]);

    if(~use_68)
        inds_frontal = [1:60,62:64,66:68];
        landmarks = landmarks(inds_frontal,:);
    end
    
    images(imgs).img = [dataset_loc,  name '.png'];
    
    labels(imgs,:,:) = landmarks;
    
    detections(imgs,:) = bounding_boxes{imgs}.bb_detector;
    
end

detections(:,3) = detections(:,3) - detections(:,1);
detections(:,4) = detections(:,4) - detections(:,2);


end

function [images, detections, labels] = Collect_ibug(root_test_data, use_68)
    
dataset_loc = [root_test_data, '/ibug/'];

landmarkLabels = dir([dataset_loc '\*.pts']);

num_imgs = size(landmarkLabels,1);

images = struct;

if(use_68)
    labels = zeros(num_imgs, 68, 2);    
else
    labels = zeros(num_imgs, 66, 2);
end

detections = zeros(num_imgs, 4);

load([root_test_data, '/Bounding Boxes/bounding_boxes_ibug.mat']);
num_landmarks = 68;

for imgs = 1:num_imgs

    [~,name,~] = fileparts(landmarkLabels(imgs).name);
        
    landmarks = dlmread([dataset_loc, landmarkLabels(imgs).name], ' ', [3,0,num_landmarks+2,1]);

    if(~use_68)
        inds_frontal = [1:60,62:64,66:68];
        landmarks = landmarks(inds_frontal,:);
    end
    
    images(imgs).img = [dataset_loc,  name '.jpg'];
    
    labels(imgs,:,:) = landmarks;
    
    detections(imgs,:) = bounding_boxes{imgs}.bb_detector;
    
end

detections(:,3) = detections(:,3) - detections(:,1);
detections(:,4) = detections(:,4) - detections(:,2);

end

function [images, detections, labels] = Collect_helen(root_test_data, use_68)
    
dataset_loc = [root_test_data, '/helen/testset/'];

landmarkLabels = dir([dataset_loc '\*.pts']);

num_imgs = size(landmarkLabels,1);

images = struct;

if(use_68)
    labels = zeros(num_imgs, 68, 2);    
else
    labels = zeros(num_imgs, 66, 2);
end

detections = zeros(num_imgs, 4);

load([root_test_data, '/Bounding Boxes/bounding_boxes_helen_testset.mat']);
num_landmarks = 68;
for imgs = 1:num_imgs

    [~,name,~] = fileparts(landmarkLabels(imgs).name);
    
    landmarks = dlmread([dataset_loc, landmarkLabels(imgs).name], ' ', [3,0,num_landmarks+2,1]);
    
    if(~use_68)
        inds_frontal = [1:60,62:64,66:68];
        landmarks = landmarks(inds_frontal,:);
    end
    
    images(imgs).img = [dataset_loc,  name '.jpg'];
    
    labels(imgs,:,:) = landmarks;
    
    detections(imgs,:) = bounding_boxes{imgs}.bb_detector;
    
end

detections(:,3) = detections(:,3) - detections(:,1);
detections(:,4) = detections(:,4) - detections(:,2);

end
