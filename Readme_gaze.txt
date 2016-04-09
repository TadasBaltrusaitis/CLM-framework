For Windows this software comes prepackaged with all the necessary binaries and dll's for compilation of the project, you still need to compile it in order to run it. You don't need to download anything additional, just open "CLM_framework_vs2012.sln" using Visual Studio 2012.

--------------------------- Matlab example ---------------------------------

An example of how to use the code to extract gaze can be found in:
matlab_runners/Demos/gaze_extraction_demo_vid.m

The output gaze vectors (for each eye) will be found in a specified CSV file (see command line arguments).

-------- Command line parameters for FeatureExtraction executable --------------------------

Parameters for input (if nothing is specified attempts to read from a webcam with default values and no output)

Parameters for input
	-f <filename> - the video file being input
	-device <device_num> the webcam from which to read images (default 0)
	-fdir <directory> loads all the images in the directory and treats them as having come from a single video (if -asvid is specified as well)
	-asvid - need to specify is -fdir is used

Parameters for output
	-op <location of output pose file>, the file format is as follows: frame_number, timestamp(seconds), confidence, detection_success, X, Y, Z, Rx, Ry, Rz
	-ogaze <location of output file>, the file format is as follows: frame_number, timestamp(seconds), confidence, detection_success, x_0, y_0, z_0, x_1, y_1, z_1
		The gaze is output as 2 vectors, the vectors are in world coordinate space describing the gaze direction of both eyes as normalized gaze vectors
	-of <location of output landmark points file>, the file format is as follows: frame_number, timestamp(seconds), confidence, detection_success, x_1 x_2 ... x_n y_1 y_2 ... y_n
	-of3D <location of output 3D landmark points file>, the file format is as follows: frame_number, timestamp(seconds), confidence, detection_success, X_1 X_2 ... X_n Y_1 Y_2 ... Y_n Z_1 Z_2 ... Z_n
	-ov <location of tracked video>
	-oparams <output geom params file>, the file format is as follows: frame_number, timestamp(seconds), confidence, detection_success, scale, rx, ry, rz, tx, ty, p0, p1, p2, p3, p4, p5, p6, p7, p8 ... (rigid and non rigid shape parameters)
	-oaus <output AU file>, the file format is as follows: frame_number, timestamp(seconds), confidence, detection_success, AU01_r, AU02_r, AU04_r, ... (_r implies regression _c classification)
	-hogalign <output HOG feature location>, outputs HOG in a binary file format (see ./matlab_runners/Demos/Read_HOG_files.m for a script to read it in Matlab)
	-simalign <output directory for aligned face image>, outputs similarity aligned faces to a specified directory
    -cp <1/0, should rotation be measured with respect to the camera plane or camera, see Head pose section for more details>

---- Note about timestamps ------
OpenCV is not good at dealing with variable framerates in recorded videos (or sometimes at reporting the framerate of the video), so the timestamps will not always be accurate. However, the number of frames and their ordering will be correct.

//     * Any publications arising from the use of this software, including but
//       not limited to academic journal and conference publications, technical
//       reports and manuals, must cite one of the following works:
//
//       Tadas Baltrusaitis, Marwa Mahmoud, and Peter Robinson.
//       Cross-dataset learning and person-speci?c normalisation for automatic Action Unit detection
//       in Facial Expression Recognition and Analysis Challenge 2015, IEEE International Conference on Automatic Face and Gesture Recognition, 2015
//
//
//       Erroll Wood, Tadas Baltrušaitis, Xucong Zhang, Yusuke Sugano, Peter Robinson, and Andreas Bulling
//	     Rendering of Eyes for Eye-Shape Registration and Gaze Estimation
//       in IEEE International. Conference on Computer Vision (ICCV), 2015