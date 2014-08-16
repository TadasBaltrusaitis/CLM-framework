function Create_data_66_large()

load '../models/pdm/pdm_66_multi_pie';
load '../models/tri_66.mat';

% This script uses the same format used for patch expert training, and
% expects the data to be there (this can be found in
% https://github.com/TadasBaltrusaitis/CCNF)

% Replace with your location of training data
dataset_loc = 'C:/Users/Tadas/Documents/CCNF/patch_experts/data_preparation/prepared_data/';

addpath('../PDM_helpers/');

scale = '0.5';
prefix= 'combined_';

% Find the available positive training data
data_files = dir(sprintf('%s/%s%s*.mat', dataset_loc, prefix, scale));
centres_all = [];
for i=1:numel(data_files)
   
    % Load the orientation of the training data
    load([dataset_loc, '/', data_files(i).name], 'centres');
    centres_all = cat(1, centres_all, centres);
    
end

label_inds = [1:60,62:64,66:68];

% Construct mirror indices (which views need to be flipped to create other
% profile training data)
mirror_inds = zeros(size(centres_all,1), 1);
for i=1:numel(data_files)
    
    % mirrored image has inverse yaw
    mirrored_centre = centres_all(i,:);
    mirrored_centre(2) = -mirrored_centre(2);

    % if mirrored version has same orientation, do not need mirroring
    if(~isequal(mirrored_centre, centres_all(i,:)))
       
        centres_all = cat(1, centres_all, mirrored_centre);
        mirror_inds = cat(1, mirror_inds, i);
        
    end    
    
end
        
outputLocation = 'F:/datasets/detection_validation/prep_data/';
    
num_more_neg = 10;

% Make sure same data generated all the time
rng(0);

neg_image_loc = 'F:/datasets/detection_validation/neg/';

neg_images = cat(1,dir([neg_image_loc, '/*.jpg']),dir([neg_image_loc, '/*.png']));

max_img_used = 2500;

% do it separately for centers due to memory limitations
for r=1:size(centres_all,1)
    
    a_mod = 0.4;
    
    mirror = false;
    
    if(mirror_inds(r) ~= 0 )
        mirror = true;
        label_mirror_inds = [1,17;2,16;3,15;4,14;5,13;6,12;7,11;8,10;18,27;19,26;20,25;21,24;22,23;...
                  32,36;33,35;37,46;38,45;39,44;40,43;41,48;42,47;49,55;50,54;51,53;60,56;59,57;...
                  61,63;66,64];    
        load([dataset_loc, '/', data_files(mirror_inds(r)).name]);
    else            
        load([dataset_loc, '/', data_files(r).name]);
    end
    
    % Convert to 66 point model
    landmark_locations = landmark_locations(:,label_inds,:);
    
    visiCurrent = logical(visiIndex);
    
    if(mirror)
        centres = [centres(1), -centres(2), -centres(3)];
        tmp1 = visiCurrent(label_mirror_inds(:,1));
        tmp2 = visiCurrent(label_mirror_inds(:,2));            
        visiCurrent(label_mirror_inds(:,2)) = tmp1;
        visiCurrent(label_mirror_inds(:,1)) = tmp2; 
    end    
            
    visibleVerts = 1:numel(visiCurrent);
    visibleVerts = visibleVerts(visiCurrent)-1;

    % Correct the triangulation to take into account the vertex
    % visibilities
    triangulation = [];

    shape = a_mod * Euler2Rot(centres * pi/180) * reshape(M, numel(M)/3, 3)';
    shape = shape';

    for i=1:size(T,1)
        visib = 0;
        for j=1:numel(visibleVerts)
            if(T(i,1)==visibleVerts(j))
                visib = visib+1;
            end
            if(T(i,2)==visibleVerts(j))
                visib = visib+1;
            end
            if(T(i,3)==visibleVerts(j))
                visib = visib+1;
            end
        end
        
        % Only if all three of the vertices are visible
        if(visib == 3)            
            
            % Also want to remove triangles facing the wrong way (self occluded)
            v1 = [shape(T(i,1)+1,1), shape(T(i,1)+1,2), shape(T(i,1)+1,3)];
            v2 = [shape(T(i,2)+1,1), shape(T(i,2)+1,2), shape(T(i,2)+1,3)];
            v3 = [shape(T(i,3)+1,1), shape(T(i,3)+1,2), shape(T(i,3)+1,3)];
            normal = cross((v2-v1), v3 - v2);
            normal = normal / norm(normal);
            direction = normal * [0,0,1]';

            % And only if the triangle is facing the camera
            if(direction > 0)
                triangulation = cat(1, triangulation, T(i,:)); 
            end
        end                
    end
        
    % Initialise the warp
    [ alphas, betas, triX, mask, minX, minY, nPix ] = InitialisePieceWiseAffine(triangulation, shape);

    mask = logical(mask);
    
    imgs_to_use = randperm(size(landmark_locations, 1));
    
    if(size(landmark_locations, 1) > max_img_used)
        imgs_to_use = imgs_to_use(1:max_img_used);
    end
    
    % Extracting relevant filenames
    examples = zeros(numel(imgs_to_use) * (num_more_neg+1), nPix);    
    errors = zeros(numel(imgs_to_use) * (num_more_neg+1), 1);
    
    unused_pos = 0;
    
    curr_filled = 0;
           
    for j=imgs_to_use
        
        labels = squeeze(landmark_locations(j,:,:));

        img = squeeze(all_images(j,:,:));

        if(mirror)
            img = fliplr(img);
            imgSize = size(img);
            flippedLbls = labels;
            flippedLbls(:,1) = imgSize(1) - flippedLbls(:,1);
            tmp1 = flippedLbls(label_mirror_inds(:,1),:);
            tmp2 = flippedLbls(label_mirror_inds(:,2),:);            
            flippedLbls(label_mirror_inds(:,2),:) = tmp1;
            flippedLbls(label_mirror_inds(:,1),:) = tmp2;   
            labels = flippedLbls;
        end            
        
        % If for some reason some of the labels are not visible in the
        % current sample skip this label
        non_existent_labels = labels(:,1)==0 | labels(:,2)==0;
        non_existent_inds = find(non_existent_labels)-1;
        if(numel(intersect(triangulation(:), non_existent_inds)) > 0)
           unused_pos = unused_pos + 1; 
           continue;
        end
        
        curr_filled = curr_filled + 1;        
        [features] = ExtractFaceFeatures(img, labels, triangulation, triX, mask, alphas, betas, nPix, minX, minY);
        examples(curr_filled,:) = features;
        errors(curr_filled,:) = 0;
        
        % Extract the correct PDM parameters for the model (we will perturb
        % them for some negative examples)
        [ a_orig, R_orig, trans_orig, ~, params_orig] = fit_PDM_ortho_proj_to_2D(M, E, V, labels);
        eul_orig = Rot2Euler(R_orig);
        
        % a slightly perturbed example, too tight
        % from 0.3 to 0.9
        a_mod = a_orig * (0.6 + (randi(7) - 4)*0.1);                        
        p_global = [a_mod; eul_orig'; trans_orig];
        
        labels_mod = GetShapeOrtho(M, V, params_orig, p_global);
        labels_mod = labels_mod(:,1:2);
        
        [features] = ExtractFaceFeatures(img, labels_mod, triangulation, triX, mask, alphas, betas, nPix, minX, minY);

        curr_filled = curr_filled + 1;        
        examples(curr_filled,:) = features;
        
        % Compute the badness of fit
        error = norm(labels_mod(:) - labels(:)) / (max(labels(:,2))-min(labels(:,2)));
        errors(curr_filled,:) = error;

        % a slightly perturbed example, too broad
        % from 1.2 to 0.6
        a_mod = a_orig * (1.4 + (randi(5) - 3)*0.1);        
        p_global = [a_mod; eul_orig'; trans_orig];
        
        labels_mod = GetShapeOrtho(M, V, params_orig, p_global);
        labels_mod = labels_mod(:,1:2);
        
        [features] = ExtractFaceFeatures(img, labels_mod, triangulation, triX, mask, alphas, betas, nPix, minX, minY);
                
        curr_filled = curr_filled + 1;        
        examples(curr_filled,:) = features;
        
        error = norm(labels_mod(:) - labels(:)) / (max(labels(:,2))-min(labels(:,2)));
        errors(curr_filled,:) = error;

        % A somewhat offset example
        
        trans_mod = trans_orig + randn(2,1) * 10;
        p_global = [a_orig; eul_orig'; trans_mod];
        
        labels_mod = GetShapeOrtho(M, V, params_orig, p_global);
        labels_mod = labels_mod(:,1:2);
        
        [features] = ExtractFaceFeatures(img, labels_mod, triangulation, triX, mask, alphas, betas, nPix, minX, minY);
        
        curr_filled = curr_filled + 1;        
        examples(curr_filled,:) = features;
    
        error = norm(labels_mod(:) - labels(:)) / (max(labels(:,2))-min(labels(:,2)));
        errors(curr_filled,:) = error;
       
        % A rotated sample
        eul_mod = eul_orig + randn(1,3)*0.2;
        p_global = [a_orig; eul_mod'; trans_orig];
        
        labels_mod = GetShapeOrtho(M, V, params_orig, p_global);
        labels_mod = labels_mod(:,1:2);
        
        [features] = ExtractFaceFeatures(img, labels_mod, triangulation, triX, mask, alphas, betas, nPix, minX, minY);
        
        curr_filled = curr_filled + 1;        
        examples(curr_filled,:) = features;
    
        error = norm(labels_mod(:) - labels(:)) / (max(labels(:,2))-min(labels(:,2)));
        errors(curr_filled,:) = error;
        
        % A sample with modified shape parameters
        p_global = [a_orig; eul_orig'; trans_orig];
        params_mod = params_orig + randn(size(params_orig)).*sqrt(E);
        labels_mod = GetShapeOrtho(M, V, params_mod, p_global);
        labels_mod = labels_mod(:,1:2);
        
        [features] = ExtractFaceFeatures(img, labels_mod, triangulation, triX, mask, alphas, betas, nPix, minX, minY);
        
        curr_filled = curr_filled + 1;        
        examples(curr_filled,:) = features;
    
        error = norm(labels_mod(:) - labels(:)) / (max(labels(:,2))-min(labels(:,2)));
        errors(curr_filled,:) = error;     
                
        % pick a random image from negative inriaperson dataset, use original location if
        % first, otherwhise resize it to fit
        for n=6:num_more_neg
            n_img = randi(numel(neg_images));
            
            neg_image = imread([neg_image_loc, neg_images(n_img).name]);
            
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
            
            [features] = ExtractFaceFeatures(neg_image, labels, triangulation, triX, mask, alphas, betas, nPix, minX, minY);
            
            curr_filled = curr_filled + 1;        
            examples(curr_filled,:) = features;

            % Set high error to 3
            errors(curr_filled,:) = 3;
        end
        
        
        if(mod(curr_filled, 10) == 0)
            fprintf('%d/%d done\n', curr_filled/(num_more_neg+1), numel(imgs_to_use));
        end
        % add the pos example to the background
        
    end    
                
    examples = examples(1:curr_filled,:);
    errors = errors(1:curr_filled);
    
    % svm training
    filename = sprintf('%s/face_checker_general_training_large_66_%d.mat', outputLocation, r);
    save(filename, 'examples', 'errors', 'alphas', 'betas', 'triangulation', 'minX', 'minY', 'nPix', 'shape', 'triX', 'mask', 'centres');
    
    
end

end

function [features] = ExtractFaceFeatures(img, labels, triangulation, triX, mask, alphas, betas, nPix, minX, minY)

    % Make sure labels are within range
    [hRes, wRes] = size(img);
    labels(labels(:,1) < 1,1) = 1;
    labels(labels(:,2) < 1,2) = 1;

    labels(labels(:,1) > wRes-1,1) = wRes-1;
    labels(labels(:,2) > hRes-1,2) = hRes-1;   

    crop_img = Crop(img, labels, triangulation, triX, mask, alphas, betas, nPix, minX, minY);
    crop_img(isnan(crop_img)) = 0;
    
    % vectorised version
    features = reshape(crop_img(logical(mask)), 1, nPix);

    % normalisations
    features = (features - mean(features));
    norms = std(features);
    if(norms==0)            
        norms = 1;
    end
    features = features / norms;

end