// Copyright (C) 2015, University of Cambridge,
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
//       Tadas Baltrusaitis, Marwa Mahmoud, and Peter Robinson.
//		 Cross-dataset learning and person-specific normalisation for automatic Action Unit detection
//       Facial Expression Recognition and Analysis Challenge 2015,
//       IEEE International Conference on Automatic Face and Gesture Recognition, 2015
//
///////////////////////////////////////////////////////////////////////////////

#include "FaceAnalyser.h"

#include "Face_utils.h"

#include "CLM_core.h"

#include <stdio.h>
#include <iostream>

#include <string>

#include <filesystem.hpp>
#include <filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace FaceAnalysis;

using namespace std;

// Constructor from a model file (or a default one if not provided
FaceAnalyser::FaceAnalyser(vector<Vec3d> orientation_bins, double scale, int width, int height, std::string au_location, std::string tri_location)
{
	this->ReadAU(au_location);
		
	align_scale = scale;	
	align_width = width;
	align_height = height;

	// Initialise the histograms that will represent bins from 0 - 1 (as HoG values are only stored as those)
	// Set the number of bins for the histograms
	num_bins_hog = 600;
	max_val_hog = 1;
	min_val_hog = 0;

	// The geometry histogram ranges from -3 to 3 TODO change this as we don't use scaling anymore?
	num_bins_geom = 10000;
	max_val_geom = 60;
	min_val_geom = -60;
		
	// Keep track for how many frames have been tracked so far
	frames_tracking = 0;
	
	if(orientation_bins.empty())
	{
		// Just using frontal currently
		head_orientations.push_back(Vec3d(0,0,0));
	}
	else
	{
		head_orientations = orientation_bins;
	}
	hog_hist_sum.resize(head_orientations.size());
	face_image_hist_sum.resize(head_orientations.size());
	hog_desc_hist.resize(head_orientations.size());
	geom_hist_sum = 0;
	face_image_hist.resize(head_orientations.size());

	au_prediction_correction_count.resize(head_orientations.size(), 0);
	au_prediction_correction_histogram.resize(head_orientations.size());
	dyn_scaling.resize(head_orientations.size());

	// The triangulation used for masking out the non-face parts of aligned image
	std::ifstream triangulation_file(tri_location);	
	CLMTracker::ReadMat(triangulation_file, triangulation);

}

Mat_<int> FaceAnalyser::GetTriangulation()
{
	return triangulation.clone();
}

void FaceAnalyser::GetLatestHOG(Mat_<double>& hog_descriptor, int& num_rows, int& num_cols)
{
	hog_descriptor = this->hog_desc_frame.clone();

	if(!hog_desc_frame.empty())
	{
		num_rows = this->num_hog_rows;
		num_cols = this->num_hog_cols;
	}
	else
	{
		num_rows = 0;
		num_cols = 0;
	}
}

void FaceAnalyser::GetLatestAlignedFace(Mat& image)
{
	image = this->aligned_face.clone();
}

void FaceAnalyser::GetLatestNeutralHOG(Mat_<double>& hog_descriptor, int& num_rows, int& num_cols)
{
	hog_descriptor = this->hog_desc_median;
	if(!hog_desc_median.empty())
	{
		num_rows = this->num_hog_rows;
		num_cols = this->num_hog_cols;
	}
	else
	{
		num_rows = 0;
		num_cols = 0;
	}
}

void FaceAnalyser::GetLatestNeutralFace(Mat& image)
{

}

// Getting the closest view center based on orientation
int GetViewId(const vector<Vec3d> orientations_all, const cv::Vec3d& orientation)
{
	int id = 0;

	double dbest = -1.0;

	for(size_t i = 0; i < orientations_all.size(); i++)
	{
	
		// Distance to current view
		double d = cv::norm(orientation, orientations_all[i]);

		if(i == 0 || d < dbest)
		{
			dbest = d;
			id = i;
		}
	}
	return id;
	
}

