For Windows this software comes prepackaged with all the necessary binaries and dll's for compilation of the project, you still need to compile it in order to run it. You don't need to download anything additional, just open "CLM_framework.sln" using Visual Studio 2010 (or "CLM_framework_vs2012.sln" using Visual Studio 2012) and compile the code. The project was built and tested on Visual Studio 2010 and 2012 (can't guarantee compatibility with other versions). Running was extensively tested on Windows Vista, Windows 7 and Windows 8 can't guarantee compatibility with other versions and platforms. NOTE be sure to run the project without debugger attached and in Release mode for speed (if running from Visual Studio), this can be done by using CTRL + F5 instead of F5, this can mean the difference between running at 5fps and 30fps on 320x240px videos.

For Unix based systems and different compilers, I included Cmake files for cross-platform and cross-IDE support, note that for VisualStudio you will need to change the working directory from ProjectDir to TargetDir, as it uses relative paths to find the model location. For running the code on Ubuntu please see readme-ubuntu.txt. NOTE the Ubuntu version of dlib face detector seems to be quite slow, so using HAAR face detector is recommended (by setting clm_parameters.curr_face_detector = CLMTracker::CLMParameters::HAAR_DETECTOR) for now.

You have to respect boost, TBB, dlib, and OpenCV licenses.

---------------------- Copyright information ----------------------

Copyright can be found in the Copyright.txt

---------------------- Code Layout --------------------------------
	
./lib
	local - the actual meat of the code where the relevant computer vision algorithms reside
		CLM - The CLM, CLNF and CLM-Z algorithms
	3rdParty - place for 3rd party libraries
		boost - prepackaged relevant parts of the boost library
		OpenCV3.0 - prepackaged OpenCV 3.0 beta library that is used extensively internally to provide support for basic computer vision functionallity
		dlib - a header only dlib library (includes the face detector used for in-the-wild images)
./exe - the runner and executables that show how to use the libraries for facial expression and head pose tracking, these best demonstrate how to use the libraries
	SimpleCLM/ - running clm, clnf or clm-z if depth is supplied, alternatively running CLNF and CLM from a connected webcam
	SimpleCLMImg/ - running clm or clm-z on a images, individual or in a folder
	MultiTrackCLM/ - tracking multiple faces using the CLM libraries
	FeatureExtraction/ - a utility executable for extracting similarity normalised faces and HOG features for further facial expression analysis (experimental)	
./matlab_runners
	helper scripts for running the experiments and demos
./Release
	The created directory after compilation containing the desired executables
	
--------------- Useful API calls ---------------------------------

CLM class is the main class you will interact with, it performs the main landmark detection algorithms and stores the results. The interaction with the class is declared mainly in the CLMTracker.h, and will require an initialised CLM object. 

The best way to understand how landmark detection is performed in videos or images is to just compile and run SimpleCLM and SimpleCLMImg projects, which perform landmark detection in videos/webcam and images respectively. See later in the readme for the command line arguments for these projects.

A minimal code example for landmark detection is as follows:

CLMTracker::CLMParameters clm_parameters;
CLMTracker::CLM clm_model(clm_parameters.model_location);	

CLMTracker::DetectLandmarksInImage(grayscale_image, Mat_<float>(), clm_model, clm_parameters);

A minimal code example for landmark tracking is as follows:

CLMTracker::CLMParameters clm_parameters;
CLMTracker::CLM clm_model(clm_parameters.model_location);	

while(video)
{
	CLMTracker::DetectLandmarksInVideo(grayscale_image, clm_model, clm_parameters);
}

After landmark detection is done clm_model stores the landmark locations and local and global Point Distribution Model parameters inferred from the image. To access them and more use:

2D landmark location (in image):
	clm_model.detected_landmarks contains a double matrix in following format [x1;x2;...xn;y1;y2...yn] describing the detected landmark locations in the image
	
3D landmark location (with respect to camera):
	clm_model.GetShape(fx, fy, cx, cy);
	// fx,fy,cx,cy are camera callibration parameters needed to infer the 3D position of the head with respect to camera, a good assumption for webcams is 500, 500, img_width/2, img_height/2	
	// This returns a column matrix with the following format [X1;X2;...Xn;Y1;Y2;...Yn;Z1;Z2;...Zn], here every element is in millimeters and represents the facial landmark locations with respect to camera
	
3D landmark location in object space:
	clm_model.pdm.CalcShape3D(landmarks_3D, clm_model.params_local);

Head Pose:

	// Head pose is stored in the following format (X, Y, Z, rot_x, roty_y, rot_z), translation is in millimeters with respect to camera and rotation is in radians around X,Y,Z axes with the convention R = Rx * Ry * Rz, left-handed positive sign, the rotation can be either with respect to camera or the camera plane (for visualisation we want rotation with respect to camera plane)

	There are four methods in total that can return the head pose
	
	//Getting the head pose w.r.t. camera assuming orthographic projection
	Vec6d GetPoseCamera(CLM& clm_model, double fx, double fy, double cx, double cy, CLMParameters& params);
	
	//Getting the head pose w.r.t. camera plane assuming orthographic projection
	Vec6d GetPoseCameraPlane(CLM& clm_model, double fx, double fy, double cx, double cy, CLMParameters& params);
	
	//Getting the head pose w.r.t. camera with a perspective camera correction
	Vec6d GetCorrectedPoseCamera(CLM& clm_model, double fx, double fy, double cx, double cy, CLMParameters& params);

	//Getting the head pose w.r.t. camera plane with a perspective camera correction
	Vec6d GetCorrectedPoseCameraPlane(CLM& clm_model, double fx, double fy, double cx, double cy, CLMParameters& params);

	// fx,fy,cx,cy are camera callibration parameters needed to infer the 3D position of the head with respect to camera, a good assumption for webcams providing 640x480 images is 500, 500, img_width/2, img_height/2	
	
--------------------------- CLM Matlab runners ---------------------------------

These are provided for recreation of some of the experiments described in the papers and to demonstrate the command line interface for Windows.

To run them you will need to change the dataset locations to those on your disc

run_clm_head_pose_tests_svr.m - runs CLM, and CLM-Z on the 3 head pose datasets (Boston University, Biwi Kinect, and ICT-3DHP you need to acquire the datasets yourself)
run_clm_head_pose_tests_clnf.m - runs CLNF on the 3 head pose datasets (Boston University, Biwi Kinect, and ICT-3DHP you need to acquire the datasets yourself)
run_clm_feature_point_tests_wild.m - runs CLM and CLNF on the in the wild face datasets, using already defined bounding boxes of faces (these are produced using the 'matlab runners/ExtractBoundingBoxes.m' script on the in the wild datasets from http://ibug.doc.ic.ac.uk/resources/300-W/)

run_demo_images - runs CLNF on some sample images that come with the code

run_demo_videos - runs CLNF on some sample videos that come with the code (some taken from the Youtube celebrity dataset)
	
feature_extraction_demo_img_seq.m - Running the FeatureExtraction project, it demonstrates how to specify parameters for extracting a number of features from a sequence of images in a folder and how to read those features into Matlab.	

feature_extraction_demo_vid.m - Running the FeatureExtraction project, it demonstrates how to specify parameters for extracting a number of features from a video and how to read those features into Matlab.	
	
-------- Command line parameters for video (SimpleCLM) --------------------------

Parameters for input (if nothing is specified attempts to read from a webcam with default values)

	-f <filename> - the video file being input
	-device <device_num> the webcam from which to read images (default 0)
	-fd <depth directory/> - the directory where depth files are stored

	optional camera parameters for proper head pose visualisation

	-fx <focal length in x>
	-fy <focal length in y>
	-cx <optical centre in x> 
	-cy <optical centre in y>

Parameters for output
	-op <location of output pose file>, the file format is as follows: frame_number timestamp detection_success X Y Z Rx Ry Rz
	-of <location of output landmark points file>, the file format is as follows: frame_number detection_success x_1 x_2 ... x_n y_1 y_2 ... y_n
	-of3D <location of output 3D landmark points file>, the file format is as follows: frame_number detection_success X_1 X_2 ... X_n Y_1 Y_2 ... Y_n Z_1 Z_2 ... Z_n
	-ov <location of tracked video>

    -cp <1/0, should rotation be measured with respect to the camera plane or camera, see Head pose section for more details>

Model parameters (apply to images and videos)
	-mloc <the location of CLM model>
		"model/main_ccnf_general.txt" (default) - trained on Multi-PIE of varying pose and illumination and In-the-wild data, works well for head pose tracking
		"model/main_ccnf_mpie.txt" - trained on Multi-PIE of varying pose and illumination, works well in clear and easy conditions
		"model/main_ccnf_wild.txt" - trained on In-the-wild data, works better in noisy environments (not very well suited for head pose tracking)
		"model/main_svr_general.txt" (default) - trained on Multi-PIE of varying pose and illumination and In-the-wild data, works well for head pose tracking
		"model/main_svr_mpie.txt" - trained on Multi-PIE of varying pose and illumination, works well in clear and easy conditions
		"model/main_svr_wild.txt" - trained on In-the-wild data, works better in noisy environments (not very well suited for head pose tracking)
		"model/main_clm-z.txt" - trained on Multi-PIE and BU-4DFE datasets, works with both intensity and depth signals (CLM-Z)
	-clm_sigma <sigma value from the RLMS and NU-RLMS algorithms, best range 1-2, will affect the fitting>
	-reg <regularisation value from the RLMS and NU-RLMS algorithms, best range 5-40, will affect the fitting, higher values will be more robust but have issues with extreme expressions>
	
All of the models (except CLM-Z) use a 68 point convention for tracking (see http://ibug.doc.ic.ac.uk/resources/300-W/)
	
For more examples of how to run the code, please refer to the Matlab runner code which calls the compiled executables with the command line parameters.
	
-------- Command line parameters for feature extraction from images, image sequences and videos (FeatureExtraction) --------------------------

Parameters for input (if nothing is specified attempts to read from a webcam with default values). This module is still under development and experimental, let me know if something goes wrong.

	-root

	-f <filename> - the video file being input
	-device <device_num> the webcam from which to read images (default 0)
	-fdir <directory> run the feature extraction on every image (.jpg and .png) in a directory (the output will be stored in individual files for the whole directory)
	-asvid (if this flag is specified the images in -fdir directory will be treated as if they came from a video, that is they form a sequence, so tracking will be done instead of detection per videos)
	
	optional camera parameters for proper head pose visualisation

	-fx <focal length in x>
	-fy <focal length in y>
	-cx <optical centre in x> 
	-cy <optical centre in y>
	
Parameters for output
	-outroot <the root directory relevant to which the output files are created> (optional)
	
	-op <location of output pose file>, the file format is as follows: frame_number timestamp detection_success X Y Z Rx Ry Rz
	-of <location of output landmark points file>, the file format is as follows: frame_number detection_success x_1 x_2 ... x_n y_1 y_2 ... y_n
	-of3D <location of output 3D landmark points file>, the file format is as follows: frame_number detection_success X_1 X_2 ... X_n Y_1 Y_2 ... Y_n Z_1 Z_2 ... Z_n
	-ov <location of tracked video>

	-simaligndir <directory> output similarity aligned face images into the following directory
	-simalignvid <filename> output similarity aligned face images into the following video
	-hogalign <filename> output file for HOG features (FHOG of cell size 8) extracted from similarity aligned face images
	-oparams <filename> output file for rigid and non-rigid shape parameters
	-cp <1/0>, should rotation be measured with respect to the camera plane or camera, see Head pose section for more details>

	Additional parameters for output
	
	-verbose visualise the HOG features if they are being output
	-rigid use a slightly more robust version of similarity alignment. This uses and experimentally determined set of more stable feature points instead of all of them for determining similarity alignment
	-simscale <default 0.7> scale of the face for similarity alignment
	-simsize <default 112> width and height of image in pixels when similarity aligned
	-g output images should be grayscale (for saving space)
	
Model parameters (apply to images and videos)
	-mloc <the location of CLM model>
		"model/main_ccnf_general.txt" (default) - trained on Multi-PIE of varying pose and illumination and In-the-wild data, works well for head pose tracking
		"model/main_ccnf_mpie.txt" - trained on Multi-PIE of varying pose and illumination, works well in clear and easy conditions
		"model/main_ccnf_wild.txt" - trained on In-the-wild data, works better in noisy environments (not very well suited for head pose tracking)
		"model/main_svr_general.txt" (default) - trained on Multi-PIE of varying pose and illumination and In-the-wild data, works well for head pose tracking
		"model/main_svr_mpie.txt" - trained on Multi-PIE of varying pose and illumination, works well in clear and easy conditions
		"model/main_svr_wild.txt" - trained on In-the-wild data, works better in noisy environments (not very well suited for head pose tracking)
	-clm_sigma <sigma value from the RLMS and NU-RLMS algorithms, best range 1-2, will affect the fitting>
	-reg <regularisation value from the RLMS and NU-RLMS algorithms, best range 5-40, will affect the fitting, higher values will be more robust but have issues with extreme expressions>
	
All of the models (except CLM-Z) use a 68 point convention for tracking (see http://ibug.doc.ic.ac.uk/resources/300-W/)
	
For more examples of how to run the code, please refer to the Matlab runner code which calls the compiled executables with the command line parameters.
	
------------ Command line parameters for images (SimpleCLMImg) ----------------

Parameters for input

Flags:
	-clmwild - specify when the images are more difficult, this makes the landmark detector use a different face detector, and also makes it consider different hypotheses for landmark detection together with an extended search region

Single image analysis:
	-f <filename> - the image file being input
	-fd <depth file> - the corresoinding depth file location
	-of <location of output landmark points file> the file format is same as 300 faces in the wild challenge annotations (http://ibug.doc.ic.ac.uk/resources/facial-point-annotations/)
	-oi <location of tracked video>

Batch image analysis:	
	-fdir <directory> - runs landmark detection on all images (.jpg and .png) in a directory, if the directory contains .txt files (image_name.txt) with bounding boxes, it will use those for initialisation
	-ofdir <directory> - where detected landmarks should be written
	-oidir <directory> - where images with detected landmarks should be stored

Model parameters (apply to images and videos)
	-mloc <the location of CLM model>
		"model/main_ccnf_general.txt" (default) - trained on Multi-PIE of varying pose and illumination and In-the-wild data, works well for head pose tracking
		"model/main_ccnf_mpie.txt" - trained on Multi-PIE of varying pose and illumination, works well in clear and easy conditions
		"model/main_ccnf_wild.txt" - trained on In-the-wild data, works better in noisy environments (not very well suited for head pose tracking)
		"model/main_svr_general.txt" (default) - trained on Multi-PIE of varying pose and illumination and In-the-wild data, works well for head pose tracking
		"model/main_svr_mpie.txt" - trained on Multi-PIE of varying pose and illumination, works well in clear and easy conditions
		"model/main_svr_wild.txt" - trained on In-the-wild data, works better in noisy environments (not very well suited for head pose tracking)
		"model/main_clm-z.txt" - trained on Multi-PIE and BU-4DFE datasets, works with both intensity and depth signals (CLM-Z)
	-clm_sigma <sigma value from the RLMS and NU-RLMS algorithms, best range 1-2, will affect the fitting>
	-reg <regularisation value from the RLMS and NU-RLMS algorithms, best range 5-40, will affect the fitting, higher values will be more robust but have issues with extreme expressions>
	-multi-view <0/1>, should multi-view initialisation be used (more robust, but slower)

--------------------- Basic demos -----------------------------------------

Can run these after compiling the code in Release mode.

Just running SimpleCLM.exe or MultiTrackCLM.exe will track either a single face or multiple (in the case of the latter executable) from the webcam connected to the computer.

Basic landmark detection in images. From Matlab run "matlab_runners/run_demo_images.m", alternatively go to Release folder and from command line execute:

SimpleCLMImg.exe -clmwild -fdir "../videos/" -ofdir "../matlab_runners/demo_img/" -oidir "../matlab_runners/demo_img/"

or

SimpleCLMImg.exe -fdir "../videos/" -ofdir "../matlab_runners/demo_img/" -oidir "../matlab_runners/demo_img/"

Basic landmark tracking in videos. From Matlab run "matlab_runners/run_demo_video.m", alternatively go to Release folder and from command line execute: SimpleCLM.exe -f "../videos/changeLighting.wmv" -f "../videos/0188_03_021_al_pacino.avi" -f "../videos/0217_03_006_alanis_morissette.avi" -f "../videos/0244_03_004_anderson_cooper.avi" -f "../videos/0294_02_004_angelina_jolie.avi" -f "../videos/0417_02_003_bill_clinton.avi" -f "../videos/0490_03_007_bill_gates.avi" -f "../videos/0686_02_003_gloria_estefan.avi" -f "../videos/1034_03_006_jet_li.avi" -f "../videos/1192_01_006_julia_roberts.avi" -f "../videos/1461_01_021_noam_chomsky.avi" -f "../videos/1804_03_006_sylvester_stallone.avi" -f "../videos/1815_01_008_tony_blair.avi" -f "../videos/1869_03_009_victoria_beckham.avi" -f "../videos/1878_01_002_vladimir_putin.avi"
		
------------ Depth data ------------------------------------------------
Currently depth stream is expected to be in the format of a collection of 8 or 16-bit .png files in a folder with a naming scheme: depth00001.png, depth00002.png, ... depthxxxxx.png, with each .png.
Each depth image is expected to correspond to an intensity image from a video sequence. Colour and depth images are expected to be aligned (a pixel in one is correspondent to a pixel in another).

For example of this sort of data see the ICT-3DHP dataset (http://multicomp.ict.usc.edu/?p=1738)

------------- CLM parameters ---------------------------------------------

The default parameters are already set (they work well enough on the datasets tested (BU, BU-4DFE, ICT-3DHP, and Biwi)), however,
tweaking them might results in better tracking for your specific datasets.

Main ones to tweak can be found in lib\local\CLM\include\CLMParameters.h:

window sizes - window_sizes_current specified window sizes for each scale iteration, if set smaller the tracking accuracy might suffer but it would be faster, setting to larger area would slow down the tracking, but potentially make it more accurate.
validate_detections - if set to true an SVM classifier is used to determine convergence of feature points and make sure it's a face, this is useful to know for reinitialisation
decision_boundary - this is used to determine when the SVM classifier thinks that convergence failed (currently -0.5), decrease for a less conservative boundary (more false positives, but fewer false negatives) and increase to be more conservative (fewer false positives, but more false negatives)

--------------- Results -------------------------------------------------
	
Results that you should expect on running the code on the publicly available datasets can be found in:

matlab_runners/results/landmark_detections.txt - the results on landmark detection on in the wild dataset

matlab_runners/results/Pose_clm_ccnf_v1.txt - the results of head pose tracking using CLNF on 3 datasets (BU, BIWI and ICT-3DHP)
matlab_runners/results/Pose_clm_svr_v1.txt - the results of head pose tracking uding CLM and CLM-Zon 3 datasets (BU, BIWI and ICT-3DHP)

--------------------------------------- Final remarks -----------------------------------------------------------------------------	

I did my best to make sure that the code runs out of the box but there are always issues and I would be grateful for your understanding that this is research code and not a commercial level product. However, if you encounter any probles please contact me at Tadas.Baltrusaitis@cl.cam.ac.uk for any bug reports/questions/suggestions. 
