function [fps, resDir] = run_ict_experiment(rootDir, ictDir, outputRoot, verbose, depth, version, varargin)
%EVALUATEICTDATABASE Summary of this function goes here
%   Detailed explanation goes here

executable = '"../../x64/Release/FeatureExtraction.exe"';

output = 'Tracker_';

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
        
    command = [executable  ' -fx 535 -fy 536 -cx 327 -cy 241 -no2Dfp -no3Dfp -noMparams -noAUs -noGaze '];

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
        
    dos(command);
end

timeTaken = toc;
fps = 10661 / timeTaken;
resDir = outputDir;

end

