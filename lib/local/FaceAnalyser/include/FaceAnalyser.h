#ifndef __FACEANALYSER_h_
#define __FACEANALYSER_h_

#include "SVR_dynamic_lin_regressors.h"
#include "SVR_static_lin_regressors.h"
#include "SVM_static_lin.h"
#include "SVM_dynamic_lin.h"

#include <string>
#include <vector>

#include <cv.h>

#include "CLM_core.h"

namespace FaceAnalysis
{

class FaceAnalyser{

public:


	enum RegressorType{ SVR_appearance_static_linear = 0, SVR_appearance_dynamic_linear = 1, SVR_dynamic_geom_linear = 2, SVR_combined_linear = 3, SVM_linear_stat = 4, SVM_linear_dyn = 5, SVR_linear_static_seg = 6, SVR_linear_dynamic_seg =7};

	// Constructor from a model file (or a default one if not provided
	// TODO scale width and height should be read in as part of the model as opposed to being here?
	FaceAnalyser(vector<Vec3d> orientation_bins = vector<Vec3d>(), double scale = 0.7, int width = 112, int height = 112, std::string au_location = "AU_predictors/AU_all_best.txt", std::string tri_location = "model/tris_68_full.txt");

	void AddNextFrame(const cv::Mat& frame, const CLMTracker::CLM& clm, double timestamp_seconds, bool dynamic_shift, bool dynamic_scale, bool visualise = true);

	// If the features are extracted manually
	void PredictAUs(const cv::Mat_<double>& hog_features, const cv::Mat_<double>& geom_features, const CLMTracker::CLM& clm_model, bool dyn_shift, bool dyn_scale);

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
	
	Mat_<int> FaceAnalyser::GetTriangulation();

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

private:

	// Where the predictions are kept
	std::vector<std::pair<std::string, double>> AU_predictions_reg;
	std::vector<std::pair<std::string, double>> AU_predictions_class;

	std::vector<std::pair<std::string, double>> AU_predictions_combined;

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
	std::vector<std::pair<std::string, double>> PredictCurrentAUs(int view, bool dyn_shift = false, bool dyn_scale = false, bool update_track = true);
	std::vector<std::pair<std::string, double>> PredictCurrentAUsClass(int view);

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
