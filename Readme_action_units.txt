For Windows this software comes prepackaged with all the necessary binaries and dll's for compilation of the project, you still need to compile it in order to run it. You don't need to download anything additional, just open "CLM_framework_vs2012.sln" using Visual Studio 2012.

--------------------------- Matlab example ---------------------------------

An example of how to use the code to extract facial action units and other features can be found in:
matlab_runners/Demos/feature_extraction_demo_vid.m
and
matlab_runners/Demos/feature_extraction_demo_img_seq.m

The output AUs will be found in a specified CSV file (see command line arguments) that contains the AU predictions for intensity (e.g. AU01_r) and occurence (e.g. AU04_c).

-------- Command line parameters for Action Unit and other feature extraction (FeatureExtraction) --------------------------

Parameters for input (if nothing is specified attempts to read from a webcam with default values and no output)

Parameters for input
	-f <filename> - the video file being input
	-device <device_num> the webcam from which to read images (default 0)
	-fdir <directory> loads all the images in the directory and treats them as having come from a single video (if -asvid is specified as well)
	-asvid - need to specify is -fdir is used

Parameters for output
	-op <location of output pose file>, the file format is as follows: frame_number, confidence, detection_success X Y Z Rx Ry Rz
	-of <location of output landmark points file>, the file format is as follows: frame_number detection_success x_1 x_2 ... x_n y_1 y_2 ... y_n
	-of3D <location of output 3D landmark points file>, the file format is as follows: frame_number detection_success X_1 X_2 ... X_n Y_1 Y_2 ... Y_n Z_1 Z_2 ... Z_n
	-ov <location of tracked video>
	-oparams <output geom params file>, the file format is as follows: frame, success, scale, rx, ry, rz, tx, ty, p0, p1, p2, p3, p4, p5, p6, p7, p8 ... (rigid and non rigid shape parameters)
	-oaus <output AU file>, the file format is as follows: frame, success, confidence, AU01_r, AU02_r, AU04_r, ... (_r implies regression _c classification)
	-hogalign <output HOG feature location>, outputs HOG in a binary file format (see ./matlab_runners/Demos/Read_HOG_files.m for a script to read it in Matlab)
	-simalignvid <output video file of aligned faces>, outputs similarity aligned faces to a video (need HFYU video codec to read it)
	-simaligndir <output directory for aligned face image>, same as above but instead of video the aligned faces are put in a directory
    -cp <1/0, should rotation be measured with respect to the camera plane or camera, see Head pose section for more details>

//     * Any publications arising from the use of this software, including but
//       not limited to academic journal and conference publications, technical
//       reports and manuals, must cite one of the following works:
//
//       Tadas Baltrusaitis, Marwa Mahmoud, and Peter Robinson.
//       Cross-dataset learning and person-speci?c normalisation for automatic Action Unit detection
//       in Facial Expression Recognition and Analysis Challenge 2015, IEEE International Conference on Automatic Face and Gesture Recognition, 2015