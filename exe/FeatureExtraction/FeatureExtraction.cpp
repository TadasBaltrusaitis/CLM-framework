///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016, Carnegie Mellon University and University of Cambridge,
// all rights reserved.
//
// THIS SOFTWARE IS PROVIDED “AS IS” FOR ACADEMIC USE ONLY AND ANY EXPRESS
// OR IMPLIED WARRANTIES WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY.
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Notwithstanding the license granted herein, Licensee acknowledges that certain components
// of the Software may be covered by so-called “open source” software licenses (“Open Source
// Components”), which means any software licenses approved as open source licenses by the
// Open Source Initiative or any substantially similar licenses, including without limitation any
// license that, as a condition of distribution of the software licensed under such license,
// requires that the distributor make the software available in source code format. Licensor shall
// provide a list of Open Source Components for a particular version of the Software upon
// Licensee’s request. Licensee will comply with the applicable terms of such licenses and to
// the extent required by the licenses covering Open Source Components, the terms of such
// licenses will apply in lieu of the terms of this Agreement. To the extent the terms of the
// licenses applicable to Open Source Components prohibit any of the restrictions in this
// License Agreement with respect to such Open Source Component, such restrictions will not
// apply to such Open Source Component. To the extent the terms of the licenses applicable to
// Open Source Components require Licensor to make an offer to provide source code or
// related information in connection with the Software, such offer is hereby made. Any request
// for source code or related information should be directed to cl-face-tracker-distribution@lists.cam.ac.uk
// Licensee acknowledges receipt of notices for the Open Source Components for the initial
// delivery of the Software.

//     * Any publications arising from the use of this software, including but
//       not limited to academic journal and conference publications, technical
//       reports and manuals, must cite at least one of the following works:
//
//       OpenFace: an open source facial behavior analysis toolkit
//       Tadas Baltrušaitis, Peter Robinson, and Louis-Philippe Morency
//       in IEEE Winter Conference on Applications of Computer Vision, 2016  
//
//       Rendering of Eyes for Eye-Shape Registration and Gaze Estimation
//       Erroll Wood, Tadas Baltrušaitis, Xucong Zhang, Yusuke Sugano, Peter Robinson, and Andreas Bulling 
//       in IEEE International. Conference on Computer Vision (ICCV),  2015 
//
//       Cross-dataset learning and person-speci?c normalisation for automatic Action Unit detection
//       Tadas Baltrušaitis, Marwa Mahmoud, and Peter Robinson 
//       in Facial Expression Recognition and Analysis Challenge, 
//       IEEE International Conference on Automatic Face and Gesture Recognition, 2015 
//
//       Constrained Local Neural Fields for robust facial landmark detection in the wild.
//       Tadas Baltrušaitis, Peter Robinson, and Louis-Philippe Morency. 
//       in IEEE Int. Conference on Computer Vision Workshops, 300 Faces in-the-Wild Challenge, 2013.    
//
///////////////////////////////////////////////////////////////////////////////


// FeatureExtraction.cpp : Defines the entry point for the feature extraction console application.

// System includes
#include <fstream>
#include <sstream>

// OpenCV includes
#include <opencv2/videoio/videoio.hpp>  // Video write
#include <opencv2/videoio/videoio_c.h>  // Video write
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// Boost includes
#include <filesystem.hpp>
#include <filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>

// Local includes
#include "LandmarkCoreIncludes.h"

#include <Face_utils.h>
#include <FaceAnalyser.h>
#include <GazeEstimation.h>


#define INFO_STREAM( stream ) \
std::cout << stream << std::endl

#define WARN_STREAM( stream ) \
std::cout << "Warning: " << stream << std::endl

#define ERROR_STREAM( stream ) \
std::cout << "Error: " << stream << std::endl

static void printErrorAndAbort( const std::string & error )
{
    std::cout << error << std::endl;
}

#define FATAL_STREAM( stream ) \
printErrorAndAbort( std::string( "Fatal error: " ) + stream )

using namespace std;

using namespace boost::filesystem;

vector<string> get_arguments(int argc, char **argv)
{

	vector<string> arguments;

	// First argument is reserved for the name of the executable
	for(int i = 0; i < argc; ++i)
	{
		arguments.push_back(string(argv[i]));
	}
	return arguments;
}

// Useful utility for creating directories for storing the output files
void create_directory_from_file(string output_path)
{

	// Creating the right directory structure
	
	// First get rid of the file
	auto p = path(path(output_path).parent_path());

	if(!p.empty() && !boost::filesystem::exists(p))		
	{
		bool success = boost::filesystem::create_directories(p);
		if(!success)
		{
			cout << "Failed to create a directory... " << p.string() << endl;
		}
	}
}

