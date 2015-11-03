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

#ifndef __FACEANALYSER_h_
#define __FACEANALYSER_h_

#include "SVR_dynamic_lin_regressors.h"
#include "SVR_static_lin_regressors.h"
#include "SVM_static_lin.h"
#include "SVM_dynamic_lin.h"

#include <string>
#include <vector>

#include <opencv2/core/core.hpp>

#include "CLM_core.h"

namespace FaceAnalysis
{

class FaceAnalyser{

public:


	enum RegressorType{ SVR_appearance_static_linear = 0, SVR_appearance_dynamic_linear = 1, SVR_dynamic_geom_linear = 2, SVR_combined_linear = 3, SVM_linear_stat = 4, SVM_linear_dyn = 5, SVR_linear_static_seg = 6, SVR_linear_dynamic_seg =7};

	// Constructor from a model file (or a default one if not provided
	// TODO scale width and height should be read in as part of the model as opposed to being here?
	FaceAnalyser(vector<Vec3d> orientation_bins = vector<Vec3d>(), double scale = 0.7, int width = 112, int height = 112, std::string au_location = "AU_predictors/AU_all_best.txt", std::string tri_location = "model/tris_68_full.txt");

	void AddNextFrame(const cv::Mat& frame, const CLMTracker::CLM& clm, double timestamp_seconds, bool online = false, bool visualise = true);

	// If the features are extracted manually (shouldn't really be used)
	void PredictAUs(const cv::Mat_<double>& hog_features, const cv::Mat_<double>& geom_features, const CLMTracker::CLM& clm_model, bool online);

	Mat GetLatestHOGDescriptorVisualisation();

	double GetCurrentTimeSeconds();
	
	std::vector<std::pair<std::string, double>> GetCurrentAUsClass();
	std::vector<std::pair<std::string, double>> GetCurrentAUsReg();

	std::vector<std::pair<std::string, double>> GetCurrentAUsCombined();

	void Reset();

	void GetLatestHOG(Mat_<double>& hog_descriptor, int& num_rows, int& num_cols);
	void GetLatestAlignedFace(Mat& image);
	
	void GetLatestNeutralHOG(Mat_<double>& hog_descriptor, int& num_rows, int& num_cols);
	void GetLatestNeutralFace(Mat& image);
	
	Mat_<int> GetTriangulation();

	Mat_<uchar> GetLatestAlignedFaceGrayscale();
	
	void GetGeomDescriptor(Mat_<double>& geom_desc);

	void ExtractCurrentMedians(vector<Mat>& hog_medians, vector<Mat>& face_image_medians, vector<Vec3d>& orientations);

	std::vector<std::string> GetAUClassNames()
	{
		std::vector<std::string> au_class_names_all;
		std::vector<std::string> au_class_names_stat = AU_SVM_static_appearance_lin.GetAUNames();
		std::vector<std::string> au_class_names_dyn = AU_SVM_dynamic_appearance_lin.GetAUNames();
		
		for(size_t i = 0; i < au_class_names_stat.size(); ++i)
		{
			au_class_names_all.push_back(au_class_names_stat[i]);
		}
		for(size_t i = 0; i < au_class_names_dyn.size(); ++i)
		{
			au_class_names_all.push_back(au_class_names_dyn[i]);
		}

		return au_class_names_all;
	}

	std::vector<std::string> GetAURegNames()
	{
		std::vector<std::string> au_reg_names_all;
		std::vector<std::string> au_reg_names_stat = AU_SVR_static_appearance_lin_regressors.GetAUNames();
		std::vector<std::string> au_reg_names_dyn = AU_SVR_dynamic_appearance_lin_regressors.GetAUNames();
		
		for(size_t i = 0; i < au_reg_names_stat.size(); ++i)
		{
			au_reg_names_all.push_back(au_reg_names_stat[i]);
		}
		for(size_t i = 0; i < au_reg_names_dyn.size(); ++i)
		{
			au_reg_names_all.push_back(au_reg_names_dyn[i]);
		}

		return au_reg_names_all;
	}

	void ExtractAllPredictionsOfflineReg(vector<std::pair<std::string, vector<double>>>& au_predictions, vector<double>& confidences, vector<bool>& successes);
	void ExtractAllPredictionsOfflineClass(vector<std::pair<std::string, vector<double>>>& au_predictions, vector<double>& confidences, vector<bool>& successes);

private:

	// Where the predictions are kept
	std::vector<std::pair<std::string, double>> AU_predictions_reg;
	std::vector<std::pair<std::string, double>> AU_predictions_class;

	std::vector<std::pair<std::string, double>> AU_predictions_combined;

	std::map<std::string, vector<double>> AU_predictions_reg_all_hist;
	std::map<std::string, vector<double>> AU_predictions_class_all_hist;
	std::vector<double> confidences;
	std::vector<bool> valid_preds;

