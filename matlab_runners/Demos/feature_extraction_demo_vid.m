clm_exe = '"../../x64/Release/FeatureExtraction.exe"';

output = './output_features_vid/';

if(~exist(output, 'file'))
    mkdir(output)
end
    
in_files = dir('../../videos/1815_01_008_tony_blair.avi');
% some parameters
verbose = true;

command = clm_exe;
command = cat(2, command, ' -rigid ');

% Remove for a speedup
command = cat(2, command, ' -verbose ');

% add all videos to single argument list (so as not to load the model anew
% for every video)
for i=1:numel(in_files)
    
    inputFile = ['../../videos/', in_files(i).name];
    [~, name, ~] = fileparts(inputFile);
    
    % where to output tracking results
    outputFile = [output name '.txt'];
            
    if(~exist([output name], 'file'))
        mkdir([output name]);
    end
    
    outputDir_aligned = [output name];
    
    outputHOG_aligned = [output name '.hog'];
    
    output_shape_params = [output name '.params.txt'];
    
    command = cat(2, command, [' -f "' inputFile '" -of "' outputFile '"']);        
    command = cat(2, command, [' -simalign "' outputDir_aligned '" -hogalign "' outputHOG_aligned '"' ]);    
                 
end

dos(command);

%% Demonstrating reading the output files

% First read in the column names
tab = readtable(outputFile);
column_names = tab.Properties.VariableNames;

all_params  = dlmread(outputFile, ',', 1, 0);

% This indicates which frames were succesfully tracked
valid_frames = logical(all_params(:,4));
time = all_params(valid_frames, 2);

%% Finding which header line starts with p_ (basically model params)
shape_inds = cellfun(@(x) ~isempty(x) && x==1, strfind(column_names, 'p_'));

% Output rigid (first 6) and non-rigid shape parameters
shape_params  = all_params(valid_frames, shape_inds);

figure
plot(time, shape_params);
title('Shape parameters');
xlabel('Time (s)');

%% Demonstrate 2D landmarks
landmark_inds_x = cellfun(@(x) ~isempty(x) && x==1, strfind(column_names, 'x_'));
landmark_inds_y = cellfun(@(x) ~isempty(x) && x==1, strfind(column_names, 'y_'));

xs = all_params(valid_frames, landmark_inds_x);
ys = all_params(valid_frames, landmark_inds_y);

figure

for j = 1:size(xs,1)
    plot(xs(j,:), -ys(j,:), '.');
    xlim([min(xs(1,:)) * 0.5, max(xs(2,:))*1.4]);
    ylim([min(-ys(1,:)) * 1.4, max(-ys(2,:))*0.5]);
    xlabel('x (px)');
    ylabel('y (px)');
    drawnow
end


%% Demonstrate 3D landmarks
landmark_inds_x = cellfun(@(x) ~isempty(x) && x==1, strfind(column_names, 'X_'));
landmark_inds_y = cellfun(@(x) ~isempty(x) && x==1, strfind(column_names, 'Y_'));
landmark_inds_z = cellfun(@(x) ~isempty(x) && x==1, strfind(column_names, 'Z_'));

xs = all_params(valid_frames, landmark_inds_x);
ys = all_params(valid_frames, landmark_inds_y);
zs = all_params(valid_frames, landmark_inds_z);

figure
for j = 1:size(xs,1)
    plot3(xs(j,:), ys(j,:), zs(j,:), '.');axis equal;
    xlabel('X (mm)');
    ylabel('Y (mm)');    
    zlabel('Z (mm)');    
    drawnow
end

%% Demonstrate AUs
au_reg_inds = cellfun(@(x) ~isempty(x) && x==5, strfind(column_names, '_r'));

aus = all_params(valid_frames, au_reg_inds);
figure
plot(time, aus);
title('Facial Action Units (intensity)');
xlabel('Time (s)');
ylabel('Intensity');
ylim([0,6]);

au_class_inds = cellfun(@(x) ~isempty(x) && x==5, strfind(column_names, '_c'));

aus = all_params(valid_frames, au_class_inds);
figure
plot(time, aus);
title('Facial Action Units (presense)');
xlabel('Time (s)');
ylim([0,2]);
%% Demo pose
pose_inds = cellfun(@(x) ~isempty(x) && x==1, strfind(column_names, 'pose_'));

pose = all_params(valid_frames, pose_inds);
figure
plot(pose);
title('Pose (rotation and translation)');
xlabel('Time (s)');

%% Output HOG files
[hog_data, valid_inds, vid_id] = Read_HOG_files({name}, output);

%% Output aligned images
img_files = dir([outputDir_aligned, '/*.png']);
imgs = cell(numel(img_files, 1));
for i=1:numel(img_files)
   imgs{i} = imread([ outputDir_aligned, '/', img_files(i).name]);
   imshow(imgs{i})
   drawnow
end