exe = '"../../x64/Release/FeatureExtraction.exe"';

output = './output_features_vid/';

if(~exist(output, 'file'))
    mkdir(output)
end
    
in_files = dir('../../videos/2015-10-15-15-14.avi');
% some parameters
verbose = true;

command = exe;
% Remove for a speedup
command = cat(2, command, ' -verbose ');

% add all videos to single argument list (so as not to load the model anew
% for every video)
for i=1:numel(in_files)
    
    inputFile = ['../../videos/', in_files(i).name];
    [~, name, ~] = fileparts(inputFile);
    
    % where to output tracking results
    outputFile_gaze = [output name '_gaze.txt'];
    
    if(~exist([output name], 'file'))
        mkdir([output name]);
    end
    
    command = cat(2, command, ['-fx 700 -fy 700 -f "' inputFile '" -ogaze "' outputFile_gaze '"']);
               
end

dos(command);

%% Demonstrating reading the output files
filename = [output name];

% Read gaze (x,y,z) for one eye and (x,y,z) for another
gaze  = dlmread([filename, '_gaze.txt'], ',', 1, 0);

% This indicates which frames were succesfully tracked
valid_frames = gaze(:,4);

% only picking left, right and up down views for visualisation
gaze = gaze(:,[5,6,7,8,9,10]);
gaze = (gaze(:,[1,2,3]) + gaze(:,[4,5,6]))/2;
gaze(:,1) = smooth(gaze(:,1));
gaze(:,2) = smooth(gaze(:,2));
gaze(:,3) = smooth(gaze(:,3));

plot(gaze(:,1), 'DisplayName', 'Left - right');
hold on;
plot(gaze(:,2), 'DisplayName', 'Up - down');
xlabel('Frame') % x-axis label
ylabel('Gaze vector size') % y-axis label
legend('show');
hold off;