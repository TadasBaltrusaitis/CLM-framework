///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2014, University of Southern California and University of Cambridge,
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
#include "stdafx.h"

#include <CLMTracker.h>

using namespace CLMTracker;
using namespace cv;

// Getting a head pose estimate from the currently detected landmarks (rotation with respect to point camera)
// The format returned is [Tx, Ty, Tz, Eul_x, Eul_y, Eul_z]
Vec6d CLMTracker::GetPoseCamera(const CLM& clm_model, double fx, double fy, double cx, double cy)
{
	if(!clm_model.detected_landmarks.empty() && clm_model.params_global[0] != 0)
	{
		double Z = fx / clm_model.params_global[0];
	
		double X = ((clm_model.params_global[4] - cx) * (1.0/fx)) * Z;
		double Y = ((clm_model.params_global[5] - cy) * (1.0/fy)) * Z;
	
		return Vec6d(X, Y, Z, clm_model.params_global[1], clm_model.params_global[2], clm_model.params_global[3]);
	}
	else
	{
		return Vec6d(0,0,0,0,0,0);
	}
}

// Getting a head pose estimate from the currently detected landmarks (rotation in world coordinates)
// The format returned is [Tx, Ty, Tz, Eul_x, Eul_y, Eul_z]
Vec6d CLMTracker::GetPoseWorld(const CLM& clm_model, double fx, double fy, double cx, double cy)
{
	if(!clm_model.detected_landmarks.empty() && clm_model.params_global[0] != 0)
	{
		double Z = fx / clm_model.params_global[0];
	
		double X = ((clm_model.params_global[4] - cx) * (1.0/fx)) * Z;
		double Y = ((clm_model.params_global[5] - cy) * (1.0/fy)) * Z;
	
		// Here we correct for the camera orientation, for this need to determine the angle the camera makes with the head pose
		double z_x = cv::sqrt(X * X + Z * Z);
		double eul_x = atan2(Y, z_x);

		double z_y = cv::sqrt(Y * Y + Z * Z);
		double eul_y = -atan2(X, z_y);

		Matx33d camera_rotation = CLMTracker::Euler2RotationMatrix(Vec3d(eul_x, eul_y, 0));		
		Matx33d head_rotation = CLMTracker::AxisAngle2RotationMatrix(Vec3d(clm_model.params_global[1], clm_model.params_global[2], clm_model.params_global[3]));

		Matx33d corrected_rotation = camera_rotation.t() * head_rotation;

		Vec3d euler_corrected = CLMTracker::RotationMatrix2Euler(corrected_rotation);

		return Vec6d(X, Y, Z, euler_corrected[0], euler_corrected[1], euler_corrected[2]);
	}
	else
	{
		return Vec6d(0,0,0,0,0,0);
	}
}

// Getting a head pose estimate from the currently detected landmarks, with appropriate correction due to orthographic camera issue
// This is because rotation estimate under orthographic assumption is only correct close to the centre of the image
// This method returns a corrected pose estimate with respect to world coordinates (Experimental)
// The format returned is [Tx, Ty, Tz, Eul_x, Eul_y, Eul_z]
Vec6d CLMTracker::GetCorrectedPoseWorld(const CLM& clm_model, double fx, double fy, double cx, double cy)
{
	if(!clm_model.detected_landmarks.empty() && clm_model.params_global[0] != 0)
	{
		// This is used as an initial estimate for the iterative PnP algorithm
		double Z = fx / clm_model.params_global[0];
	
		double X = ((clm_model.params_global[4] - cx) * (1.0/fx)) * Z;
		double Y = ((clm_model.params_global[5] - cy) * (1.0/fy)) * Z;
 
		// Correction for orientation

		// 2D points
		Mat_<double> landmarks_2D = clm_model.detected_landmarks;

		landmarks_2D = landmarks_2D.reshape(1, 2).t();

		// 3D points
		Mat_<double> landmarks_3D;
		clm_model.pdm.CalcShape3D(landmarks_3D, clm_model.params_local);

		landmarks_3D = landmarks_3D.reshape(1, 3).t();

		// Solving the PNP model

		// The camera matrix
		Matx33d camera_matrix(fx, 0, cx, 0, fy, cy, 0, 0, 1);
		
		Vec3d vec_trans(X, Y, Z);
		Vec3d vec_rot(clm_model.params_global[1], clm_model.params_global[2], clm_model.params_global[3]);
		
		cv::solvePnP(landmarks_3D, landmarks_2D, camera_matrix, Mat(), vec_rot, vec_trans, true);

		Vec3d euler = CLMTracker::AxisAngle2Euler(vec_rot);
		
		return Vec6d(vec_trans[0], vec_trans[1], vec_trans[2], vec_rot[0], vec_rot[1], vec_rot[2]);
	}
	else
	{
		return Vec6d(0,0,0,0,0,0);
	}
}