void FaceAnalyser::ExtractCurrentMedians(vector<Mat>& hog_medians, vector<Mat>& face_image_medians, vector<Vec3d>& orientations)
{

	orientations = this->head_orientations;

	for(size_t i = 0; i < orientations.size(); ++i)
	{
		Mat_<double> median_face(this->face_image_median.rows, this->face_image_median.cols, 0.0);
		Mat_<double> median_hog(this->hog_desc_median.rows, this->hog_desc_median.cols, 0.0);

		ExtractMedian(this->face_image_hist[i], this->face_image_hist_sum[i], median_face, 256, 0, 255);		
		ExtractMedian(this->hog_desc_hist[i], this->hog_hist_sum[i], median_hog, this->num_bins_hog, 0, 1);

		// Add the HOG sample
		hog_medians.push_back(median_hog.clone());

		// For the face image need to convert it to suitable format
		Mat_<uchar> aligned_face_cols_uchar;
		median_face.convertTo(aligned_face_cols_uchar, CV_8U);

		Mat aligned_face_uchar;
		if(aligned_face.channels() == 1)
		{
			aligned_face_uchar = Mat(aligned_face.rows, aligned_face.cols, CV_8U, aligned_face_cols_uchar.data);
		}
		else
		{
			aligned_face_uchar = Mat(aligned_face.rows, aligned_face.cols, CV_8UC3, aligned_face_cols_uchar.data);
		}

		face_image_medians.push_back(aligned_face_uchar.clone());
		
	}
}

