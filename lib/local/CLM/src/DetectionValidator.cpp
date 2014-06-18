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

#include <DetectionValidator.h>

#include <highgui.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "CLM_utils.h"

using namespace CLMTracker;

//===========================================================================
// Read in the landmark detection validation module
void DetectionValidator::Read(string location)
{

	ifstream detection_validator_stream(location);
	if(!detection_validator_stream.is_open())
	{
		cout << "WARNING: Can't find the Face checker location" << endl;
	}

	CLMTracker::SkipComments(detection_validator_stream);
	
	// Read the number of views (orientations) within the validator
	int n;
	detection_validator_stream >> n;
	
	CLMTracker::SkipComments(detection_validator_stream);

	orientations.resize(n);
	for(int i = 0; i < n; i++)
	{
		Mat_<double> orientation_tmp;
		CLMTracker::ReadMat(detection_validator_stream, orientation_tmp);		
		
		orientations[i] = Vec3d(orientation_tmp.at<double>(0), orientation_tmp.at<double>(1), orientation_tmp.at<double>(2));

		// Convert from degrees to radians
		orientations[i] = orientations[i] * M_PI / 180.0;
	}

	// Initialise the piece-wise affine warps, biases and weights
	paws.resize(n);
	bs.resize(n);
	ws.resize(n);

	// Initialise the normalisation terms
	mean_images.resize(n);
	standard_deviations.resize(n);

	// Read in the validators for each of the views
	for(int i = 0; i < n; i++)
	{
		CLMTracker::SkipComments(detection_validator_stream);
		
		// Read in the principal components for the dimensionality reduction
		Mat_<double> principal_components;

		CLMTracker::ReadMat(detection_validator_stream, principal_components);

		CLMTracker::SkipComments(detection_validator_stream);
		CLMTracker::ReadMat(detection_validator_stream, mean_images[i]);
		mean_images[i] = mean_images[i].t();
	
		CLMTracker::SkipComments(detection_validator_stream);
		CLMTracker::ReadMat(detection_validator_stream, standard_deviations[i]);
		standard_deviations[i] = standard_deviations[i].t();

		// Reading in the biases and standard deviations
		CLMTracker::SkipComments(detection_validator_stream);
		detection_validator_stream >> bs[i];
		CLMTracker::ReadMat(detection_validator_stream, ws[i]);
	
		// Collapse the weight with the principal components for speed
		ws[i] = ws[i].t() * principal_components.t();

		// Read in the piece-wise affine warps
		paws[i].Read(detection_validator_stream);
	}

}

//===========================================================================
// Check if the fitting actually succeeded
double DetectionValidator::Check(const Vec3d& orientation, const Mat_<uchar>& intensity_img, Mat_<double>& detected_landmarks)
{

	int id = GetViewId(orientation);
	
	// The warped (cropped) image, corresponding to a face lying withing the detected lanmarks
	Mat_<uchar> warped;
	
	// the piece-wise affine image
	paws[id].Warp(intensity_img, warped, detected_landmarks);	
		
	Mat_<uchar> warped_t = warped.t();

	
	// the vector to be filled with paw values
	cv::MatIterator_<double> vp;	
	cv::MatIterator_<uchar>  cp;

	cv::Mat_<double> vec(paws[id].number_of_pixels,1);
	vp = vec.begin();

	cp = warped_t.begin();		

	int wInt = warped.cols;
	int hInt = warped.rows;

	// the mask indicating if point is within or outside the face region
	cv::Mat maskT = paws[id].pixel_mask.t();

	cv::MatIterator_<uchar>  mp = maskT.begin<uchar>();

	for(int i=0; i < wInt; ++i)
	{
		for(int j=0; j < hInt; ++j, ++mp, ++cp)
		{
			// if is within mask
			if(*mp)
			{
				*vp++ = (double)*cp;
			}
		}
	}

	cv::Scalar mean;
	cv::Scalar std;
	cv::meanStdDev(vec, mean, std);

	// subtract the mean image
	vec -= mean[0];

	// Normalise the image
	if(std[0] == 0)
	{
		std[0] = 1;
	}
	
	vec /= std[0];

	vec = (vec - mean_images[id])  / standard_deviations[id];
	
	double dec = (ws[id].dot(vec.t()) + bs[id]);

	return dec;
}

// Getting the closest view center based on orientation
int DetectionValidator::GetViewId(const cv::Vec3d& orientation) const
{
	int id = 0;

	double dbest = -1.0;

	for(size_t i = 0; i < this->orientations.size(); i++)
	{
	
		// Distance to current view
		double d = cv::norm(orientation, this->orientations[i]);

		if(i == 0 || d < dbest)
		{
			dbest = d;
			id = i;
		}
	}
	return id;
	
}


