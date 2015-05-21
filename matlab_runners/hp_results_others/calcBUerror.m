function [meanError, all_rot_preds, all_rot_gts, all_errors] = calcBUerror(pred_pose, gtDir)

seqNames = {'jam1','jam2','jam3','jam4','jam5','jam6','jam7','jam8','jam9', ...
    'jim1','jim2','jim3','jim4','jim5','jim6','jim7','jim8','jim9', ...
    'llm1','llm2','llm3','llm4','llm5','llm6','llm7','llm8','llm9', ...
    'ssm1','ssm2','ssm3','ssm4','ssm5','ssm6','ssm7','ssm8','ssm9', ...
    'vam1','vam2','vam3','vam4','vam5','vam6','vam7','vam8','vam9'};

rotMeanErr = zeros(numel(seqNames),3);
rotRMS = zeros(numel(seqNames),3);
rot = cell(1,numel(seqNames));
rotg = cell(1,numel(seqNames));

for i = 1:numel(seqNames)
    
    posesGround =  load ([gtDir seqNames{i} '.dat']);
    
    rot{i} = pred_pose(strcmp({pred_pose.name}, seqNames{i})).pose * (pi/180);   
    % Flip because of different conventions
    x_rot = -rot{i}(:,3);
    y_rot = -rot{i}(:,2);
    z_rot = -rot{i}(:,1);
    rot{i} = cat(2, x_rot, y_rot, z_rot);
    % Convert to radians
    rotg{i} = posesGround(2:end,[7 6 5]) * (pi/180);
        
    % Correct the first frame so it corresponds to (0,0,0), as slightly
    % different pose might be assumed frontal and this corrects for
    % that
    
    % Work out the correction matrix for ground truth
    rot_corr_gt = Euler2Rot(rotg{i}(1,:));
    for r_e = 1:size(rotg{i},1)
        rot_curr_gt = Euler2Rot(rotg{i}(r_e,:));
        rot_new_gt = rot_corr_gt' * rot_curr_gt;
        rotg{i}(r_e,:) = Rot2Euler(rot_new_gt);
    end
    
    % Work out the correction matrix for estimates
    rot_corr_est = Euler2Rot(rot{i}(1,:));
    for r_e = 1:size(rot{i},1)
        rot_curr_est = Euler2Rot(rot{i}(r_e,:));
        rot_new_est = rot_corr_est' * rot_curr_est;
        rot{i}(r_e,:) = Rot2Euler(rot_new_est);
    end
    
    % Convert to degrees
    rotg{i} = rotg{i} * 180 / pi;
    rot{i} = rot{i} * 180 / pi;  
  
    rot{i}( rot{i} > 90) =  rot{i}(rot{i} > 90) - 180;
    rot{i}( rot{i} < -90) =  rot{i}(rot{i} < -90) + 180;
    
    rotMeanErr(i,:) = mean(abs((rot{i}(:,:)-rotg{i}(:,:))));
    rotRMS(i,:) = sqrt(mean(((rot{i}(:,:)-rotg{i}(:,:))).^2)); 
    
end
allRot = cell2mat(rot');
allRotg = cell2mat(rotg');

meanError = mean(abs((allRot(:,:)-allRotg(:,:))));
all_errors = abs(allRot-allRotg);

all_rot_preds = allRot;
all_rot_gts = allRotg;