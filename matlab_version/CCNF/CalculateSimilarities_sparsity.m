function [ Similarities, PrecalcQ2s, PrecalcQ2sFlat, PrecalcYqDs ] = CalculateSimilarities_sparsity( n_sequences, x, similarityFNs, sparsityFNs, y, const)
%CALCULATESIMILARITIES Summary of this function goes here
%   Detailed explanation goes here

    K = numel(similarityFNs);
    K2 = numel(sparsityFNs);
    
    %calculate similarity measures for each of the sequences
    Similarities = cell(n_sequences, 1);
    PrecalcQ2s = cell(n_sequences,1);
    PrecalcQ2sFlat = cell(n_sequences,1);
    
    PrecalcYqDs = zeros(n_sequences, K + K2);
    
    if(iscell(x))
        for q = 1 : n_sequences

            xq = x{q};

            n = size(xq, 1);
            Similarities{q} = zeros([n, n, K+K2]);
            
            PrecalcQ2s{q} = cell(K+K2,1);

            PrecalcQ2sFlat{q} = zeros((n*(n+1))/2,K+K2);
            % go over all of the similarity metrics and construct the
            % similarity matrices

            if(nargin > 4)
                yq = y{q};
            end

            for k=1:K
                Similarities{q}(:,:,k) = similarityFNs{k}(xq);
                S = Similarities{q}(:,:,k);
                D =  diag(sum(S));
    %             PrecalcQ2s{q}(:,:,k) = D - S;
                PrecalcQ2s{q}{k} = D - S;
                B = D - S;
    %             PrecalcQ2sFlat{q}{k} = PrecalcQ2s{q}{k}(logical(tril(ones(size(S)))));
                PrecalcQ2sFlat{q}(:,k) = B(logical(tril(ones(size(S)))));
                if(nargin > 4)        
                    PrecalcYqDs(q,k) = -yq'*B*yq;
                end
            end
            for k=1:K2
                Similarities{q}(:,:,K+k) = sparsityFNs{k}(xq);
                S = Similarities{q}(:,:,K+k);
                D =  diag(sum(S));
    %             PrecalcQ2s{q}(:,:,k) = D - S;
                PrecalcQ2s{q}{K+k} = D + S;
                B = D +  S;
    %             PrecalcQ2sFlat{q}{k} = PrecalcQ2s{q}{k}(logical(tril(ones(size(S)))));
                PrecalcQ2sFlat{q}(:,K+k) = B(logical(tril(ones(size(S)))));
                if(nargin > 4)        
                    PrecalcYqDs(q,K+k) = -yq'*B*yq;
                end
            end            
        end
    elseif(~const)
        sample_length = size(x,2)/n_sequences;

        similarities = cell(K, 1);
        sparsities = cell(K2, 1);

        for q = 1 : n_sequences

            beg_ind = (q-1)*sample_length + 1;
            end_ind = q*sample_length;
            
            % don't take the bias term
            xq = x(2:end, beg_ind:end_ind);

            Similarities{q} = zeros([sample_length, sample_length, K+K2]);
            
            PrecalcQ2s{q} = cell(K+K2,1);

            PrecalcQ2sFlat{q} = zeros((sample_length*(sample_length+1))/2,K+K2);
            
            % go over all of the similarity metrics and construct the
            % similarity matrices

            if(nargin > 4)
                yq = y(:,q);
            end

            for k=1:K
                if(q==1)
                    similarities{k} = similarityFNs{k}(xq);
                end
                Similarities{q}(:,:,k) = similarities{k};
                S = Similarities{q}(:,:,k);
                D =  diag(sum(S));
    %             PrecalcQ2s{q}(:,:,k) = D - S;
                PrecalcQ2s{q}{k} = D - S;
                B = D - S;
    %             PrecalcQ2sFlat{q}{k} = PrecalcQ2s{q}{k}(logical(tril(ones(size(S)))));
                PrecalcQ2sFlat{q}(:,k) = B(logical(tril(ones(size(S)))));
                if(nargin > 4)        
                    PrecalcYqDs(q,k) = -yq'*B*yq;
                end
            end
            for k=1:K2
                % this is constant so don't need to recalc
                if(q==1)
                   sparsities{k} = sparsityFNs{k}(xq);
                end
                
                Similarities{q}(:,:,K+k) = sparsities{k};
                S = Similarities{q}(:,:,K+k);
                D =  diag(sum(S));
    %             PrecalcQ2s{q}(:,:,k) = D - S;
                PrecalcQ2s{q}{K+k} = D + S;
                B = D +  S;
    %             PrecalcQ2sFlat{q}{k} = PrecalcQ2s{q}{k}(logical(tril(ones(size(S)))));
                PrecalcQ2sFlat{q}(:,K+k) = B(logical(tril(ones(size(S)))));
                if(nargin > 4)        
                    PrecalcYqDs(q,K+k) = -yq'*B*yq;
                end
            end

        end
    else
        sample_length = size(x,2)/n_sequences;

        similarities = cell(K, 1);
        sparsities = cell(K2, 1);
        
        PrecalcQ2s = {cell(K+K2,1)};
        PrecalcQ2sFlat = {zeros((sample_length*(sample_length+1))/2,K+K2)};
        Similarities = {zeros([sample_length, sample_length, K+K2])};
            
        beg_ind = 1;
        end_ind = sample_length;

        % don't take the bias term
        xq = x(2:end, beg_ind:end_ind);

        % go over all of the similarity metrics and construct the
        % similarity matrices
        for k=1:K
            similarities{k} = similarityFNs{k}(xq');

            Similarities{1}(:,:,k) = similarities{k};
            S = Similarities{1}(:,:,k);
            D =  diag(sum(S));
            PrecalcQ2s{1}{k} = D - S;
            B = D - S;
            % flatten the symmetric matrix to save space
            PrecalcQ2sFlat{1}(:,k) = B(logical(tril(ones(size(S)))));
            if(nargin > 4)
                PrecalcYqDs(:,k) = diag(-y'*B*y);
            end
        end
        for k=1:K2
            % this is constant so don't need to recalc
            sparsities{k} = sparsityFNs{k}(xq');

            Similarities{1}(:,:,K+k) = sparsities{k};
            S = Similarities{1}(:,:,K+k);
            D =  diag(sum(S));
%             PrecalcQ2s{q}(:,:,k) = D - S;
            PrecalcQ2s{1}{K+k} = D + S;
            B = D +  S;
%             PrecalcQ2sFlat{q}{k} = PrecalcQ2s{q}{k}(logical(tril(ones(size(S)))));
            PrecalcQ2sFlat{1}(:,K+k) = B(logical(tril(ones(size(S)))));
            if(nargin > 4)        
                PrecalcYqDs(:,K+k) = diag(-y'*B*y);
            end
        end   
    end
end

