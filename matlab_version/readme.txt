The project was tested on Matlab 2012a and 2012b on 64 bit Windows 7 and Windows 8 machines (can't guarantee compatibility with other configurations).
Comes prepackaged with all the necessary code and some of the data (that I'm allowed to share). You have to respect nrsfm, OpenCV and other licenses.
You don't need to download anything additional.

--------------------------------------- Copyright information -----------------------------------------------------------------------------	

Copyright can be found in the copyright.txt

--------------------------------------- Code Layout -----------------------------------------------------------------------------

//======================= Core ========================//
./fitting - Where the actual CLM and CLNF model fitting happens
./models - the pre-trained models for Constrained Local Neural Fields, this includes Point Distribution Model, Patch experts, landmark validation
./CCNF - the libraries that contain CCNF functions needed for landmark detection

//======================= Demos and experiments =======//
./demo - contains a number of useful scripts that demonstrate the running of CLM, CLM-Z and CLNF models on videos and images
	face_image_demo - running CLNF or CLM on images of faces
	face_image_depth_demo - running CLM-Z on grayscale and range images of faces
	face_video_demo - running CLNF or CLM on videos of faces
./experiments_iccv_300w - These are provided for recreation of some of the experiments described in the papers
./experiments_in_the_wild - These are provided for demonstrating results on in the wild data when trained on more general training data (both Multi-PIE and in-the-wild)

//======================= Utilities ===================//
./face_detection - Provides utilities for face detection, possible choices between three detectors: Matlab inbuild one, Zhu and Ramanan, and Yu et al.
    ./face_detection_yu - The face detector from Xiang Yu, more details in ./face_detection_yu/README.txt. Only tested on windows machines
    ./face_detection_zhu - The face detector from Zhu and Ramanan, might need to compile it using ./face_detection_yu/face-release1.0-basic/compile.m
./face_validation - A module for validating face detections (training and inference), it is used for tracking in videos so as to know when reinitialisation is needed
./PDM_helpers - utility functions that deal with PDM fitting, Jacobians and other shape manipulations
./bounding_box_mapping - learning the mapping from face detector bounding box to one suitable for landmark detection initialisation

//====================== Model Training ===============//
./pdm_generation - code for training the Point Distribution Model (PDM)
./face_validation_svr - A module for validating face detections (training and inference), it is used for tracking in videos so as to know when reinitialisation is needed

You can find the patch training code here - https://github.com/TadasBaltrusaitis/CCNF
	
--------------------------------------- Results -----------------------------------------------------------------------------	
	
Results that you should expect on running the code on the publicly available datasets can be found in:

experiments_iccv_300w/results/ - the results on landmark detection on in the wild datasets when trained on other in-the-wild data (together with results from some other baselines)
experiments_in_the_wild/results/ - the results on landmark detection on in the wild datasets when trained on in-the-wild data and Multi-PIE data, comparing CLM and CLNF

--------------------------------------- Final remarks -----------------------------------------------------------------------------	

I did my best to make sure that the code runs out of the box but there are always issues and I would be grateful for your understanding that this is research code and not a commercial
level product. However, if you encounter any probles please contact me at Tadas.Baltrusaitis@cl.cam.ac.uk for any bug reports/questions/suggestions. 
