#ifndef __RAPPORTANALYSER_h_
#define __RAPPORTANALYSER_h_

#include "opencv2/core/core.hpp"
#include "CLM_core.h"
#include "FaceAnalyser.h"

using namespace cv;

namespace FaceAnalysis
{
class RapportAnalyser{

public:
	
	// Constructor
	RapportAnalyser(std::string config_location = "rapport_config/config.txt");

	// Adding the observation of the current track and the face_analyser
	void AddObservation(const CLMTracker::CLM& clm_model, const FaceAnalyser& face_analyser, const Point3f& gaze_left, const Point3f& gaze_right, double fx, double fy, double cx, double cy);

	double GetRapportEstimate();
	double GetAttentionEstimate();
	double GetValenceEstimate();
	double GetArousalEstimate();
	double GetEyeAttention();
	double GetHeadAttention();

	double GetSpeech();

	string RapportAnalyser::GetAllContent();

private:

	double RapportAnalyser::PredictArousal(const CLMTracker::CLM& clm_model, const FaceAnalyser& face_analyser);

	cv::Mat_<double> geom_desc_track;

	int frames_tracking;

	// Facial Actions
	double speech;

	double current_rapport;
	double current_attention;
	double current_valence;
	double current_arousal;

	double eye_attention;
	double head_attention;

	vector<double> rapport_history;

	double prev_time_step;

	vector<double> AU1_history;
	vector<double> AU2_history;
	vector<double> AU4_history;
	vector<double> AU6_history;
	vector<double> AU12_history;
	vector<double> AU15_history;
	vector<double> AU17_history;
	vector<double> AU25_history;
	vector<double> time_step_history;

	double arousal_scaling;

	double head_attention_scaling;
	double gaze_attention_scaling;
	double rapport_rate_of_change;
	double head_gaze_threshold;
	double eye_gaze_threshold;

	double head_gaze_change_rate_neg;
	double head_gaze_change_rate_pos;
	double eye_gaze_change_rate_neg;
	double eye_gaze_change_rate_pos;

	double smile_rate_of_change_valence;
	double smile_rate_of_change_rapport;

	double frown_rate_of_change_valence;
	double frown_rate_of_change_rapport;
	
	double brow_furrow_rapport_change;
	double brow_furrow_valence_change;

	double valence_return_to_neutral_rate;

	double arousal_rapport_affect_pos;
	double arousal_rapport_affect_neg; 

	double brow_flash_rapport_change;

	double speech_threshold;
	double speech_rapport_threshold;
	double speech_rapport_rate;

	double max_rate_of_change;

	double smoothing_factor;

	double min_rapport;
	double max_rapport;

};
}
#endif