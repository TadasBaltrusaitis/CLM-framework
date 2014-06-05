---------------------------------------------------------------------------------
Copyright (c) 2013 Xiang Yu, yuxiang03@gmail.com
---------------------------------------------------------------------------------
The code is released with the ICCV 2013 paper in [1]. Further improvements are 
quite welcome. It is for research purpose only. Any merchandizing is prohibited. 
If you want to re-ensemble or improve the code, please contact the authors in 
[1] for permission. 
---------------------------------------------------------------------------------
Code content:

"demo.m" is the main entry for running the code.

"face_detect_32" and "face_detect_64" folders contain the inferencing code 
(detecting code) from Zhu and Ramanan [3], we would thank them for sharing the 
code. Training code for the optimized model as described in the paper [1] is not 
included in this demo.

"mex_32" and "mex_64" are mex wrappers of viola-jones face detector, feature 
extraction and matching.

"test" folder contains images selected from multiple public databases.

"Model" folder are the model parameters for face detecting. The original Point 
Distribution Model parameters have been provided by Saragih et al. [2] that was 
trained on Multi-PIE database. We would thank them for providing the resource. 
Besides, we added real 3D shape model trained from 3DFE database [4] in order 
to solve the pose problem.

The code is provided as is. The authors in [1] have no responsibility for any 
debugging, improvement or environment setup.
----------------------------------------------------------------------------------
Landmark initialization ensembling:

In order to robustly provide the initial landmarks for further fitting, we not 
only provide our optimized part mixture model (group_sparse_profile) but also 
ensembled Zhu-Ramanan detector(P146 model, detect_additional) [3] and viola-jones 
face detector to accomplish the task.
----------------------------------------------------------------------------------
Acknowledgement:

We sincerely thank authors in [2] and [3] for their sharing of resource in 
developing the code. We also thank authors in [4] for the providing of 3D face 
database.
----------------------------------------------------------------------------------
References:

[1] X.Yu, J. Huang, S. Zhang, W. Yan and D.N. Metaxas,
Pose-free Facial Landmark Fitting via Optimized Part Mixtures and Cascaded 
Deformable Shape Model.In ICCV 2013.

[2] J. Saragih, S. Lucey and J. Cohn,
Deformable Model Fitting by Regularized Landmark Mean-Shift. In IJCV 2010.

[3] X. Zhu and D. Ramanan.
Face detection, pose estimation and landmark localization in the wild. In CVPR 
2012.

[4] Lijun Yin, Xiaozhou Wei, Yi Sun, Jun Wang and Matthew J. Rosato, 
A 3D Facial Expression Database For Facial Behavior Research.
7th International Conference on Automatic Face and Gesture Recognition (FGR06), 
10-12 April 2006 P:211 - 216
----------------------------------------------------------------------------------