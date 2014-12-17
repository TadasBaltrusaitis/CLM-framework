--------------------------------------- CLM Matlab runners -----------------------------------------------------------------------------	

These are provided for recreation of some of the experiments described in the publications and to demonstrate the command line interface by calling the C++ executables from matlab. This is intended to test the Windows version of the code.

======================== Demos ==================================

run_demo_images.m - running the SimpleCLMImg landmark detection on the demo images packaged with the code
run_demo_videos.m - running the SimpleCLM landmark detection and tracking on prepackaged demo videos
feature_extraction_demo.m - Running the FeatureExtraction project, it demonstrates how to specify parameters for extracting a number of features from videos and how to read those features into Matlab.

The demos are configured to use CCNF patch experts trained on in-the-wild and Multi-PIE datasets, it is possible to uncomment other model file definitions in the scripts to run them instead.

======================== Experiments ============================
To run them you will need to have the appropriate datasets and to change the dataset locations.

-- head pose ---
run_clm_head_pose_tests_svr.m - runs CLM, and CLM-Z on the 3 head pose datasets (Boston University, Biwi Kinect, and ICT-3DHP you need to acquire the datasets yourself)
run_clm_head_pose_tests_clnf.m - runs CLNF on the 3 head pose datasets (Boston University, Biwi Kinect, and ICT-3DHP you need to acquire the datasets yourself)

-- landmark detection ---
"run_clm_feature_point_tests_wild.m" runs CLM and CLNF on the in the wild face datasets acquired from  http://ibug.doc.ic.ac.uk/resources/300-W/ 
The code uses the already defined bounding boxes of faces (these are produced using the 'ExtractBoundingBoxes.m' script on the in the wild datasets). The code relies on there being a .txt file of the same name as the image containing the bounding box. (Note that if the bounding box is not provided the code will use OpenCV Viola-Jones detector)

To run the code you will need to download the 300-W challenge datasets and run the bounding box extraction script, then replace the database_root with the dataset location.

This script also includes code to draw a graph displaying error curves of the CLNF and CLM methods trained on in the wild data.

For convenient comparisons to other state-of-art approaches it also includes results of using the following approaches on the 300-W datasets:

Xiangxin Zhu and Deva Ramanan, Face detection, pose estimation, and landmark localization in the wild, IEEE Conference on Computer Vision and Pattern Recognition, 2012
Xuehan Xiong and Fernando De la Torre, Supervised Descent Method and its Applications to Face Alignment, IEEE Conference on Computer Vision and Pattern Recognition, 2013 
Akshay Asthana, Stefanos Zafeiriou,  Shiyang Cheng, and Maja Pantic, Robust Discriminative Response Map Fitting with Constrained Local Models,IEEE Conference on Computer Vision and Pattern Recognition, 2013