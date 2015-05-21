function [meanError, all_rot_preds, all_rot_gts, meanErrors, all_errors] = calcIctError(pred_pose, gtDir)
%CALCICTERROR Summary of this function goes here
%   Detailed explanation goes here

    polhemus = 'polhemusNorm.csv';

    sequences = {'02', '04', '06', '08', '10', '12', '14', '16', '18', '20'};

    rotMeanErr = zeros(numel(sequences),3);
    rotRMS = zeros(numel(sequences),3);
    rot = cell(1,numel(sequences));
    rotg = cell(1,numel(sequences));

    for i = 1:numel(sequences)

        [~, name,~] = fileparts(sequences{i});
        [txg tyg tzg rxg ryg rzg] =  textread([gtDir name '/'  polhemus], '%f,%f,%f,%f,%f,%f');

        rot{i} = pred_pose(strcmp({pred_pose.name}, name)).pose * (pi/180);   
        % Flip because of different conventions
        x_rot = -rot{i}(:,3);
        y_rot = rot{i}(:,2);
        z_rot = rot{i}(:,1);
        rot{i} = cat(2, x_rot, y_rot, z_rot);
        
        rotg{i} = [rxg ryg rzg];
        
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
        
        % Convert the ground truth and estimates to degrees
        rot{i} = rot{i} * (180/ pi);
        rotg{i} = rotg{i} * (180/ pi);

        rot{i}( rot{i} > 90) =  rot{i}(rot{i} > 90) - 180;
        rot{i}( rot{i} < -90) =  rot{i}(rot{i} < -90) + 180;

        % Now compute the errors
        rotMeanErr(i,:) = mean(abs((rot{i}(:,:)-rotg{i}(:,:))));
        rotRMS(i,:) = sqrt(mean(((rot{i}(:,:)-rotg{i}(:,:))).^2)); 
        
    end
    allRot = cell2mat(rot');
    allRotg = cell2mat(rotg');
    meanErrors = rotMeanErr;
    meanError = mean(abs((allRot(:,:)-allRotg(:,:))));
    all_errors = abs(allRot-allRotg);
    rmsError = sqrt(mean(((allRot(:,:)-allRotg(:,:))).^2)); 
    errorVariance = var(abs((allRot(:,:)-allRotg(:,:))));  

    all_rot_preds = allRot;
    all_rot_gts = allRotg;
end

