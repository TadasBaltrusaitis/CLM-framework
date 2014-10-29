///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012, Tadas Baltrusaitis, all rights reserved.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
//     * The software is provided under the terms of this licence stricly for
//       academic, non-commercial, not-for-profit purposes.
//     * Redistributions of source code must retain the above copyright notice, 
//       this list of conditions (licence) and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright 
//       notice, this list of conditions (licence) and the following disclaimer 
//       in the documentation and/or other materials provided with the 
//       distribution.
//     * The name of the author may not be used to endorse or promote products 
//       derived from this software without specific prior written permission.
//     * As this software depends on other libraries, the user must adhere to 
//       and keep in place any licencing terms of those libraries.
//     * Any publications arising from the use of this software, including but
//       not limited to academic journal and conference publications, technical
//       reports and manuals, must cite one of the following works:
//
//       Tadas Baltrusaitis, Peter Robinson, and Louis-Philippe Morency. 3D
//       Constrained Local Model for Rigid and Non-Rigid Facial Tracking.
//       IEEE Conference on Computer Vision and Pattern Recognition (CVPR), 2012.    
//
//       Tadas Baltrusaitis, Peter Robinson, and Louis-Philippe Morency. 
//       Constrained Local Neural Fields for robust facial landmark detection in the wild.
//       in IEEE Int. Conference on Computer Vision Workshops, 300 Faces in-the-Wild Challenge, 2013.    
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED 
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO 
// EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///////////////////////////////////////////////////////////////////////////////


// SimpleCLM.cpp : Defines the entry point for the console application.

#include <CLM.h>
#include <CLMTracker.h>
#include <CLMParameters.h>
#include <CLM_utils.h>

#include <fstream>
#include <sstream>

#include <cv.h>

#include <filesystem.hpp>
#include <filesystem/fstream.hpp>

#include <dlib/gui_widgets.h>

#define INFO_STREAM( stream ) \
std::cout << stream << std::endl

#define WARN_STREAM( stream ) \
std::cout << "Warning: " << stream << std::endl

#define ERROR_STREAM( stream ) \
std::cout << "Error: " << stream << std::endl

static void printErrorAndAbort( const std::string & error )
{
    std::cout << error << std::endl;
    abort();
}

#define FATAL_STREAM( stream ) \
printErrorAndAbort( std::string( "Fatal error: " ) + stream )

using namespace std;
using namespace cv;

using namespace boost::filesystem;

vector<string> get_arguments(int argc, char **argv)
{

	vector<string> arguments;

	for(int i = 1; i < argc; ++i)
	{
		arguments.push_back(string(argv[i]));
	}
	return arguments;
}

// Extracting the following command line arguments -f, -fd, -op, -of, -ov (and possible ordered repetitions)
void get_output_feature_params(vector<string> &output_similarity_aligned_files, vector<string> &output_hog_aligned_files, vector<string> &output_model_param_files, double &similarity_scale, int &similarity_size, bool &video, bool &grayscale, bool &rigid, vector<string> &arguments)
{
	output_similarity_aligned_files.clear();
	output_hog_aligned_files.clear();
	output_model_param_files.clear();

	bool* valid = new bool[arguments.size()];
	video = false;

	for(size_t i = 0; i < arguments.size(); ++i)
	{
		valid[i] = true;
	}

	string input_root = "";
	string output_root = "";

	// First check if there is a root argument (so that videos and outputs could be defined more easilly)
	for(size_t i = 0; i < arguments.size(); ++i)
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

	for(size_t i = 0; i < arguments.size(); ++i)
	{
		if (arguments[i].compare("-simalign") == 0) 
		{                    
			output_similarity_aligned_files.push_back(output_root + arguments[i + 1]);
			valid[i] = false;
			valid[i+1] = false;			
			i++;
		}		
		else if(arguments[i].compare("-hogalign") == 0) 
		{
			output_hog_aligned_files.push_back(output_root + arguments[i + 1]);
			valid[i] = false;
			valid[i+1] = false;			
			i++;
		}
		else if(arguments[i].compare("-oparams") == 0) 
		{
			output_model_param_files.push_back(output_root + arguments[i + 1]);
			valid[i] = false;
			valid[i+1] = false;			
			i++;
		}
		else if(arguments[i].compare("-rigid") == 0) 
		{
			rigid = true;
		}
		else if(arguments[i].compare("-vid") == 0) 
		{
			video = true;
			valid[i] = false;
		}
		else if(arguments[i].compare("-g") == 0) 
		{
			grayscale = true;
			valid[i] = false;
		}
		else if (arguments[i].compare("-simscale") == 0) 
		{                    
			similarity_scale = stod(arguments[i + 1]);
			valid[i] = false;
			valid[i+1] = false;			
			i++;
		}		
		else if (arguments[i].compare("-simsize") == 0) 
		{                    
			similarity_size = stoi(arguments[i + 1]);
			valid[i] = false;
			valid[i+1] = false;			
			i++;
		}		
		else if (arguments[i].compare("-help") == 0)
		{
			cout << "Output features are defined as: -simalign <outputfile>\n"; // Inform the user of how to use the program				
		}
	}

	for(int i=arguments.size()-1; i >= 0; --i)
	{
		if(!valid[i])
		{
			arguments.erase(arguments.begin()+i);
		}
	}

}

