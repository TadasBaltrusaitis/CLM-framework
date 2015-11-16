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
RapportAnalyser::RapportAnalyser() :rapport_history(), time_step_history()
{
	// start with average rapport first
	current_rapport = 4;
	current_valence = 4;
	current_attention = 4;

	prev_time_step = 0;

}

void RapportAnalyser::AddObservation(const CLMTracker::CLM& clm_model, const FaceAnalyser& face_analyser, const Point3f& gaze_left, const Point3f& gaze_right, double fx, double fy, double cx, double cy)
{
	double time = face_analyser.GetCurrentTimeSeconds();
	
	if (!clm_model.detection_success)
		return;

	// TODO what head gaze should be based on the target which should be just below the camera
	// Although for mobile this might not be a huge issue

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

	//cout << head_gaze_away << " " << eye_gaze_away << " " << eye_gaze <<endl;

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

	//cout << brow_flash << " " << brow_furrow << " " << smile << " " << frown << " " << talk << endl;

	// Two options accomulating model or a direct mapping

	double cummulator = 0;
	double cummulator_valence = 0;
	double cummulator_attention = 0;

	double add_size = 0.02;
	if (head_gaze_away > 15)
	{
		cummulator = cummulator - add_size / 1.5;
		cummulator_attention = cummulator_attention - add_size / 1.5;
	}
	else
	{
		cummulator_attention = cummulator_attention + add_size / 3.0;
	}

	if (eye_gaze_away > 7)
	{
		cummulator = cummulator - add_size / 1.5;
		cummulator_attention = cummulator_attention - add_size / 1.5;
	}
	else
	{
		cummulator_attention = cummulator_attention + add_size / 3.0;
	}

	//if (AU12 > 1.5 && AU6 > 1.5)
	if ((AU12 > 1.5 && AU6 > 0.5) || smile > 1.5)
	{
		// Look at smiling behaviour (AU12 + AU6)
		cummulator = cummulator + add_size * 2.0;
		cummulator_valence = cummulator_valence + add_size * 3.0;
	}
	else if (current_valence > 5)
	{
		cummulator_valence = cummulator_valence - add_size * 1.5;
	}

	// If brows are being raised (poss showing interest)
	//if (AU1 > 2.0 || AU2 > 2.0)
	if (brow_flash > 2)
	{
		// Look at smiling behaviour (AU1 + AU2)
		cummulator = cummulator + add_size;
	}

	if (AU4 > 2.5 || brow_furrow > 2)
	{
		// Look at frowning behaviour (AU4 + AU15 and AU17)
		cummulator = cummulator - add_size;
		cummulator_valence = cummulator_valence - add_size;
	}

	if (AU15 > 2 || AU17 > 2 || frown > 1.5)
	{
		// Look at frowning behaviour (AU4 + AU15 and AU17)
		cummulator = cummulator - add_size * 2.0;
		cummulator_valence = cummulator_valence - add_size * 3.0;
	}

	// Looks if lips are not moving (AU25)
	if (talk > 2)
	{

		cummulator = cummulator + add_size / 2.0;
	}

	double time_passed = time - prev_time_step;

	if (time_passed > 0)
	{
		cummulator = cummulator * time_passed / 75.0;
		cummulator_attention = cummulator_attention * time_passed / 75.0;
		cummulator_valence = cummulator_valence * time_passed / 75.0;
	}

	// Do not let a single frame change by more that 0.5
	if (cummulator > 0.5)
		cummulator = 0.5;

	if (cummulator < -0.5)
		cummulator = -0.5;

	// Some smoothing
	double old_rapport = current_rapport;
	double old_attention = current_attention;
	double old_valence = current_valence;

	current_rapport = 0.3 * current_rapport + 0.7 * (old_rapport + cummulator);
	current_attention = 0.3 * current_attention + 0.7 * (old_attention + cummulator_attention);
	current_valence = 0.3 * current_valence + 0.7 * (old_valence + cummulator_valence);

	if (current_rapport < 1)
		current_rapport = 1;
	if (current_rapport > 7)
		current_rapport = 7;

	if (current_attention < 1)
		current_attention = 1;
	if (current_attention > 7)
		current_attention = 7;

	if (current_valence < 1)
		current_valence = 1;
	if (current_valence > 7)
		current_valence = 7;

	//cout << (current_rapport - 1.0) / 6.0 << " " << (current_valence - 1.0) / 6.0 << " " << (current_attention - 1.0) / 6.0 << endl;

	// TODO smoothing and incorporation of time steps
	// TODO lips parting has to look at the derivative or std?
	// Need a cummulator of each of the statistics?
	// for past 3 seconds

	prev_time_step = time;

}

double RapportAnalyser::GetAttentionEstimate()
{

	return current_attention;

}

double RapportAnalyser::GetValenceEstimate()
{

	return current_valence;

}

double RapportAnalyser::GetRapportEstimate()
{

	return current_rapport;

}


