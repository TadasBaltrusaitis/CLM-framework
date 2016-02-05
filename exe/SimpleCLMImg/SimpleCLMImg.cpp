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

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <filesystem.hpp>
#include <filesystem/fstream.hpp>

#include <dlib/image_processing/frontal_face_detector.h>

#include <tbb/tbb.h>

#include <FaceAnalyser.h>
#include <GazeEstimation.h>

using namespace std;
using namespace cv;

vector<string> get_arguments(int argc, char **argv)
{

	vector<string> arguments;

	for(int i = 0; i < argc; ++i)
	{
		arguments.push_back(string(argv[i]));
	}
	return arguments;
}

void convert_to_grayscale(const Mat& in, Mat& out)
{
	if(in.channels() == 3)
	{
		// Make sure it's in a correct format
		if(in.depth() != CV_8U)
		{
			if(in.depth() == CV_16U)
			{
				Mat tmp = in / 256;
				tmp.convertTo(tmp, CV_8U);
				cvtColor(tmp, out, CV_BGR2GRAY);
			}
		}
		else
		{
			cvtColor(in, out, CV_BGR2GRAY);
		}
	}
	else if(in.channels() == 4)
	{
		cvtColor(in, out, CV_BGRA2GRAY);
	}
	else
	{
		if(in.depth() == CV_16U)
		{
			Mat tmp = in / 256;
			out = tmp.clone();
		}
		else if(in.depth() != CV_8U)
		{
			in.convertTo(out, CV_8U);
		}
		else
		{
			out = in.clone();
		}
	}
}

// Useful utility for creating directories for storing the output files
void create_directory_from_file(string output_path)
{

	// Creating the right directory structure

	// First get rid of the file
	auto p = boost::filesystem::path(boost::filesystem::path(output_path).parent_path());

	if (!p.empty() && !boost::filesystem::exists(p))
	{
		bool success = boost::filesystem::create_directories(p);
		if (!success)
		{
			cout << "Failed to create a directory... " << p.string() << endl;
		}
	}
}

// This will only be accurate when camera parameters are accurate, useful for work on 3D data
void write_out_pose_landmarks(const string& outfeatures, const Mat_<double>& shape3D, const Vec6d& pose, const Point3f& gaze0, const Point3f& gaze1)
{
	create_directory_from_file(outfeatures);
	std::ofstream featuresFile;
	featuresFile.open(outfeatures);

	if (featuresFile.is_open())
	{
		int n = shape3D.cols;
		featuresFile << "version: 1" << endl;
		featuresFile << "npoints: " << n << endl;
		featuresFile << "{" << endl;

		for (int i = 0; i < n; ++i)
		{
			// Use matlab format, so + 1
			featuresFile << shape3D.at<double>(i) << " " << shape3D.at<double>(i + n) << " " << shape3D.at<double>(i + 2*n) << endl;
		}
		featuresFile << "}" << endl;

		// Do the pose and eye gaze if present as well
		featuresFile << "pose: eul_x, eul_y, eul_z: " << endl;
		featuresFile << "{" << endl;
		featuresFile << pose[3] << " " << pose[4] << " " << pose[5] << endl;
		featuresFile << "}" << endl;

		// Do the pose and eye gaze if present as well
		featuresFile << "gaze: dir_x_1, dir_y_1, dir_z_1, dir_x_2, dir_y_2, dir_z_2: " << endl;
		featuresFile << "{" << endl;
		featuresFile << gaze0.x << " " << gaze0.y << " " << gaze0.z << " " << gaze1.x << " " << gaze1.y << " " << gaze1.z << endl;
		featuresFile << "}" << endl;
		featuresFile.close();
	}
}

void write_out_landmarks(const string& outfeatures, const CLMTracker::CLM& clm_model)
{
	create_directory_from_file(outfeatures);
	std::ofstream featuresFile;
	featuresFile.open(outfeatures);		

	if(featuresFile.is_open())
	{	
		int n = clm_model.patch_experts.visibilities[0][0].rows;
		featuresFile << "version: 1" << endl;
		featuresFile << "npoints: " << n << endl;
		featuresFile << "{" << endl;
		
		for (int i = 0; i < n; ++ i)
		{
			// Use matlab format, so + 1
			featuresFile << clm_model.detected_landmarks.at<double>(i) + 1 << " " << clm_model.detected_landmarks.at<double>(i+n) + 1 << endl;
		}
		featuresFile << "}" << endl;		

	}
}

