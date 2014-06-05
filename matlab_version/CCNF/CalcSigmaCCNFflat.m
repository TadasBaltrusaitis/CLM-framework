function [ SigmaInv] = CalcSigmaCCNFflat(alphas, betas, n, precalcQ2withoutBeta, precalc_eye, precalc_zeros)
%CALCSIGMAPRF Summary of this function goes here
%   Detailed explanation goes here
% constructing the sigma
 
%     A = zeros(n);
%          
%     for i=1:n
% 
%         A(i,i) = alphas' * mask(i,:)';
% 
%     end

    % this is simplification of above code
%     if(useIndicators)
%         A = diag(mask * alphas);
%     else

%         A = sum(alphas) .* eye(n);
        A = sum(alphas) .* precalc_eye;
%         A = sum(alphas) * eye(n);
        % not faster
%         a = mtimesx(sum(alphas), eye(n), 'SPEED');
%         a2 = mtimesx(sum(alphas), eye(n), 'SPEEDOMP');
%     end
        
    % calculating the B from the paper
    
%     for i=1:n
%         for j=1:n
% 
%             if(i == j)
%                 q2(i,j) = beta * (sum(S(i,:)) - S(i,i));
%             else
%                 q2(i,j) = -beta * S(i,j);
%             end
%         end            
%     end

    % the above code can be simplified by the following lines of code     
    % using the precalculated lower triangular elements of B without beta
    Btmp = precalcQ2withoutBeta * betas;        
    % not faster
%     Btmp = mtimesx(precalcQ2withoutBeta, betas, 'SPEED');
%     Btmp = mtimesx(precalcQ2withoutBeta, betas, 'SPEEDOMP');
    % now make it into a square symmetric matrix
%     B = zeros(n,n);
    B = precalc_zeros;
    on = tril(true(n,n));
    B(on) = Btmp;
    B = B';
    B(on) = Btmp;
    
    SigmaInv = 2 * (A + B);

end