void FaceAnalyser::AddNextFrame(const cv::Mat& frame, const CLMTracker::CLM& clm_model, double timestamp_seconds, bool online, bool visualise)
{
	// Check if a reset is needed first (TODO same person no reset)
	//if(face_bounding_box.area() > 0)
	//{
	//	Rect_<double> new_bounding_box = clm.GetBoundingBox();

	//	// If the box overlaps do not need a reset
	//	double intersection_area = (face_bounding_box & new_bounding_box).area();
	//	double union_area = face_bounding_box.area() + new_bounding_box.area() - 2 * intersection_area;

	//	// If the model is already tracking what we're detecting ignore the detection, this is determined by amount of overlap
	//	if( intersection_area/union_area < 0.5)
	//	{
	//		this->Reset();
	//	}

	//	face_bounding_box = new_bounding_box;
	//}
	//if(!clm.detection_success)
	//{
	//	this->Reset();
	//}

	frames_tracking++;

	// First align the face if tracking was successfull
	if(clm_model.detection_success)
	{
		AlignFaceMask(aligned_face, frame, clm_model, triangulation, true, align_scale, align_width, align_height);
	}
	else
	{
		aligned_face = Mat(align_height, align_width, CV_8UC3);
		aligned_face.setTo(0);
	}

	if(aligned_face.channels() == 3)
	{
		cvtColor(aligned_face, aligned_face_grayscale, CV_BGR2GRAY);
	}
	else
	{
		aligned_face_grayscale = aligned_face.clone();
	}

	// Extract HOG descriptor from the frame and convert it to a useable format
	Mat_<double> hog_descriptor;
	Extract_FHOG_descriptor(hog_descriptor, aligned_face, this->num_hog_rows, this->num_hog_cols);

	// Store the descriptor
	hog_desc_frame = hog_descriptor;

	Vec3d curr_orient(clm_model.params_global[1], clm_model.params_global[2], clm_model.params_global[3]);
	int orientation_to_use = GetViewId(this->head_orientations, curr_orient);

	// Only update the running median if predictions are not high
	// That is don't update it when the face is expressive (just retrieve it)
	bool update_median = true;

	// TODO test if this would be useful or not
	//if(!this->AU_predictions.empty())
	//{
	//	for(size_t i = 0; i < this->AU_predictions.size(); ++i)
	//	{
	//		if(this->AU_predictions[i].second > 1)
	//		{
	//			update_median = false;				
	//			break;
	//		}
	//	}
	//}

	update_median = update_median & clm_model.detection_success;

	// A small speedup
	if(frames_tracking % 2 == 1)
	{
		UpdateRunningMedian(this->hog_desc_hist[orientation_to_use], this->hog_hist_sum[orientation_to_use], this->hog_desc_median, hog_descriptor, update_median, this->num_bins_hog, this->min_val_hog, this->max_val_hog);
	}	
	// Geom descriptor and its median
	geom_descriptor_frame = clm_model.params_local.t();
	
	if(!clm_model.detection_success)
	{
		geom_descriptor_frame.setTo(0);
	}

	// Stack with the actual feature point locations (without mean)
	Mat_<double> locs = clm_model.pdm.princ_comp * geom_descriptor_frame.t();
	
	cv::hconcat(locs.t(), geom_descriptor_frame.clone(), geom_descriptor_frame);
	
	// A small speedup
	if(frames_tracking % 2 == 1)
	{
		UpdateRunningMedian(this->geom_desc_hist, this->geom_hist_sum, this->geom_descriptor_median, geom_descriptor_frame, update_median, this->num_bins_geom, this->min_val_geom, this->max_val_geom);
	}

	// First convert the face image to double representation as a row vector
	Mat_<uchar> aligned_face_cols(1, aligned_face.cols * aligned_face.rows * aligned_face.channels(), aligned_face.data, 1);
	Mat_<double> aligned_face_cols_double;
	aligned_face_cols.convertTo(aligned_face_cols_double, CV_64F);
	
	// TODO get rid of this completely as it takes too long?
	//UpdateRunningMedian(this->face_image_hist[orientation_to_use], this->face_image_hist_sum[orientation_to_use], this->face_image_median, aligned_face_cols_double, update_median, 256, 0, 255);

	// Visualising the median HOG
	if(visualise)
	{
		FaceAnalysis::Visualise_FHOG(hog_descriptor, num_hog_rows, num_hog_cols, hog_descriptor_visualisation);
	}

	// Perform AU prediction	
	AU_predictions_reg = PredictCurrentAUs(orientation_to_use);

	std::vector<std::pair<std::string, double>> AU_predictions_reg_corrected;
	if(online)
	{
		AU_predictions_reg_corrected = CorrectOnlineAUs(AU_predictions_reg, orientation_to_use, true, false, clm_model.detection_success);
	}

	// Keep only closer to in-plane faces
	double angle_norm = cv::sqrt(clm_model.params_global[2] * clm_model.params_global[2] + clm_model.params_global[3] * clm_model.params_global[3]);

	// Add the reg predictions to the historic data
	for (size_t au = 0; au < AU_predictions_reg.size(); ++au)
	{

		// Find the appropriate AU (if not found add it)		
		// Only add if the detection was successful and not too out of plane
		if(clm_model.detection_success && angle_norm < 0.5)
		{
			AU_predictions_reg_all_hist[AU_predictions_reg[au].first].push_back(AU_predictions_reg[au].second);
		}
		else
		{
			AU_predictions_reg_all_hist[AU_predictions_reg[au].first].push_back(-100.0);
		}
	}
	
	AU_predictions_class = PredictCurrentAUsClass(orientation_to_use);

	for (size_t au = 0; au < AU_predictions_class.size(); ++au)
	{

		// Find the appropriate AU (if not found add it)		
		// Only add if the detection was successful and not too out of plane
		if(clm_model.detection_success && angle_norm < 0.5)
		{
			AU_predictions_class_all_hist[AU_predictions_class[au].first].push_back(AU_predictions_class[au].second);
		}
		else
		{
			AU_predictions_class_all_hist[AU_predictions_class[au].first].push_back(-100.0);
		}
	}
	

	if(online)
	{
		AU_predictions_reg = AU_predictions_reg_corrected;
	}

	this->current_time_seconds = timestamp_seconds;

	view_used = orientation_to_use;
			
	bool success = clm_model.detection_success && angle_norm < 0.5;

	confidences.push_back(clm_model.detection_certainty);
	valid_preds.push_back(success);
	timestamps.push_back(timestamp_seconds);
}

void FaceAnalyser::GetGeomDescriptor(Mat_<double>& geom_desc)
{
	geom_desc = this->geom_descriptor_frame.clone();
}

