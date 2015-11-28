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

#include "RapportAnalyser.h"

#include "Face_utils.h"
#include "CLM_core.h"
#include <stdio.h>
#include <iostream>
#include <math.h>

#include <string>

using namespace FaceAnalysis;

using namespace std;

// Constructor from a model file (or a default one if not provided
RapportAnalyser::RapportAnalyser(std::string config_location) :rapport_history(), time_step_history(), geom_desc_track()
{
	// start with average rapport first
	current_rapport = 4;
	current_valence = 4;
	current_attention = 4;

	prev_time_step = 0;

	eye_attention = 0;
	head_attention = 0;	

	frames_tracking = 0;
	speech = 0;

	arousal_scaling = 0.25;
	head_attention_scaling = 40.0;
	gaze_attention_scaling = 15.0;
	rapport_rate_of_change = 0.02;

	head_gaze_threshold = 15.0;
	eye_gaze_threshold = 7.0;

	head_gaze_change_rate_neg = 0.66;
	eye_gaze_change_rate_neg = 0.66;

	head_gaze_change_rate_pos = 0.33;
	eye_gaze_change_rate_pos = 0.33;

	smile_rate_of_change_valence = 3.0;
	smile_rate_of_change_rapport = 2.0;

	frown_rate_of_change_valence = 3.0;
	frown_rate_of_change_rapport = 2.0;

	valence_return_to_neutral_rate = 1.5;

	brow_flash_rapport_change = 1.0;

	speech_threshold = 0.3;
	speech_rapport_threshold = 2.0;
	speech_rapport_rate = 0.66;

	max_rate_of_change = 0.5;

	smoothing_factor = 0.3;

	min_rapport = 1.0;
	max_rapport = 7.0;

	brow_furrow_rapport_change = -2.0;
	brow_furrow_valence_change = -3.0;

	arousal_rapport_affect_pos = 0.25;
	arousal_rapport_affect_neg = 0.1; 

	// Now read in the rapport config file possibly overwriting the default parameters
	ifstream config_file(config_location);
	if(config_file.is_open())
	{
		string line;
		while(std::getline(config_file, line))
		{
			stringstream ss(line);
			string variable;
			double value;
			ss >> variable;
			ss >> value;

			if(variable.compare("arousal_scaling")==0)
			{
				arousal_scaling = value;
			}
			else if(variable.compare("head_attention_scaling")==0)
			{
				head_attention_scaling = value;
			}
			else if(variable.compare("gaze_attention_scaling")==0)
			{
				gaze_attention_scaling = value;
			}
			else if(variable.compare("rapport_rate_of_change")==0)
			{
				rapport_rate_of_change = value;
			}
			else if(variable.compare("head_gaze_threshold")==0)
			{
				head_gaze_threshold = value;
			}
			else if(variable.compare("eye_gaze_threshold")==0)
			{
				eye_gaze_threshold = value;
			}
			else if(variable.compare("head_gaze_change_rate_neg")==0)
			{
				head_gaze_change_rate_neg = value;
			}
			else if(variable.compare("head_gaze_change_rate_pos")==0)
			{
				head_gaze_change_rate_pos = value;
			}
			else if(variable.compare("eye_gaze_change_rate_neg")==0)
			{
				eye_gaze_change_rate_neg = value;
			}
			else if(variable.compare("eye_gaze_change_rate_pos")==0)
			{
				eye_gaze_change_rate_pos = value;
			}
			else if(variable.compare("smile_rate_of_change_valence")==0)
			{
				smile_rate_of_change_valence = value;
			}
			else if(variable.compare("smile_rate_of_change_rapport")==0)
			{
				smile_rate_of_change_rapport = value;
			}
			else if(variable.compare("frown_rate_of_change_valence")==0)
			{
				frown_rate_of_change_valence = value;
			}
			else if(variable.compare("frown_rate_of_change_rapport")==0)
			{
				frown_rate_of_change_rapport = value;
			}
			else if(variable.compare("brow_furrow_rapport_change")==0)
			{
				brow_furrow_rapport_change = value;
			}
			else if(variable.compare("brow_furrow_valence_change")==0)
			{
				brow_furrow_valence_change = value;
			}
			else if(variable.compare("valence_return_to_neutral_rate")==0)
			{
				valence_return_to_neutral_rate = value;
			}
			else if(variable.compare("arousal_rapport_affect_pos")==0)
			{
				arousal_rapport_affect_pos = value;
			}
			else if(variable.compare("arousal_rapport_affect_neg")==0)
			{
				arousal_rapport_affect_neg = value;
			}
			else if(variable.compare("brow_flash_rapport_change")==0)
			{
				brow_flash_rapport_change = value;
			}
			else if(variable.compare("speech_threshold")==0)
			{
				speech_threshold = value;
			}
			else if(variable.compare("speech_rapport_threshold")==0)
			{
				speech_rapport_threshold = value;
			}
			else if(variable.compare("speech_rapport_rate")==0)
			{
				speech_rapport_rate = value;
			}
			else if(variable.compare("max_rate_of_change")==0)
			{
				max_rate_of_change = value;
			}
			else if(variable.compare("smoothing_factor")==0)
			{
				smoothing_factor = value;
			}
			else if(variable.compare("min_rapport")==0)
			{
				min_rapport = value;
			}
			else if(variable.compare("max_rapport")==0)
			{
				max_rapport = value;
			}

		}
	}
	else
	{
		cout << "ERROR - Can't open rapport config file, file not found" << endl;
	}


}

