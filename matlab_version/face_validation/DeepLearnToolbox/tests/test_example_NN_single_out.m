%function test_example_NN
load mnist_uint8;

train_x = double(train_x) / 255;
test_x  = double(test_x)  / 255;
train_y = double(train_y);
test_y  = double(test_y);

train_y = train_y(:,1);
test_y = test_y(:,1);

% normalize
[train_x, mu, sigma] = zscore(train_x);
test_x = normalize(test_x, mu, sigma);

%% ex1 vanilla neural net
rand('state',0)
nn = nnsetup([784 100 1]);
nn.output = 'sigm';
opts.numepochs =  5;   %  Number of full sweeps through data
opts.batchsize = 100;  %  Take a mean gradient step over this many samples
[nn, L] = nntrain(nn, train_x, train_y, opts);

% [er, bad] = nntest(nn, test_x, test_y);
nn = nnff(nn, test_x, zeros(size(test_x,1), nn.size(end)));
% pred_y = nnpredict(nn, test_x);
pred_y = nn.a{end};

fprintf('Prediction error 1 %f\n', sqrt(mean((pred_y - test_y).^2)));

% assert(er < 0.08, 'Too big error');

%% ex2 neural net with L2 weight decay
rand('state',0)
nn = nnsetup([784 100 1]);

nn.weightPenaltyL2 = 1e-4;  %  L2 weight decay
opts.numepochs =  5;        %  Number of full sweeps through data
opts.batchsize = 100;       %  Take a mean gradient step over this many samples

nn = nntrain(nn, train_x, train_y, opts);

nn = nnff(nn, test_x, zeros(size(test_x,1), nn.size(end)));
% pred_y = nnpredict(nn, test_x);
pred_y = nn.a{end};

fprintf('Prediction error 2 %f\n', sqrt(mean((pred_y - test_y).^2)));



%% ex3 neural net with dropout
rand('state',0)
nn = nnsetup([784 100 1]);

nn.dropoutFraction = 0.5;   %  Dropout fraction 
opts.numepochs =  5;        %  Number of full sweeps through data
opts.batchsize = 100;       %  Take a mean gradient step over this many samples

nn = nntrain(nn, train_x, train_y, opts);

nn = nnff(nn, test_x, zeros(size(test_x,1), nn.size(end)));
% pred_y = nnpredict(nn, test_x);
pred_y = nn.a{end};

fprintf('Prediction error 3 %f\n', sqrt(mean((pred_y - test_y).^2)));


%% ex4 neural net with sigmoid activation function
rand('state',0)
nn = nnsetup([784 100 1]);

nn.activation_function = 'sigm';    %  Sigmoid activation function
nn.learningRate = 1;                %  Sigm require a lower learning rate
opts.numepochs =  5;                %  Number of full sweeps through data
opts.batchsize = 100;               %  Take a mean gradient step over this many samples

nn = nntrain(nn, train_x, train_y, opts);

nn = nnff(nn, test_x, zeros(size(test_x,1), nn.size(end)));
% pred_y = nnpredict(nn, test_x);
pred_y = nn.a{end};

fprintf('Prediction error 4 %f\n', sqrt(mean((pred_y - test_y).^2)));


%% ex5 plotting functionality
rand('state',0)
nn = nnsetup([784 20 1]);
opts.numepochs         = 5;            %  Number of full sweeps through data
nn.output              = 'sigm';    %  use softmax output
opts.batchsize         = 1000;         %  Take a mean gradient step over this many samples
opts.plot              = 1;            %  enable plotting

nn = nntrain(nn, train_x, train_y, opts);

nn = nnff(nn, test_x, zeros(size(test_x,1), nn.size(end)));
% pred_y = nnpredict(nn, test_x);
pred_y = nn.a{end};

fprintf('Prediction error 5 %f\n', sqrt(mean((pred_y - test_y).^2)));


%% ex6 neural net with sigmoid activation and plotting of validation and training error
% split training data into training and validation data
vx   = train_x(1:10000,:);
tx = train_x(10001:end,:);
vy   = train_y(1:10000,:);
ty = train_y(10001:end,:);

rand('state',0)
nn                      = nnsetup([784 20 1]);     
nn.output               = 'sigm';                   %  use softmax output
opts.numepochs          = 5;                           %  Number of full sweeps through data
opts.batchsize          = 1000;                        %  Take a mean gradient step over this many samples
opts.plot               = 1;                           %  enable plotting
nn = nntrain(nn, tx, ty, opts, vx, vy);                %  nntrain takes validation set as last two arguments (optionally)

nn = nnff(nn, test_x, zeros(size(test_x,1), nn.size(end)));
% pred_y = nnpredict(nn, test_x);
pred_y = nn.a{end};

fprintf('Prediction error 6 %f\n', sqrt(mean((pred_y - test_y).^2)));

