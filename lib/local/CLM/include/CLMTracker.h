///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012, Tadas Baltrusaitis, all rights reserved.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
//     * The software is provided under the terms of this licence stricly for
//       academic, non-commercial, not-for-profit purposes.
//     * Redistributions of source code must retain the above copyright notice, 
//       this list of conditions (licence) and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright 
//       notice, this list of conditions (licence) and the following disclaimer 
//       in the documentation and/or other materials provided with the 
//       distribution.
//     * The name of the author may not be used to endorse or promote products 
//       derived from this software without specific prior written permission.
//     * As this software depends on other libraries, the user must adhere to 
//       and keep in place any licencing terms of those libraries.
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
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED 
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO 
// EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///////////////////////////////////////////////////////////////////////////////


//  Header for all external CLM methods of interest to the user
//
//
//  Tadas Baltrusaitis
//  01/05/2012

#ifndef __CLM_TRACKER_h_
#define __CLM_TRACKER_h_

#include <CLMParameters.h>
#include <CLM_utils.h>

#include <CLM.h>

#include <cv.h>

#include <iostream>

using namespace std;
using namespace cv;

namespace CLMTracker
{

	//================================================================================================================
	// Landmark detection in videos, need to provide an image and model parameters (default values work well)
	// Optionally can provide a bounding box from which to start tracking
	//================================================================================================================
	bool DetectLandmarksInVideo(const Mat_<uchar> &grayscale_image, CLM& clm_model, CLMParameters& params);
	bool DetectLandmarksInVideo(const Mat_<uchar> &grayscale_image, const Mat_<float> &depth_image, CLM& clm_model, CLMParameters& params);

	bool DetectLandmarksInVideo(const Mat_<uchar> &grayscale_image, const Rect_<double> bounding_box, CLM& clm_model, CLMParameters& params);
	bool DetectLandmarksInVideo(const Mat_<uchar> &grayscale_image, const Mat_<float> &depth_image, const Rect_<double> bounding_box, CLM& clm_model, CLMParameters& params);

	//================================================================================================================
	// Landmark detection in image, need to provide an image and optionally CLM model together with parameters (default values work well)
	// Optionally can provide a bounding box in which detection is performed (this is useful if multiple faces are to be detected in images)
	//================================================================================================================
	bool DetectLandmarksInImage(const Mat_<uchar> &grayscale_image, CLM& clm_model, CLMParameters& params);
	// Providing a bounding box
	bool DetectLandmarksInImage(const Mat_<uchar> &grayscale_image, const Rect_<double> bounding_box, CLM& clm_model, CLMParameters& params);

	//================================================
	// CLM-Z versions
	bool DetectLandmarksInImage(const Mat_<uchar> &grayscale_image, const Mat_<float> depth_image, CLM& clm_model, CLMParameters& params);
	bool DetectLandmarksInImage(const Mat_<uchar> &grayscale_image, const Mat_<float> depth_image, const Rect_<double> bounding_box, CLM& clm_model, CLMParameters& params);

	//================================================================
	// Helper function for getting head pose from CLM parameters

	// The head pose returned is in camera space, however, the orientation can be either with respect to camera itself or the camera plane
	// The format returned is [Tx, Ty, Tz, Eul_x, Eul_y, Eul_z]
	Vec6d GetPoseCamera(CLM& clm_model, double fx, double fy, double cx, double cy, CLMParameters& params);
	Vec6d GetPoseCameraPlane(CLM& clm_model, double fx, double fy, double cx, double cy, CLMParameters& params);
	
	// Getting a head pose estimate from the currently detected landmarks, with appropriate correction due to orthographic camera issue
	// This is because rotation estimate under orthographic assumption is only correct close to the centre of the image
	// These methods attempt to correct for that (Experimental)
	// The format returned is [Tx, Ty, Tz, Eul_x, Eul_y, Eul_z]
	Vec6d GetCorrectedPoseCamera(CLM& clm_model, double fx, double fy, double cx, double cy, CLMParameters& params);
	Vec6d GetCorrectedPoseCameraPlane(CLM& clm_model, double fx, double fy, double cx, double cy, CLMParameters& params);

	//===========================================================================

}
#endif
