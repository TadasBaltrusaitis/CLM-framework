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
RapportAnalyser::RapportAnalyser() :rapport_history()
{
	// start with average rapport first
	current_rapport = 4;
}

void RapportAnalyser::AddObservation(const CLMTracker::CLM& clm_model, const FaceAnalyser& face_analyser, const Point3f& gaze_left, const Point3f& gaze_right, double fx, double fy, double cx, double cy)
{

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

	double AU4 = 0;
	double AU6 = 0;
	double AU12 = 0;
	double AU15 = 0;
	double AU17 = 0;
	double AU25 = 0;

	// Find AUs of interest
	for (auto au_reg : aus_reg)
	{
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

	//cout << (int)(AU4 * 10) << " " << (int)(AU6 * 10) << " " << (int)(AU12 * 10) << " " << (int)(AU15 * 10) << " " << (int)(AU17 * 10) << " " << (int)(AU25 * 10) << endl;

	// Look at smiling behaviour (AU12 + AU6)
	// Look at frowning behaviour (AU4 + AU15 and AU17)

	// Looks if lips are not moving (AU25)

	// Two options accomulating model or a direct mapping

	double cummulator = 0;
	double add_size = 0.1;
	if (head_gaze_away > 15)
	{
		cummulator = cummulator - add_size / 4.0;
	}
	if (eye_gaze_away > 7)
	{
		cummulator = cummulator - add_size / 4.0;
	}

	if (AU12 > 1.5 && AU6 > 1.5)
	{
		cummulator = cummulator + add_size * 2.0;
	}

	if (AU4 > 2.5)
	{
		cummulator = cummulator - add_size;
	}

	if (AU15 > 2 || AU17 > 2)
	{
		cummulator = cummulator - add_size * 2.0;
	}

	if (AU25 > 2.5)
	{

		cummulator = cummulator + add_size / 2.0;
	}

	// Do not let a single frame change by more that 0.5
	if (cummulator > 0.5)
		cummulator = 0.5;

	if (cummulator < -0.5)
		cummulator = -0.5;

	current_rapport += cummulator;

	if (current_rapport < 1)
		current_rapport = 1;
	if (current_rapport > 7)
		current_rapport = 7;
	//cout << (current_rapport - 1.0) / 6.0<< endl;
}

double RapportAnalyser::GetRapportEstimate()
{

	return current_rapport;

}