void FaceAnalyser::PredictAUs(const cv::Mat_<double>& hog_features, const cv::Mat_<double>& geom_features, const CLMTracker::CLM& clm_model, bool online)
{
	// Store the descriptor
	hog_desc_frame = hog_features.clone();
	this->geom_descriptor_frame = geom_features.clone();

	Vec3d curr_orient(clm_model.params_global[1], clm_model.params_global[2], clm_model.params_global[3]);
	int orientation_to_use = GetViewId(this->head_orientations, curr_orient);

	// Perform AU prediction	
	AU_predictions_reg = PredictCurrentAUs(orientation_to_use);

	std::vector<std::pair<std::string, double>> AU_predictions_reg_corrected;
	if(online)
	{
		AU_predictions_reg_corrected = CorrectOnlineAUs(AU_predictions_reg, orientation_to_use, true, false, clm_model.detection_success);
	}

	// Keep only closer to in-plane faces
	double angle_norm = cv::sqrt(clm_model.params_global[2] * clm_model.params_global[2] + clm_model.params_global[3] * clm_model.params_global[3]);

	// Add the reg predictions to the historic data
	for (size_t au = 0; au < AU_predictions_reg.size(); ++au)
	{

		// Find the appropriate AU (if not found add it)		
		// Only add if the detection was successful and not too out of plane
		if(clm_model.detection_success && angle_norm < 0.5)
		{
			AU_predictions_reg_all_hist[AU_predictions_reg[au].first].push_back(AU_predictions_reg[au].second);
		}
		else
		{
			AU_predictions_reg_all_hist[AU_predictions_reg[au].first].push_back(-100.0);
		}
	}

	AU_predictions_class = PredictCurrentAUsClass(orientation_to_use);

	for (size_t au = 0; au < AU_predictions_class.size(); ++au)
	{

		// Find the appropriate AU (if not found add it)		
		// Only add if the detection was successful and not too out of plane
		if(clm_model.detection_success && angle_norm < 0.5)
		{
			AU_predictions_class_all_hist[AU_predictions_class[au].first].push_back(AU_predictions_class[au].second);
		}
		else
		{
			AU_predictions_class_all_hist[AU_predictions_class[au].first].push_back(-100.0);
		}
	}

	if(online)
	{
		AU_predictions_reg = AU_predictions_reg_corrected;
	}

	for(size_t i = 0; i < AU_predictions_reg.size(); ++i)
	{
		AU_predictions_combined.push_back(AU_predictions_reg[i]);
	}
	for(size_t i = 0; i < AU_predictions_class.size(); ++i)
	{
		AU_predictions_combined.push_back(AU_predictions_class[i]);
	}

	view_used = orientation_to_use;

	bool success = clm_model.detection_success && angle_norm < 0.5;

	confidences.push_back(clm_model.detection_certainty);
	valid_preds.push_back(success);
}

void FaceAnalyser::ExtractAllPredictionsOfflineReg(vector<std::pair<std::string, vector<double>>>& au_predictions, vector<double>& confidences, vector<bool>& successes, vector<double>& timestamps)
{
	timestamps = this->timestamps;
	au_predictions.clear();
	// First extract the valid AU values and put them in a different format
	vector<vector<double>> aus_valid;
	vector<double> offsets;
	confidences = this->confidences;
	successes = this->valid_preds;

	for(auto au_iter = AU_predictions_reg_all_hist.begin(); au_iter != AU_predictions_reg_all_hist.end(); ++au_iter)
	{
		vector<double> au_good;
		string au_name = au_iter->first;
		vector<double> au_vals = au_iter->second;
		
		au_predictions.push_back(std::pair<string,vector<double>>(au_name, au_vals));

		for(size_t frame = 0; frame < au_vals.size(); ++frame)
		{

			if(successes[frame])
			{
				au_good.push_back(au_vals[frame]);
			}

		}

		if(!au_good.empty())
		{
			std::sort(au_good.begin(), au_good.end());
			offsets.push_back(au_good.at((int)au_good.size()/4));
		}
		else
		{
			offsets.push_back(0.0);
		}
		aus_valid.push_back(au_good);
	}

	// sort each of the aus
	for(size_t au = 0; au < au_predictions.size(); ++au)
	{
		for(size_t frame = 0; frame < au_predictions[au].second.size(); ++frame)
		{

			if(successes[frame])
			{
				double scaling = 1;
				
				au_predictions[au].second[frame] = (au_predictions[au].second[frame] - offsets[au]) * scaling;
				
				if(au_predictions[au].second[frame] < 0.5)
					au_predictions[au].second[frame] = 0;

				if(au_predictions[au].second[frame] > 5)
					au_predictions[au].second[frame] = 5;

			}
			else
			{
				au_predictions[au].second[frame] = 0;
			}
		}
	}


}