void create_directory(string output_path)
{

	// Creating the right directory structure
	auto p = path(output_path);

	if(!boost::filesystem::exists(p))		
	{
		bool success = boost::filesystem::create_directories(p);
		
		if(!success)
		{
			cout << "Failed to create a directory..." << p.string() << endl;
		}
	}
}

void get_output_feature_params(vector<string> &output_similarity_aligned, vector<string> &output_hog_aligned_files, double &similarity_scale, 
	int &similarity_size, bool &grayscale, bool &rigid, bool& verbose, 
	bool &output_2D_landmarks, bool &output_3D_landmarks, bool &output_model_params, bool &output_pose, bool &output_AUs, bool &output_gaze,
	vector<string> &arguments)
{
	output_similarity_aligned.clear();
	output_hog_aligned_files.clear();

	bool* valid = new bool[arguments.size()];

	for (size_t i = 0; i < arguments.size(); ++i)
	{
		valid[i] = true;
	}

	string input_root = "";
	string output_root = "";

	// First check if there is a root argument (so that videos and outputs could be defined more easilly)
	for (size_t i = 0; i < arguments.size(); ++i)
	{
		if (arguments[i].compare("-root") == 0)
		{
			input_root = arguments[i + 1];
			output_root = arguments[i + 1];
			i++;
		}
		if (arguments[i].compare("-inroot") == 0)
		{
			input_root = arguments[i + 1];
			i++;
		}
		if (arguments[i].compare("-outroot") == 0)
		{
			output_root = arguments[i + 1];
			i++;
		}
	}

	for (size_t i = 0; i < arguments.size(); ++i)
	{
		if (arguments[i].compare("-simalign") == 0)
		{
			output_similarity_aligned.push_back(output_root + arguments[i + 1]);
			create_directory(output_root + arguments[i + 1]);
			valid[i] = false;
			valid[i + 1] = false;
			i++;
		}
		else if (arguments[i].compare("-hogalign") == 0)
		{
			output_hog_aligned_files.push_back(output_root + arguments[i + 1]);
			create_directory_from_file(output_root + arguments[i + 1]);
			valid[i] = false;
			valid[i + 1] = false;
			i++;
		}
		else if (arguments[i].compare("-verbose") == 0)
		{
			verbose = true;
		}
		else if (arguments[i].compare("-rigid") == 0)
		{
			rigid = true;
		}
		else if (arguments[i].compare("-g") == 0)
		{
			grayscale = true;
			valid[i] = false;
		}
		else if (arguments[i].compare("-simscale") == 0)
		{
			similarity_scale = stod(arguments[i + 1]);
			valid[i] = false;
			valid[i + 1] = false;
			i++;
		}
		else if (arguments[i].compare("-simsize") == 0)
		{
			similarity_size = stoi(arguments[i + 1]);
			valid[i] = false;
			valid[i + 1] = false;
			i++;
		} 
		else if (arguments[i].compare("-no2Dfp") == 0)
		{
			output_2D_landmarks = false;
			valid[i] = false;
		}
		else if (arguments[i].compare("-no3Dfp") == 0)
		{
			output_3D_landmarks = false;
			valid[i] = false;
		}
		else if (arguments[i].compare("-noMparams") == 0)
		{
			output_model_params = false;
			valid[i] = false;
		}
		else if (arguments[i].compare("-noPose") == 0)
		{
			output_pose = false;
			valid[i] = false;
		}
		else if (arguments[i].compare("-noAUs") == 0)
		{
			output_AUs = false;
			valid[i] = false;
		}
		else if (arguments[i].compare("-noGaze") == 0)
		{
			output_gaze = false;
			valid[i] = false;
		}
	}

	for (int i = arguments.size() - 1; i >= 0; --i)
	{
		if (!valid[i])
		{
			arguments.erase(arguments.begin() + i);
		}
	}

}
// Can process images via directories creating a separate output file per directory
void get_image_input_output_params_feats(vector<vector<string> > &input_image_files, bool& as_video, vector<string> &arguments)
{
	bool* valid = new bool[arguments.size()];
		
	for(size_t i = 0; i < arguments.size(); ++i)
	{
		valid[i] = true;
		if (arguments[i].compare("-fdir") == 0) 
		{                    

			// parse the -fdir directory by reading in all of the .png and .jpg files in it
			path image_directory (arguments[i+1]); 

			try
			{
				 // does the file exist and is it a directory
				if (exists(image_directory) && is_directory(image_directory))   
				{
					
					vector<path> file_in_directory;                                
					copy(directory_iterator(image_directory), directory_iterator(), back_inserter(file_in_directory));

					// Sort the images in the directory first
					sort(file_in_directory.begin(), file_in_directory.end()); 

					vector<string> curr_dir_files;

					for (vector<path>::const_iterator file_iterator (file_in_directory.begin()); file_iterator != file_in_directory.end(); ++file_iterator)
					{
						// Possible image extension .jpg and .png
						if(file_iterator->extension().string().compare(".jpg") == 0 || file_iterator->extension().string().compare(".png") == 0)
						{																
							curr_dir_files.push_back(file_iterator->string());															
						}
					}

					input_image_files.push_back(curr_dir_files);
				}
			}
			catch (const filesystem_error& ex)
			{
				cout << ex.what() << '\n';
			}

			valid[i] = false;
			valid[i+1] = false;		
			i++;
		}
		else if (arguments[i].compare("-asvid") == 0) 
		{
			as_video = true;
		}
		else if (arguments[i].compare("-help") == 0)
		{
			cout << "Input output files are defined as: -fdir <image directory (can have multiple ones)> -asvid <the images in a folder are assumed to come from a video (consecutive)>" << endl; // Inform the user of how to use the program				
		}
	}
	
	// Clear up the argument list
	for(int i=arguments.size()-1; i >= 0; --i)
	{
		if(!valid[i])
		{
			arguments.erase(arguments.begin()+i);
		}
	}

}

