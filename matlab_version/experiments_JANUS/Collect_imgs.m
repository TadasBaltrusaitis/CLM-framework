function [images, detections, labels, rotations] = Collect_imgs(csv_loc, csv_loc_meta, root_data_loc)
          
    landmarks = csvread(csv_loc, 0, 1);
    
    meta_data = csvread(csv_loc_meta, 0, 5);
    
    rotations = meta_data(:,1:3);
    yaw = rotations(:,3);
    roll = -rotations(:,1);
    pitch = -rotations(:,2);
    rotations = cat(2, pitch, yaw, roll);

    image_locs = readtable(csv_loc, 'ReadVariableNames', false);
    image_locs = table2cell(image_locs(:,1));
    
    num_imgs = size(image_locs,1);

    images = struct;
    labels = zeros(num_imgs, 68, 2);
        
    detections = meta_data(:,5:8);

    for imgs = 1:num_imgs

        images(imgs).img = [root_data_loc,  image_locs{imgs}];
        labels(imgs, :, 1) = landmarks(imgs, 1:2:end);
        labels(imgs, :, 2) = landmarks(imgs, 2:2:end);
        
%         imshow(imread(images(imgs).img));
%         hold on;
%         plot(labels(imgs,:,1), labels(imgs,:,2), '.r');
%         rectangle('Position', [detections(imgs,1), detections(imgs,2), detections(imgs, 3), detections(imgs,4)]);
%         hold off;
    end       
    
end
