clm_exe = '"../Release/SimpleCLM.exe"';

output = '../matlab_runners/demo_vid/';

if(~exist(output, 'file'))
    mkdir(output)
end
    
in_files = dir('../videos/*.wmv');
in_files = cat(1, in_files, dir('../videos/*.avi'));
% some parameters
verbose = true;

% Trained on in the wild and multi-pie data (less accurate SVR model)
%model = 'model/main_svr_general.txt';
% Trained on multi-pie
%model = 'model/main_svr_mpie.txt';
% Trained on in-the-wild
%model = 'model/main_svr_wild.txt';

% Trained on in the wild and multi-pie data (more accurate CCNF model)
model = 'model/main_ccnf_general.txt';
% Trained on multi-pie
%model = 'model/main_ccnf_mpie.txt';
% Trained on in-the-wild
%model = 'model/main_ccnf_wild.txt';

command = clm_exe;
command = cat(2, command, [' -mloc "', model, '"']);
% add all videos to single argument list (so as not to load the model anew
% for every video)
for i=1:numel(in_files)
    
    inputFile = ['../videos/', in_files(i).name];
    [~, name, ~] = fileparts(inputFile);
    
    % where to output tracking results
    outputFile_pose = [output name '_pose.txt'];
    outputFile_fp = [output name '_fp.txt'];
    
    command = cat(2, command, [' -f "' inputFile '" -op "' outputFile_pose '"' ' -of "' outputFile_fp '"']);
    
    if(verbose)
        outputVideo = ['"' output name '.avi' '"'];
        command = cat(2, command, [' -ov ' outputVideo]);
    end
                 
end

dos(command);