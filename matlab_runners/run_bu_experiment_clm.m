function [fps, resDir] = run_bu_experiment_clm(bu_dir, verbose, version, varargin)
   
    oldDir = chdir('../Release/');
    clm_exe = '"SimpleCLM.exe"';

    output = 'CLM';
    
    % listing the output based on the current revision
    output = [output 'r' num2str(version)];
    
    output = cat(2, output, '/');
      
    if(~exist([bu_dir output], 'dir'))
        mkdir([bu_dir output]);    
    end

    buFiles = dir([bu_dir '*.avi']);
    
    numTogether = 25;
    
    tic;
    for i=1:numTogether:numel(buFiles)
        
        command = clm_exe;
        command = cat(2, command, [' -root ' '"' bu_dir '/"']);
        
        % BU dataset orientation is in terms of camera plane, instruct the
        % tracker to output it in that format
        command = cat(2, command, [' -cp ']);
        
        % deal with edge cases
        if(numTogether + i > numel(buFiles))
            numTogether = numel(buFiles) - i + 1;
        end
        
        for n=0:numTogether-1
            inputFile = [buFiles(n+i).name];
            [~, name, ~] = fileparts(inputFile);   

            % where to output results
            outputFile = [output name '.txt'];
            
            command = cat(2, command, [' -f "' inputFile '" -op "' outputFile '"']);

            if(verbose)
                outputVideo = ['"' output name '.avi' '"'];
                command = cat(2, command, [' -ov ' outputVideo]);
            end
        end
        
        command = cat(2, command,  ' -fx 500 -fy 500 -cx 160 -cy 120');
        
        if(any(strcmp('model', varargin)))
            command = cat(2, command, [' -mloc "', varargin{find(strcmp('model', varargin))+1}, '"']);
        end
        
        if(any(strcmp('clm_sigma', varargin)))
            command = cat(2, command, [' -clm_sigma ' num2str(varargin{find(strcmp('clm_sigma', varargin))+1})]);        
        end

        if(any(strcmp('w_reg', varargin)))
            command = cat(2, command, [' -w_reg ' num2str(varargin{find(strcmp('w_reg', varargin))+1})]);

        end

        if(any(strcmp('reg', varargin)))
            command = cat(2, command, [' -reg ' num2str(varargin{find(strcmp('reg', varargin))+1})]);        
        end                    
        
        dos(command);
    end
    
    timeTakenGAVAM = toc;
    fps = 9000 / timeTakenGAVAM;

    chdir(oldDir);
    
    % tell the caller where the output was written
    resDir = [bu_dir output];
    
end