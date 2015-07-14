function [ clmParams, pdm_mouth] = Load_CLM_params_brows() 
%LOAD_CLM_PARAMS_WILD Summary of this function goes here
%   Detailed explanation goes here
    clmParams.window_size = [21,21; 19,19];
    clmParams.numPatchIters = size(clmParams.window_size,1);
    
    % the PDM created from in the wild data
    pdmLoc = ['../models/hierarch_pdm/pdm_10_brows.mat'];

    load(pdmLoc);

    pdm_mouth = struct;
    pdm_mouth.M = double(M);
    pdm_mouth.E = double(E);
    pdm_mouth.V = double(V);
    
    % the default model parameters to use
    clmParams.regFactor = 10;               
    clmParams.sigmaMeanShift = 3.5;
    clmParams.tikhonov_factor = 0;

    clmParams.startScale = 1;
    clmParams.num_RLMS_iter = 5;
    clmParams.fTol = 0.01;
    clmParams.useMultiScale = true;
    clmParams.use_multi_modal = 1;

end