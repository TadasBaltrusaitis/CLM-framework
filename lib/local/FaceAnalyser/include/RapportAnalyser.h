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

private:

	double current_rapport;
	vector<double> rapport_history;

};
}
#endif