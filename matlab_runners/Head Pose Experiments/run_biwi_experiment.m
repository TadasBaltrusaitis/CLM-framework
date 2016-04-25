function [fps, resDir] = run_biwi_experiment(rootDir, biwiDir, outputDir, verbose, depth, version, varargin)
% Biwi dataset experiment

executable = '"../../x64/Release/FeatureExtraction.exe"';

output = 'Tracker_';

dbSeqDir = dir([rootDir biwiDir]);

% listing the output based on the current revision
output = [output 'r' num2str(version)];
   
if(depth)
    output = cat(2, output, '_depth');
end

outputDir = cat(2, outputDir, ['/' output '/']);

if(~exist([outputDir], 'dir'))
    mkdir([outputDir]);    
end

offset = 0;

r = 1 + offset;

tic;
    
numTogether = 25;


for i=3 + offset:numTogether:numel(dbSeqDir)
    
       
    command = executable;
           
    command = cat(2, command, [' -root ' '"' rootDir '"']);
     
    % deal with edge cases
    if(numTogether + i > numel(dbSeqDir))
        numTogether = numel(dbSeqDir) - i + 1;
    end

    for n=0:numTogether-1
        
        inputFile = [biwiDir dbSeqDir(i+n).name '/colour.avi'];
        outputFile = [outputDir dbSeqDir(i+n).name '.txt'];

        command = cat(2, command, [' -f "' inputFile '" -of "' outputFile  '"']);

        if(depth)
            dDir = [biwiDir dbSeqDir(i+n).name '/depthAligned/'];
            command = cat(2, command, [' -fd "' dDir '"']);    
        end

        if(verbose)
            outputVideo = [outputDir dbSeqDir(i).name '.avi'];
            command = cat(2, command, [' -ov "' outputVideo '"']);    
        end
    end    
    command = cat(2, command, [' -fx 505 -fy 505 -cx 320 -cy 240 -no2Dfp -no3Dfp -noMparams -noAUs -noGaze']);
        
    if(any(strcmp('model', varargin)))
        command = cat(2, command, [' -mloc "', varargin{find(strcmp('model', varargin))+1}, '"']);
    end
            
    r = r+1;    
    dos(command);
end

timeTaken = toc;
fps = 15678 / timeTaken;
resDir = outputDir;