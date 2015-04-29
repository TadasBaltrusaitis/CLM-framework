clm_exe = '"../Release/SimpleCLM.exe"';

output = 'yt_features/';

if(~exist(output, 'file'))
    mkdir(output)
end
    
% Run the BU test with CLM
if(exist([getenv('USERPROFILE') '/Dropbox/AAM/test data/'], 'file'))
    database_root = [getenv('USERPROFILE') '/Dropbox/AAM/test data/'];    
else
    database_root = 'F:/Dropbox/Dropbox/AAM/test data/';
end

database_root = [database_root, '/ytceleb_annotations_CVPR2014/'];

in_vids = dir([database_root '/*.avi']);

command = clm_exe;

% add all videos to single argument list (so as not to load the model anew
% for every video)
for i=1:numel(in_vids)
    
    [~, name, ~] = fileparts(in_vids(i).name);
    
    % where to output tracking results
    outputFile_fp = [output name '_fp.txt'];
    in_file_name = [database_root, '/', in_vids(i).name];        
    
    command = cat(2, command, [' -f "' in_file_name '" -of "' outputFile_fp '"']);                     
end

dos(command);