// Getting a head pose estimate from the currently detected landmarks, with appropriate correction due to perspective projection
// This method returns a corrected pose estimate with respect to a point camera (NOTE not the world coordinates) (Experimental)
// The format returned is [Tx, Ty, Tz, Eul_x, Eul_y, Eul_z]
Vec6d CLMTracker::GetCorrectedPoseCamera(const CLM& clm_model, double fx, double fy, double cx, double cy)
{
	if(!clm_model.detected_landmarks.empty() && clm_model.params_global[0] != 0)
	{

		double Z = fx / clm_model.params_global[0];
	
		double X = ((clm_model.params_global[4] - cx) * (1.0/fx)) * Z;
		double Y = ((clm_model.params_global[5] - cy) * (1.0/fy)) * Z;
	
		// Correction for orientation

		// 3D points
		Mat_<double> landmarks_3D;
		clm_model.pdm.CalcShape3D(landmarks_3D, clm_model.params_local);

		landmarks_3D = landmarks_3D.reshape(1, 3).t();

		// 2D points
		Mat_<double> landmarks_2D = clm_model.detected_landmarks;
				
		landmarks_2D = landmarks_2D.reshape(1, 2).t();

		// Solving the PNP model

		// The camera matrix
		Matx33d camera_matrix(fx, 0, cx, 0, fy, cy, 0, 0, 1);
		
		Vec3d vec_trans(X, Y, Z);
		Vec3d vec_rot(clm_model.params_global[1], clm_model.params_global[2], clm_model.params_global[3]);
		
		cv::solvePnP(landmarks_3D, landmarks_2D, camera_matrix, Mat(), vec_rot, vec_trans, true);

		// Here we correct for the camera orientation, for this need to determine the angle the camera makes with the head pose
		double z_x = cv::sqrt(vec_trans[0] * vec_trans[0] + vec_trans[2] * vec_trans[2]);
		double eul_x = atan2(vec_trans[1], z_x);

		double z_y = cv::sqrt(vec_trans[1] * vec_trans[1] + vec_trans[2] * vec_trans[2]);
		double eul_y = -atan2(vec_trans[0], z_y);

		Matx33d camera_rotation = CLMTracker::Euler2RotationMatrix(Vec3d(eul_x, eul_y, 0));		
		Matx33d head_rotation = CLMTracker::AxisAngle2RotationMatrix(vec_rot);

		Matx33d corrected_rotation = camera_rotation * head_rotation;

		Vec3d euler_corrected = CLMTracker::RotationMatrix2Euler(corrected_rotation);
		
		return Vec6d(vec_trans[0], vec_trans[1], vec_trans[2], euler_corrected[0], euler_corrected[1], euler_corrected[2]);
	}
	else
	{
		return Vec6d(0,0,0,0,0,0);
	}
}

