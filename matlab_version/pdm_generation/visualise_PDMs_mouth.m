%% Torresani visualisation of in-the wild PDM
load './Wild_data_pdm/pdm_20_mouth.mat';

T = delaunay(M(1:20), M(21:40));
% Visualise it
visualisePDM(M, E, V, T-1, 5, 5)
