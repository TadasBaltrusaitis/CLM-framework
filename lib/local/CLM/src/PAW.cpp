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

#include <PAW.h>

#include <cv.h>
#include <highgui.h>

#include "CLM_utils.h"

using namespace CLMTracker;

using namespace cv;

//===========================================================================
void PAW::Read(std::ifstream& stream)
{
	CLMTracker::SkipComments(stream);
	stream >> number_of_pixels >> min_x >> min_y;

	CLMTracker::SkipComments(stream);
	CLMTracker::ReadMat(stream, destination_landmarks);

	CLMTracker::SkipComments(stream);
	CLMTracker::ReadMat(stream, triangulation);

	CLMTracker::SkipComments(stream);
	CLMTracker::ReadMat(stream, triangle_id);
	
	cv::Mat tmpMask;
	CLMTracker::SkipComments(stream);		
	CLMTracker::ReadMat(stream, tmpMask);	
	tmpMask.convertTo(pixel_mask, CV_8U);	

	CLMTracker::SkipComments(stream);
	CLMTracker::ReadMat(stream, alpha);

	CLMTracker::SkipComments(stream);
	CLMTracker::ReadMat(stream, beta);

	map_x.create(pixel_mask.rows,pixel_mask.cols);
	map_y.create(pixel_mask.rows,pixel_mask.cols);

	coefficients.create(this->NumberOfTriangles(),6);
	
	source_landmarks = destination_landmarks;
}

//=============================================================================
// cropping from the source image to the destination image using the shape in s, used to determine if shape fitting converged successfully
void PAW::Warp(const Mat_<uchar>& image_to_warp, Mat_<uchar>& destination_image, Mat_<double>& landmarks_to_warp)
{
  
	// set the current shape
	source_landmarks = landmarks_to_warp;

	// prepare the mapping coefficients using the current shape
	this->CalcCoeff();

	// Do the actual mapping computation (where to warp from)
	this->WarpRegion(map_x, map_y);
  
	// Do the actual warp (with bi-linear interpolation)
	remap(image_to_warp, destination_image, map_x, map_y, CV_INTER_LINEAR);
  
}


//=============================================================================
// Calculate the warping coefficients
void PAW::CalcCoeff()
{
	int p = this->NumberOfLandmarks();

	for(int l = 0; l < this->NumberOfTriangles(); l++)
	{
	  
		int i = triangulation.at<int>(l,0);
		int j = triangulation.at<int>(l,1);
		int k = triangulation.at<int>(l,2);

		double c1 = source_landmarks.at<double>(i    , 0);
		double c2 = source_landmarks.at<double>(j    , 0) - c1;
		double c3 = source_landmarks.at<double>(k    , 0) - c1;
		double c4 = source_landmarks.at<double>(i + p, 0);
		double c5 = source_landmarks.at<double>(j + p, 0) - c4;
		double c6 = source_landmarks.at<double>(k + p, 0) - c4;

		// Get a pointer to the coefficient we will be precomputing
		double *coeff = coefficients.ptr<double>(l);

		// Extract the relevant alphas and betas
		double *c_alpha = alpha.ptr<double>(l);
		double *c_beta  = beta.ptr<double>(l);

		coeff[0] = c1 + c2 * c_alpha[0] + c3 * c_beta[0];
		coeff[1] =      c2 * c_alpha[1] + c3 * c_beta[1];
		coeff[2] =      c2 * c_alpha[2] + c3 * c_beta[2];
		coeff[3] = c4 + c5 * c_alpha[0] + c6 * c_beta[0];
		coeff[4] =      c5 * c_alpha[1] + c6 * c_beta[1];
		coeff[5] =      c5 * c_alpha[2] + c6 * c_beta[2];
	}
}

//======================================================================
// Compute the mapping coefficients
void PAW::WarpRegion(Mat_<float>& mapx, Mat_<float>& mapy)
{
	
	cv::MatIterator_<float> xp = mapx.begin();
	cv::MatIterator_<float> yp = mapy.begin();
	cv::MatIterator_<uchar> mp = pixel_mask.begin();
	cv::MatIterator_<int>   tp = triangle_id.begin();
	
	// The coefficients corresponding to the current triangle
	double * a;

	// Current triangle being processed	
	int k=-1;

	for(int y = 0; y < pixel_mask.rows; y++)
	{
		double yi = double(y) + min_y;
	
		for(int x = 0; x < pixel_mask.cols; x++)
		{
			double xi = double(x) + min_x;

			if(*mp == 0)
			{
				*xp = -1;
				*yp = -1;
			}
			else
			{
				// triangle corresponding to the current pixel
				int j = *tp;

				// If it is different from the previous triangle point to new coefficients
				// This will always be the case in the first iteration, hence a will not point to nothing
				if(j != k)
				{
					// Update the coefficient pointer if a new triangle is being processed
					a = coefficients.ptr<double>(j);			
					k = j;
				}  	

				//ap is now the pointer to the coefficients
				double *ap = a;							

				//look at the first coefficient (and increment). first coefficient is an x offset
				double xo = *ap++;						
				//second coefficient is an x scale as a function of x
				xo += *ap++ * xi;						
				//third coefficient ap(2) is an x scale as a function of y
				*xp = float(xo + *ap++ * yi);			

				//then fourth coefficient ap(3) is a y offset
				double yo = *ap++;						
				//fifth coeff adds coeff[4]*x to y
				yo += *ap++ * xi;						
				//final coeff adds coeff[5]*y to y
				*yp = float(yo + *ap++ * yi);			

			}
			mp++; tp++; xp++; yp++;	
		}
	}
	
}