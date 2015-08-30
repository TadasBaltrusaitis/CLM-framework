%% Torresani visualisation of in-the wild PDM
load './Wild_data_pdm/pdm_10_brows.mat';

T = delaunay(M(1:10), M(11:20));
% Visualise it
visualisePDM(M, E, V, T-1, 5, 5)