void create_display_image(const Mat& orig, Mat& display_image, CLMTracker::CLM& clm_model)
{
	
	// Draw head pose if present and draw eye gaze as well

	// preparing the visualisation image
	display_image = orig.clone();		

	// Creating a display image			
	Mat xs = clm_model.detected_landmarks(Rect(0, 0, 1, clm_model.detected_landmarks.rows/2));
	Mat ys = clm_model.detected_landmarks(Rect(0, clm_model.detected_landmarks.rows/2, 1, clm_model.detected_landmarks.rows/2));
	double min_x, max_x, min_y, max_y;

	cv::minMaxLoc(xs, &min_x, &max_x);
	cv::minMaxLoc(ys, &min_y, &max_y);

	double width = max_x - min_x;
	double height = max_y - min_y;

	int minCropX = max((int)(min_x-width/3.0),0);
	int minCropY = max((int)(min_y-height/3.0),0);

	int widthCrop = min((int)(width*5.0/3.0), display_image.cols - minCropX - 1);
	int heightCrop = min((int)(height*5.0/3.0), display_image.rows - minCropY - 1);

	double scaling = 350.0/widthCrop;
	
	// first crop the image
	display_image = display_image(Rect((int)(minCropX), (int)(minCropY), (int)(widthCrop), (int)(heightCrop)));
		
	// now scale it
	cv::resize(display_image.clone(), display_image, Size(), scaling, scaling);

	// Make the adjustments to points
	xs = (xs - minCropX)*scaling;
	ys = (ys - minCropY)*scaling;

	Mat shape = clm_model.detected_landmarks.clone();

	xs.copyTo(shape(Rect(0, 0, 1, clm_model.detected_landmarks.rows/2)));
	ys.copyTo(shape(Rect(0, clm_model.detected_landmarks.rows/2, 1, clm_model.detected_landmarks.rows/2)));

	// Do the shifting for the hierarchical models as well
	for (size_t part = 0; part < clm_model.hierarchical_models.size(); ++part)
	{
		Mat xs = clm_model.hierarchical_models[part].detected_landmarks(Rect(0, 0, 1, clm_model.hierarchical_models[part].detected_landmarks.rows / 2));
		Mat ys = clm_model.hierarchical_models[part].detected_landmarks(Rect(0, clm_model.hierarchical_models[part].detected_landmarks.rows / 2, 1, clm_model.hierarchical_models[part].detected_landmarks.rows / 2));

		xs = (xs - minCropX)*scaling;
		ys = (ys - minCropY)*scaling;

		Mat shape = clm_model.hierarchical_models[part].detected_landmarks.clone();

		xs.copyTo(shape(Rect(0, 0, 1, clm_model.hierarchical_models[part].detected_landmarks.rows / 2)));
		ys.copyTo(shape(Rect(0, clm_model.hierarchical_models[part].detected_landmarks.rows / 2, 1, clm_model.hierarchical_models[part].detected_landmarks.rows / 2)));

	}

	CLMTracker::Draw(display_image, clm_model);
						
}