void output_HOG_frame(std::ofstream* hog_file, bool good_frame, const cv::Mat_<double>& hog_descriptor, int num_rows, int num_cols)
{

	// Using FHOGs, hence 31 channels
	int num_channels = 31;

	hog_file->write((char*)(&num_cols), 4);
	hog_file->write((char*)(&num_rows), 4);
	hog_file->write((char*)(&num_channels), 4);

	// Not the best way to store a bool, but will be much easier to read it
	float good_frame_float;
	if(good_frame)
		good_frame_float = 1;
	else
		good_frame_float = -1;

	hog_file->write((char*)(&good_frame_float), 4);

	cv::MatConstIterator_<double> descriptor_it = hog_descriptor.begin();

	for(int y = 0; y < num_cols; ++y)
	{
		for(int x = 0; x < num_rows; ++x)
		{
			for(unsigned int o = 0; o < 31; ++o)
			{

				float hog_data = (float)(*descriptor_it++);
				hog_file->write ((char*)&hog_data, 4);
			}
		}
	}
}

// Some globals for tracking timing information for visualisation
double fps_tracker = -1.0;
int64 t0 = 0;

// Visualising the results
void visualise_tracking(cv::Mat& captured_image, const LandmarkDetector::CLNF& clnf_model, const LandmarkDetector::FaceModelParameters& det_parameters, cv::Point3f gazeDirection0, cv::Point3f gazeDirection1, int frame_count, double fx, double fy, double cx, double cy)
{

	// Drawing the facial landmarks on the face and the bounding box around it if tracking is successful and initialised
	double detection_certainty = clnf_model.detection_certainty;
	bool detection_success = clnf_model.detection_success;

	double visualisation_boundary = 0.2;

	// Only draw if the reliability is reasonable, the value is slightly ad-hoc
	if (detection_certainty < visualisation_boundary)
	{
		LandmarkDetector::Draw(captured_image, clnf_model);

		double vis_certainty = detection_certainty;
		if (vis_certainty > 1)
			vis_certainty = 1;
		if (vis_certainty < -1)
			vis_certainty = -1;

		vis_certainty = (vis_certainty + 1) / (visualisation_boundary + 1);

		// A rough heuristic for box around the face width
		int thickness = (int)std::ceil(2.0* ((double)captured_image.cols) / 640.0);

		cv::Vec6d pose_estimate_to_draw = LandmarkDetector::GetCorrectedPoseWorld(clnf_model, fx, fy, cx, cy);

		// Draw it in reddish if uncertain, blueish if certain
		LandmarkDetector::DrawBox(captured_image, pose_estimate_to_draw, cv::Scalar((1 - vis_certainty)*255.0, 0, vis_certainty * 255), thickness, fx, fy, cx, cy);

		if (det_parameters.track_gaze && detection_success && clnf_model.eye_model)
		{
			FaceAnalysis::DrawGaze(captured_image, clnf_model, gazeDirection0, gazeDirection1, fx, fy, cx, cy);
		}
	}

	// Work out the framerate
	if (frame_count % 10 == 0)
	{
		double t1 = cv::getTickCount();
		fps_tracker = 10.0 / (double(t1 - t0) / cv::getTickFrequency());
		t0 = t1;
	}

	// Write out the framerate on the image before displaying it
	char fpsC[255];
	std::sprintf(fpsC, "%d", (int)fps_tracker);
	string fpsSt("FPS:");
	fpsSt += fpsC;
	cv::putText(captured_image, fpsSt, cv::Point(10, 20), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(255, 0, 0));

	if (!det_parameters.quiet_mode)
	{
		cv::namedWindow("tracking_result", 1);
		cv::imshow("tracking_result", captured_image);
	}
}

