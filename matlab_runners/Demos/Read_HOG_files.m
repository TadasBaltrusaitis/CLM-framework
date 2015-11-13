function [hog_data, valid_inds, vid_id] = Read_HOG_files(users, hog_data_dir)

    hog_data = [];
    vid_id = {};
    valid_inds = [];
    
    feats_filled = 0;

    for i=1:numel(users)
        
        hog_files = dir([hog_data_dir, users{i} '*.hog']);
        
        for h=1:numel(hog_files)
            hog_file = [hog_data_dir, hog_files(h).name];
            f = fopen(hog_file, 'r');

            curr_data = [];
            curr_ind = 0;

            while(~feof(f))

                if(curr_ind == 0)
                    num_cols = fread(f, 1, 'int32');
                    if(isempty(num_cols))
                        break;
                    end

                    num_rows = fread(f, 1, 'int32');
                    num_chan = fread(f, 1, 'int32');

                    curr_ind = curr_ind + 1;            

                    % preallocate some space
                    if(curr_ind == 1)
                        curr_data = zeros(1000, 1 + num_rows * num_cols * num_chan);
                        num_feats =  1 + num_rows * num_cols * num_chan;
                    end

                    if(curr_ind > size(curr_data,1))
                        curr_data = cat(1, curr_data, zeros(1000, 1 + num_rows * num_cols * num_chan));
                    end
                    feature_vec = fread(f, [1, 1 + num_rows * num_cols * num_chan], 'float32');
                    curr_data(curr_ind, :) = feature_vec;
                else

                    % Reading in batches of 5000

                    feature_vec = fread(f, [4 + num_rows * num_cols * num_chan, 5000], 'float32');
                    feature_vec = feature_vec(4:end,:)';

                    num_rows_read = size(feature_vec,1);

                    if(~isempty(feature_vec))
                        curr_data(curr_ind+1:curr_ind+num_rows_read,:) = feature_vec;
                        curr_ind = curr_ind + size(feature_vec,1);
                    end
                end

            end

            fclose(f);

            curr_data = curr_data(1:curr_ind,:);
            vid_id_curr = cell(curr_ind,1);
            vid_id_curr(:) = users(i);

            vid_id = cat(1, vid_id, vid_id_curr);

            % Assume same number of frames per video
            if(i==1 && h == 1)
                hog_data = zeros(curr_ind * numel(users) * 8, num_feats);
            end

            if(size(hog_data,1) < feats_filled+curr_ind)
               hog_data = cat(1, hog_data, zeros(size(hog_data,1), num_feats));
            end

            hog_data(feats_filled+1:feats_filled+curr_ind,:) = curr_data;

            feats_filled = feats_filled + curr_ind;
        end
    end
    
    if(~isempty(hog_data))        
        valid_inds = hog_data(1:feats_filled,1);
        hog_data = hog_data(1:feats_filled,2:end);
    end
end