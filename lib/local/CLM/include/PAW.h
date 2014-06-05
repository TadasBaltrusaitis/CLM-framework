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

#ifndef __PAW_h_
#define __PAW_h_

#include <cv.h>
#include <string>

#include <stdio.h>
#include <fstream>
#include <iostream>

using namespace cv;

void parsecolour(cv::Mat image);
void ParseToPAW(cv::Mat shape,cv::Mat localshape, cv::Mat global);
void sendAvatarFile(string filename);
void sendAvatar(cv::Mat avatarHead, cv::Mat avatarShape);


namespace CLMTracker
{
  //===========================================================================
  /** 
      A Piece-wise Affine Warp
	  The ideas for this piece-wise affine triangular warping are taken from the
	  Active appearance models revisited by Iain Matthews and Simon Baker in IJCV 2004
	  This is used for both validation of landmark detection, and for avatar animation

	  The code is based on the CLM tracker by Jason Saragih et al.
  */	

class PAW{
public:    
	// Number of pixels after the warping to neutral shape
    int     number_of_pixels; 

	// Minimum x coordinate in destination
    double  min_x;

	// minimum y coordinate in destination
    double  min_y;

	// Destination points (landmarks to be warped to)
    Mat_<double> destination_landmarks;

	// Destination points (landmarks to be warped from)
    Mat_<double> source_landmarks;

	// Triangulation, each triangle is warped using an affine transform
    Mat_<int> triangulation;    

	// Triangle index, indicating which triangle each of destination pixels lies in
    Mat_<int> triangle_id;  

	// Indicating if the destination warped pixels is valid (lies within a face)
	Mat_<uchar> pixel_mask;

	// A number of precomputed coefficients that are helpful for quick warping
	
	// affine coefficients for all triangles (see Matthews and Baker 2004)
	// 6 coefficients for each triangle (are computed from alpha and beta)
	// This is computed during each warp based on source landmarks
    Mat_<double> coefficients;

	// matrix of (c,x,y) coeffs for alpha
    Mat_<double> alpha;  

	// matrix of (c,x,y) coeffs for alpha
    Mat_<double> beta;   

	// x-source of warped points
    Mat_<float> map_x;   

	// y-source of warped points
    Mat_<float> map_y;   

	// Default constructor
    PAW(){;}

	// Copy constructor
	PAW(const PAW& other): destination_landmarks(other.destination_landmarks.clone()), source_landmarks(other.source_landmarks.clone()), triangulation(other.triangulation.clone()),
		triangle_id(other.triangle_id.clone()), pixel_mask(other.pixel_mask.clone()), coefficients(other.coefficients.clone()), alpha(other.alpha.clone()), beta(other.beta.clone()), map_x(other.map_x.clone()), map_y(other.map_y.clone())
	{
		this->number_of_pixels = other.number_of_pixels; 
		this->min_x = other.min_x;
		this->min_y = other.min_y;
	}

	void Read(std::ifstream &s);

	// The actual warping
    void Warp(const Mat_<uchar>& image_to_warp, Mat_<uchar>& destination_image, Mat_<double>& landmarks_to_warp);
	
	// Compute coefficients needed for warping
    void CalcCoeff();

	// Perform the actual warping
    void WarpRegion(Mat_<float>& map_x, Mat_<float>& map_y);

    inline int NumberOfLandmarks(){return destination_landmarks.rows/2;}
    inline int NumberOfTriangles(){return triangulation.rows;}

	// The width and height of the warped image
    inline int Width(){return pixel_mask.cols;}
    inline int Height(){return pixel_mask.rows;}
    
/*********************START Needed for avatar reconstruction:*********************/
// Written by Leonardo Impett
	//also triangulation is used
	cv::Mat snew;
	cv::Mat _neutralshape;
	void WarpToNeutral(cv::Mat &image, cv::Mat &shape, cv::Mat &neutralshape);
    void unCalcCoeff(cv::Mat &s, cv::Mat &snew);
    void unWarpRegion(cv::Mat &mapx,cv::Mat &mapy);

/*********************END   Needed for avatar reconstruction:**********************/

  };
  //===========================================================================
}
#endif