int main (int argc, char **argv)
{
		
	//Convert arguments to more convenient vector form
	vector<string> arguments = get_arguments(argc, argv);

	// Some initial parameters that can be overriden from command line
	vector<string> files, depth_files, output_images, output_landmark_locations, output_pose_locations;

	// Bounding boxes for a face in each image (optional)
	vector<Rect_<double> > bounding_boxes;
	
	CLMTracker::get_image_input_output_params(files, depth_files, output_landmark_locations, output_pose_locations, output_images, bounding_boxes, arguments);
	CLMTracker::CLMParameters clm_parameters(arguments);	
	// No need to validate detections, as we're not doing tracking
	clm_parameters.validate_detections = false;

	// Grab camera parameters if provided (only used for pose and eye gaze and are quite important for accurate estimates)
	float fx = 0, fy = 0, cx = 0, cy = 0;
	int device = -1;
	CLMTracker::get_camera_params(device, fx, fy, cx, cy, arguments);

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
	cout << "Loading the model" << endl;
	CLMTracker::CLM clm_model(clm_parameters.model_location);
	cout << "Model loaded" << endl;
	
	CascadeClassifier classifier(clm_parameters.face_detector_location);	
	dlib::frontal_face_detector face_detector_hog = dlib::get_frontal_face_detector();

	bool visualise = !clm_parameters.quiet_mode;

	// Do some image loading
	for(size_t i = 0; i < files.size(); i++)
	{
		string file = files.at(i);

		// Loading image
		Mat read_image = imread(file, -1);

		// Loading depth file if exists (optional)
		Mat_<float> depth_image;

		if(depth_files.size() > 0)
		{
			string dFile = depth_files.at(i);
			Mat dTemp = imread(dFile, -1);
			dTemp.convertTo(depth_image, CV_32F);
		}

		// Making sure the image is in uchar grayscale
		Mat_<uchar> grayscale_image;		
		convert_to_grayscale(read_image, grayscale_image);
		

		// If optical centers are not defined just use center of image
		if (cx_undefined)
		{
			cx = grayscale_image.cols / 2.0f;
			cy = grayscale_image.rows / 2.0f;
		}
		// Use a rough guess-timate of focal length
		if (fx_undefined)
		{
			fx = 500 * (grayscale_image.cols / 640.0);
			fy = 500 * (grayscale_image.rows / 480.0);

			fx = (fx + fy) / 2.0;
			fy = fx;
		}


		// if no pose defined we just use a face detector
		if(bounding_boxes.empty())
		{
			
			// Detect faces in an image
			vector<Rect_<double> > face_detections;

			if(clm_parameters.curr_face_detector == CLMTracker::CLMParameters::HOG_SVM_DETECTOR)
			{
				vector<double> confidences;
				CLMTracker::DetectFacesHOG(face_detections, grayscale_image, face_detector_hog, confidences);
			}
			else
			{
				CLMTracker::DetectFaces(face_detections, grayscale_image, classifier);
			}

			// Detect landmarks around detected faces
			int face_det = 0;
			// perform landmark detection for every face detected
			for(size_t face=0; face < face_detections.size(); ++face)
			{
				// if there are multiple detections go through them
				bool success = CLMTracker::DetectLandmarksInImage(grayscale_image, depth_image, face_detections[face], clm_model, clm_parameters);

				// Estimate head pose and eye gaze				
				Vec6d headPose = CLMTracker::GetCorrectedPoseWorld(clm_model, fx, fy, cx, cy);

				// Gaze tracking, absolute gaze direction
				Point3f gazeDirection0(0, 0, -1);
				Point3f gazeDirection1(0, 0, -1);

				// Gaze with respect to head rather than camera (for example if eyes are rolled up and the head is tilted or turned this will be stable)
				Point3f gazeDirection0_head(0, 0, -1);
				Point3f gazeDirection1_head(0, 0, -1);

				if (success && clm_parameters.track_gaze)
				{
					FaceAnalysis::EstimateGaze(clm_model, gazeDirection0, gazeDirection0_head, fx, fy, cx, cy, true);
					FaceAnalysis::EstimateGaze(clm_model, gazeDirection1, gazeDirection1_head, fx, fy, cx, cy, false);

				}

				// Writing out the detected landmarks (in an OS independent manner)
				if(!output_landmark_locations.empty())
				{
					char name[100];
					// append detection number (in case multiple faces are detected)
					sprintf(name, "_det_%d", face_det);

					// Construct the output filename
					boost::filesystem::path slash("/");
					std::string preferredSlash = slash.make_preferred().string();

					boost::filesystem::path out_feat_path(output_landmark_locations.at(i));
					boost::filesystem::path dir = out_feat_path.parent_path();
					boost::filesystem::path fname = out_feat_path.filename().replace_extension("");
					boost::filesystem::path ext = out_feat_path.extension();
					string outfeatures = dir.string() + preferredSlash + fname.string() + string(name) + ext.string();
					write_out_landmarks(outfeatures, clm_model);
				}

				if (!output_pose_locations.empty())
				{
					char name[100];
					// append detection number (in case multiple faces are detected)
					sprintf(name, "_det_%d", face_det);

					// Construct the output filename
					boost::filesystem::path slash("/");
					std::string preferredSlash = slash.make_preferred().string();

					boost::filesystem::path out_pose_path(output_pose_locations.at(i));
					boost::filesystem::path dir = out_pose_path.parent_path();
					boost::filesystem::path fname = out_pose_path.filename().replace_extension("");
					boost::filesystem::path ext = out_pose_path.extension();
					string outfeatures = dir.string() + preferredSlash + fname.string() + string(name) + ext.string();
					write_out_pose_landmarks(outfeatures, clm_model.GetShape(fx, fy, cx, cy), headPose, gazeDirection0, gazeDirection1);

				}

				if (clm_parameters.track_gaze)
				{
					Vec6d pose_estimate_to_draw = CLMTracker::GetCorrectedPoseWorld(clm_model, fx, fy, cx, cy);

					// Draw it in reddish if uncertain, blueish if certain
					CLMTracker::DrawBox(read_image, pose_estimate_to_draw, Scalar(255.0, 0, 0), 3, fx, fy, cx, cy);
					FaceAnalysis::DrawGaze(read_image, clm_model, gazeDirection0, gazeDirection1, fx, fy, cx, cy);
				}

				// displaying detected landmarks
				Mat display_image;
				create_display_image(read_image, display_image, clm_model);

				if(visualise && success)
				{
					imshow("colour", display_image);
					cv::waitKey(1);
				}

				// Saving the display images (in an OS independent manner)
				if(!output_images.empty() && success)
				{
					string outimage = output_images.at(i);
					if(!outimage.empty())
					{
						char name[100];
						sprintf(name, "_det_%d", face_det);

						boost::filesystem::path slash("/");
						std::string preferredSlash = slash.make_preferred().string();

						// append detection number
						boost::filesystem::path out_feat_path(outimage);
						boost::filesystem::path dir = out_feat_path.parent_path();
						boost::filesystem::path fname = out_feat_path.filename().replace_extension("");
						boost::filesystem::path ext = out_feat_path.extension();
						outimage = dir.string() + preferredSlash + fname.string() + string(name) + ext.string();
						create_directory_from_file(outimage);
						imwrite(outimage, display_image);	
						
					}

				}

				if(success)
				{
					face_det++;
				}

			}
		}
		else
		{
			// Have provided bounding boxes
			CLMTracker::DetectLandmarksInImage(grayscale_image, bounding_boxes[i], clm_model, clm_parameters);

			// Estimate head pose and eye gaze				
			Vec6d headPose = CLMTracker::GetCorrectedPoseWorld(clm_model, fx, fy, cx, cy);

			// Gaze tracking, absolute gaze direction
			Point3f gazeDirection0(0, 0, -1);
			Point3f gazeDirection1(0, 0, -1);

			// Gaze with respect to head rather than camera (for example if eyes are rolled up and the head is tilted or turned this will be stable)
			Point3f gazeDirection0_head(0,0, -1);
			Point3f gazeDirection1_head(0, 0, -1);

			if (clm_parameters.track_gaze)
			{
				FaceAnalysis::EstimateGaze(clm_model, gazeDirection0, gazeDirection0_head, fx, fy, cx, cy, true);
				FaceAnalysis::EstimateGaze(clm_model, gazeDirection1, gazeDirection1_head, fx, fy, cx, cy, false);
			}

			// Writing out the detected landmarks
			if(!output_landmark_locations.empty())
			{
				string outfeatures = output_landmark_locations.at(i);
				write_out_landmarks(outfeatures, clm_model);
			}

			// Writing out the detected landmarks
			if (!output_pose_locations.empty())
			{
				string outfeatures = output_pose_locations.at(i);
				write_out_pose_landmarks(outfeatures, clm_model.GetShape(fx, fy, cx, cy), headPose, gazeDirection0, gazeDirection1);
			}

			// displaying detected stuff
			Mat display_image;

			if (clm_parameters.track_gaze)
			{
				Vec6d pose_estimate_to_draw = CLMTracker::GetCorrectedPoseWorld(clm_model, fx, fy, cx, cy);

				// Draw it in reddish if uncertain, blueish if certain
				CLMTracker::DrawBox(read_image, pose_estimate_to_draw, Scalar(255.0, 0, 0), 3, fx, fy, cx, cy);
				FaceAnalysis::DrawGaze(read_image, clm_model, gazeDirection0, gazeDirection1, fx, fy, cx, cy);
			}

			create_display_image(read_image, display_image, clm_model);

			if(visualise)
			{
				imshow("colour", display_image);
				cv::waitKey(1);
			}

			if(!output_images.empty())
			{
				string outimage = output_images.at(i);
				if(!outimage.empty())
				{
					create_directory_from_file(outimage);
					imwrite(outimage, display_image);	
				}
			}
		}				

	}
	
	return 0;
}

