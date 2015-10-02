clm_exe = '"../Release/FeatureExtraction.exe"';

output = '../matlab_runners/output_features_vid/';

if(~exist(output, 'file'))
    mkdir(output)
end
    
in_files = dir('../videos/1815_01_008_tony_blair.avi');
% some parameters
verbose = true;

command = clm_exe;
command = cat(2, command, ' -rigid ');

% Remove for a speedup
command = cat(2, command, ' -verbose ');

% add all videos to single argument list (so as not to load the model anew
% for every video)
for i=1:numel(in_files)
    
    inputFile = ['../videos/', in_files(i).name];
    [~, name, ~] = fileparts(inputFile);
    
    % where to output tracking results
    outputFile_pose = [output name '_pose.txt'];
    outputFile_fp = [output name '_fp.txt'];
    outputFile_3Dfp = [output name '_fp3D.txt'];
        
    output_aus = [output name '_au.txt'];
    
    if(~exist([output name], 'file'))
        mkdir([output name]);
    end
    
    outputDir_aligned = [output name];
    
    outputHOG_aligned = [output name '.hog'];
    
    output_shape_params = [output name '.params.txt'];
    
    command = cat(2, command, [' -f "' inputFile '" -op "' outputFile_pose '"' ' -of "' outputFile_fp '"' ' -of3D "' outputFile_3Dfp '"']);
    
    command = cat(2, command, [' -oaus "' output_aus '" ']);
    
    command = cat(2, command, [' -simaligndir "' outputDir_aligned '" -hogalign "' outputHOG_aligned '"' ' -oparams "' output_shape_params '"']);    
                 
end

dos(command);

%% Demonstrating reading the output files
filename = [output name];

% Output shape parameters (6 rigid and rest non-rigid)
shape_params  = dlmread([filename, '.params.txt'], ',', 1, 0);

% This indicates which frames were succesfully tracked
valid_frames = shape_params(:,2);

shape_params = shape_params(:,3:end);

% Output landmark points
landmark_points  = dlmread([filename, '_fp.txt'], ',', 1, 0);
landmark_points = landmark_points(:,3:end);
plot(landmark_points(1,1:end/2), -landmark_points(1,end/2+1:end), '.');axis equal;

% Output 3D landmark points
landmark_3D_points  = dlmread([filename, '_fp3D.txt'], ',', 1, 0);
landmark_3D_points = landmark_3D_points(:,3:end);
figure
plot3(landmark_3D_points(1,1:end/3), landmark_3D_points(1,end/3+1:2*end/3), landmark_3D_points(1,2*end/3+1:end), '.');axis equal;

% Output pose
head_pose  = dlmread([filename, '_pose.txt'], ',', 1, 0);
head_pose = head_pose(:,4:end);

% Output HOG files
[hog_data, valid_inds, vid_id] = Read_HOG_files({name}, output);

% Output aligned images
img_files = dir([filename, '/*.png']);
imgs = cell(numel(img_files, 1));
for i=1:numel(img_files)
   imgs{i} = imread([ filename, '/', img_files(i).name]);
end