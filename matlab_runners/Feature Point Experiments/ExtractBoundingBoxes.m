% Replace with the location you download the datasets to
data_dir = [getenv('USERPROFILE') '/Dropbox/AAM/test data/'];

% location of datasets, the datasets are acquired from http://ibug.doc.ic.ac.uk/resources/300-W/
dataset_locs = { [data_dir, '/AFW/'];
                 [data_dir, '/helen/trainset/'];
                 [data_dir, '/helen/testset/'];
                 [data_dir, '/ibug/'];
                 [data_dir, 'lfpw/testset/'];
                 [data_dir, '/lfpw/trainset/'];};
                
bb_locs = {[data_dir, '/Bounding Boxes/bounding_boxes_afw.mat'];
           [data_dir, '/Bounding Boxes/bounding_boxes_helen_trainset.mat'];
           [data_dir, '/Bounding Boxes/bounding_boxes_helen_testset.mat'];
           [data_dir, '/Bounding Boxes/bounding_boxes_ibug.mat'];
           [data_dir, '/Bounding Boxes/bounding_boxes_lfpw_testset.mat'];
           [data_dir, '/Bounding Boxes/bounding_boxes_lfpw_trainset.mat'];};
       
for d=1:numel(dataset_locs)
    
    load(bb_locs{d});
    
    for i=1:numel(bounding_boxes)
        img_name = bounding_boxes{i}.imgName;
        [~, name, ~] = fileparts(img_name);
%         image = imread([dataset_locs{d}, img_name]);
%         imshow(image);
        bbox = bounding_boxes{i}.bb_detector;
        
        f = fopen([dataset_locs{d}, name, '.txt'], 'w');
        
        fprintf(f, '%f %f %f %f\r\n', bbox(1), bbox(2), bbox(3), bbox(4));
        fclose(f);
        
%         rectangle('Position', [bbox(1), bbox(2), bbox(3)-bbox(1), bbox(4)-bbox(2)]);
%         drawnow expose
%         pause(0);
    end
    
end