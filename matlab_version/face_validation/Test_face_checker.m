function [ predictions, gts, rmse, corr_coeff ] = Test_face_checker( face_check_fun )
    
    addpath('../PDM_helpers');

    % Read in the face images
    [image_locs, ~, landmark_locations] = Collect_wild_imgs('C:/Users/Tadas/Dropbox/AAM/test data/');
    
    load '../models/pdm/pdm_68_aligned_wild.mat';
    load '../models/tri_68.mat';
        
    % Make sure same data generated all the time
    rng(0);

    neg_image_loc = 'E:/datasets/detection_validation/neg/';

    neg_images = cat(1,dir([neg_image_loc, '/*.jpg']),dir([neg_image_loc, '/*.png']));

    curr_filled = 0;
           
    num_imgs = numel(image_locs);
    
    more_neg = 1;
    
    errors_all = zeros(num_imgs + num_imgs*more_neg, 1);
    predictions_all  = zeros(num_imgs + num_imgs*more_neg, 1);
    
    img_used = cell(size(errors_all));
    labels_used = cell(size(errors_all));
    
    for j=1:num_imgs
                
        labels = squeeze(landmark_locations(j,:,:));

        % Extract the correct PDM parameters for the model and evaluate how
        % well the checker detects correct detections
        curr_filled = curr_filled + 1; 
        img = imread(image_locs(j).img);
        img_used(curr_filled) = {image_locs(j).img};              

        [ a_orig, R_orig, trans_orig, ~, params_orig, err, shape] = fit_PDM_ortho_proj_to_2D(M, E, V, labels);
        eul_orig = Rot2Euler(R_orig);
        
        p_global = [a_orig; eul_orig'; trans_orig];
        
        labels_used(curr_filled) = {shape};
        
        % Test the positive example    
        predictions_all(curr_filled) = face_check_fun(img, shape, p_global);
        error = norm(shape(:) - labels(:)) / (max(labels(:,2))-min(labels(:,2)));
        errors_all(curr_filled) = error;

        curr_filled = curr_filled + 1;    
        
        img_used(curr_filled) = {image_locs(j).img};
        
        if(mod(j, 6) == 0)
            % a slightly perturbed example, too tight
            % from 0.3 to 0.9
            a_mod = a_orig * (0.6 + (randi(7) - 4)*0.1);                        
            p_global = [a_mod; eul_orig'; trans_orig];
        elseif(mod(j,6) == 1)
              % a slightly perturbed example, too broad
            % from 1.2 to 0.6
            a_mod = a_orig * (1.4 + (randi(5) - 3)*0.1);        
            p_global = [a_mod; eul_orig'; trans_orig];          
        elseif(mod(j,6) == 2)
            % A somewhat offset example
            trans_mod = trans_orig + randn(2,1) * 10;
            p_global = [a_orig; eul_orig'; trans_mod];        
        elseif(mod(j,6) == 3)
            % A rotated sample
            eul_mod = eul_orig + randn(1,3)*0.2;
            p_global = [a_orig; eul_mod'; trans_orig];
        elseif(mod(j,6) == 4)
            % A sample with modified shape parameters
            p_global = [a_orig; eul_orig'; trans_orig];
            params_orig = params_orig + randn(size(params_orig)).*sqrt(E);
        elseif(mod(j,6) == 5)
            % pick a random image from negative inriaperson dataset, use original location if
            % first, otherwhise resize it to fit
            n_img = randi(numel(neg_images));
            
            neg_image = imread([neg_image_loc, neg_images(n_img).name]);
            img_used(curr_filled) = {[neg_image_loc, neg_images(n_img).name]};
            
            if(size(neg_image,3) == 3)
                neg_image = rgb2gray(neg_image);
            end
            
            [h_neg, w_neg] = size(neg_image);
            
            % if the current labels fit just use them, if not, then resize
            % to fit
            max_x = max(labels(:,1));
            max_y = max(labels(:,2));
            
            if(max_x > w_neg || max_y > h_neg)
               neg_image = imresize(neg_image, [max_y, max_x]);
            end
            img = neg_image;

        end      
        
        labels_mod = GetShapeOrtho(M, V, params_orig, p_global);
        labels_mod = labels_mod(:,1:2);
    
        labels_used(curr_filled) = {labels_mod};
        
        predictions_all(curr_filled) = face_check_fun(img, labels_mod, p_global);
        % Compute the badness of fit
        error = norm(labels_mod(:) - labels(:)) / (max(labels(:,2))-min(labels(:,2)));
        errors_all(curr_filled,:) = error;      
        
        if(mod(j,6) == 5)   
            % Set high error to 3, if inriaperson used
            errors_all(curr_filled,:) = 3;
        end
        
        if(mod(curr_filled, 10) == 0)
            fprintf('%d/%d done\n', j, num_imgs);
        end
    end    
                
    predictions = predictions_all(1:curr_filled,:);
    gts = errors_all(1:curr_filled);
    
    gts(gts > 3) = 3;
    
    rmse = sqrt(mean((predictions - gts).^2));
    corr_coeff = corr(predictions, gts);
    
    [~, inds] = sort((predictions - gts).^2, 'descend');
    
    %% Visualise the worst errors
    num_to_vis = 20;
    samples_per_row = 4;
    num_cols = ceil(num_to_vis / samples_per_row);
    figure;    
    for i=1:num_to_vis
        subplot(num_cols, samples_per_row, i);
         
        curr_img = imread(img_used{inds(i)});
        curr_labels = labels_used{inds(i)};
        
        min_x = min(curr_labels(:,1));
        min_y = min(curr_labels(:,2));

        max_x = max(curr_labels(:,1));
        max_y = max(curr_labels(:,2));

        curr_img = imcrop(curr_img, [min_x, min_y, max_x - min_x, max_y - min_y]);
        
        imshow(curr_img);
        hold on;
        plot(curr_labels(:,1) - min_x, curr_labels(:,2) - min_y, '.');
        text(20, 20, 0, sprintf('Pred:%f, Actual:%f', predictions(inds(i)), gts(inds(i))));        
        
    end
    
end