// If landmark detection in video succeeded create a template for use in simple tracking
void UpdateTemplate(const Mat_<uchar> &grayscale_image, CLM& clm_model)
{
	Rect bounding_box;
	clm_model.pdm.CalcBoundingBox(bounding_box, clm_model.params_global, clm_model.params_local);
	// Make sure the box is not out of bounds
	bounding_box = bounding_box & Rect(0, 0, grayscale_image.cols, grayscale_image.rows);

	clm_model.face_template = grayscale_image(bounding_box).clone();
}

// This method uses basic template matching in order to allow for better tracking of fast moving faces
void CorrectGlobalParametersVideo(const Mat_<uchar> &grayscale_image, CLM& clm_model, const CLMParameters& params)
{
	Rect init_box;
	clm_model.pdm.CalcBoundingBox(init_box, clm_model.params_global, clm_model.params_local);

	Rect roi(init_box.x - init_box.width/2, init_box.y - init_box.height/2, init_box.width * 2, init_box.height * 2);
	roi = roi & Rect(0, 0, grayscale_image.cols, grayscale_image.rows);			

	int off_x = roi.x;
	int off_y = roi.y;

	double scaling = params.face_template_scale / clm_model.params_global[0];
	Mat_<uchar> image;
	if(scaling < 1)
	{
		cv::resize(clm_model.face_template, clm_model.face_template, Size(), scaling, scaling);
		cv::resize(grayscale_image(roi), image, Size(), scaling, scaling);
	}
	else
	{
		scaling = 1;
		image = grayscale_image(roi).clone();
	}
		
	// Resizing the template			
	Mat corr_out;
	cv::matchTemplate(image, clm_model.face_template, corr_out, CV_TM_CCOEFF_NORMED);

	// Actually matching it
	//double min, max;
	int max_loc[2];

	cv::minMaxIdx(corr_out, NULL, NULL, NULL, max_loc);

	Rect_<double> out_bbox(max_loc[1]/scaling + off_x, max_loc[0]/scaling + off_y, clm_model.face_template.rows / scaling, clm_model.face_template.cols / scaling);

	double shift_x = out_bbox.x - (double)init_box.x;
	double shift_y = out_bbox.y - (double)init_box.y;
			
	clm_model.params_global[4] = clm_model.params_global[4] + shift_x;
	clm_model.params_global[5] = clm_model.params_global[5] + shift_y;
	
}

