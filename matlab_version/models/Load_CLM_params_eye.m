function [ clmParams, pdm_right, pdm_left ] = Load_CLM_params_eye() 
%LOAD_CLM_PARAMS_WILD Summary of this function goes here
%   Detailed explanation goes here
    clmParams.window_size = [15,15; 13,13;];
    clmParams.numPatchIters = size(clmParams.window_size,1);
    
    % the PDM created from in the wild data
    pdmLoc = ['../models/hierarch_pdm/pdm_6_r_eye.mat'];

    load(pdmLoc);

    pdm_right = struct;
    pdm_right.M = double(M);
    pdm_right.E = double(E);
    pdm_right.V = double(V);

    pdmLoc = ['../models/hierarch_pdm/pdm_6_l_eye.mat'];

    load(pdmLoc);

    pdm_left = struct;
    pdm_left.M = double(M);
    pdm_left.E = double(E);
    pdm_left.V = double(V);
    
    % the default model parameters to use
    clmParams.regFactor = 0.1;               
    clmParams.sigmaMeanShift = 2;
    clmParams.tikhonov_factor = 0;

    clmParams.startScale = 1;
    clmParams.num_RLMS_iter = 5;
    clmParams.fTol = 0.01;
    clmParams.useMultiScale = true;
    clmParams.use_multi_modal = 1;
    clmParams.tikhonov_factor = 0;
end

