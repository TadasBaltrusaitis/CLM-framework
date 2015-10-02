function [ accuracies, F1s, corrs, ccc, rms, classes ] = evaluate_au_prediction_results( labels, ground_truth )
%EVALUATE_CLASSIFICATION_RESULTS Summary of this function goes here
%   Detailed explanation goes here

    classes = sort(unique(ground_truth));
    accuracies = zeros(numel(classes),1);
    F1s = zeros(numel(classes),1);
    
    corrs = corr(labels, ground_truth);
    
    rms = sqrt(mean((labels-ground_truth).^2));
        
    std_g = std(ground_truth);
    std_p = std(labels);
    
    ccc = 2 * corrs * std_g * std_p / (std_g^2 + std_p^2 + (mean(labels) - mean(ground_truth))^2);
    
    % the label is taken to belong to a class it is closest to
    label_dists = zeros(numel(labels), numel(classes));
    
    for i=1:numel(classes)
        label_dists(:,i) = abs(labels - classes(i));
    end
    
    [~, labels] = min(label_dists');
    labels = labels';
    
    for i=1:numel(classes)
       labels(labels==i) = classes(i); 
    end
    
    for i=1:numel(classes)
        
        pos_samples = ground_truth == classes(i);
        neg_samples = ground_truth ~= classes(i);
        
        pos_labels = labels == classes(i);
        neg_labels = labels ~= classes(i);
        
        TPs = sum(pos_samples & pos_labels);
        TNs = sum(neg_samples & neg_labels);
        
        FPs = sum(pos_labels & neg_samples);
        FNs = sum(neg_labels & pos_samples);
        
        accuracies(i) = (TPs + TNs) / numel(pos_samples);
               
        F1s(i) = 2 * TPs / (2*TPs + FNs + FPs);
        
    end
    
    
end