double RapportAnalyser::PredictArousal(const CLMTracker::CLM& clm_model, const FaceAnalyser& face_analyser)
{
	// Arousal prediction
	// Adding the geometry descriptor 
	Mat_<double> geom_params;

	// This is for tracking median of geometry parameters to subtract from the other models
	Vec3d g_params(clm_model.params_global[1], clm_model.params_global[2], clm_model.params_global[3]);
	geom_params.push_back(Mat(g_params));
	geom_params.push_back(clm_model.params_local);
	geom_params = geom_params.t();

	AddDescriptor(geom_desc_track, geom_params, this->frames_tracking, 200);
	Mat_<double> sum_stats_geom;
	ExtractSummaryStatistics(geom_desc_track, sum_stats_geom, false, true, false);

	Mat_<double> scales = arousal_scaling * Mat_<double>::ones(3, 1);

	cv::vconcat(scales.clone(), clm_model.pdm.eigen_values.t(), scales);

	scales = scales.t();
	
	sum_stats_geom = sum_stats_geom / scales;

	double arousal = cv::sum(sum_stats_geom)[0];

	if(this->frames_tracking < 100)
	{
		// Damp predictions before enough evidence is seen
		double a_tmp = (arousal - 0.5) / 3.5 - 0.5;
		return 0.5 + a_tmp * 0.2;
	}
	else
	{
		return (arousal- 0.5) / 3.5;
	}
}
void RapportAnalyser::AddObservation(const CLMTracker::CLM& clm_model, const FaceAnalyser& face_analyser, const Point3f& gaze_left, const Point3f& gaze_right, double fx, double fy, double cx, double cy)
{
	double time = face_analyser.GetCurrentTimeSeconds();
	
	if (!clm_model.detection_success)
		return;

	current_arousal = PredictArousal(clm_model, face_analyser);

	// First extract head gaze
	Vec6d pose = CLMTracker::GetCorrectedPoseCamera(clm_model, fx, fy, cx, cy);
	
	// Convert this to head gaze vector
	Point3d head_gaze(0, 0, -1);
	Vec3d rotation(pose[3], pose[4], pose[5]);
	auto rot_mat = CLMTracker::Euler2RotationMatrix(rotation);

	head_gaze = Point3d(Mat(Mat(head_gaze).t() * Mat(rot_mat.t())));

	Point3d head_gaze_straight(0, 0, -1);
	double head_gaze_away = acos(head_gaze.dot(head_gaze_straight) / (cv::norm(head_gaze) * cv::norm(head_gaze_straight)));
	head_gaze_away = head_gaze_away * 180 / 3.14159265359;

	// if the gaze away if over 20 degrees assume the person is not looking

	// See if the gaze vector is within range
	
	// TODO eye model isn't great, something seems to be off

	// Get a joint gaze vector from both eyes
	Point3d eye_gaze(gaze_left.x + gaze_right.x, gaze_left.y + gaze_right.y, gaze_left.z + gaze_right.z);
	eye_gaze = eye_gaze / cv::norm(eye_gaze);
	
	// The ideal eye gaze direction if looking straight into the camera	
	Point3d eye_gaze_straight(-pose[0], -pose[1], -pose[2]);
	eye_gaze_straight = eye_gaze_straight / cv::norm(eye_gaze_straight);

	// Look if eye gaze is towards the camera 
	double eye_gaze_away = acos(eye_gaze.dot(eye_gaze_straight));
	eye_gaze_away = eye_gaze_away * 180 / 3.14159265359;

	head_attention = 1.0 - head_gaze_away / head_attention_scaling;
	eye_attention = 1.0 - eye_gaze_away / gaze_attention_scaling;

	auto aus_reg = face_analyser.GetCurrentAUsReg();

	double AU1 = 0;
	double AU2 = 0;
	double AU4 = 0;
	double AU6 = 0;
	double AU12 = 0;
	double AU15 = 0;
	double AU17 = 0;
	double AU25 = 0;

	// Find AUs of interest
	for (auto au_reg : aus_reg)
	{
		if (au_reg.first.compare("AU01") == 0)
		{
			AU1 = au_reg.second;
		}
		if (au_reg.first.compare("AU02") == 0)
		{
			AU2 = au_reg.second;
		}
		if (au_reg.first.compare("AU04") == 0)
		{
			AU4 = au_reg.second;
		}
		if (au_reg.first.compare("AU06") == 0)
		{
			AU6 = au_reg.second;
		}
		if (au_reg.first.compare("AU12") == 0)
		{
			AU12 = au_reg.second;
		}
		if (au_reg.first.compare("AU15") == 0)
		{
			AU15 = au_reg.second;
		}
		if (au_reg.first.compare("AU17") == 0)
		{
			AU17 = au_reg.second;
		}
		if (au_reg.first.compare("AU25") == 0)
		{
			AU25 = au_reg.second;
		}
	}
	
	AU1_history.push_back(AU1);
	AU2_history.push_back(AU2);
	AU4_history.push_back(AU4);
	AU6_history.push_back(AU6);
	AU12_history.push_back(AU12);
	AU15_history.push_back(AU15);
	AU17_history.push_back(AU17);
	AU25_history.push_back(AU25);
	time_step_history.push_back(time);

	if (time - *time_step_history.begin() > 500)
	{
		AU1_history.erase(AU1_history.begin());
		AU2_history.erase(AU2_history.begin());
		AU4_history.erase(AU4_history.begin());
		AU6_history.erase(AU6_history.begin());
		AU12_history.erase(AU12_history.begin());
		AU15_history.erase(AU15_history.begin());
		AU17_history.erase(AU17_history.begin());
		AU25_history.erase(AU25_history.begin());
		time_step_history.erase(time_step_history.begin());
	}

	double min_au1, max_au1;
	cv::minMaxLoc(cv::Mat(std::vector<double>(AU1_history)), &min_au1, &max_au1);
	double min_au2, max_au2;
	cv::minMaxLoc(cv::Mat(std::vector<double>(AU2_history)), &min_au2, &max_au2);
	double brow_flash = 0.5 * ((max_au1 - min_au1) + (max_au2 - min_au2));

	double min_au4, max_au4;
	cv::minMaxLoc(cv::Mat(std::vector<double>(AU4_history)), &min_au4, &max_au4);
	double brow_furrow = max_au4 - min_au4;

	double min_au6, max_au6;
	cv::minMaxLoc(cv::Mat(std::vector<double>(AU6_history)), &min_au6, &max_au6);
	double min_au12, max_au12;
	cv::minMaxLoc(cv::Mat(std::vector<double>(AU12_history)), &min_au12, &max_au12);
	double smile = 0.5 * ((max_au6 - min_au6) + (max_au12 - min_au12));

	double min_au15, max_au15;
	cv::minMaxLoc(cv::Mat(std::vector<double>(AU15_history)), &min_au15, &max_au15);
	double min_au17, max_au17;
	cv::minMaxLoc(cv::Mat(std::vector<double>(AU17_history)), &min_au17, &max_au17);
	double frown = ((max_au15 - min_au15) + (max_au17 - min_au17));

	double min_au25, max_au25;
	cv::minMaxLoc(cv::Mat(std::vector<double>(AU25_history)), &min_au25, &max_au25);
	double talk = max_au25 - min_au25;

	// Have some smoothing
	speech = 0.2 * speech +  0.8 * talk / 5.0;

	if (speech < speech_threshold)
		speech = 0;

	// Two options accomulating model or a direct mapping

	double cummulator = 0;
	double cummulator_valence = 0;
	double cummulator_attention = 0;

	if (head_gaze_away > head_gaze_threshold)
	{
		cummulator = cummulator - rapport_rate_of_change * head_gaze_change_rate_neg;
		cummulator_attention = cummulator_attention - rapport_rate_of_change * head_gaze_change_rate_neg;
	}
	else
	{
		cummulator_attention = cummulator_attention + rapport_rate_of_change / head_gaze_change_rate_neg;
	}

	if (eye_gaze_away > eye_gaze_threshold)
	{
		cummulator = cummulator - rapport_rate_of_change * eye_gaze_change_rate_neg;
		cummulator_attention = cummulator_attention - rapport_rate_of_change * eye_gaze_change_rate_neg;
	}
	else
	{
		cummulator_attention = cummulator_attention + rapport_rate_of_change / eye_gaze_change_rate_pos;
	}

	if ((AU12 > 1.5 && AU6 > 0.5) || smile > 1.5)
	{
		double smile_int = (AU12 + AU25 + AU6) / 15.0;
		// Look at smiling behaviour (AU12 + AU6)
		cummulator = cummulator + rapport_rate_of_change * smile_rate_of_change_rapport * smile_int;
		cummulator_valence = cummulator_valence + rapport_rate_of_change * smile_rate_of_change_valence * smile_int;
	}
	else if (current_valence > 4.5)
	{
		cummulator_valence = cummulator_valence - rapport_rate_of_change * valence_return_to_neutral_rate;
	}

	// If brows are being raised (poss showing interest)
	//if (AU1 > 2.0 || AU2 > 2.0)
	if (brow_flash > 2)
	{
		// Look at smiling behaviour (AU1 + AU2)
		cummulator = cummulator + rapport_rate_of_change * brow_flash_rapport_change;
	}

	if (AU4 > 2.5 || brow_furrow > 2)
	{
		// Look at frowning behaviour (AU4 + AU15 and AU17)
		cummulator = cummulator + rapport_rate_of_change * brow_furrow_rapport_change;
		cummulator_valence = cummulator_valence + rapport_rate_of_change * brow_furrow_valence_change;
	}

	if (AU15 > 2 || AU17 > 2 || frown > 1.5)
	{
		// Look at frowning behaviour (AU4 + AU15 and AU17)
		cummulator = cummulator - rapport_rate_of_change * frown_rate_of_change_rapport;
		cummulator_valence = cummulator_valence - rapport_rate_of_change * frown_rate_of_change_valence;
	}
	else if (current_valence < 3.5)
	{
		cummulator_valence = cummulator_valence + rapport_rate_of_change * valence_return_to_neutral_rate;
	}

	// Add the arousal observation (only with positive valence though)
	if(current_arousal > 0.5 && current_valence > 3.5)
	{
		cummulator = cummulator + (current_arousal - 0.5) * arousal_rapport_affect_pos;
	}
	else
	{
		cummulator = cummulator - (0.5 - current_arousal) * arousal_rapport_affect_neg;
	}

	// Looks if lips are not moving (AU25)
	if (talk > speech_rapport_threshold)
	{
		cummulator = cummulator + rapport_rate_of_change * speech_rapport_rate;
	}

	double time_passed = time - prev_time_step;

	if (time_passed > 0)
	{
		cummulator = cummulator * time_passed / 75.0;
		cummulator_attention = cummulator_attention * time_passed / 75.0;
		cummulator_valence = cummulator_valence * time_passed / 75.0;
	}

	// Do not let a single frame change by more that 0.5
	if (cummulator > max_rate_of_change)
		cummulator = max_rate_of_change;

	if (cummulator < -max_rate_of_change)
		cummulator = -max_rate_of_change;

	// Some smoothing
	double old_rapport = current_rapport;
	double old_attention = current_attention;
	double old_valence = current_valence;

	current_rapport = smoothing_factor * current_rapport + (1.0-smoothing_factor) * (old_rapport + cummulator);
	current_attention = smoothing_factor * current_attention + (1.0-smoothing_factor) * (old_attention + cummulator_attention);
	current_valence = smoothing_factor * current_valence + (1.0-smoothing_factor) * (old_valence + cummulator_valence);

	if (current_rapport < min_rapport)
		current_rapport = min_rapport;
	if (current_rapport > max_rapport)
		current_rapport = max_rapport;

	if (current_attention < 1)
		current_attention = 1;
	if (current_attention > 7)
		current_attention = 7;

	if (current_valence < 1)
		current_valence = 1;
	if (current_valence > 7)
		current_valence = 7;

	prev_time_step = time;

	frames_tracking++;

}

double RapportAnalyser::GetAttentionEstimate()
{

	return current_attention;

}

double RapportAnalyser::GetEyeAttention()
{
	return eye_attention;
}

double RapportAnalyser::GetHeadAttention()
{
	return head_attention;
}

double RapportAnalyser::GetValenceEstimate()
{
	return current_valence;
}

double RapportAnalyser::GetArousalEstimate()
{
	return current_arousal;
}

double RapportAnalyser::GetSpeech()
{
	return speech;
}

double RapportAnalyser::GetRapportEstimate()
{

	return current_rapport;

}

string RapportAnalyser::GetAllContent()
{
	stringstream all_content;
	all_content << current_rapport << "," << current_arousal << "," << current_valence << "," << speech << "," << eye_attention << "," << head_attention << "," << GetAttentionEstimate();
	return all_content.str();
}