void FaceAnalyser::ExtractAllPredictionsOfflineClass(vector<std::pair<std::string, vector<double>>>& au_predictions, vector<double>& confidences, vector<bool>& successes, vector<double>& timestamps)
{
	timestamps = this->timestamps;
	au_predictions.clear();

	for(auto au_iter = AU_predictions_class_all_hist.begin(); au_iter != AU_predictions_class_all_hist.end(); ++au_iter)
	{
		string au_name = au_iter->first;
		vector<double> au_vals = au_iter->second;
		
		au_predictions.push_back(std::pair<string,vector<double>>(au_name, au_vals));

	}

	confidences = this->confidences;
	successes = this->valid_preds;
}

// Reset the models
void FaceAnalyser::Reset()
{
	frames_tracking = 0;

	this->hog_desc_median.setTo(Scalar(0));
	this->face_image_median.setTo(Scalar(0));

	for( size_t i = 0; i < hog_desc_hist.size(); ++i)
	{
		this->hog_desc_hist[i] = Mat_<unsigned int>(hog_desc_hist[i].rows, hog_desc_hist[i].cols, (unsigned int)0);
		this->hog_hist_sum[i] = 0;


		this->face_image_hist[i] = Mat_<unsigned int>(face_image_hist[i].rows, face_image_hist[i].cols, (unsigned int)0);
		this->face_image_hist_sum[i] = 0;

		// 0 callibration predictions
		this->au_prediction_correction_count[i] = 0;
		this->au_prediction_correction_histogram[i] = Mat_<unsigned int>(au_prediction_correction_histogram[i].rows, au_prediction_correction_histogram[i].cols, (unsigned int)0);
	}

	this->geom_descriptor_median.setTo(Scalar(0));
	this->geom_desc_hist = Mat_<unsigned int>(geom_desc_hist.rows, geom_desc_hist.cols, (unsigned int)0);
	geom_hist_sum = 0;

	// Reset the predictions
	AU_prediction_track = Mat_<double>(AU_prediction_track.rows, AU_prediction_track.cols, 0.0);

	geom_desc_track = Mat_<double>(geom_desc_track.rows, geom_desc_track.cols, 0.0);

	dyn_scaling = vector<vector<double>>(dyn_scaling.size(), vector<double>(dyn_scaling[0].size(), 5.0));	

	AU_predictions_reg.clear();
	AU_predictions_class.clear();
	AU_predictions_combined.clear();
	timestamps.clear();
	AU_predictions_reg_all_hist.clear();
	AU_predictions_class_all_hist.clear();
	confidences.clear();
	valid_preds.clear();

}

void FaceAnalyser::UpdateRunningMedian(cv::Mat_<unsigned int>& histogram, int& hist_count, cv::Mat_<double>& median, const cv::Mat_<double>& descriptor, bool update, int num_bins, double min_val, double max_val)
{

	double length = max_val - min_val;
	if(length < 0)
		length = -length;

	// The median update
	if(histogram.empty())
	{
		histogram = Mat_<unsigned int>(descriptor.cols, num_bins, (unsigned int)0);
		median = descriptor.clone();
	}

	if(update)
	{
		// Find the bins corresponding to the current descriptor
		Mat_<double> converted_descriptor = (descriptor - min_val)*((double)num_bins)/(length);

		// Capping the top and bottom values
		converted_descriptor.setTo(Scalar(num_bins-1), converted_descriptor > num_bins - 1);
		converted_descriptor.setTo(Scalar(0), converted_descriptor < 0);

		// Only count the median till a certain number of frame seen?
		for(int i = 0; i < histogram.rows; ++i)
		{
			int index = (int)converted_descriptor.at<double>(i);
			histogram.at<unsigned int>(i, index)++;
		}

		// Update the histogram count
		hist_count++;
	}

	if(hist_count == 1)
	{
		median = descriptor.clone();
	}
	else
	{
		// Recompute the median
		int cutoff_point = (hist_count + 1)/2;

		// For each dimension
		for(int i = 0; i < histogram.rows; ++i)
		{
			int cummulative_sum = 0;
			for(int j = 0; j < histogram.cols; ++j)
			{
				cummulative_sum += histogram.at<unsigned int>(i, j);
				if(cummulative_sum > cutoff_point)
				{
					median.at<double>(i) = min_val + j * (length/num_bins) + (0.5*(length)/num_bins);
					break;
				}
			}
		}
	}
}


