clear

root_loc = 'E:/datasets/detection_validation/';

location = [root_loc, '/prep_data/'];

faceCheckersLoc = dir([location 'face_checker_general_training_large_68_*']);   

training_ratio = 0.9;

rng(0);
for i=1:numel(faceCheckersLoc)
   
    load([location faceCheckersLoc(i).name]);
    
    % set a max value to the error
    errors(errors > 3) = 3;
    
    num_examples = size(examples, 1);

    training_cutoff = round(num_examples * training_ratio);
    
    % Randomise the samples before training
    new_inds = randperm(size(examples,1));
    examples = examples(new_inds, :);
    errors = errors(new_inds);
        
    % Extract the mean and standard deviation and normalise by it, the
    % examples have been normalised individually, but this is a global
    % normalisation that follows it 
    
    % picking training data for normalisation
    examples_train = examples(1:training_cutoff,:);
    
    mean_ex = mean(examples_train);
    std_ex = std(examples_train);
        
    examples = bsxfun(@times, bsxfun(@minus, examples, mean_ex), 1./std_ex);
        
    kernel_1_size = 7;
    kernel_2_size = 5;
        
    % Convert the examples to wanted format (number rows and cols to allow from proper subsampling)
    
    % keep adding rows
    while(mod(size(mask,1)-kernel_1_size + 1, 2) == 1 || mod((size(mask,1)-kernel_1_size + 1)/2 - kernel_2_size + 1, 2) == 1)        
        mask = cat(1, mask, false(1, size(mask,2)));
        triX = cat(1, triX, -ones(1, size(mask,2)));
    end
    
    % keep adding cols
    while(mod(size(mask,2)-kernel_1_size + 1, 2) == 1 || mod((size(mask,2)-kernel_1_size + 1)/2 - kernel_2_size + 1, 2) == 1)        
        mask = cat(2, mask, false(size(mask,1),1));
        triX = cat(2, triX, -ones(size(triX,1),1));
    end

    examples_r = zeros(size(mask, 1), size(mask, 2), num_examples);
    
    img_curr = zeros(size(mask));
    for e=1:num_examples
        
        img_curr(mask) = examples(e,:);
        examples_r(:, :, e) = img_curr;
        
    end
    
    % picking training data for SVM (positive and negative samples)
    examples_train = examples_r(:, :, 1:training_cutoff);
    
    examples_valid = examples_r(:, :, training_cutoff+1:end);
    
    % Normalise to 0-1 labels
    labels_train = errors(1:training_cutoff) / 3;
    labels_valid = errors(training_cutoff+1:end,:)/3;
    
    addpath(genpath('DeepLearnToolbox'));
               
    % This needs to be validated
    
    num_kern = [3];
    alphas_learn = [0.5, 0.75, 1];
    
    res = zeros(numel(num_kern), numel(alphas_learn));
    corrs = zeros(numel(num_kern), numel(alphas_learn));
    
    % Set up model options
    opts.batchsize = 50;    
    
    % Can go up if needed
    opts.numepochs = 150;
    
    cnns = cell(numel(num_kern), numel(alphas_learn));

    for n=1:numel(num_kern)
        
        for a=1:numel(alphas_learn)
                        
            %%
            opts.alpha = alphas_learn(a);

            rand(0);

            cnn.layers = {
                struct('type', 'i') %input layer
                struct('type', 'c', 'outputmaps', num_kern(n), 'kernelsize', kernel_1_size) %convolution layer
                struct('type', 's', 'scale', 2) %sub sampling layer
                struct('type', 'c', 'outputmaps', num_kern(n) * 2, 'kernelsize', kernel_2_size) %convolution layer
                struct('type', 's', 'scale', 2) %subsampling layer
            };

            cnn = cnnsetup(cnn, examples_train, labels_train');

            % For some reason the model does not always converge properly
            cnn = cnntrain(cnn, examples_train, labels_train', opts);

            cnn = cnnff(cnn, examples_valid);

            pred_y = cnn.o';     

            res(n, a) = sqrt(mean((labels_valid - pred_y).^2));
            corrs(n, a) = corr(labels_valid, pred_y); 

            fprintf('curr corr res %d layers: %f alpha %f\n', num_kern(n), alphas_learn(a), corrs(n, a));

            % A hack to save space
            cnn = cnnff(cnn, examples_valid(:,:,1));

            cnn.o = [];            
            cnn.rL = [];
            cnn.fV = [];
            cnn.fvd = [];
            cnn.dffW = [];
            
            for l=1:numel(cnn.layers)
                cnn.layers{l}.a = [];
                if(isfield(cnn.layers{l}, 'd'))
                    cnn.layers{l}.d = [];
                end
            end            
            
            cnns(n, a) = {cnn};
        end
        
    end
    
    [val,~] = min(res(:));
    [a, b] = ind2sub(size(res), find(res == val));
    fprintf('---------------------------------\n');
    fprintf('Best model num kernels %d, alpha %f\n', num_kern(a), alphas_learn(b));
    fprintf('---------------------------------\n');
    
    cnn = cnns{a, b};
    
    face_check_cnns(i).cnn = cnn;
    face_check_cnns(i).destination = shape(:,1:2);    
    face_check_cnns(i).triangulation = triangulation;
    face_check_cnns(i).triX = triX;
    face_check_cnns(i).mask = mask;
    face_check_cnns(i).centres = centres;
    face_check_cnns(i).alphas = alphas;
    face_check_cnns(i).betas = betas;
    face_check_cnns(i).minX = minX;
    face_check_cnns(i).minY = minY;
    face_check_cnns(i).nPix = nPix;
    face_check_cnns(i).mean_ex = mean_ex;
    face_check_cnns(i).std_ex = std_ex;
end

%the actual testing before it is released
% Create the function
face_check_fun = @(img, shape, global_params) face_check_cnn(img, shape, global_params, face_check_cnns);

[ predictions, gts, rmse, corr_coeff ] = Test_face_checker(face_check_fun);

locationOut = ['./trained/face_check_cnn_68.txt'];
locationOutM = ['./trained/face_check_cnn_68.mat'];

save(locationOutM, 'face_check_cnns', 'rmse', 'gts', 'predictions', 'corr_coeff');

WriteOutFaceCheckersCNNbinary(locationOut, face_check_cnns);

% as a side effect write out a triangulation file as well
WriteOutTriangulation('./trained/tris_68.txt', './trained/tris_68.mat', face_check_cnns);
