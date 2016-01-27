clear
     
curr_dir = cd('.');

% Replace this with your downloaded 300-W train data
if(exist([getenv('USERPROFILE') '/Dropbox/AAM/eye_clm/mpii_data/'], 'file'))
    database_root = [getenv('USERPROFILE') '/Dropbox/AAM/eye_clm/mpii_data/'];    
else
    fprintf('MPII gaze dataset not found\n');
end
output_loc = './gaze_estimates_MPII/';
if(~exist(output_loc, 'dir'))
    mkdir(output_loc);
end

%% Perform actual gaze predictions
command = sprintf('"../../Release/FeatureExtraction.exe" -fx 1028 -fy 1028 ');
p_dirs = dir([database_root, 'p*']);

parfor p=1:numel(p_dirs)
    tic

    all_files = dir([database_root, p_dirs(p).name, '/*.jpg']);

    batch = 25;

    for i=1:batch:numel(all_files)

        command_c = command;
        for k=i:i+batch
            if(k > numel(all_files))
                break;
            end
            file = [database_root, p_dirs(p).name, '/', all_files(k).name];

            input_loc = ['-f "', file, '" '];
            command_c = cat(2, command_c, input_loc);
            out_file = [' -ogaze "', output_loc, '/', all_files(k).name '.gaze.txt" '];
            command_c = cat(2, command_c, out_file);

        end

        command_c = cat(2, command_c, ' -clmwild ');
        dos(command_c);

    end
    toc    

end
%%

% Extract the results
predictions_l = zeros(750, 3);
predictions_r = zeros(750, 3);
gt_l = zeros(750, 3);
gt_r = zeros(750, 3);

angle_err_l = zeros(750,1);
angle_err_r = zeros(750,1);

p_dirs = dir([database_root, 'p*']);
curr = 1;
for p=1:numel(p_dirs)
    load([database_root, p_dirs(p).name, '/Data.mat']);

    for i=1:size(filenames, 1)

        fname = sprintf('%s/%d_%d_%d_%d_%d_%d_%d.jpg.gaze.txt', output_loc,...
             filenames(i,1), filenames(i,2), filenames(i,3), filenames(i,4),...
             filenames(i,5), filenames(i,6), filenames(i,7));
        try            
            A = dlmread(fname, ',', 'E2..J2');
            valid = dlmread(fname, ',', 'D2..D2');
        catch
            A = zeros(1,6);
            A(1,3) = -1;
            A(1,6) = -1;
        end

        head_rot = headpose(i,1:3);
     
        predictions_r(curr,:) = A(1:3);
        predictions_l(curr,:) = A(4:6);  
        
        if(~valid)
            predictions_r(curr,:) = [0,0,-1];
            predictions_l(curr,:) = [0,0,-1];
        end
                
        gt_r(curr,:) = data.right.gaze(i,:)';
        gt_r(curr,:) = gt_r(curr,:) / norm(gt_r(curr,:));
        gt_l(curr,:) = data.left.gaze(i,:)';
        gt_l(curr,:) = gt_l(curr,:) / norm(gt_l(curr,:));

        angle_err_l(curr) = acos(predictions_l(curr,:) * gt_l(curr,:)') * 180/pi;
        angle_err_r(curr) = acos(predictions_r(curr,:) * gt_r(curr,:)') * 180/pi;

        curr = curr + 1;
    end

end
all_errors = cat(1, angle_err_l, angle_err_r);
mean_error = mean(all_errors);
median_error = median(all_errors);
save('mpii_1500_errs.mat', 'all_errors', 'mean_error', 'median_error');

f = fopen('mpii_1500_errs.txt', 'w');
fprintf(f, 'Mean error, median error\n');
fprintf(f, '%.3f, %.3f\n', mean_error, median_error);
fclose(f);
