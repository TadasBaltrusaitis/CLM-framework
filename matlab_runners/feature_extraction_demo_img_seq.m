oldDir = chdir('../Release/');
clm_exe = '"FeatureExtraction.exe"';

output = '../matlab_runners/output_features_seq/';

if(~exist(output, 'file'))
    mkdir(output)
end
    
in_dirs = {'../image_sequence/'};
% some parameters
verbose = true;

command = clm_exe;
command = cat(2, command, ' -rigid ');

% Remove for a speedup
command = cat(2, command, ' -verbose ');

% add all videos to single argument list (so as not to load the model anew
% for every video)
for i=1:numel(in_dirs)
    
    [~, name, ~] = fileparts(in_dirs{i});
    
    % where to output tracking results
    outputFile_pose = [output name '_pose.txt'];
    outputFile_fp = [output name '_fp.txt'];
    outputFile_3Dfp = [output name '_fp3D.txt'];
        
    if(~exist([output name], 'file'))
        mkdir([output name]);
    end
    
    output_avi_aligned = [output name '/aligned.avi'];
    
    outputHOG_aligned = [output name '.hog'];
    
    output_shape_params = [output name '.params.txt'];
    
    command = cat(2, command, ['-asvid -fdir "' in_dirs{i} '" -op "' outputFile_pose '"' ' -of "' outputFile_fp '"' ' -of3D "' outputFile_3Dfp '"']);
    
    command = cat(2, command, [' -simalignvid "' output_avi_aligned '" -hogalign "' outputHOG_aligned '"' ' -oparams "' output_shape_params '"']);    
                 
end

dos(command);
chdir(oldDir);

%% Demonstrating reading the output files
filename = [output name];

% Output shape parameters (6 rigid and rest non-rigid)
shape_params  = dlmread([filename, '.params.txt'], ' ');

% This indicates which frames were succesfully tracked
valid_frames = shape_params(:,2);

shape_params = shape_params(:,3:end);

% Output landmark points
landmark_points  = dlmread([filename, '_fp.txt'], ' ');
landmark_points = landmark_points(:,3:end);
plot(landmark_points(1,1:end/2), -landmark_points(1,end/2+1:end), '.');axis equal;

% Output 3D landmark points
landmark_3D_points  = dlmread([filename, '_fp3D.txt'], ' ');
landmark_3D_points = landmark_3D_points(:,3:end);
figure
plot3(landmark_3D_points(1,1:end/3), landmark_3D_points(1,end/3+1:2*end/3), landmark_3D_points(1,2*end/3+1:end), '.');axis equal;

% Output pose
head_pose  = dlmread([filename, '_pose.txt'], ' ');
head_pose = head_pose(:,4:end);

% Output HOG files
[hog_data, valid_inds, vid_id] = Read_HOG_files({name}, output);