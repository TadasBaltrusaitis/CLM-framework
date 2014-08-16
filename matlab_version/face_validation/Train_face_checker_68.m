clear
location = './prep_data/';

faceCheckersLoc = dir([location 'face_checker_general_training_68_*']);   

training_ratio = 0.7;

rng(0);
for i=1:numel(faceCheckersLoc)
   
    load([location faceCheckersLoc(i).name]);
    
    num_examples = size(examples, 1);

    training_cutoff = round(num_examples * training_ratio);
    
    % picking training data for SVM (positive and negative samples)
    examples_train = examples(1:training_cutoff,:);
    
    % Extract the mean and standard deviation and normalise by it
    mean_ex = mean(examples_train);
    std_ex = std(examples_train);
    examples_train = bsxfun(@times, bsxfun(@minus, examples_train, mean_ex), 1./std_ex);
    errors_train = errors(1:training_cutoff);
    
    [prin_comps, score, latent] = pca(examples_train);
    
    % Keep enough data to explain 95 percent of variability
    variance_explained = cumsum(latent) / sum(latent);
    num_components = find(variance_explained > 0.90, 1, 'first');
    prin_comps = prin_comps(:,1:num_components);
    
    examples_train_ld = prin_comps' * examples_train';
    examples_train_ld = examples_train_ld';
          
    min_err = min( errors(1:training_cutoff));
    max_err = max( errors(1:training_cutoff));
    
    labels_train = 2*((errors(1:training_cutoff) - min_err)/(max_err-min_err)-0.5);
    
    % liblinear SVR training
    addpath('C:\liblinear\matlab');
        
    cs = [-4:-1];
    ps = [-10:-2];
    
    cmd = ['-s 11 -B 1 -q '];
    
    res = zeros(numel(cs), numel(ps));
    for c=1:numel(cs)
        for p=1:numel(ps)

            validation_p = sprintf('%s -v 5 -p %f -c %f', cmd, 10^cs(c), 2^ps(p));
            rms_valid = train(labels_train, sparse(double(examples_train_ld)), validation_p);
            res(c,p) = rms_valid;
        end
    end
    
    [val,~] = min(min(res));
    [a, b] = ind2sub(size(res), find(res == val));

    best_c = cs(a);
    best_p = ps(b);

    best_params = sprintf('%s -p %f -c %f', cmd, 10^best_c, 2^best_p);
    
    regressor_lsvr = train(labels_train, sparse(double(examples_train_ld)), best_params);
    
    w = regressor_lsvr.w(1:end-1)';
    b = regressor_lsvr.w(end);
    
    examples_test = examples(training_cutoff+1:end,:);
    examples_test = bsxfun(@times, bsxfun(@minus, examples_test, mean_ex), 1./std_ex);
    
    examples_test_ld = prin_comps' * examples_test';
    examples_test_ld = examples_test_ld';
    
    labels_test = 2*((errors(training_cutoff+1:end) - min_err)/(max_err-min_err)-0.5);
    
    dec = examples_test_ld * w + b;
    
    rmse = sqrt(mean((dec - labels_test).^2));
    corr_dec = corr(dec, labels_test);
    
    % Need to decide on a threshold for positive and negative
    
    thresh_corr = -0.85;
        
    class_test = zeros(size(labels_test));
    class_predict = zeros(size(labels_test));

    class_test(labels_test < thresh_corr) = 1;
    class_test(labels_test >= thresh_corr) = -1;
    
    class_predict(dec < thresh_corr) = 1;
    class_predict(dec >= thresh_corr) = -1;
        
    TP = sum(class_predict==1 & class_test == 1);
    FP = sum(class_predict==1 & class_test == -1);
    FN = sum(class_predict==-1 & class_test == 1);

    Precission = TP / (TP+FP);
    Recall = TP / (TP+FN);

    F1 = 2 * (Precission * Recall) / (Precission + Recall);      
    
    faceChecker.corr = corr_dec;
    faceChecker.principal_components = prin_comps;
    faceChecker.mean_ex = mean_ex;
    faceChecker.std_ex = std_ex;
    faceChecker.triangulation = triangulation;
    faceChecker.centres = centres;
    faceChecker.triX = triX;
    faceChecker.nPix = nPix;
    faceChecker.minX = minX;
    faceChecker.minY = minY;
    faceChecker.source = shape(:,1:2);       
    faceChecker.mask = mask;
    faceChecker.alphas = alphas;
    faceChecker.betas = betas;
    faceChecker.w = w;
    faceChecker.b = b;
    
    if(i==1)
        faceCheckers = faceChecker;
    else
        faceCheckers = [faceCheckers faceChecker];
    end
end

locationOut = './trained/face_check_general_68.txt';
locationOutM = './trained/face_check_general_68.mat';

WriteOutFaceCheckers(locationOut, locationOutM, faceCheckers);

% as a side effect write out a triangulation file as well
WriteOutTriangulation('./trained/tris_68.txt', './trained/tris_68.mat', faceCheckers);
