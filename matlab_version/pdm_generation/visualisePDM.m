function [ output_args ] = visualisePDM(  MeanVals, EigenVals, EigenVectors, Triangulation, numParams, numExamples)
%VISUALISEPDM Visualise the given PDM model using openGL
%   Detailed explanation goes here

    % use opengl to display the model

    %figure;
    figure('Color',[1 1 1]);

    % from -5 to 5 sqrt(var), which is represented by eigenvalues
    inc = 6 / (numExamples - 1);

%     set(gcf, 'Renderer', 'opengl');

    for i=1:numParams

        for j=1:numExamples
%             handle = subplot(numParams, numExamples, (i-1)*numExamples + j);
%             subplot(numParams, numExamples, (i-1)*numExamples + j);
            left = (j - 1) / numParams;
            bottom = 1 - i  / numExamples;
            
            width = 1.0 / (numParams);
            height = 1.0 / (numExamples);
            
            subplot('Position',[left bottom width height]);
            % get the shape
            params = zeros(size(EigenVals));
%             params(i) = sqrt(EigenVals(i))*(-3 + inc*(j-1));
            params(i) = sqrt(EigenVals(i)) * (-3 + inc*(j-1));
            
            ShapeOffset = EigenVectors * params;

            shape = MeanVals + ShapeOffset;            

            shape = reshape(shape, numel(ShapeOffset)/3, 3);

%             fig_pos = get(handle, 'Position');
%             %fig_pos(1) = 0;
%             %fig_pos(2) = 0;
%             fig_pos(3) = 1.0 / (numParams + 2);
%             fig_pos(4) = 1.0 / (numParams + 2);
%             set(handle, 'Position', fig_pos);

            mesh_h = trimesh(...
                Triangulation + 1, shape(:, 1), shape(:, 2), shape(:, 3), ...
                'EdgeColor', 'blue', ...
                'FaceColor', 'interp', ...
                'FaceLighting', 'phong' ...
            );
%             set(gcf, 'Color', [ 0 0 0 ]); 

%             material([.5, .5, .1 1  ])
%             camlight('headlight');


            axis equal
            axis([-100 100 -100 100 -100 100 -100 100])
            caxis auto
            set(gca, 'CameraPosition', [0 0 -100]);
            set(gca, 'CameraTarget', [0 0 1]);
            set(gca, 'CameraUpVector', [0 -1 0]);
            set(gca, 'XTick', [], 'YTick', [], 'XColor', [1 1 1], 'YColor', [1 1 1], 'ZColor', [1 1 1]);
            set(gca,'Color',[1 1 1]);
        end

    end
    
end

