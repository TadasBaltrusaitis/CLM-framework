function [fps, resDir] = run_bu_experiment(bu_dir, verbose, version, varargin)
   
    executable = '"../../x64/Release/FeatureExtraction.exe"';

    output = 'Tracker_';
    
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
        
        command = executable;
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
            
            command = cat(2, command, [' -f "' inputFile '" -of "' outputFile '"']);

            if(verbose)
                outputVideo = ['"' output name '.avi' '"'];
                command = cat(2, command, [' -ov ' outputVideo]);
            end
        end
        
        command = cat(2, command,  ' -fx 500 -fy 500 -cx 160 -cy 120 -no2Dfp -no3Dfp -noMparams -noAUs -noGaze ');        
    
        if(any(strcmp('model', varargin)))
            command = cat(2, command, [' -mloc "', varargin{find(strcmp('model', varargin))+1}, '"']);
        end  
        
        dos(command);
    end
    
    timeTaken = toc;
    fps = 9000 / timeTaken;
    
    % tell the caller where the output was written
    resDir = [bu_dir output];
    
end