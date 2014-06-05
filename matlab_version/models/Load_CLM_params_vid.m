function [ clmParams, pdm ] = Load_CLM_params_vid() 
%LOAD_CLM_PARAMS_WILD Summary of this function goes here
%   Detailed explanation goes here
    clmParams.window_size = [21,21; 19,19; 17,17;];
    clmParams.numPatchIters = size(clmParams.window_size,1);
    
    % the PDM created from Multi-PIE data
    pdmLoc = ['../models/pdm/pdm_68_multi_pie.mat'];

    load(pdmLoc);

    pdm = struct;
    pdm.M = double(M);
    pdm.E = double(E);
    pdm.V = double(V);

    % the default model parameters to use
    clmParams.regFactor = 25;               
    clmParams.sigmaMeanShift = 2;
    clmParams.tikhonov_factor = 5;

    clmParams.startScale = 1;
    clmParams.num_RLMS_iter = 10;
    clmParams.fTol = 0.01;
    clmParams.useMultiScale = true;
    clmParams.use_multi_modal = 1;

end

