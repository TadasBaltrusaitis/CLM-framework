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
	RapportAnalyser();

	// Adding the observation of the current track and the face_analyser
	void AddObservation(const CLMTracker::CLM& clm_model, const FaceAnalyser& face_analyser, const Point3f& gaze_left, const Point3f& gaze_right, double fx, double fy, double cx, double cy);

	double GetRapportEstimate();
	double GetAttentionEstimate();
	double GetValenceEstimate();

private:

	double current_rapport;
	double current_attention;
	double current_valence;

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

};
}
#endif