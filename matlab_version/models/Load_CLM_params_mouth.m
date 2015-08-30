function [ clmParams, pdm_mouth] = Load_CLM_params_mouth() 
%LOAD_CLM_PARAMS_WILD Summary of this function goes here
%   Detailed explanation goes here
    clmParams.window_size = [17,17; 17,17];
    clmParams.numPatchIters = size(clmParams.window_size,1);
    
    % the PDM created from in the wild data
    pdmLoc = ['../models/hierarch_pdm/pdm_20_mouth.mat'];

    load(pdmLoc);

    pdm_mouth = struct;
    pdm_mouth.M = double(M);
    pdm_mouth.E = double(E);
    pdm_mouth.V = double(V);
    
    % the default model parameters to use
    clmParams.regFactor = 1;               
    clmParams.sigmaMeanShift = 2.0;
    clmParams.tikhonov_factor = 0;

    clmParams.startScale = 1;
    clmParams.num_RLMS_iter = 5;
    clmParams.fTol = 0.01;
    clmParams.useMultiScale = true;
    clmParams.use_multi_modal = 1;

end