	int frames_tracking;

	// Cache of intermediate images
	Mat_<uchar> aligned_face_grayscale;
	Mat aligned_face;
	Mat hog_descriptor_visualisation;

	// Private members to be used for predictions
	// The HOG descriptor of the last frame
	Mat_<double> hog_desc_frame;
	int num_hog_rows;
	int num_hog_cols;

	// Keep a running median of the hog descriptors and a aligned images
	Mat_<double> hog_desc_median;
	Mat_<double> face_image_median;

	// Use histograms for quick (but approximate) median computation
	// Use the same for
	vector<Mat_<unsigned int> > hog_desc_hist;

	// This is not being used at the moment as it is a bit slow
	// TODO check if this would be more useful than keeping median of HoG
	vector<Mat_<unsigned int> > face_image_hist;
	vector<int> face_image_hist_sum;

	vector<Vec3d> head_orientations;

	int num_bins_hog;
	double min_val_hog;
	double max_val_hog;
	vector<int> hog_hist_sum;
	int view_used;

	// The geometry descriptor (rigid followed by non-rigid shape parameters from CLM)
	Mat_<double> geom_descriptor_frame;
	Mat_<double> geom_descriptor_median;
	
	int geom_hist_sum;
	Mat_<unsigned int> geom_desc_hist;
	int num_bins_geom;
	double min_val_geom;
	double max_val_geom;
	
	// Using the bounding box of previous analysed frame to determine if a reset is needed
	Rect_<double> face_bounding_box;
	
	// The AU predictions internally
	std::vector<std::pair<std::string, double>> PredictCurrentAUs(int view);
	std::vector<std::pair<std::string, double>> PredictCurrentAUsClass(int view);

	// special step for online (rather than offline AU prediction)
	std::vector<pair<string, double>> CorrectOnlineAUs(std::vector<std::pair<std::string, double>> predictions_orig, int view, bool dyn_shift = false, bool dyn_scale = false, bool update_track = true, bool clip_values = false);

	void ReadAU(std::string au_location);

	void ReadRegressor(std::string fname, const vector<string>& au_names);

	// A utility function for keeping track of approximate running medians used for AU and emotion inference using a set of histograms (the histograms are evenly spaced from min_val to max_val)
	// Descriptor has to be a row vector
	// TODO this duplicates some other code
	void UpdateRunningMedian(cv::Mat_<unsigned int>& histogram, int& hist_sum, cv::Mat_<double>& median, const cv::Mat_<double>& descriptor, bool update, int num_bins, double min_val, double max_val);
	void ExtractMedian(cv::Mat_<unsigned int>& histogram, int hist_count, cv::Mat_<double>& median, int num_bins, double min_val, double max_val);
	
	// The linear SVR regressors
	SVR_static_lin_regressors AU_SVR_static_appearance_lin_regressors;
	SVR_dynamic_lin_regressors AU_SVR_dynamic_appearance_lin_regressors;
		
	// The linear SVM classifiers
	SVM_static_lin AU_SVM_static_appearance_lin;
	SVM_dynamic_lin AU_SVM_dynamic_appearance_lin;

	// The AUs predicted by the model are not always 0 calibrated to a person. That is they don't always predict 0 for a neutral expression
	// Keeping track of the predictions we can correct for this, by assuming that at least "ratio" of frames are neutral and subtract that value of prediction, only perform the correction after min_frames
	void UpdatePredictionTrack(Mat_<unsigned int>& prediction_corr_histogram, int& prediction_correction_count, vector<double>& correction, const vector<pair<string, double>>& predictions, double ratio=0.25, int num_bins = 200, double min_val = -3, double max_val = 5, int min_frames = 10);	
	void GetSampleHist(Mat_<unsigned int>& prediction_corr_histogram, int prediction_correction_count, vector<double>& sample, double ratio, int num_bins = 200, double min_val = 0, double max_val = 5);	

	vector<std::pair<std::string, vector<double>>> PostprocessPredictions();

	vector<cv::Mat_<unsigned int>> au_prediction_correction_histogram;
	vector<int> au_prediction_correction_count;

	// Some dynamic scaling (the logic is that before the extreme versions of expression or emotion are shown,
	// it is hard to tell the boundaries, this allows us to scale the model to the most extreme seen)
	// They have to be view specific
	vector<vector<double>> dyn_scaling;
	
	// Keeping track of predictions for summary stats
	cv::Mat_<double> AU_prediction_track;
	cv::Mat_<double> geom_desc_track;

	double current_time_seconds;

	// Used for face alignment
	Mat_<int> triangulation;
	double align_scale;	
	int align_width;
	int align_height;
};
  //===========================================================================
}
#endif
