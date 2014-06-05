function [fps, resDir] = run_ict_experiment_clm(rootDir, ictDir, outputRoot, verbose, depth, version, varargin)
%EVALUATEICTDATABASE Summary of this function goes here
%   Detailed explanation goes here

oldDir = chdir('../Release/');
gavamExe = '"SimpleCLM.exe"';

output = 'CLM';

dbSeqDir = dir([rootDir ictDir]);

% listing the output based on the current revision
output = [output 'r' num2str(version)];
   
if(depth)
    output = cat(2, output, '_depth');
end

outputDir = cat(2, outputRoot, ['/' output '/']);

if(~exist([rootDir outputDir], 'dir'))
    mkdir([rootDir outputDir]);    
end

tic;

numTogether = 10;

for i=3:numTogether:numel(dbSeqDir)
        
    command = [gavamExe  ' -fx 535 -fy 536 -cx 327 -cy 241'];

    command = cat(2, command, [' -root ' '"' rootDir '/"']);

    % deal with edge cases
    if(numTogether + i > numel(dbSeqDir))
        numTogether = numel(dbSeqDir) - i + 1;
    end
    
    for n=0:numTogether-1
        
        inputFile = [ictDir dbSeqDir(i+n).name '/colour undist.avi'];
        outputFile = [outputDir dbSeqDir(i+n).name '.txt'];
        
        command = cat(2, command,  [' -f "' inputFile '" -op "' outputFile  '" ']);
        
        if(depth)
            dDir = [ictDir dbSeqDir(i+n).name '/depthAligned/'];
            command = cat(2, command, [' -fd "' dDir '"']);
        end
        
        if(verbose)
            outputVideo = [outputDir dbSeqDir(i+n).name '.avi'];
            command = cat(2, command, [' -ov "' outputVideo '"']);
        end
    end
    
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

timeTaken = toc;
fps = 10661 / timeTaken;
resDir = outputDir;
chdir(oldDir);

end