void FaceAnalyser::ExtractMedian(cv::Mat_<unsigned int>& histogram, int hist_count, cv::Mat_<double>& median, int num_bins, double min_val, double max_val)
{

	double length = max_val - min_val;
	if(length < 0)
		length = -length;

	// The median update
	if(histogram.empty())
	{
		return;
	}
	else
	{
		if(median.empty())
		{
			median = Mat_<double>(1, histogram.rows, 0.0);
		}

		// Compute the median
		int cutoff_point = (hist_count + 1)/2;

		// For each dimension
		for(int i = 0; i < histogram.rows; ++i)
		{
			int cummulative_sum = 0;
			for(int j = 0; j < histogram.cols; ++j)
			{
				cummulative_sum += histogram.at<unsigned int>(i, j);
				if(cummulative_sum > cutoff_point)
				{
					median.at<double>(i) = min_val + j * (max_val/num_bins) + (0.5*(length)/num_bins);
					break;
				}
			}
		}
	}
}
// Apply the current predictors to the currently stored descriptors
vector<pair<string, double>> FaceAnalyser::PredictCurrentAUs(int view)
{

	vector<pair<string, double>> predictions;

	if(!hog_desc_frame.empty())
	{
		vector<string> svr_lin_stat_aus;
		vector<double> svr_lin_stat_preds;

		AU_SVR_static_appearance_lin_regressors.Predict(svr_lin_stat_preds, svr_lin_stat_aus, hog_desc_frame, geom_descriptor_frame);

		for(size_t i = 0; i < svr_lin_stat_preds.size(); ++i)
		{
			predictions.push_back(pair<string, double>(svr_lin_stat_aus[i], svr_lin_stat_preds[i]));
		}

		vector<string> svr_lin_dyn_aus;
		vector<double> svr_lin_dyn_preds;

		AU_SVR_dynamic_appearance_lin_regressors.Predict(svr_lin_dyn_preds, svr_lin_dyn_aus, hog_desc_frame, geom_descriptor_frame,  this->hog_desc_median, this->geom_descriptor_frame);

		for(size_t i = 0; i < svr_lin_dyn_preds.size(); ++i)
		{
			predictions.push_back(pair<string, double>(svr_lin_dyn_aus[i], svr_lin_dyn_preds[i]));
		}

	}

	return predictions;
}

