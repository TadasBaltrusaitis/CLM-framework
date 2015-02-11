function [ clmParams, pdm ] = Load_CLM_params_eye() 
%LOAD_CLM_PARAMS_WILD Summary of this function goes here
%   Detailed explanation goes here
    clmParams.window_size = [19,19; 17,17; 15,15];
    clmParams.numPatchIters = size(clmParams.window_size,1);
    
    % the PDM created from in the wild data
    pdmLoc = ['C:\Users\Tadas\Dropbox\AAM\patch_experts_eyes\data_preparation\pdm_generation\eye_pdm/pdm_20_eye.mat'];

    load(pdmLoc);

    pdm = struct;
    pdm.M = double(M);
    pdm.E = double(E);
    pdm.V = double(V);

    % the default model parameters to use
    clmParams.regFactor = 1;               
    clmParams.sigmaMeanShift = 1;
    clmParams.tikhonov_factor = 0;

    clmParams.startScale = 1;
    clmParams.num_RLMS_iter = 10;
    clmParams.fTol = 0.01;
    clmParams.useMultiScale = true;
    clmParams.use_multi_modal = 1;

end