// TODO make sure the ordering is the same
void prepareOutputFile(std::ofstream* output_file, bool output_2D_landmarks, bool output_3D_landmarks,
	bool output_model_params, bool output_pose, bool output_AUs, bool output_gaze,
	int num_landmarks, int num_model_modes, vector<string> au_names_class, vector<string> au_names_reg)
{

	*output_file << "frame, timestamp, confidence, success";

	if (output_gaze)
	{
		*output_file << ", x_0, y_0, z_0, x_1, y_1, z_1";
	}

	if (output_pose)
	{
		*output_file << ", Tx, Ty, Tz, Rx, Ry, Rz";
	}

	if (output_2D_landmarks)
	{
		for (int i = 0; i < num_landmarks; ++i)
		{
			*output_file << ", x" << i;
		}
		for (int i = 0; i < num_landmarks; ++i)
		{
			*output_file << ", y" << i;
		}
	}

	if (output_3D_landmarks)
	{
		for (int i = 0; i < num_landmarks; ++i)
		{
			*output_file << ", X" << i;
		}
		for (int i = 0; i < num_landmarks; ++i)
		{
			*output_file << ", Y" << i;
		}
		for (int i = 0; i < num_landmarks; ++i)
		{
			*output_file << ", Z" << i;
		}
	}

	// Outputting model parameters (rigid and non-rigid), the first parameters are the 6 rigid shape parameters, they are followed by the non rigid shape parameters
	if (output_model_params)
	{
		*output_file << ", scale, rx, ry, rz, tx, ty";
		for (int i = 0; i < num_model_modes; ++i)
		{
			*output_file << ", p" << i;
		}
	}

	if (output_AUs)
	{
		std::sort(au_names_reg.begin(), au_names_reg.end());
		for (string reg_name : au_names_reg)
		{
			*output_file << ", " << reg_name << "_r";
		}

		std::sort(au_names_class.begin(), au_names_class.end());
		for (string class_name : au_names_class)
		{
			*output_file << ", " << class_name << "_c";
		}
	}

	*output_file << endl;

}

// Output all of the information into one file in one go (quite a few parameters, but simplifies the flow)
void outputAllFeatures(std::ofstream* output_file, bool output_2D_landmarks, bool output_3D_landmarks,
	bool output_model_params, bool output_pose, bool output_AUs, bool output_gaze,
	const LandmarkDetector::CLNF& clnf_model, int frame_count, double time_stamp, bool detection_success,
	cv::Point3f gazeDirection0, cv::Point3f gazeDirection1, const cv::Vec6d& pose_estimate, double fx, double fy, double cx, double cy,
	const FaceAnalysis::FaceAnalyser& face_analyser)
{

	double confidence = 0.5 * (1 - clnf_model.detection_certainty);

	*output_file << frame_count + 1 << ", " << time_stamp << ", " << confidence << ", " << detection_success;

	// Output the detected 2D facial landmarks
	if (output_2D_landmarks)
	{
		for (int i = 0; i < clnf_model.pdm.NumberOfPoints() * 2; ++i)
		{
			*output_file << ", " << clnf_model.detected_landmarks.at<double>(i);
		}
	}

	// Output the detected 3D facial landmarks
	if (output_3D_landmarks)
	{
		cv::Mat_<double> shape_3D = clnf_model.GetShape(fx, fy, cx, cy);
		for (int i = 0; i < clnf_model.pdm.NumberOfPoints() * 3; ++i)
		{
			*output_file << ", " << shape_3D.at<double>(i);
		}
	}

	if (output_model_params)
	{
		for (int i = 0; i < 6; ++i)
		{
			*output_file << ", " << clnf_model.params_global[i];
		}
		for (int i = 0; i < clnf_model.pdm.NumberOfModes(); ++i)
		{
			*output_file << ", " << clnf_model.params_local.at<double>(i, 0);
		}
	}

	// Output the estimated head pose
	if (output_pose)
	{
		*output_file << ", " << pose_estimate[0] << ", " << pose_estimate[1] << ", " << pose_estimate[2]
			<< ", " << pose_estimate[3] << ", " << pose_estimate[4] << ", " << pose_estimate[5];
	}

	// Output the estimated gaze
	if (output_gaze)
	{
		*output_file << ", " << gazeDirection0.x << ", " << gazeDirection0.y << ", " << gazeDirection0.z
			<< ", " << gazeDirection1.x << ", " << gazeDirection1.y << ", " << gazeDirection1.z;
	}

	if (output_AUs)
	{
		auto aus_reg = face_analyser.GetCurrentAUsReg();

		vector<string> au_reg_names = face_analyser.GetAURegNames();
		std::sort(au_reg_names.begin(), au_reg_names.end());

		// write out ar the correct index
		for (string au_name : au_reg_names)
		{
			for (auto au_reg : aus_reg)
			{
				if (au_name.compare(au_reg.first) == 0)
				{
					*output_file << ", " << au_reg.second;
					break;
				}
			}
		}

		if (aus_reg.size() == 0)
		{
			for (size_t p = 0; p < face_analyser.GetAURegNames().size(); ++p)
			{
				*output_file << ", 0";
			}
		}

		auto aus_class = face_analyser.GetCurrentAUsClass();

		vector<string> au_class_names = face_analyser.GetAUClassNames();
		std::sort(au_class_names.begin(), au_class_names.end());

		// write out ar the correct index
		for (string au_name : au_class_names)
		{
			for (auto au_class: aus_class)
			{
				if (au_name.compare(au_class.first) == 0)
				{
					*output_file << ", " << au_class.second;
					break;
				}
			}
		}

		if (aus_class.size() == 0)
		{
			for (size_t p = 0; p < face_analyser.GetAUClassNames().size(); ++p)
			{
				*output_file << ", 0";
			}
		}
	}
	*output_file << endl;
}