vector<pair<string, double>> FaceAnalyser::CorrectOnlineAUs(std::vector<std::pair<std::string, double>> predictions_orig, int view, bool dyn_shift, bool dyn_scale, bool update_track, bool clip_values)
{
	// Correction that drags the predicion to 0 (assuming the bottom 10% of predictions are of neutral expresssions)
	vector<double> correction(predictions_orig.size(), 0.0);

	vector<pair<string, double>> predictions = predictions_orig;

	if(update_track)
	{
		UpdatePredictionTrack(au_prediction_correction_histogram[view], au_prediction_correction_count[view], correction, predictions, 0.10, 200, -3, 5, 10);
	}

	if(dyn_shift)
	{
		for(size_t i = 0; i < correction.size(); ++i)
		{
			predictions[i].second = predictions[i].second - correction[i];
		}
	}
	if(dyn_scale)
	{
		// Some scaling for effect better visualisation
		// Also makes sense as till the maximum expression is seen, it is hard to tell how expressive a persons face is
		if(dyn_scaling[view].empty())
		{
			dyn_scaling[view] = vector<double>(predictions.size(), 5.0);
		}
		
		for(size_t i = 0; i < predictions.size(); ++i)
		{
			// First establish presence (assume it is maximum as we have not seen max) 
			// TODO this could be more robust by removing some outliers, or by doing it only for certain AUs?
			if(predictions[i].second > 1)
			{
				double scaling_curr = 5.0 / predictions[i].second;
				
				if(scaling_curr < dyn_scaling[view][i])
				{
					dyn_scaling[view][i] = scaling_curr;
				}
				predictions[i].second = predictions[i].second * dyn_scaling[view][i];
			}

			if(predictions[i].second > 5)
			{
				predictions[i].second = 5;
			}
		}
	}

	if(clip_values)
	{
		for(size_t i = 0; i < correction.size(); ++i)
		{
			if(predictions[i].second < 0)
				predictions[i].second = 0;
			if(predictions[i].second > 5)
				predictions[i].second = 5;
		}
	}
	return predictions;
}

// Apply the current predictors to the currently stored descriptors (classification)
vector<pair<string, double>> FaceAnalyser::PredictCurrentAUsClass(int view)
{

	vector<pair<string, double>> predictions;

	if(!hog_desc_frame.empty())
	{
		vector<string> svm_lin_stat_aus;
		vector<double> svm_lin_stat_preds;
		
		AU_SVM_static_appearance_lin.Predict(svm_lin_stat_preds, svm_lin_stat_aus, hog_desc_frame, geom_descriptor_frame);

		for(size_t i = 0; i < svm_lin_stat_aus.size(); ++i)
		{
			predictions.push_back(pair<string, double>(svm_lin_stat_aus[i], svm_lin_stat_preds[i]));
		}

		vector<string> svm_lin_dyn_aus;
		vector<double> svm_lin_dyn_preds;

		AU_SVM_dynamic_appearance_lin.Predict(svm_lin_dyn_preds, svm_lin_dyn_aus, hog_desc_frame, geom_descriptor_frame, this->hog_desc_median, this->geom_descriptor_median);

		for(size_t i = 0; i < svm_lin_dyn_aus.size(); ++i)
		{
			predictions.push_back(pair<string, double>(svm_lin_dyn_aus[i], svm_lin_dyn_preds[i]));
		}
		
	}

	return predictions;
}


Mat_<uchar> FaceAnalyser::GetLatestAlignedFaceGrayscale()
{
	return aligned_face_grayscale.clone();
}

Mat FaceAnalyser::GetLatestHOGDescriptorVisualisation()
{
	return hog_descriptor_visualisation;
}

vector<pair<string, double>> FaceAnalyser::GetCurrentAUsClass()
{
	return AU_predictions_class;
}

vector<pair<string, double>> FaceAnalyser::GetCurrentAUsReg()
{
	return AU_predictions_reg;
}

vector<pair<string, double>> FaceAnalyser::GetCurrentAUsCombined()
{
	return AU_predictions_combined;
}

// Reading in AU prediction modules
void FaceAnalyser::ReadAU(std::string au_model_location)
{

	// Open the list of the regressors in the file
	ifstream locations(au_model_location.c_str(), ios::in);

	if(!locations.is_open())
	{
		cout << "Couldn't open the AU prediction files at: " << au_model_location.c_str() << " aborting" << endl;
		cout.flush();
		return;
	}

	string line;
	
	// The other module locations should be defined as relative paths from the main model
	boost::filesystem::path root = boost::filesystem::path(au_model_location).parent_path();		
	
	// The main file contains the references to other files
	while (!locations.eof())
	{ 
		
		getline(locations, line);

		stringstream lineStream(line);

		string name;
		string location;

		// figure out which module is to be read from which file
		lineStream >> location;

		// Parse comma separated names that this regressor produces
		name = lineStream.str();
		int index = name.find_first_of(' ');

		if(index >= 0)
		{
			name = name.substr(index+1);
			
			// remove carriage return at the end for compatibility with unix systems
			if(name.size() > 0 && name.at(name.size()-1) == '\r')
			{
				name = name.substr(0, location.size()-1);
			}
		}
		vector<string> au_names;
		boost::split(au_names, name, boost::is_any_of(","));

		// append the lovstion to root location (boost syntax)
		location = (root / location).string();
				
		ReadRegressor(location, au_names);
	}
  
}

