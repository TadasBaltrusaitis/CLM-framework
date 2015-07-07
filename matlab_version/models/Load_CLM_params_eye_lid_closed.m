function [ clmParams, pdm_right, pdm_left ] = Load_CLM_params_eye_lid_closed() 
%LOAD_CLM_PARAMS_WILD Summary of this function goes here
%   Detailed explanation goes here
    clmParams.window_size = [19,19; 19,19; 19,19; 19,19];
    clmParams.numPatchIters = size(clmParams.window_size,1);
    
    % the PDM created from in the wild data
    pdmLoc = ['../models/pdm/pdm_6_eye_3D_lid_with_closed.mat'];

    load(pdmLoc);

    pdm_right = struct;
    pdm_right.M = double(M);
    pdm_right.E = double(E);
    pdm_right.V = double(V);

    pdmLoc = ['../models/pdm/pdm_6_l_eye_3D_lid_with_closed.mat'];

    load(pdmLoc);

    pdm_left = struct;
    pdm_left.M = double(M);
    pdm_left.E = double(E);
    pdm_left.V = double(V);
    
    % the default model parameters to use
    clmParams.regFactor = 5;               
    clmParams.sigmaMeanShift = 1.5;
    clmParams.tikhonov_factor = 0;

    clmParams.startScale = 1;
    clmParams.num_RLMS_iter = 10;
    clmParams.fTol = 0.01;
    clmParams.useMultiScale = true;
    clmParams.use_multi_modal = 1;

end

