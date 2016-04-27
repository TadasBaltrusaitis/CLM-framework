clm_exe = '"../../x64/Release/FaceLandmarkVid.exe"';

output = './demo_vid/';

if(~exist(output, 'file'))
    mkdir(output)
end
    
in_files = dir('../../videos/*.wmv');
in_files = cat(1, in_files, dir('../../videos/*.avi'));
% some parameters
verbose = true;

% Trained on in the wild and multi-pie data (less accurate SVR/CLM model)
%model = 'model/main_clm_general.txt';
% Trained on in-the-wild
%model = 'model/main_clm_wild.txt';

% Trained on in the wild and multi-pie data (more accurate CLNF model)
model = 'model/main_clnf_general.txt';
% Trained on in-the-wild
%model = 'model/main_clnf_wild.txt';

command = clm_exe;
command = cat(2, command, [' -mloc "', model, '"']);
% add all videos to single argument list (so as not to load the model anew
% for every video)
for i=1:numel(in_files)
    
    inputFile = ['../../videos/', in_files(i).name];
    [~, name, ~] = fileparts(inputFile);
        
    command = cat(2, command, [' -f "' inputFile '" ']);
    
    if(verbose)
        outputVideo = ['"' output name '.avi' '"'];
        command = cat(2, command, [' -ov ' outputVideo]);
    end
                 
end

dos(command);