void post_process_output_file(FaceAnalysis::FaceAnalyser& face_analyser, string output_file);


int main (int argc, char **argv)
{

	vector<string> arguments = get_arguments(argc, argv);

	// Some initial parameters that can be overriden from command line	
	vector<string> input_files, depth_directories, output_files, tracked_videos_output;
	
	LandmarkDetector::FaceModelParameters det_parameters(arguments);
	// Always track gaze in feature extraction
	det_parameters.track_gaze = true;

	// Get the input output file parameters
	
	// Indicates that rotation should be with respect to camera or world coordinates
	bool use_world_coordinates;
	LandmarkDetector::get_video_input_output_params(input_files, depth_directories, output_files, tracked_videos_output, use_world_coordinates, arguments);

	bool video_input = true;
	bool verbose = true;
	bool images_as_video = false;

	vector<vector<string> > input_image_files;

	// Adding image support for reading in the files
	if(input_files.empty())
	{
		vector<string> d_files;
		vector<string> o_img;
		vector<cv::Rect_<double>> bboxes;
		get_image_input_output_params_feats(input_image_files, images_as_video, arguments);	

		if(!input_image_files.empty())
		{
			video_input = false;
		}

	}

	// Grab camera parameters, if they are not defined (approximate values will be used)
	float fx = 0, fy = 0, cx = 0, cy = 0;
	int d = 0;
	// Get camera parameters
	LandmarkDetector::get_camera_params(d, fx, fy, cx, cy, arguments);    
	
	// If cx (optical axis centre) is undefined will use the image size/2 as an estimate
	bool cx_undefined = false;
	bool fx_undefined = false;
	if (cx == 0 || cy == 0)
	{
		cx_undefined = true;
	}
	if (fx == 0 || fy == 0)
	{
		fx_undefined = true;
	}

	// The modules that are being used for tracking
	LandmarkDetector::CLNF clnf_model(det_parameters.model_location);	

	vector<string> output_similarity_align;
	vector<string> output_hog_align_files;

	double sim_scale = 0.7;
	int sim_size = 112;
	bool grayscale = false;	
	bool video_output = false;
	bool rigid = false;	
	int num_hog_rows;
	int num_hog_cols;

	// By default output all parameters, but these can be turned off to get smaller files or slightly faster processing times
	// use -no2Dfp, -no3Dfp, -noMparams, -noPose, -noAUs, -noGaze to turn them off
	bool output_2D_landmarks = true;
	bool output_3D_landmarks = true;
	bool output_model_params = true;
	bool output_pose = true; 
	bool output_AUs = true;
	bool output_gaze = true;

	get_output_feature_params(output_similarity_align, output_hog_align_files, sim_scale, sim_size, grayscale, rigid, verbose, 
		output_2D_landmarks, output_3D_landmarks, output_model_params, output_pose, output_AUs, output_gaze, arguments);
	
	// Used for image masking

	cv::Mat_<int> triangulation;
	string tri_loc;
	if(boost::filesystem::exists(path("model/tris_68_full.txt")))
	{
		std::ifstream triangulation_file("model/tris_68_full.txt");
		LandmarkDetector::ReadMat(triangulation_file, triangulation);
		tri_loc = "model/tris_68_full.txt";
	}
	else
	{
		path loc = path(arguments[0]).parent_path() / "model/tris_68_full.txt";
		tri_loc = loc.string();

		if(exists(loc))
		{
			std::ifstream triangulation_file(loc.string());
			LandmarkDetector::ReadMat(triangulation_file, triangulation);
		}
		else
		{
			cout << "Can't find triangulation files, exiting" << endl;
			return 0;
		}
	}	

	// Will warp to scaled mean shape
	cv::Mat_<double> similarity_normalised_shape = clnf_model.pdm.mean_shape * sim_scale;
	// Discard the z component
	similarity_normalised_shape = similarity_normalised_shape(cv::Rect(0, 0, 1, 2*similarity_normalised_shape.rows/3)).clone();

	// If multiple video files are tracked, use this to indicate if we are done
	bool done = false;	
	int f_n = -1;
	int curr_img = -1;

	string au_loc;
	if(boost::filesystem::exists(path("AU_predictors/AU_all_best.txt")))
	{
		au_loc = "AU_predictors/AU_all_best.txt";
	}
	else
	{
		path loc = path(arguments[0]).parent_path() / "AU_predictors/AU_all_best.txt";

		if(exists(loc))
		{
			au_loc = loc.string();
		}
		else
		{
			cout << "Can't find AU prediction files, exiting" << endl;
			return 0;
		}
	}	

	// Creating a  face analyser that will be used for AU extraction
	FaceAnalysis::FaceAnalyser face_analyser(vector<cv::Vec3d>(), 0.7, 112, 112, au_loc, tri_loc);
		
	while(!done) // this is not a for loop as we might also be reading from a webcam
	{
		
		string current_file;
		
		cv::VideoCapture video_capture;
		
		cv::Mat captured_image;
		int total_frames = -1;
		int reported_completion = 0;

		double fps_vid_in = -1.0;

		if(video_input)
		{
			// We might specify multiple video files as arguments
			if(input_files.size() > 0)
			{
				f_n++;			
				current_file = input_files[f_n];
			}
			else
			{
				// If we want to write out from webcam
				f_n = 0;
			}
			// Do some grabbing
			if( current_file.size() > 0 )
			{
				INFO_STREAM( "Attempting to read from file: " << current_file );
				video_capture = cv::VideoCapture( current_file );
				total_frames = (int)video_capture.get(CV_CAP_PROP_FRAME_COUNT);
				fps_vid_in = video_capture.get(CV_CAP_PROP_FPS);

				// Check if fps is nan or less than 0
				if (fps_vid_in != fps_vid_in || fps_vid_in <= 0)
				{
					INFO_STREAM("FPS of the video file cannot be determined, assuming 30");
					fps_vid_in = 30;
				}
			}

			if (!video_capture.isOpened())
			{
				FATAL_STREAM("Failed to open video source, exiting");
				return 1;
			}
			else
			{
				INFO_STREAM("Device or file opened");
			}

			video_capture >> captured_image;	
		}
		else
		{
			f_n++;	
			curr_img++;
			if(!input_image_files[f_n].empty())
			{
				string curr_img_file = input_image_files[f_n][curr_img];
				captured_image = cv::imread(curr_img_file, -1);
			}
			else
			{
				FATAL_STREAM( "No .jpg or .png images in a specified drectory, exiting" );
				return 1;
			}

		}	
		
		// If optical centers are not defined just use center of image
		if(cx_undefined)
		{
			cx = captured_image.cols / 2.0f;
			cy = captured_image.rows / 2.0f;
		}
		// Use a rough guess-timate of focal length
		if (fx_undefined)
		{
			fx = 500 * (captured_image.cols / 640.0);
			fy = 500 * (captured_image.rows / 480.0);

			fx = (fx + fy) / 2.0;
			fy = fx;
		}
	
		// Creating output files
		std::ofstream output_file;

		if (!output_files.empty())
		{
			output_file.open(output_files[f_n], ios_base::out);
			prepareOutputFile(&output_file, output_2D_landmarks, output_3D_landmarks, output_model_params, output_pose, output_AUs, output_gaze, clnf_model.pdm.NumberOfPoints(), clnf_model.pdm.NumberOfModes(), face_analyser.GetAUClassNames(), face_analyser.GetAURegNames());
		}

		// Saving the HOG features
		std::ofstream hog_output_file;
		if(!output_hog_align_files.empty())
		{
			hog_output_file.open(output_hog_align_files[f_n], ios_base::out | ios_base::binary);
		}

		// saving the videos
		cv::VideoWriter writerFace;
		if(!tracked_videos_output.empty())
		{
			writerFace = cv::VideoWriter(tracked_videos_output[f_n], CV_FOURCC('D', 'I', 'V', 'X'), fps_vid_in, captured_image.size(), true);
		}

		int frame_count = 0;
		
		// This is useful for a second pass run (if want AU predictions)
		vector<cv::Vec6d> params_global_video;
		vector<bool> successes_video;
		vector<cv::Mat_<double>> params_local_video;
		vector<cv::Mat_<double>> detected_landmarks_video;
				
		// Use for timestamping if using a webcam
		int64 t_initial = cv::getTickCount();

		bool visualise_hog = verbose;

		// Timestamp in seconds of current processing
		double time_stamp = 0;

		INFO_STREAM( "Starting tracking");
		while(!captured_image.empty())
		{		

			// Grab the timestamp first
			if (video_input)
			{
				time_stamp = (double)frame_count * (1.0 / fps_vid_in);				
			}
			else
			{
				time_stamp = 0.0;
			}

			// Reading the images
			cv::Mat_<uchar> grayscale_image;

			if(captured_image.channels() == 3)
			{
				cvtColor(captured_image, grayscale_image, CV_BGR2GRAY);				
			}
			else
			{
				grayscale_image = captured_image.clone();				
			}
		
			// The actual facial landmark detection / tracking
			bool detection_success;
			
			if(video_input || images_as_video)
			{
				detection_success = LandmarkDetector::DetectLandmarksInVideo(grayscale_image, clnf_model, det_parameters);
			}
			else
			{
				detection_success = LandmarkDetector::DetectLandmarksInImage(grayscale_image, clnf_model, det_parameters);
			}
			
			// Gaze tracking, absolute gaze direction
			cv::Point3f gazeDirection0(0, 0, -1);
			cv::Point3f gazeDirection1(0, 0, -1);

			// Gaze with respect to head rather than camera (for example if eyes are rolled up and the head is tilted or turned this will be stable)
			cv::Point3f gazeDirection0_head(0, 0, -1);
			cv::Point3f gazeDirection1_head(0, 0, -1);

			if (det_parameters.track_gaze && detection_success && clnf_model.eye_model)
			{
				FaceAnalysis::EstimateGaze(clnf_model, gazeDirection0, fx, fy, cx, cy, true);
				FaceAnalysis::EstimateGaze(clnf_model, gazeDirection1, fx, fy, cx, cy, false);
			}

			// Do face alignment
			cv::Mat sim_warped_img;
			cv::Mat_<double> hog_descriptor;

			// But only if needed in output, TODO if AU is being output
			if(!output_similarity_align.empty() || hog_output_file.is_open() || output_AUs)
			{
				face_analyser.AddNextFrame(captured_image, clnf_model, time_stamp, false, !det_parameters.quiet_mode);
				face_analyser.GetLatestAlignedFace(sim_warped_img);

				if(!det_parameters.quiet_mode)
				{
					cv::imshow("sim_warp", sim_warped_img);			
				}
				if(hog_output_file.is_open())
				{
					FaceAnalysis::Extract_FHOG_descriptor(hog_descriptor, sim_warped_img, num_hog_rows, num_hog_cols);						

					if(visualise_hog && !det_parameters.quiet_mode)
					{
						cv::Mat_<double> hog_descriptor_vis;
						FaceAnalysis::Visualise_FHOG(hog_descriptor, num_hog_rows, num_hog_cols, hog_descriptor_vis);
						cv::imshow("hog", hog_descriptor_vis);	
					}
				}
			}

			// Work out the pose of the head from the tracked model
			cv::Vec6d pose_estimate;
			if(use_world_coordinates)
			{
				pose_estimate = LandmarkDetector::GetCorrectedPoseWorld(clnf_model, fx, fy, cx, cy);
			}
			else
			{
				pose_estimate = LandmarkDetector::GetCorrectedPoseCamera(clnf_model, fx, fy, cx, cy);
			}

			if(hog_output_file.is_open())
			{
				output_HOG_frame(&hog_output_file, detection_success, hog_descriptor, num_hog_rows, num_hog_cols);
			}

			// Write the similarity normalised output
			if(!output_similarity_align.empty())
			{

				if (sim_warped_img.channels() == 3 && grayscale)
				{
					cvtColor(sim_warped_img, sim_warped_img, CV_BGR2GRAY);
				}

				char name[100];
					
				// output the frame number
				std::sprintf(name, "frame_det_%06d.png", frame_count);

				// Construct the output filename
				boost::filesystem::path slash("/");
					
				std::string preferredSlash = slash.make_preferred().string();
				
				string out_file = output_similarity_align[f_n] + preferredSlash + string(name);
				imwrite(out_file, sim_warped_img);

			}

			// Visualising the tracker
			visualise_tracking(captured_image, clnf_model, det_parameters, gazeDirection0, gazeDirection1, frame_count, fx, fy, cx, cy);

			// Output the landmarks, pose, gaze, parameters and AUs
			outputAllFeatures(&output_file, output_2D_landmarks, output_3D_landmarks, output_model_params, output_pose, output_AUs, output_gaze,
				clnf_model, frame_count, time_stamp, detection_success, gazeDirection0, gazeDirection1,
				pose_estimate, fx, fy, cx, cy, face_analyser);

			// output the tracked video
			if(!tracked_videos_output.empty())
			{		
				writerFace << captured_image;
			}

			if(video_input)
			{
				video_capture >> captured_image;
			}
			else
			{
				curr_img++;
				if(curr_img < (int)input_image_files[f_n].size())
				{
					string curr_img_file = input_image_files[f_n][curr_img];
					captured_image = cv::imread(curr_img_file, -1);
				}
				else
				{
					captured_image = cv::Mat();
				}
			}
			// detect key presses
			char character_press = cv::waitKey(1);
			
			// restart the tracker
			if(character_press == 'r')
			{
				clnf_model.Reset();
			}
			// quit the application
			else if(character_press=='q')
			{
				return(0);
			}

			// Update the frame count
			frame_count++;

			if(total_frames != -1)
			{
				if((double)frame_count/(double)total_frames >= reported_completion / 10.0)
				{
					cout << reported_completion * 10 << "% ";
					reported_completion = reported_completion + 1;
				}
			}

		}

		if(output_files.size() > 0)
		{
			output_file.close();
		
			// If the video is long enough post-process it for AUs
			if (output_AUs && frame_count > 100)
			{
				post_process_output_file(face_analyser, output_files[f_n]);
			}
		}
		// Reset the models for the next video
		face_analyser.Reset();
		clnf_model.Reset();

		frame_count = 0;
		curr_img = -1;

		if (total_frames != -1)
		{
			cout << endl;
		}

		// break out of the loop if done with all the files (or using a webcam)
		if(f_n == input_files.size() -1 || input_files.empty())
		{
			done = true;
		}
	}

	return 0;
}