bool CLMTracker::DetectLandmarksInVideo(const Mat_<uchar> &grayscale_image, const Mat_<float> &depth_image, CLM& clm_model, CLMParameters& params)
{
	// First need to decide if the landmarks should be "detected" or "tracked"
	// Detected means running face detection and a larger search area, tracked means initialising from previous step
	// and using a smaller search area

	// Indicating that this is a first detection in video sequence or after restart
	bool initial_detection = !clm_model.tracking_initialised;

	// Only do it if there was a face detection at all
	if(clm_model.tracking_initialised)
	{

		// The area of interest search size will depend if the previous track was successful
		if(!clm_model.detection_success)
		{
			params.window_sizes_current = params.window_sizes_init;
		}
		else
		{
			params.window_sizes_current = params.window_sizes_small;
		}

		// Before the expensive landmark detection step apply a quick template tracking approach
		if(params.use_face_template && !clm_model.face_template.empty() && clm_model.detection_success)
		{
			CorrectGlobalParametersVideo(grayscale_image, clm_model, params);
		}

		bool track_success = clm_model.DetectLandmarks(grayscale_image, depth_image, params);
		if(!track_success)
		{
			// Make a record that tracking failed
			clm_model.failures_in_a_row++;
		}
		else
		{
			// indicate that tracking is a success
			clm_model.failures_in_a_row = -1;			
			UpdateTemplate(grayscale_image, clm_model);
		}
	}

	// This is used for both detection (if it the tracking has not been initialised yet) or if the tracking failed (however we do this every n frames, for speed)
	// This also has the effect of an attempt to reinitialise just after the tracking has failed, which is useful during large motions
	if((!clm_model.tracking_initialised && (clm_model.failures_in_a_row + 1) % (params.reinit_video_every * 6) == 0) 
		|| (clm_model.tracking_initialised && !clm_model.detection_success && params.reinit_video_every > 0 && clm_model.failures_in_a_row % params.reinit_video_every == 0))
	{

		Rect_<double> bounding_box;

		// If the face detector has not been initialised read it in
		if(clm_model.face_detector_HAAR.empty())
		{
			clm_model.face_detector_HAAR.load(params.face_detector_location);
			clm_model.face_detector_location = params.face_detector_location;
		}

		Point preference_det(-1, -1);
		if(clm_model.preference_det.x != -1 && clm_model.preference_det.y != -1)
		{
			preference_det.x = clm_model.preference_det.x * grayscale_image.cols;
			preference_det.y = clm_model.preference_det.y * grayscale_image.rows;
			clm_model.preference_det = Point(-1, -1);
		}

		bool face_detection_success;
		if(params.curr_face_detector == CLMParameters::HOG_SVM_DETECTOR)
		{
			double confidence;
			face_detection_success = CLMTracker::DetectSingleFaceHOG(bounding_box, grayscale_image, clm_model.face_detector_HOG, confidence, preference_det);
		}
		else if(params.curr_face_detector == CLMParameters::HAAR_DETECTOR)
		{
			face_detection_success = CLMTracker::DetectSingleFace(bounding_box, grayscale_image, clm_model.face_detector_HAAR, preference_det);
		}

		// Attempt to detect landmarks using the detected face (if unseccessful the detection will be ignored)
		if(face_detection_success)
		{
			// Indicate that tracking has started as a face was detected
			clm_model.tracking_initialised = true;
						
			// Keep track of old model values so that they can be restored if redetection fails
			Vec6d params_global_init = clm_model.params_global;
			Mat_<double> params_local_init = clm_model.params_local.clone();
			double likelihood_init = clm_model.model_likelihood;
			Mat_<double> detected_landmarks_init = clm_model.detected_landmarks.clone();
			Mat_<double> landmark_likelihoods_init = clm_model.landmark_likelihoods.clone();

			// Use the detected bounding box and empty local parameters
			clm_model.params_local.setTo(0);
			clm_model.pdm.CalcParams(clm_model.params_global, bounding_box, clm_model.params_local);		

			// Make sure the search size is large
			params.window_sizes_current = params.window_sizes_init;

			// Do the actual landmark detection (and keep it only if successful)
			bool landmark_detection_success = clm_model.DetectLandmarks(grayscale_image, depth_image, params);

			// If landmark reinitialisation unsucessful continue from previous estimates
			// if it's initial detection however, do not care if it was successful as the validator might be wrong, so continue trackig
			// regardless
			if(!initial_detection && !landmark_detection_success)
			{

				// Restore previous estimates
				clm_model.params_global = params_global_init;
				clm_model.params_local = params_local_init.clone();
				clm_model.pdm.CalcShape2D(clm_model.detected_landmarks, clm_model.params_local, clm_model.params_global);
				clm_model.model_likelihood = likelihood_init;
				clm_model.detected_landmarks = detected_landmarks_init.clone();
				clm_model.landmark_likelihoods = landmark_likelihoods_init.clone();

				return false;
			}
			else
			{
				clm_model.failures_in_a_row = -1;				
				UpdateTemplate(grayscale_image, clm_model);
				return true;
			}
		}
	}

	// if the model has not been initialised yet class it as a failure
	if(!clm_model.tracking_initialised)
	{
		clm_model.failures_in_a_row++;
	}

	// un-initialise the tracking
	if(	clm_model.failures_in_a_row > 100)
	{
		clm_model.tracking_initialised = false;
	}

	return clm_model.detection_success;
	
}

