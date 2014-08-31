function [ coeffs ] = CalculateCoefficients( alphas, betas, triangulation, destination )
%CALCULATECOEFFICIENTS Summary of this function goes here
%   Detailed explanation goes here

    xs = destination(:,1);
    ys = destination(:,2);
    triangulation = triangulation + 1;
    numTri = size(triangulation, 1);
    coeffs = zeros(numTri, 3);
    
    for l=1:numTri

        i = triangulation(l,1);
        j = triangulation(l,2);
        k = triangulation(l,3);

        c1 = xs(i);
        c2 = xs(j) - c1;
        c3 = xs(k) - c1;
        c4 = ys(i);
        c5 = ys(j) - c4;
        c6 = ys(k) - c4;

        coeffs(l,1) = c1 + c2*alphas(l,1) + c3*betas(l,1);
        coeffs(l,2) =      c2*alphas(l,2) + c3*betas(l,2);
        coeffs(l,3) =      c2*alphas(l,3) + c3*betas(l,3);
        coeffs(l,4) = c4 + c5*alphas(l,1) + c6*betas(l,1);
        coeffs(l,5) =      c5*alphas(l,2) + c6*betas(l,2);
        coeffs(l,6) =      c5*alphas(l,3) + c6*betas(l,3);
    end

end

