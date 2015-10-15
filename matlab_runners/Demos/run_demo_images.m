clear

clm_exe = '"../../Release/SimpleCLMImg.exe"';
    
in_dir  = '../../videos/';
out_dir = './demo_img/';

if(~exist(out_dir, 'file'))
    mkdir(out_dir);
end

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

command = cat(2, command, [' -fdir "' in_dir '"']);

if(verbose)
    command = cat(2, command, [' -ofdir "' out_dir '"']);
    command = cat(2, command, [' -oidir "' out_dir '"']);
end

command = cat(2, command, [' -mloc "', model, '"']);

% Demonstrates the multi-hypothesis slow landmark detection (more accurate
% when dealing with non-frontal faces and less accurate face detections)
% Comment to skip this functionality
command = cat(2, command, ' -clmwild ');

dos(command);