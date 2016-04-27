clear

clm_exe = '"../../x64/Release/FaceLandmarkImg.exe"';
    
in_dir  = '../../videos/';
out_dir = './demo_img/';

if(~exist(out_dir, 'file'))
    mkdir(out_dir);
end

% some parameters
verbose = true;

% Trained on in the wild and multi-pie data (less accurate CLM model)
% model = 'model/main_clm_general.txt';
% Trained on in-the-wild
%model = 'model/main_clm_wild.txt';

% Trained on in the wild and multi-pie data (more accurate CLNF model)
model = 'model/main_clnf_general.txt';
% Trained on in-the-wild
%model = 'model/main_clnf_wild.txt';

command = clm_exe;

command = cat(2, command, [' -fdir "' in_dir '"']);

if(verbose)
    command = cat(2, command, [' -ofdir "' out_dir '"']);
    command = cat(2, command, [' -oidir "' out_dir '"']);
end

command = cat(2, command, [' -mloc "', model, '"']);

% Demonstrates the multi-hypothesis slow landmark detection (more accurate
% when dealing with non-frontal faces and less accurate face detections)
% Comment to skip this functionality
command = cat(2, command, ' -wild ');

dos(command);