bool CLMTracker::DetectLandmarksInVideo(const Mat_<uchar> &grayscale_image, const Mat_<float> &depth_image, const Rect_<double> bounding_box, CLM& clm_model, CLMParameters& params)
{
	if(bounding_box.width > 0)
	{
		// calculate the local and global parameters from the generated 2D shape (mapping from the 2D to 3D because camera params are unknown)
		clm_model.params_local.setTo(0);
		clm_model.pdm.CalcParams(clm_model.params_global, bounding_box, clm_model.params_local);		

		// indicate that face was detected so initialisation is not necessary
		clm_model.tracking_initialised = true;
	}

	return DetectLandmarksInVideo(grayscale_image, depth_image, clm_model, params);

}

bool CLMTracker::DetectLandmarksInVideo(const Mat_<uchar> &grayscale_image, CLM& clm_model, CLMParameters& params)
{
	return DetectLandmarksInVideo(grayscale_image, Mat_<float>(), clm_model, params);
}

bool CLMTracker::DetectLandmarksInVideo(const Mat_<uchar> &grayscale_image, const Rect_<double> bounding_box, CLM& clm_model, CLMParameters& params)
{
	return DetectLandmarksInVideo(grayscale_image, Mat_<float>(), clm_model, params);
}

//================================================================================================================
// Landmark detection in image, need to provide an image and optionally CLM model together with parameters (default values work well)
// Optionally can provide a bounding box in which detection is performed (this is useful if multiple faces are to be detected in images)
//================================================================================================================

// This is the one where the actual work gets done, other DetectLandmarksInImage calls lead to this one
bool CLMTracker::DetectLandmarksInImage(const Mat_<uchar> &grayscale_image, const Mat_<float> depth_image, const Rect_<double> bounding_box, CLM& clm_model, CLMParameters& params)
{

	// Can have multiple hypotheses
	vector<Vec3d> rotation_hypotheses;

	if(params.multi_view)
	{
		// Try out different orientation initialisations
		// It is possible to add other orientation hypotheses easilly by just pushing to this vector
		rotation_hypotheses.push_back(Vec3d(0,0,0));
		rotation_hypotheses.push_back(Vec3d(0,0.5236,0));
		rotation_hypotheses.push_back(Vec3d(0,-0.5236,0));
		rotation_hypotheses.push_back(Vec3d(0.5236,0,0));
		rotation_hypotheses.push_back(Vec3d(-0.5236,0,0));
	}
	else
	{
		// Assume the face is close to frontal
		rotation_hypotheses.push_back(Vec3d(0,0,0));
	}
	
	// Use the initialisation size for the landmark detection
	params.window_sizes_current = params.window_sizes_init;
	
	// Store the current best estimate
	double best_likelihood;
	Vec6d best_global_parameters;
	Mat_<double> best_local_parameters;
	Mat_<double> best_detected_landmarks;
	Mat_<double> best_landmark_likelihoods;
	bool best_success;

	// The hierarchical model parameters
	vector<double> best_likelihood_h(clm_model.hierarchical_models.size());
	vector<Vec6d> best_global_parameters_h(clm_model.hierarchical_models.size());
	vector<Mat_<double>> best_local_parameters_h(clm_model.hierarchical_models.size());
	vector<Mat_<double>> best_detected_landmarks_h(clm_model.hierarchical_models.size());
	vector<Mat_<double>> best_landmark_likelihoods_h(clm_model.hierarchical_models.size());

	for(size_t hypothesis = 0; hypothesis < rotation_hypotheses.size(); ++hypothesis)
	{
		// Reset the potentially set clm_model parameters
		clm_model.params_local.setTo(0.0);

		for (size_t part = 0; part < clm_model.hierarchical_models.size(); ++part)
		{
			clm_model.hierarchical_models[part].params_local.setTo(0.0);
		}

		// calculate the local and global parameters from the generated 2D shape (mapping from the 2D to 3D because camera params are unknown)
		clm_model.pdm.CalcParams(clm_model.params_global, bounding_box, clm_model.params_local, rotation_hypotheses[hypothesis]);
	
		bool success = clm_model.DetectLandmarks(grayscale_image, depth_image, params);	

		if(hypothesis == 0 || best_likelihood < clm_model.model_likelihood)
		{
			best_likelihood = clm_model.model_likelihood;
			best_global_parameters = clm_model.params_global;
			best_local_parameters = clm_model.params_local.clone();
			best_detected_landmarks = clm_model.detected_landmarks.clone();
			best_landmark_likelihoods = clm_model.landmark_likelihoods.clone();
			best_success = success;
		}

		for (size_t part = 0; part < clm_model.hierarchical_models.size(); ++part)
		{
			if (hypothesis == 0 || best_likelihood < clm_model.hierarchical_models[part].model_likelihood)
			{
				best_likelihood_h[part] = clm_model.hierarchical_models[part].model_likelihood;
				best_global_parameters_h[part] = clm_model.hierarchical_models[part].params_global;
				best_local_parameters_h[part] = clm_model.hierarchical_models[part].params_local.clone();
				best_detected_landmarks_h[part] = clm_model.hierarchical_models[part].detected_landmarks.clone();
				best_landmark_likelihoods_h[part] = clm_model.hierarchical_models[part].landmark_likelihoods.clone();
			}
		}

	}

	// Store the best estimates in the clm_model
	clm_model.model_likelihood = best_likelihood;
	clm_model.params_global = best_global_parameters;
	clm_model.params_local = best_local_parameters.clone();
	clm_model.detected_landmarks = best_detected_landmarks.clone();
	clm_model.detection_success = best_success;
	clm_model.landmark_likelihoods = best_landmark_likelihoods.clone();

	for (size_t part = 0; part < clm_model.hierarchical_models.size(); ++part)
	{
		clm_model.hierarchical_models[part].params_global = best_global_parameters_h[part];
		clm_model.hierarchical_models[part].params_local = best_local_parameters_h[part].clone();
		clm_model.hierarchical_models[part].detected_landmarks = best_detected_landmarks_h[part].clone();
		clm_model.hierarchical_models[part].landmark_likelihoods = best_landmark_likelihoods_h[part].clone();
	}

	return best_success;
}