// TODO sort indices with AU predictions
void post_process_output_file(FaceAnalysis::FaceAnalyser& face_analyser, string output_file)
{

	vector<double> certainties;
	vector<bool> successes;
	vector<double> timestamps;
	vector<std::pair<std::string, vector<double>>> predictions_reg;
	vector<std::pair<std::string, vector<double>>> predictions_class;

	// Construct the new values to overwrite the output file with
	face_analyser.ExtractAllPredictionsOfflineReg(predictions_reg, certainties, successes, timestamps);
	face_analyser.ExtractAllPredictionsOfflineClass(predictions_class, certainties, successes, timestamps);

	int num_class = predictions_class.size();
	int num_reg = predictions_reg.size();

	// Extract the indices of writing out first
	vector<string> au_reg_names = face_analyser.GetAURegNames();
	std::sort(au_reg_names.begin(), au_reg_names.end());
	vector<int> inds_reg;

	// write out ar the correct index
	for (string au_name : au_reg_names)
	{
		for (int i = 0; i < num_reg; ++i)
		{
			if (au_name.compare(predictions_reg[i].first) == 0)
			{
				inds_reg.push_back(i);
				break;
			}
		}
	}

	vector<string> au_class_names = face_analyser.GetAUClassNames();
	std::sort(au_class_names.begin(), au_class_names.end());
	vector<int> inds_class;

	// write out ar the correct index
	for (string au_name : au_class_names)
	{
		for (int i = 0; i < num_class; ++i)
		{
			if (au_name.compare(predictions_class[i].first) == 0)
			{
				inds_class.push_back(i);
				break;
			}
		}
	}
	// Read all of the output file in
	vector<string> output_file_contents;

	std::ifstream infile(output_file);
	string line;

	while (std::getline(infile, line))
		output_file_contents.push_back(line);

	infile.close();

	// Read the header and find all _r and _c parts in a file and use their indices
	std::vector<std::string> tokens;
	boost::split(tokens, output_file_contents[0], boost::is_any_of(","));

	int begin_ind = -1;
	
	for (int i = 0; i < tokens.size(); ++i)
	{
		if (tokens[i].find("_r") != string::npos && begin_ind == -1)
		{
			begin_ind = i;
			break;
		}
	}
	int end_ind = begin_ind + num_class + num_reg;

	// Now overwrite the whole file
	std::ofstream outfile(output_file, ios_base::out);
	// Write the header
	outfile << output_file_contents[0].c_str() << endl;
	
	// Write the contents
	for (int i = 1; i < output_file_contents.size(); ++i)
	{
		std::vector<std::string> tokens;
		boost::split(tokens, output_file_contents[i], boost::is_any_of(","));

		outfile << tokens[0];

		for (int t = 1; t < tokens.size(); ++t)
		{
			if (t >= begin_ind && t < end_ind)
			{
				if(t - begin_ind < num_reg)
				{
					outfile << "," << predictions_reg[inds_reg[t - begin_ind]].second[i - 1];
				}
				else
				{
					outfile << "," << predictions_class[inds_class[t - begin_ind - num_reg]].second[i - 1];
				}
			}
			else
			{
				outfile << "," << tokens[t];
			}
		}
		outfile << endl;
	}
		

}