// Pick only the more stable/rigid points under changes of expression
void extract_rigid_points(Mat_<double>& source_points, Mat_<double>& destination_points)
{
	if(source_points.rows == 68)
	{
		Mat_<double> tmp_source = source_points.clone();
		source_points = Mat_<double>();

		// Push back the rigid points (some face outline, eyes, and nose)
		source_points.push_back(tmp_source.row(0));
		source_points.push_back(tmp_source.row(2));
		source_points.push_back(tmp_source.row(14));
		source_points.push_back(tmp_source.row(16));
		source_points.push_back(tmp_source.row(36));
		source_points.push_back(tmp_source.row(39));
		source_points.push_back(tmp_source.row(43));
		source_points.push_back(tmp_source.row(38));
		source_points.push_back(tmp_source.row(42));
		source_points.push_back(tmp_source.row(45));
		source_points.push_back(tmp_source.row(31));
		source_points.push_back(tmp_source.row(33));
		source_points.push_back(tmp_source.row(35));

		Mat_<double> tmp_dest = destination_points.clone();
		destination_points = Mat_<double>();

		// Push back the rigid points
		destination_points.push_back(tmp_dest.row(0));
		destination_points.push_back(tmp_dest.row(2));
		destination_points.push_back(tmp_dest.row(14));
		destination_points.push_back(tmp_dest.row(16));
		destination_points.push_back(tmp_dest.row(36));
		destination_points.push_back(tmp_dest.row(39));
		destination_points.push_back(tmp_dest.row(43));
		destination_points.push_back(tmp_dest.row(38));
		destination_points.push_back(tmp_dest.row(42));
		destination_points.push_back(tmp_dest.row(45));
		destination_points.push_back(tmp_dest.row(31));
		destination_points.push_back(tmp_dest.row(33));
		destination_points.push_back(tmp_dest.row(35));
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

void output_HOG_frame(std::ofstream* hog_file, dlib::array2d<dlib::matrix<float,31,1> >* hog)
{
	int num_cols = hog->nc();
	int num_rows = hog->nr();

	int num_channels = 31;

	hog_file->write((char*)(&num_cols), 4);
	hog_file->write((char*)(&num_rows), 4);
	hog_file->write((char*)(&num_channels), 4);

	for(int y = 0; y < num_cols; ++y)
	{
		for(int x = 0; x < num_rows; ++x)
		{
			for(unsigned int o = 0; o < 31; ++o)
			{
				float hog_data = (*hog)[y][x](o);
				hog_file->write ((char*)&hog_data, 4);
			}
		}
	}
}

int main (int argc, char **argv)
{

	vector<string> arguments = get_arguments(argc, argv);

	// Some initial parameters that can be overriden from command line	
	vector<string> files, depth_directories, pose_output_files, tracked_videos_output, landmark_output_files;
	
	// By default try webcam 0
	int device = 0;

	// cx and cy aren't necessarilly in the image center, so need to be able to override it (start with unit vals and init them if none specified)
    float fx = 500, fy = 500, cx = 0, cy = 0;
			
	CLMTracker::CLMParameters clm_parameters(arguments);
			
	// Get the input output file parameters
	
	// Indicates that rotation should be with respect to camera plane or with respect to camera
	bool use_camera_plane_pose;
	CLMTracker::get_video_input_output_params(files, depth_directories, pose_output_files, tracked_videos_output, landmark_output_files, use_camera_plane_pose, arguments);

	bool video = true;
	bool images_as_video = false;

	vector<vector<string> > input_image_files;

	// Adding image support for reading in the files
	if(files.empty())
	{
		vector<string> d_files;
		vector<string> o_img;
		vector<Rect_<double>> bboxes;
		get_image_input_output_params_feats(input_image_files, images_as_video, arguments);	

		if(!input_image_files.empty())
		{
			video = false;
		}

	}
	// Get camera parameters
	CLMTracker::get_camera_params(device, fx, fy, cx, cy, arguments);    
	
	// The modules that are being used for tracking
	CLMTracker::CLM clm_model(clm_parameters.model_location);	

	vector<string> output_similarity_align_files;
	vector<string> output_hog_align_files;
	vector<string> params_output_files;

	double sim_scale = 0.6;
	int sim_size = 96;
	bool video_output;
	bool grayscale = false;
	bool rigid = false;
	get_output_feature_params(output_similarity_align_files, output_hog_align_files, params_output_files, sim_scale, sim_size, video_output, grayscale, rigid, arguments);

	// Will warp to scaled mean shape
	Mat_<double> similarity_normalised_shape = clm_model.pdm.mean_shape * sim_scale;
	// Discard the z component
	similarity_normalised_shape = similarity_normalised_shape(Rect(0, 0, 1, 2*similarity_normalised_shape.rows/3)).clone();

	// If multiple video files are tracked, use this to indicate if we are done
	bool done = false;	
	int f_n = -1;
	int curr_img = -1;

	// If cx (optical axis centre) is undefined will use the image size/2 as an estimate
	bool cx_undefined = false;
	if(cx == 0 || cy == 0)
	{
		cx_undefined = true;
	}		
	
	// TODO rem
	//dlib::image_window hogwin;
	//hogwin.set_title("HOG image");

	while(!done) // this is not a for loop as we might also be reading from a webcam
	{
		
		string current_file;
		
		VideoCapture video_capture;
		
		Mat captured_image;

		if(video)
		{
			// We might specify multiple video files as arguments
			if(files.size() > 0)
			{
				f_n++;			
				current_file = files[f_n];
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
				video_capture = VideoCapture( current_file );
			}
			else
			{
				INFO_STREAM( "Attempting to capture from device: " << device );
				video_capture = VideoCapture( device );

				// Read a first frame often empty in camera
				Mat captured_image;
				video_capture >> captured_image;
			}

			if( !video_capture.isOpened() ) FATAL_STREAM( "Failed to open video source" );
			else INFO_STREAM( "Device or file opened");

			video_capture >> captured_image;	
		}
		else
		{
			f_n++;	
			curr_img++;
			if(!input_image_files[f_n].empty())
			{
				string curr_img_file = input_image_files[f_n][curr_img];
				captured_image = imread(curr_img_file, -1);
			}
			else
			{
				FATAL_STREAM( "No .jpg or .png images in a specified drectory" );
			}

		}	
		
		// If optical centers are not defined just use center of image
		if(cx_undefined)
		{
			cx = captured_image.cols / 2.0f;
			cy = captured_image.rows / 2.0f;
		}
	
		// Creating output files
		std::ofstream pose_output_file;
		if(!pose_output_files.empty())
		{
			pose_output_file.open (pose_output_files[f_n]);
		}
	
		std::ofstream landmarks_output_file;		
		if(!landmark_output_files.empty())
		{
			landmarks_output_file.open(landmark_output_files[f_n]);
		}

		// Outputting model parameters (rigid and non-rigid), the first parameters are the 6 rigid shape parameters, they are followed by the non rigid shape parameters
		std::ofstream params_output_file;		
		if(!params_output_files.empty())
		{
			params_output_file.open(params_output_files[f_n]);
		}

		// saving the videos
		VideoWriter output_similarity_aligned_video;
		if(!output_similarity_align_files.empty())
		{
			if(video_output)
			{
				double fps = video_capture.get(CV_CAP_PROP_FPS);
				output_similarity_aligned_video = VideoWriter(output_similarity_align_files[f_n], CV_FOURCC('H','F','Y','U'), fps, Size(sim_size, sim_size), true);
			}			
			else
			{
				if(!boost::filesystem::exists(boost::filesystem::path(output_similarity_align_files[f_n])))
				{
					bool success = boost::filesystem::create_directory(output_similarity_align_files[f_n]);

					if(!success)
					{
						cout << "Failed to create a directory... exiting";
						cin.get();
						return 0;
					}
				}
			}

		}
		
		// Saving the HOG features
		std::ofstream hog_output_file;
		if(!output_hog_align_files.empty())
		{
			hog_output_file.open(output_hog_align_files[f_n], ios_base::out | ios_base::binary);
		}

		// saving the videos
		VideoWriter writerFace;
		if(!tracked_videos_output.empty())
		{
			writerFace = VideoWriter(tracked_videos_output[f_n], CV_FOURCC('D','I','V','X'), 30, captured_image.size(), true);		
		}

		int frame_count = 0;
		
		// For measuring the timings
		int64 t1,t0 = cv::getTickCount();
		double fps = 10;

		INFO_STREAM( "Starting tracking");
		while(!captured_image.empty())
		{		

			// Reading the images
			Mat_<uchar> grayscale_image;

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
			
			if(video || images_as_video)
			{
				detection_success = CLMTracker::DetectLandmarksInVideo(grayscale_image, clm_model, clm_parameters);
			}
			else
			{
				detection_success = CLMTracker::DetectLandmarksInImage(grayscale_image, clm_model, clm_parameters);
			}

			// Work out the pose of the head from the tracked model
			Vec6d pose_estimate_CLM;
			if(use_camera_plane_pose)
			{
				pose_estimate_CLM = CLMTracker::GetCorrectedPoseCameraPlane(clm_model, fx, fy, cx, cy, clm_parameters);
			}
			else
			{
				pose_estimate_CLM = CLMTracker::GetCorrectedPoseCamera(clm_model, fx, fy, cx, cy, clm_parameters);
			}

			Mat_<double> source_landmarks = clm_model.detected_landmarks.reshape(1, 2).t();
			Mat_<double> destination_landmarks = similarity_normalised_shape.reshape(1, 2).t();

			// Pick only rigid points
			if(rigid)
			{
				 extract_rigid_points(source_landmarks, destination_landmarks);
			}

			Matx22d scale_rot_matrix = CLMTracker::AlignShapesWithScale(source_landmarks, destination_landmarks);
			Matx23d warp_matrix;
			warp_matrix(0,0) = scale_rot_matrix(0,0);
			warp_matrix(0,1) = scale_rot_matrix(0,1);
			warp_matrix(1,0) = scale_rot_matrix(1,0);
			warp_matrix(1,1) = scale_rot_matrix(1,1);

			double tx = clm_model.params_global[4];
			double ty = clm_model.params_global[5];

			Vec2d T(tx, ty);
			T = scale_rot_matrix * T;

			// Make sure centering is correct
			warp_matrix(0,2) = -T(0) + sim_size/2;
			warp_matrix(1,2) = -T(1) + sim_size/2;

			Mat sim_warped_img;
			//cv::warpAffine(captured_image, sim_warped_img, warp_matrix, Size(sim_size, sim_size), INTER_CUBIC);
			cv::warpAffine(captured_image, sim_warped_img, warp_matrix, Size(sim_size, sim_size), INTER_LINEAR);
			//cv::warpAffine(captured_image, sim_warped_img, warp_matrix, Size(sim_size, sim_size), INTER_LANCZOS4);
			cv::imshow("sim_warp", sim_warped_img);
			

			Mat_<uchar> sim_warped_img_gray;

			if(sim_warped_img.channels() == 3)
			{
				cv::cvtColor(sim_warped_img, sim_warped_img_gray, CV_BGR2GRAY);
				if(grayscale)
				{					
					sim_warped_img = sim_warped_img_gray.clone();
				}
			}	
			else
			{
				sim_warped_img_gray = sim_warped_img.clone();
			}

			// TODO use colour here?
			dlib::cv_image<uchar> dlib_warped_img(sim_warped_img_gray);

			dlib::array2d<dlib::matrix<float,31,1> > hog;
			dlib::extract_fhog_features(dlib_warped_img, hog, 8);
			
			//hogwin.set_image(dlib::draw_fhog(hog));

			if(hog_output_file.is_open())
			{
				output_HOG_frame(&hog_output_file, &hog);
			}

			// Write the similarity normalised output
			if(!output_similarity_align_files.empty())
			{
				if(video_output)
				{
					if(output_similarity_aligned_video.isOpened())
					{
						output_similarity_aligned_video << sim_warped_img;
					}
				}
				else
				{
					char name[100];
					
					// output the frame number
					sprintf(name, "frame_det_%06d.png", frame_count);

					// Construct the output filename
					boost::filesystem::path slash("/");
					
					std::string preferredSlash = slash.make_preferred().string();
				
					string out_file = output_similarity_align_files[f_n] + preferredSlash + string(name);
					imwrite(out_file, sim_warped_img);
				}
			}
			// Visualising the results
			// Drawing the facial landmarks on the face and the bounding box around it if tracking is successful and initialised
			double detection_certainty = clm_model.detection_certainty;

			double visualisation_boundary = 0.2;
			
			// Only draw if the reliability is reasonable, the value is slightly ad-hoc
			if(detection_certainty < visualisation_boundary)
			{
				CLMTracker::Draw(captured_image, source_landmarks);
				//CLMTracker::Draw(captured_image, clm_model);

				if(detection_certainty > 1)
					detection_certainty = 1;
				if(detection_certainty < -1)
					detection_certainty = -1;

				detection_certainty = (detection_certainty + 1)/(visualisation_boundary +1);

				// A rough heuristic for box around the face width
				int thickness = (int)std::ceil(2.0* ((double)captured_image.cols) / 640.0);
				
				Vec6d pose_estimate_to_draw = CLMTracker::GetCorrectedPoseCameraPlane(clm_model, fx, fy, cx, cy, clm_parameters);

				// Draw it in reddish if uncertain, blueish if certain
				CLMTracker::DrawBox(captured_image, pose_estimate_to_draw, Scalar((1-detection_certainty)*255.0,0, detection_certainty*255), thickness, fx, fy, cx, cy);

			}

			// Work out the framerate
			if(frame_count % 10 == 0)
			{      
				t1 = cv::getTickCount();
				fps = 10.0 / (double(t1-t0)/cv::getTickFrequency()); 
				t0 = t1;
			}
			
			// Write out the framerate on the image before displaying it
			char fpsC[255];
			sprintf(fpsC, "%d", (int)fps);
			string fpsSt("FPS:");
			fpsSt += fpsC;
			cv::putText(captured_image, fpsSt, cv::Point(10,20), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(255,0,0));		
			
			if(!clm_parameters.quiet_mode)
			{
				namedWindow("tracking_result",1);		
				imshow("tracking_result", captured_image);
			}

			// Output the detected facial landmarks
			if(!landmark_output_files.empty())
			{
				landmarks_output_file << frame_count + 1 << " " << detection_success;
				for (int i = 0; i < clm_model.pdm.NumberOfPoints() * 2; ++i)
				{
					landmarks_output_file << " " << clm_model.detected_landmarks.at<double>(i) << " ";
				}
				landmarks_output_file << endl;
			}
			
			if(!params_output_files.empty())
			{
				params_output_file << frame_count + 1 << " " << detection_success;
				for (int i = 0; i < 6; ++i)
				{
					params_output_file << " " << clm_model.params_global[i] << " "; 
				}
				for (int i = 0; i < clm_model.pdm.NumberOfModes(); ++i)
				{
					params_output_file << " " << clm_model.params_local.at<double>(i,0) << " "; 
				}
				params_output_file << endl;
			}

			// Output the estimated head pose
			if(!pose_output_files.empty())
			{
				pose_output_file << frame_count + 1 << " " << (float)frame_count * 1000/30 << " " << 1 << " " << pose_estimate_CLM[0] << " " << pose_estimate_CLM[1] << " " << pose_estimate_CLM[2] << " " << pose_estimate_CLM[3] << " " << pose_estimate_CLM[4] << " " << pose_estimate_CLM[5] << endl;
			}				

			// output the tracked video
			if(!tracked_videos_output.empty())
			{		
				writerFace << captured_image;
			}

			if(video)
			{
				video_capture >> captured_image;
			}
			else
			{
				curr_img++;
				if(curr_img < (int)input_image_files[f_n].size())
				{
					string curr_img_file = input_image_files[f_n][curr_img];
					captured_image = imread(curr_img_file, -1);
				}
				else
				{
					captured_image = Mat();
				}
			}
			// detect key presses
			char character_press = cv::waitKey(1);
			
			// restart the tracker
			if(character_press == 'r')
			{
				clm_model.Reset();
			}
			// quit the application
			else if(character_press=='q')
			{
				return(0);
			}

			// Update the frame count
			frame_count++;

		}
		
		frame_count = 0;
		curr_img = -1;

		// Reset the model, for the next video
		clm_model.Reset();

		pose_output_file.close();
		landmarks_output_file.close();

		// break out of the loop if done with all the files (or using a webcam)
		if(f_n == files.size() -1 || files.empty())
		{
			done = true;
		}
	}

	return 0;
}