bool CLMTracker::DetectLandmarksInImage(const Mat_<uchar> &grayscale_image, const Mat_<float> depth_image, CLM& clm_model, CLMParameters& params)
{

	Rect_<double> bounding_box;

	// If the face detector has not been initialised read it in
	if(clm_model.face_detector_HAAR.empty())
	{
		clm_model.face_detector_HAAR.load(params.face_detector_location);
		clm_model.face_detector_location = params.face_detector_location;
	}
		
	// Detect the face first
	if(params.curr_face_detector == CLMParameters::HOG_SVM_DETECTOR)
	{
		double confidence;
		CLMTracker::DetectSingleFaceHOG(bounding_box, grayscale_image, clm_model.face_detector_HOG, confidence);
	}
	else if(params.curr_face_detector == CLMParameters::HAAR_DETECTOR)
	{
		CLMTracker::DetectSingleFace(bounding_box, grayscale_image, clm_model.face_detector_HAAR);
	}

	if(bounding_box.width == 0)
	{
		return false;
	}
	else
	{
		return DetectLandmarksInImage(grayscale_image, depth_image, bounding_box, clm_model, params);
	}
}

// Versions not using depth images
bool CLMTracker::DetectLandmarksInImage(const Mat_<uchar> &grayscale_image, const Rect_<double> bounding_box, CLM& clm_model, CLMParameters& params)
{
	return DetectLandmarksInImage(grayscale_image, Mat_<float>(), bounding_box, clm_model, params);
}

bool CLMTracker::DetectLandmarksInImage(const Mat_<uchar> &grayscale_image, CLM& clm_model, CLMParameters& params)
{
	return DetectLandmarksInImage(grayscale_image, Mat_<float>(), clm_model, params);
}

