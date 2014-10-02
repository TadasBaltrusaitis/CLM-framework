% Biwi dataset experiment
oldDir = chdir('../Release/');
features_exe = '"FeatureExtraction.exe"';

DISFA_loc = 'D:/Databases/DISFA/';

DISFA_loc_1 = [DISFA_loc, 'Videos_LeftCamera/'];
DISFA_loc_2 = [DISFA_loc, 'Video_RightCamera/'];

output = 'D:/Databases/DISFA/aligned/';
if(~exist(output, 'dir'))
    mkdir(output);
end

disfa_loc_1_files = dir([DISFA_loc_1, '/*.avi']);
disfa_loc_2_files = dir([DISFA_loc_2, '/*.avi']);

%%

tic;

for i=1:numel(disfa_loc_1_files)
           
    command = features_exe;
               
    input_file = [DISFA_loc_1 disfa_loc_1_files(i).name];
        
    output_file = [output disfa_loc_1_files(i).name];

    command = cat(2, command, [' -f "' input_file '" -simalign "' output_file  '" -simscale 0.75 -simsize 120']);
            
    dos(command);
end

%%
for i=1:numel(disfa_loc_2_files)
           
    command = features_exe;
               
    input_file = [DISFA_loc_2 disfa_loc_2_files(i).name];
        
    output_file = [output disfa_loc_2_files(i).name];

    command = cat(2, command, [' -f "' input_file '" -simalign "' output_file  '" -simscale 0.75 -simsize 120']);
            
    dos(command);
end

timeTaken = toc;
chdir(oldDir);