void FaceAnalyser::UpdatePredictionTrack(Mat_<unsigned int>& prediction_corr_histogram, int& prediction_correction_count, vector<double>& correction, const vector<pair<string, double>>& predictions, double ratio, int num_bins, double min_val, double max_val, int min_frames)
{
	double length = max_val - min_val;
	if(length < 0)
		length = -length;

	correction.resize(predictions.size(), 0);

	// The median update
	if(prediction_corr_histogram.empty())
	{
		prediction_corr_histogram = Mat_<unsigned int>(predictions.size(), num_bins, (unsigned int)0);
	}
	
	for(int i = 0; i < prediction_corr_histogram.rows; ++i)
	{
		// Find the bins corresponding to the current descriptor
		int index = (predictions[i].second - min_val)*((double)num_bins)/(length);
		if(index < 0)
		{
			index = 0;
		}
		else if(index > num_bins - 1)
		{
			index = num_bins - 1;
		}
		prediction_corr_histogram.at<unsigned int>(i, index)++;
	}

	// Update the histogram count
	prediction_correction_count++;

	if(prediction_correction_count >= min_frames)
	{
		// Recompute the correction
		int cutoff_point = ratio * prediction_correction_count;

		// For each dimension
		for(int i = 0; i < prediction_corr_histogram.rows; ++i)
		{
			int cummulative_sum = 0;
			for(int j = 0; j < prediction_corr_histogram.cols; ++j)
			{
				cummulative_sum += prediction_corr_histogram.at<unsigned int>(i, j);
				if(cummulative_sum > cutoff_point)
				{
					double corr = min_val + j * (length/num_bins);
					correction[i] = corr;
					break;
				}
			}
		}
	}
}

void FaceAnalyser::GetSampleHist(Mat_<unsigned int>& prediction_corr_histogram, int prediction_correction_count, vector<double>& sample, double ratio, int num_bins, double min_val, double max_val)
{

	double length = max_val - min_val;
	if(length < 0)
		length = -length;

	sample.resize(prediction_corr_histogram.rows, 0);

	// Recompute the correction
	int cutoff_point = ratio * prediction_correction_count;

	// For each dimension
	for(int i = 0; i < prediction_corr_histogram.rows; ++i)
	{
		int cummulative_sum = 0;
		for(int j = 0; j < prediction_corr_histogram.cols; ++j)
		{
			cummulative_sum += prediction_corr_histogram.at<unsigned int>(i, j);
			if(cummulative_sum > cutoff_point)
			{
				double corr = min_val + j * (length/num_bins);
				sample[i] = corr;
				break;
			}
		}
	}

}

void FaceAnalyser::ReadRegressor(std::string fname, const vector<string>& au_names)
{
	ifstream regressor_stream(fname.c_str(), ios::in | ios::binary);

	// First read the input type
	int regressor_type;
	regressor_stream.read((char*)&regressor_type, 4);

	if(regressor_type == SVR_appearance_static_linear)
	{
		AU_SVR_static_appearance_lin_regressors.Read(regressor_stream, au_names);		
	}
	else if(regressor_type == SVR_appearance_dynamic_linear)
	{
		AU_SVR_dynamic_appearance_lin_regressors.Read(regressor_stream, au_names);		
	}
	else if(regressor_type == SVM_linear_stat)
	{
		AU_SVM_static_appearance_lin.Read(regressor_stream, au_names);		
	}
	else if(regressor_type == SVM_linear_dyn)
	{
		AU_SVM_dynamic_appearance_lin.Read(regressor_stream, au_names);		
	}
}

double FaceAnalyser::GetCurrentTimeSeconds() {
	return current_time_seconds;
}