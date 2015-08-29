///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2014, University of Southern California and University of Cambridge,
// all rights reserved.
//
// THIS SOFTWARE IS PROVIDED “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY. OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
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
///////////////////////////////////////////////////////////////////////////////
#include "CLM_core.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <opencv2/videoio/videoio.hpp>  // Video write
#include <opencv2/videoio/videoio_c.h>  // Video write

// Boost stuff
#include <filesystem.hpp>
#include <filesystem/fstream.hpp>

#include <boost/tokenizer.hpp>

using namespace boost;
using namespace std;
using namespace cv;

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


using namespace boost::filesystem;

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
			std::cout << "Failed to create a directory... " << p.string() << endl;
		}
	}
}


// Extracting the following command line arguments -f, -ip, -if, -iac, -iar, -ov (and possible ordered repetitions)
void get_video_input_output_params(vector<string> &input_video_files,  vector<string>& timestamps,
	vector<string> &input_pose_files, string &output_video_file, vector<string> &input_2d_landmark_files, vector<string> &input_au_class, vector<string> &input_au_reg, double& offset, string& sum_doc, string& sum_pat, vector<string> &arguments)
{
	bool* valid = new bool[arguments.size()];

	for(size_t i = 0; i < arguments.size(); ++i)
	{
		valid[i] = true;
	}

	string root = "";
	// First check if there is a root argument (so that videos and outputs could be defined more easilly)
	for(size_t i = 0; i < arguments.size(); ++i)
	{
		if (arguments[i].compare("-root") == 0) 
		{                    
			root = arguments[i + 1];
			// Do not discard root as it might be used in other later steps
			i++;
		}		
	}

	for(size_t i = 0; i < arguments.size(); ++i)
	{
		if (arguments[i].compare("-f") == 0) 
		{                    
			input_video_files.push_back(root + arguments[i + 1]);			
			valid[i] = false;
			valid[i+1] = false;			
			i++;
		}		
		else if (arguments[i].compare("-offset") == 0) 
		{                    
			offset = stod(arguments[i + 1]);			
			valid[i] = false;
			valid[i+1] = false;			
			i++;
		}		
		else if (arguments[i].compare("-ip") == 0)
		{
			input_pose_files.push_back(root + arguments[i + 1]);
			valid[i] = false;
			valid[i+1] = false;
			i++;
		} 
		else if (arguments[i].compare("-osd") == 0)
		{
			sum_doc = root + arguments[i + 1];
			valid[i] = false;
			valid[i+1] = false;
			i++;
		} 
		else if (arguments[i].compare("-osp") == 0)
		{
			sum_pat = root + arguments[i + 1];
			valid[i] = false;
			valid[i+1] = false;
			i++;
		} 
		else if (arguments[i].compare("-if") == 0)
		{
			input_2d_landmark_files.push_back(root + arguments[i + 1]);
			valid[i] = false;
			valid[i+1] = false;
			i++;
		} 
		else if (arguments[i].compare("-iac") == 0)
		{
			input_au_class.push_back(root + arguments[i + 1]);
			valid[i] = false;
			valid[i+1] = false;
			i++;
		} 
		else if (arguments[i].compare("-t") == 0)
		{
			timestamps.push_back(root + arguments[i + 1]);
			valid[i] = false;
			valid[i+1] = false;
			i++;
		} 
		else if (arguments[i].compare("-iar") == 0)
		{
			input_au_reg.push_back(root + arguments[i + 1]);
			valid[i] = false;
			valid[i+1] = false;
			i++;
		} 
		else if (arguments[i].compare("-ov") == 0)
		{
			output_video_file = (root + arguments[i + 1]);
			create_directory_from_file(root + arguments[i + 1]);
			valid[i] = false;
			valid[i+1] = false;
			i++;
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

vector<string> get_arguments(int argc, char **argv)
{

	vector<string> arguments;

	for(int i = 0; i < argc; ++i)
	{
		arguments.push_back(string(argv[i]));
	}
	return arguments;
}

void process_features(string fp_loc, string pose_loc, string au_class_loc, string au_reg_loc, string timestamp_loc,
					  vector<cv::Mat_<double>>& fps, vector<cv::Vec6d>& poses, vector<string>& au_names_class,
					  vector<vector<int>>& au_values, vector<string>& au_names_reg, vector<vector<double>>& au_values_reg, 
					  vector<double>& confidences, vector<double>& timestamps)
{

	// Open the input steams
	std::ifstream pose_input_file;
	pose_input_file.open(pose_loc);

	std::ifstream timestamp_input_file;
	timestamp_input_file.open(timestamp_loc);

	std::ifstream fp_input_file;
	fp_input_file.open(fp_loc);

	std::ifstream au_class_input_file;
	au_class_input_file.open(au_class_loc);

	std::ifstream au_reg_input_file;
	au_reg_input_file.open(au_reg_loc);

	// Reading the poses
	while(!pose_input_file.eof())
	{
		string pose_line;
		std::getline(pose_input_file, pose_line);

		if(pose_line.empty())
			break;

		stringstream ss(pose_line);
		Vec6d pose_curr;
		for(size_t i = 0; i < 9; ++i)
		{
			double val;
			ss >> val;
			
			if(i > 2)
			{
				pose_curr[i-3] = val;
			}
		}
		poses.push_back(pose_curr);
	}

	// The first lines of AU class and reg are different
	string first_line_class;
	std::getline(au_class_input_file, first_line_class);
    char_separator<char> sep(", ");
	tokenizer<char_separator<char>> tokens(first_line_class, sep);
	
    for (const auto& t : tokens) {
		au_names_class.push_back(t);
    }
	au_names_class.erase(au_names_class.begin());
	au_names_class.erase(au_names_class.begin());
	au_names_class.erase(au_names_class.begin());

	au_values.reserve(poses.size());
	while(!au_class_input_file.eof())
	{
		string class_line;
		std::getline(au_class_input_file, class_line);

		if(class_line.empty())
			break;


		stringstream ss(class_line);
		vector<int> au_val_curr;
		for(size_t i = 0; i < au_names_class.size() + 3; ++i)
		{
			double val;
			ss >> val;
			
			if(i == 2)
			{
				confidences.push_back(val);
			}
			if(i > 2)
			{
				au_val_curr.push_back((int)val);
			}
		}

		// Remove the unreliable ones
		au_val_curr.erase(au_val_curr.begin()+8);
		au_val_curr.erase(au_val_curr.begin()+7);
		au_val_curr.erase(au_val_curr.begin()+5);
		au_val_curr.erase(au_val_curr.begin()+2);
		au_val_curr.erase(au_val_curr.begin());
		au_values.push_back(au_val_curr);
	}

	// Remove unreliable names
	au_names_class.erase(au_names_class.begin()+8);
	au_names_class.erase(au_names_class.begin()+7);
	au_names_class.erase(au_names_class.begin()+5);
	au_names_class.erase(au_names_class.begin()+2);
	au_names_class.erase(au_names_class.begin());

	string first_line_reg;
	std::getline(au_reg_input_file, first_line_reg);
	tokenizer<char_separator<char>> tokens_name(first_line_reg, sep);
    for (const auto& t : tokens_name) {
		au_names_reg.push_back(t);
    }
	au_names_reg.erase(au_names_reg.begin());
	au_names_reg.erase(au_names_reg.begin());
	au_names_reg.erase(au_names_reg.begin());

	au_values_reg.reserve(poses.size());
	while(!au_reg_input_file.eof())
	{
		string reg_line;
		std::getline(au_reg_input_file, reg_line);

		if(reg_line.empty())
			break;

		vector<double> au_val_curr;

		tokenizer<char_separator<char>> tokens_val(reg_line, sep);
		for (const auto& t : tokens_val) {
			double val = stod(t);
			au_val_curr.push_back(val);
		}

		if(au_val_curr.size() > 0)
		{
			au_val_curr.erase(au_val_curr.begin());
			au_val_curr.erase(au_val_curr.begin());
			au_val_curr.erase(au_val_curr.begin());

			au_val_curr.erase(au_val_curr.begin()+13);
			au_val_curr.erase(au_val_curr.begin()+12);
			au_values_reg.push_back(au_val_curr);
		}
	}
	au_names_reg.erase(au_names_reg.begin()+13);
	au_names_reg.erase(au_names_reg.begin()+12);

	// Reading the features
	while(!fp_input_file.eof())
	{
		string fp_line;
		std::getline(fp_input_file, fp_line);

		if(fp_line.empty())
			break;

		stringstream ss(fp_line);
		Mat_<double> fp_curr(68,2, 0.0);
		for(size_t i = 0; i < 138; ++i)
		{
			double val;
			ss >> val;
			
			if(i > 69)
			{
				fp_curr.at<double>(i - 70, 1) = val;
			}
			else if(i > 1)
			{
				fp_curr.at<double>(i - 2, 0) = val;
			}
			
		}
		fps.push_back(fp_curr);
	}

	// skipping the headers
	string placeholder;
	std::getline(timestamp_input_file, placeholder);
	std::getline(timestamp_input_file, placeholder);
		
	// Reading the features
	while(!timestamp_input_file.eof())
	{
		string timestamp_line;
		std::getline(timestamp_input_file, timestamp_line);
		if(timestamp_line.empty())
		{
			break;
		}
		stringstream ss(timestamp_line);
		int frame;
		
		double time_ms = 0;

		double timestamp;

		ss >> frame;
		ss >> timestamp;
		ss >> timestamp;

		// hour
		time_ms += timestamp * 60 * 60 * 1000;

		// minute
		ss >> timestamp;
		time_ms += timestamp * 60 * 1000;

		// second
		ss >> timestamp;
		time_ms += timestamp * 1000;

		ss >> timestamp;
		time_ms += timestamp;

		timestamps.push_back(time_ms);
	}

}

void produce_tracked_image(Mat& io_image, double confidence, const Mat_<double> feat_points, Vec6d pose, 
						   vector<string> au_names_reg, vector<double> au_values_reg, 
						   vector<string> au_names_class, vector<int> au_values_class, 
						   double fx, double fy, double cx, double cy)
{
	double visualisation_boundary = 0.25;
	double detection_certainty = confidence;

	// Only draw if the reliability is reasonable, the value is slightly ad-hoc
	if(detection_certainty > visualisation_boundary)
	{
		Mat_<double> shape_2D = feat_points;
		CLMTracker::Draw(io_image, shape_2D);

		if(detection_certainty > 1)
			detection_certainty = 1;
		if(detection_certainty < 0)
			detection_certainty = 0;

		// A rough heuristic for box around the face width
		int thickness = (int)std::ceil(((double)io_image.cols) / 640.0);
				
		Vec6d pose_estimate_to_draw = pose;

		// Draw it in reddish if uncertain, blueish if certain
		CLMTracker::DrawBox(io_image, pose_estimate_to_draw, Scalar(255.0,0, 0), thickness, fx, fy, cx, cy);

		// Only care about x and y angles as they are out of plane
		double angle_norm = cv::sqrt(pose_estimate_to_draw[3] * pose_estimate_to_draw[3] + 
										pose_estimate_to_draw[4] * pose_estimate_to_draw[4]);

		// Draw the AUs

		//cv::rectangle(io_image, Rect(0,0, 150, io_image.rows), Scalar(255,255,255), 10);

		// TODO need to correct them and rename them, also only the reliable ones

		for (size_t au = 0; au < au_names_reg.size(); ++au)
		{
			char AUs[255];
			if(angle_norm < 0.4)
			{
				sprintf(AUs, "%.1f", au_values_reg[au]);
			}
			else
			{
				sprintf(AUs, "%.1f", 0.0);
			}
			string auSt = au_names_reg[au] + ": ";
			auSt += AUs;
			cv::putText(io_image, auSt, cv::Point(10,30 + au * 30), CV_FONT_HERSHEY_SIMPLEX, 1, CV_RGB(255,255,0), 2);		
		}

		for (size_t au = 0; au < au_names_class.size(); ++au)
		{
			char AUs[255];
			if(angle_norm < 0.4)
			{
				sprintf(AUs, "%d", au_values_class[au]*5);
			}
			else
			{
				sprintf(AUs, "%d", 0);
			}
			string auSt = au_names_class[au] + ": ";
			auSt += AUs;
			cv::putText(io_image, auSt, cv::Point(10,50 + au_names_reg.size() * 30 + au * 30), CV_FONT_HERSHEY_SIMPLEX, 1, CV_RGB(255,255,0), 2);		
		}
	}

}

void process_tracks(vector<vector<double>>& au_vals, vector<double> confidences, vector<Vec6d>& poses)
{

	// First extract the valid AU values and put them in a different format
	vector<vector<double>> aus_valid;
	vector<double> offsets;
	vector<double> maxs;

	for(size_t au = 0; au < au_vals[0].size(); ++au)
	{
		vector<double> au_good;
		for(size_t frame = 0; frame < au_vals.size(); ++frame)
		{
			double confidence = confidences[frame];
			Vec6d pose = poses[frame];
			// Only care about x and y angles as they are out of plane
			double angle_norm = cv::sqrt(pose[3] * pose[3] + 
											pose[4] * pose[4]);

			if(confidence > 0.25 && angle_norm < 0.4)
			{
				au_good.push_back(au_vals[frame][au]);
			}

		}
		std::sort(au_good.begin(), au_good.end());
		offsets.push_back(au_good.at((int)au_good.size()/4));
		maxs.push_back(au_good.at(au_good.size()-100) - offsets[au]);

		aus_valid.push_back(au_good);
	}

	// sort each of the aus
	for(size_t frame = 0; frame < au_vals.size(); ++frame)
	{
		for(size_t au = 0; au < au_vals[frame].size(); ++au)
		{
			double confidence = confidences[frame];
			Vec6d pose = poses[frame];
			// Only care about x and y angles as they are out of plane
			double angle_norm = cv::sqrt(pose[3] * pose[3] + 
											pose[4] * pose[4]);

			if(confidence < 0.25)
			{
				poses[frame] = Vec6d(0.0,0.0,0.0,0.0,0.0,0.0);
			}

			if(confidence > 0.25 && angle_norm < 0.4)
			{
				double scaling = 1;

				if(maxs[au] > 2)
				{
					scaling = 5.0 / maxs[au];
				}

				au_vals[frame][au] = (au_vals[frame][au] - offsets[au]) * scaling;
				
				if(au_vals[frame][au]<0.5)
					au_vals[frame][au] = 0;

				if(au_vals[frame][au]>5)
					au_vals[frame][au] = 5;

			}
			else
			{
				au_vals[frame][au] = 0;
			}
		}
	}
}

int main (int argc, char **argv)
{

	vector<string> arguments = get_arguments(argc, argv);

	// Some initial parameters that can be overriden from command line	
	vector<string> files, pose_input_files, landmark_input_files, au_reg_input_files, au_class_input_files, timestamp_files;
	string video_output;

	// cx and cy aren't necessarilly in the image center, so need to be able to override it (start with unit vals and init them if none specified)
    float fx = 1200, fy = 1200, cx = 0, cy = 0;
	double offset = 0;		
	string sum_doc_out, sum_pat_out;

	get_video_input_output_params(files, timestamp_files, pose_input_files, video_output, landmark_input_files, au_class_input_files, au_reg_input_files, offset, sum_doc_out, sum_pat_out, arguments);

	vector<Mat_<double>> feat_points_p, feat_points_d;
	vector<Vec6d> poses_p, poses_d;
	vector<string> au_names_class_p, au_names_class_d;
	vector<vector<int>> au_values_class_p, au_values_class_d;
	vector<string> au_names_reg_p, au_names_reg_d;
	vector<vector<double>> au_values_reg_p, au_values_reg_d;
	vector<double> confidences_p, confidences_d;
	vector<double> timestamps_p, timestamps_d;

	process_features(landmark_input_files[0], pose_input_files[0], au_class_input_files[0], au_reg_input_files[0], timestamp_files[0],
					  feat_points_p, poses_p, au_names_class_p, au_values_class_p, au_names_reg_p, au_values_reg_p, confidences_p, timestamps_p);

	if(files.size() > 1)
	{
		process_features(landmark_input_files[1], pose_input_files[1], au_class_input_files[1], au_reg_input_files[1], timestamp_files[1],
					  feat_points_d, poses_d, au_names_class_d, au_values_class_d, au_names_reg_d, au_values_reg_d, confidences_d, timestamps_d);
		double offset_start = timestamps_p[0] - timestamps_d[0];
		// need to correct the doctor timestamps
		for(size_t t = 0; t < timestamps_d.size(); ++t)
		{
			timestamps_d[t] = timestamps_d[t] + offset_start - offset;
		}
		cout << offset_start << " " << offset << endl;
	}


	// If cx (optical axis centre) is undefined will use the image size/2 as an estimate
	bool cx_undefined = false;
	if(cx == 0 || cy == 0)
	{
		cx_undefined = true;
	}		
		
	//string current_file;

	// First process one of the videos

	// Do some grabbing
	VideoCapture video_capture_p (files[0]);

	if( !video_capture_p.isOpened() ) FATAL_STREAM( "Failed to open video source" );
	else INFO_STREAM( "Device or file opened");

	// Read a first frame often empty in camera
	Mat captured_image_p;
	video_capture_p >> captured_image_p;

	// Do some grabbing
	VideoCapture video_capture_d;
	Mat captured_image_d;

	if(files.size() > 1)
	{
		video_capture_d.open(files[1]);

		if( !video_capture_d.isOpened() ) FATAL_STREAM( "Failed to open video source" );
		else INFO_STREAM( "Device or file opened");

		// Read a first frame often empty in camera
		video_capture_d >> captured_image_d;
	}

	// If optical centers are not defined just use center of image
	if(cx_undefined)
	{
		cx = captured_image_p.cols / 2.0f;
		cy = captured_image_p.rows / 2.0f;
	}
			

	
	process_tracks(au_values_reg_p, confidences_p, poses_p);
	process_tracks(au_values_reg_d, confidences_d, poses_d);

	// Writing out the features to be used for summary statistics
	// poses, x,y,z, rot_x, rot_y, rot_z, aus_r, aus_c, this needs to be synchronised though for analysis
	std::ofstream part_sum_file;
	part_sum_file.open(sum_pat_out);
	part_sum_file << "success, pose_x, pose_y, pose_z, rot_x, rot_y, rot_z";
	part_sum_file << endl;

	std::ofstream doc_sum_file;
	doc_sum_file.open(sum_doc_out);
	doc_sum_file << "success, pose_x, pose_y, pose_z, rot_x, rot_y, rot_z";

	for(size_t aur = 0; aur < au_names_reg_p.size(); ++aur)
	{
		doc_sum_file << ", " << au_names_reg_p[aur] << "_r";
	}
	for(size_t auc = 0; auc < au_names_class_p.size(); ++auc)
	{
		doc_sum_file << ", " << au_names_class_p[auc] << "_c";
	}
	doc_sum_file << endl;

	int frame_doc = 0;
	for(size_t frame_part = 0; frame_part < confidences_p.size(); ++frame_part)
	{

		// Write out participant bit
		int success = 1;

		if(confidences_p[frame_part] <= 0.25)
		{
			success = 0;
		}
		part_sum_file << success << ", ";

		part_sum_file << poses_p[frame_part][0] << ", " << poses_p[frame_part][1] << ", " << poses_p[frame_part][2] << ", "
			 << poses_p[frame_part][3] << ", " << poses_p[frame_part][4] << ", " << poses_p[frame_part][5];

		// Write the AUs now
		for(size_t aur = 0; aur < au_names_reg_p.size(); ++aur)
		{
			part_sum_file << ", " << au_values_reg_p[frame_part][aur];
		}

		for(size_t auc = 0; auc < au_names_class_p.size(); ++auc)
		{
			part_sum_file << ", " << au_values_class_p[frame_part][auc];
		}
		part_sum_file << endl;

		// Write out doctor bit (need to synchronise it though)
		if(files.size() > 1)
		{
			double time_curr_p = timestamps_p[frame_part];

			double time_curr_d = timestamps_d[frame_doc];

			int frame_count_offset = 0;
			for(int k=1; k < 1000 && frame_doc + k < timestamps_d.size(); ++k)
			{

				if(cv::abs(timestamps_d[frame_doc+k] - time_curr_p) < cv::abs(timestamps_d[frame_doc + k - 1] - time_curr_p) && confidences_d.size() > frame_doc + k)
				{					
					frame_count_offset++;
				}
				else
				{
					break;
				}

			}
			time_curr_d = timestamps_d[frame_doc+frame_count_offset];
			frame_doc = frame_doc + frame_count_offset;

			success = 1;

			if(confidences_d[frame_doc] <= 0.25)
			{
				success = 0;
			}
			doc_sum_file << success << ", ";

			doc_sum_file << poses_d[frame_doc][0] << ", " << poses_d[frame_doc][1] << ", " << poses_d[frame_doc][2] << ", "
				 << poses_d[frame_doc][3] << ", " << poses_d[frame_doc][4] << ", " << poses_d[frame_doc][5];

			// Write the AUs now
			for(size_t aur = 0; aur < au_names_reg_d.size(); ++aur)
			{
				doc_sum_file << ", " << au_values_reg_d[frame_doc][aur];
			}

			for(size_t auc = 0; auc < au_names_class_d.size(); ++auc)
			{
				doc_sum_file << ", " << au_values_class_d[frame_doc][auc];
			}
			doc_sum_file << endl;
		}
	}
	part_sum_file.close();
	doc_sum_file.close();

	int frame_count = 0;
		
	// saving the videos
	VideoWriter writerFace;

	Size writer_size = captured_image_p.size();
	if(files.size() > 1)
	{
		writer_size.width = writer_size.width * 2;
	}

	writerFace = VideoWriter(video_output, CV_FOURCC('D','I','V','X'), 30, writer_size, true);		
		
	INFO_STREAM( "Starting writing");

	int frame_count_d = 0;

	// Sync everything to participant
	while(!captured_image_p.empty())
	{		
			
		double visualisation_boundary = 0.25;
		
		produce_tracked_image(captured_image_p, confidences_p[frame_count], feat_points_p[frame_count], poses_p[frame_count],
			au_names_reg_p, au_values_reg_p[frame_count], au_names_class_p, au_values_class_p[frame_count], fx, fy, cx, cy);

		if(files.size() > 1)
		{
			double time_curr_p = timestamps_p[frame_count];

			double time_curr_d = timestamps_d[frame_count_d];

			int frame_count_offset = 0;
			for(int k=1; k < 1000 && frame_count_d + k < timestamps_d.size(); ++k)
			{

				if(cv::abs(timestamps_d[frame_count_d+k] - time_curr_p) < cv::abs(timestamps_d[frame_count_d + k - 1] - time_curr_p) && confidences_d.size() > frame_count_d + k)
				{					
					frame_count_offset++;
					video_capture_d >> captured_image_d;
				}
				else
				{
					break;
				}

			}
			time_curr_d = timestamps_d[frame_count_d+frame_count_offset];
			frame_count_d = frame_count_d + frame_count_offset;
			//cout << time_curr_d - time_curr_p << endl;

			produce_tracked_image(captured_image_d, confidences_d[frame_count_d], feat_points_d[frame_count_d], poses_d[frame_count_d],
				au_names_reg_d, au_values_reg_d[frame_count_d], au_names_class_d, au_values_class_d[frame_count_d], fx, fy, cx, cy);
			cv::hconcat(captured_image_p, captured_image_d, captured_image_p);
		}

		imshow("tracked", captured_image_p);

		writerFace << captured_image_p;
		
		video_capture_p >> captured_image_p;

		// detect key presses
		char character_press = cv::waitKey(1);
			
		if(character_press=='q')
		{
			return(0);
		}

		// Update the frame count
		frame_count++;

	}
		
	frame_count = 0;

	return 0;
}

