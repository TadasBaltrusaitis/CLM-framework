clear

root_loc = 'F:/datasets/detection_validation/';

location = [root_loc, '/prep_data/'];

faceCheckersLoc = dir([location 'face_checker_general_training_large_68_*']);   

training_ratio = 0.7;

rng(0);
for i=1:numel(faceCheckersLoc)
   
    load([location faceCheckersLoc(i).name]);
    
    % set a max value to the error
    errors(errors > 3) = 3;
    
    num_examples = size(examples, 1);

    training_cutoff = round(num_examples * training_ratio);
    
    % picking training data for SVM (positive and negative samples)
    examples_train = examples(1:training_cutoff,:);
    
    % Extract the mean and standard deviation and normalise by it
    mean_ex = mean(examples_train);
    std_ex = std(examples_train);
    examples_train = bsxfun(@times, bsxfun(@minus, examples_train, mean_ex), 1./std_ex);
    
    examples_valid = examples(training_cutoff+1:end,:);
    examples_valid = bsxfun(@times, bsxfun(@minus, examples_valid, mean_ex), 1./std_ex);
        
    errors_train = errors(1:training_cutoff);
    
    % Normalise to 0-1 labels
    labels_train = errors(1:training_cutoff) / 3;
    labels_valid = errors(training_cutoff+1:end,:)/3;
    
    % liblinear SVR training
    addpath(genpath('DeepLearnToolbox'));
    
    layer_depth = [1, 2];
    
    neural_layers = [5, 10, 20, 50];
    
    res = zeros(numel(neural_layers), numel(layer_depth));
    corrs = zeros(numel(neural_layers), numel(layer_depth));
    
    for n=1:numel(neural_layers)
        for l=1:numel(layer_depth)
            
            %% Vanilla neural net
            rand(0);
            
            % input layer
            layers = size(examples_train,2);
            
            % add hidden layers between input and output
            for nl=1:layer_depth(l)
                layers = cat(1, layers, neural_layers(n));
            end
            
            % output layer
            layers = cat(1, layers, 1);
            
            nn = nnsetup(layers);
            nn.output = 'sigm';
            opts.numepochs =  100;   %  Number of full sweeps through data
            opts.batchsize = 200;  %  Take a mean gradient step over this many samples
            opts.plot = 0;
            [nn, L] = nntrain(nn, examples_train, labels_train, opts, examples_valid, labels_valid);

            % [er, bad] = nntest(nn, test_x, test_y);
            nn = nnff(nn, examples_valid, zeros(size(examples_valid,1), nn.size(end)));
            % pred_y = nnpredict(nn, test_x);
            pred_y = nn.a{end};     

            rms_valid = sqrt(mean((labels_valid - pred_y).^2));

            res(n, l) = rms_valid;
            corrs(n, l) = corr(labels_valid, pred_y);
        end
    end
    
    [val,~] = min(res(:));
    [a, b] = ind2sub(size(res), find(res == val));

    layers = size(examples_train,2);
    
    % add hidden layers between input and output
    for nl=1:layer_depth(b)
        layers = cat(1, layers, neural_layers(a));
    end
    
    % output layer
    layers = cat(1, layers, 1);
    
    nn = nnsetup(layers);
    
    examples = bsxfun(@times, bsxfun(@minus, examples, mean_ex), 1./std_ex);    
    
    [nn, L] = nntrain(nn, examples, errors/3, opts);
               
    face_check_nns(i).nn = nn;
    face_check_nns(i).destination = shape(:,1:2);
    face_check_nns(i).triangulation = triangulation;
    face_check_nns(i).triX = triX;
    face_check_nns(i).mask = mask;
    face_check_nns(i).centres = centres;
    face_check_nns(i).alphas = alphas;
    face_check_nns(i).betas = betas;
    face_check_nns(i).minX = minX;
    face_check_nns(i).minY = minY;
    face_check_nns(i).nPix = nPix;
    face_check_nns(i).mean_ex = mean_ex;
    face_check_nns(i).std_ex = std_ex;
end

%the actual testing before it is released
% Create the function
face_check_fun = @(img, shape, global_params) face_check_nn(img, shape, global_params, face_check_nns);

[ predictions, gts, rmse, corr_coeff ] = Test_face_checker(face_check_fun);

locationOut = [root_loc, './trained/face_check_nn_68.txt'];
locationOutM = [root_loc, './trained/face_check_nn_68.mat'];

save(locationOutM, 'face_check_nns', 'rmse', 'gts', 'predictions', 'corr_coeff');

WriteOutFaceCheckersNNbinary(locationOut, face_check_nns);
% WriteOutFaceCheckers(locationOut, locationOutM, faceCheckers);

% as a side effect write out a triangulation file as well
% WriteOutTriangulation('./trained/tris_68.txt', './trained/tris_68.mat', faceCheckers);

% Results, with global normalisation