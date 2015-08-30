function [ final_global, final_local, final_lhood, landmark_lhoods ] = NU_RLMS( ...
    init_global, init_local, PDM, patchResponses, visibilities,...
    view, reliabilities, baseShape, OrigToRefTransform, rigid, ...
    clmParams, gauss_resp)
%RLMS Summary of this function goes here
%   Detailed explanation goes here

m = numel(PDM.E);
E = PDM.E;
V = PDM.V;
    
n = size(V, 1)/3;

current_local = init_local;
current_global = init_global;

pxWidth = size(patchResponses{1},1);
responseSize = size(patchResponses{1});

[iis,jjs] = meshgrid(1:pxWidth, 1:pxWidth);

% Grab all of the patch responses and convert them to single matrix
% representation for speed
patchResponsesFlat = reshape(cat(3,patchResponses{:}), responseSize(1)*responseSize(2), n)';

iisFlat = repmat(iis(:)', n, 1);
jjsFlat = repmat(jjs(:)', n, 1);

% An alternative formulation
reg_rigid = zeros(6,1);

if(rigid)
    regularisations = reg_rigid;
    regularisations = diag(regularisations);
else
    regularisations = [reg_rigid; clmParams.regFactor ./ E]; % the above version, however, does not perform as well    
    regularisations =  diag(regularisations);
end

% For generalised Tikhonov
if(clmParams.tikhonov_factor == 0)
    P = eye(n*2);
else
    % the worse the reliability the higher the variance of the
    % prediction, so inverse variance correlate with reliability
    P = clmParams.tikhonov_factor * diag(repmat(reliabilities',2,1));        

end

for iter = 1:clmParams.num_RLMS_iter
    
    % get the current estimates of x and y in image
    currentShape = GetShapeOrtho(PDM.M, PDM.V, current_local, current_global);
    currentShape = currentShape(:,1:2);
    
    if(iter > 1)
        % if the shape hasn't changed terminate
        if(norm(currentShape - previousShape) < clmParams.fTol)
            break;
        end
    end
    
    previousShape = currentShape;

    % calculate the appropriate Jacobians in 2D, even though the actual behaviour is in 3D, using small angle approximation and oriented shape
    if(rigid)
        J = CalcRigidJacobian(PDM.M, PDM.V, current_local, current_global);
    else
        J = CalcJacobian(PDM.M, PDM.V, current_local, current_global);
    end
    
    % as the mean shift is with reference to the point, we don't care about
    % the translation
    OrigToRefTransform.tdata.T(3,1:2) = 0;
    OrigToRefTransform.tdata.Tinv(3,1:2) = 0;    

    % distance from center where the response was calculated around
    % in reference frame of the patch
    offsets = (currentShape - baseShape) * OrigToRefTransform.tdata.T(1:2,1:2)';
    
    % perform the parallel version of the mean shift algorithm
    dxs = offsets(:, 1) + (pxWidth-1)/2 + 1;
    dys = offsets(:, 2) + (pxWidth-1)/2 + 1;
    
    if(numel(gauss_resp) > 0)
        meanShifts = meanShiftParallel_precalc(patchResponsesFlat, dxs, dys, iisFlat, jjsFlat, responseSize, gauss_resp.kd_precalc, gauss_resp.stepSize);
    else
        meanShifts = meanShiftParallel(patchResponsesFlat, clmParams.sigmaMeanShift, dxs, dys, iisFlat, jjsFlat, responseSize);
    end
    
    % invalidate illegal mean shifts
    illegal_inds = find(~visibilities(view, :));
    J(illegal_inds,:) = 0;
    J(illegal_inds + n,:) = 0;
    meanShifts(illegal_inds,:) = 0;
    
    % Mean shift's here are calculate in the reference image frame,
    % we want to move them back to actual image frame   
    meanShifts = meanShifts * OrigToRefTransform.tdata.Tinv(1:2,1:2)';

    % put it into column format
    meanShifts = meanShifts(:);
        
    rigid_params = current_global - init_global;
    rigid_params(2:4) = 0;
    
    if(rigid)
        params = rigid_params;
    else
        params = [rigid_params; current_local];
    end
    
    params_delta = (J'*P*J + regularisations) \ (J'*P*meanShifts - regularisations*params);
    
    % update the reference    
    [current_local, current_global] = CalcReferenceUpdate(params_delta, current_local, current_global);
    
    if(~rigid)
        % clamp to the local parameters for valid expressions
        current_local = ClampPDM(current_local, E);
    end    
    
end

if(nargout >= 4)
    
    % get the current estimates of x and y in image
    currentShape = GetShapeOrtho(PDM.M, PDM.V, current_local, current_global);
    currentShape = currentShape(:,1:2);
    
    
    % as the mean shift is with reference to the point, we don't care about
    % the translation
    OrigToRefTransform.tdata.T(3,1:2) = 0;
    OrigToRefTransform.tdata.Tinv(3,1:2) = 0;    

    % distance from center where the response was calculated around
    % in reference frame of the patch
    offsets = (currentShape - baseShape) * OrigToRefTransform.tdata.T(1:2,1:2)';
    
    % perform the parallel version of the mean shift algorithm
    dxs = offsets(:, 1) + (pxWidth-1)/2 + 1;
    dys = offsets(:, 2) + (pxWidth-1)/2 + 1;    
    
    landmark_lhoods = zeros(n,1);

    prob = 0;
    for i=1:n
        
        if(visibilities(view, i))
            dx = dxs(i);
            dy = dys(i);
            vxs = (-iis+dx).^2;
            vys = (-jjs+dy).^2;

            % Calculate the kde per patch
            vs = patchResponses{i}.*exp(-0.5*(vxs + vys)/clmParams.sigmaMeanShift);

            kde_est = sum(vs(:));
            landmark_lhoods(i) = kde_est;
            prob = prob + log(kde_est + 1e-8);
        end
        
    end
    % Do not add the local parameter prior as it overpowers the
    % log-likelihoods
    final_lhood = prob / sum(visibilities(view, :));
    % - LogPDMprior(current_local, E);
end
final_global = current_global;
final_local = current_local;

end

% This clamps the non-rigid parameters to stay within +- 3 standard
% deviations
function [non_rigid_params] = ClampPDM(non_rigid, E)

    stds = sqrt(E);
    
    non_rigid_params = non_rigid;
    
    lower = non_rigid_params < -3 * stds;
    non_rigid_params(lower) = -3*stds(lower);
    
    higher = non_rigid_params > 3 * stds;
    non_rigid_params(higher) = 3*stds(higher);

end

% This calculate the mean shift based on the kernel density response at dx,
% dy in the patch response, this can be used to find the mode
function [meanShifts] = meanShiftParallel(patchResponses, sigma, dxs, dys, iis, jjs, patchSize)
       
    % Kernel density is
    % K(x_i-x) = p(x_i)*exp(-0.5 * ||x_i-x||^2/sigma), so probability weighted
    % distance from the center

    % Mean shift is then m(x) = sum(K(x_i - dx)*x_i)/sum(K(x_i-dx))
%     step_size = 0.1;
%     gauss_resp_prec = precalc_kernel_densities(patchSize, sigma, step_size);
%     
%     %iis are row vectors of the locations of interest, for each patch
%     nYs = numel(0:step_size:patchSize(2));
%     % calculate the indices needed
%     
%     dxs2 = dxs - mod(dxs, step_size);
%     dys2 = dys - mod(dys, step_size);
%     
%     xs = round(dxs2 * nYs * 1/step_size + (dys2 /step_size) +1);
%     
%     gauss_resp_c = gauss_resp_prec(xs,:);
    
    % this part is doing (x_i - dx)^2
    vxs = bsxfun(@plus, -iis, dxs);
    vxs = vxs.^2;
    % this part is doing (y_i - dy)^2
    vys = bsxfun(@plus, -jjs, dys);
    vys = vys.^2;
    a = -0.5/(sigma.^2);
    % this part is calculating K(x_i - x)
    gauss_resp = exp(a*(vxs + vys));
    vs = patchResponses.*gauss_resp;

    % this part is calculating K(x_i - dx)*x_i
    mxss = vs.*iis;
    myss = vs.*jjs;
    
    % this part is caluclating sum(K(x_i - dx)*x_i)
    mxs = sum(mxss,2);
    mys = sum(myss,2);
    
    sumVs = sum(vs,2);
    sumVs(sumVs == 0) = 1;

    msx = mxs ./ sumVs - dxs;
    msy = mys ./ sumVs - dys;

    meanShifts = [msx, msy];
    
end

% This updates the parameters based on the updates from the RLMS
function [non_rigid, rigid] = CalcReferenceUpdate(params_delta, current_non_rigid, current_global)


    rigid = zeros(6, 1);
    % Same goes for scaling and translation parameters
    rigid(1) = current_global(1) + params_delta(1);
    rigid(5) = current_global(5) + params_delta(5);
    rigid(6) = current_global(6) + params_delta(6);
    
    % for rotation however, we want to make sure that the rotation matrix
    % approximation we have 
	% R' = [1, -wz, wy
	%       wz, 1, -wx
	%       -wy, wx, 1]	
    % is a legal rotation matrix, and then we combine it with current
    % rotation (through matrix multiplication) to acquire the new rotation
    
	R = Euler2Rot(current_global(2:4));

    wx = params_delta(2);
    wy = params_delta(3);
    wz = params_delta(4);
    
    R_delta = [1, -wz, wy;
               wz, 1, -wx;
               -wy, wx, 1];
	
	% Make sure R_delta is orthonormal
	R_delta = OrthonormaliseRotation(R_delta);
	
    % Combine rotations
	R_final = R * R_delta;

	% Extract euler angle
	euler = Rot2Euler(R_final);	
	
	rigid(2:4) = euler;
    
    if(length(params_delta) > 6)
        % non-rigid parameters can just be added together
        non_rigid = params_delta(7:end) +  current_non_rigid;
    else
        non_rigid = current_non_rigid;
    end
    
end

function R_ortho = OrthonormaliseRotation(R)
  
    % U * V' is basically what we want, as it's guaranteed to be
    % orthonormal
    [U, ~, V] = svd(R);

    % We also want to make sure no reflection happened
    
    % get the orthogonal matrix from the initial rotation matrix
    X = U*V';

    % This makes sure that the handedness is preserved and no reflection happened
    % by making sure the determinant is 1 and not -1
    W = eye(3);
    W(3,3) = det(X);
    R_ortho = U*W*V';
end

function [prob] = LogPDMprior(params, E)

    cov = diag(E);
    prob = 0.5 * params'*inv(cov)*params;

end