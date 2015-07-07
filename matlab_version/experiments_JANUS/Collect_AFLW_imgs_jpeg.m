function [images, detections, labels, rotations, jpeg_quals] = Collect_AFLW_imgs_jpeg(csv_loc, csv_loc_meta, root_data_loc)
          
    landmarks = csvread(csv_loc, 0, 1);
    
    meta_data = csvread(csv_loc_meta, 0, 5);
    
    rotations = meta_data(:,1:3);
    yaw = rotations(:,3);
    roll = -rotations(:,1);
    pitch = -rotations(:,2);
    rotations = cat(2, pitch, yaw, roll);

    image_locs = readtable(csv_loc, 'ReadVariableNames', false);
    image_locs = table2cell(image_locs(:,1));
    
    jpeg_dirs = dir([root_data_loc, 'jpeg_q_*']);
    
    jpeg_quals = zeros(numel(jpeg_dirs), 1);
    
    num_jpeg_qual = numel(jpeg_dirs);
    
    for i = 1:num_jpeg_qual
        jpeg_quals(i) = str2num(jpeg_dirs(i).name(8:end));
    end
    
    num_imgs = size(image_locs,1);

    to_use = false(size(image_locs,1),1);
    labels = zeros(num_imgs, 68, 2);
        
    detections = meta_data(:,5:8);

    images = cell(num_imgs, num_jpeg_qual);
    
    for imgs = 1:num_imgs

        labels(imgs, :, 1) = landmarks(imgs, 1:2:end);
        labels(imgs, :, 2) = landmarks(imgs, 2:2:end);

        [~, name, ext] = fileparts(image_locs{imgs});
        if(exist([root_data_loc, '/', jpeg_dirs(1).name, '/', name, ext], 'file'))
            to_use(imgs) = true;
            for i=1:num_jpeg_qual
                images{imgs, i} = [root_data_loc, '/', jpeg_dirs(i).name, '/', name, ext];
    %         imshow(imread(images(imgs).img));
    %         hold on;
    %         plot(labels(imgs,:,1), labels(imgs,:,2), '.r');
    %         rectangle('Position', [detections(imgs,1), detections(imgs,2), detections(imgs, 3), detections(imgs,4)]);
    %         hold off;
                
            end
        end                
    end       
    labels = labels(to_use,:,:);
    detections = detections(to_use,:);
    rotations = rotations(to_use,:);
    images = images(to_use,:);
end
