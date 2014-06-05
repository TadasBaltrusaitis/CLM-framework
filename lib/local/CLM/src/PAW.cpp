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

bool OPENCVWarp = false;			//this is the boolean that does the warping inside OPENCV (in this program). 
//Otherwise, the warping to neutral is done within this program, but the warping onto the map is done by the GPU with OpenGL

using namespace CLMTracker;

using namespace cv;

bool quit = false;

double oldtime = 0;

string avatarfile;
bool readfile = false;

Mat savatar;
Mat _tridx2(600, 600, CV_32S, Scalar(0));  /**<		...in the output (animated)image			*/
Mat colourimage;
Mat avatarlocal;
Mat warpedbackface;

Mat avatarWarpedHead, avatarWarpleft, avatarWarpright;
Mat avatarS, avatarSleft, avatarSright;
Mat avatarNextDst;

void sendAvatar(Mat avatarHead, Mat avatarShape){

	cvtColor(avatarHead, avatarWarpedHead, CV_BGR2RGB); 
	avatarS = avatarShape;


}

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

void sendAvatarFile(string filename){
	avatarfile = filename;
	readfile = true;
};


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

/*********************START Needed for avatar reconstruction:*********************/
// Written by Leonardo Impett

//=============================================================================


void PAW::WarpToNeutral(Mat &image, Mat &shape, Mat &neutralshape){

	Mat newimage;
	this->unCalcCoeff(shape, neutralshape);
	Mat bigmapx,bigmapy;
	this->unWarpRegion(map_x,map_y);
	resize(map_x, bigmapx, Size(128,128), 0, 0, INTER_LINEAR);
	resize(map_y, bigmapy, Size(128,128), 0, 0, INTER_LINEAR);
	remap(image,image,bigmapx,bigmapy,CV_INTER_LINEAR);

}

//=============================================================================
void PAW::unCalcCoeff(cv::Mat &s, cv::Mat &snew)
{
	
	cvtColor(colourimage, colourimage, CV_BGR2RGB); 

	int p = (s.rows/2);
	
	Mat afftrans, invafftrans, trianglematrix;
			
	int numl = this->NumberOfTriangles();
	
	Mat backgroundimage = Mat::zeros(colourimage.size(),CV_8UC3);

	for(int l = 0; l < numl; l++)
	{
	  
		int i = triangulation.at<int>(l,0);								//find the indices of the triangle in question
		int j = triangulation.at<int>(l,1);
		int k = triangulation.at<int>(l,2);

		double c1 = s.at<double>(i  ,0);							//i.x		Look up the current shape values in s
		double c2 = s.at<double>(j  ,0) - c1;						//j.x-i.x		these bits for X
		double c3 = s.at<double>(k  ,0) - c1;						//k.x-i.x		(relative shape)
		double c4 = s.at<double>(i+p,0);							//i.y		these bits for Y
		double c5 = s.at<double>(j+p,0) - c4;						//j.y-i.y
		double c6 = s.at<double>(k+p,0) - c4;						//k.y-i.y

		double *coeff = coefficients.ptr<double>(l);					//look up the alpha, beta values 
		double *c_alpha = alpha.ptr<double>(l);
		double *c_beta  = beta.ptr<double>(l);

		coeff[0] = c1 + c2 * c_alpha[0] + c3 * c_beta[0];
		coeff[1] =      c2 * c_alpha[1] + c3 * c_beta[1];
		coeff[2] =      c2 * c_alpha[2] + c3 * c_beta[2];
		coeff[3] = c4 + c5 * c_alpha[0] + c6 * c_beta[0];
		coeff[4] =      c5 * c_alpha[1] + c6 * c_beta[1];
		coeff[5] =      c5 * c_alpha[2] + c6 * c_beta[2];

		double d1 = coeff[0];
		double d2 = coeff[1];
		double d3 = coeff[2];
		double d4 = coeff[3];
		double d5 = coeff[4];
		double d6 = coeff[5];

		//these coefficients coeff[1]...coeff[6] are used to generate an INVERSE MAP, ie, in mapping the large image to the small image these coefficients create a map of the size of the smaller image...
		//... within which the indices to be read from the larger image are looked up. 

		if(i <= p)
		{

			bool AVATARWITHOPENCV = true;

			if(AVATARWITHOPENCV)
			{
				afftrans = (Mat_<double>(2, 3) << d2,d3,d1,	d5,d6,d4);
	 
	 			invertAffineTransform(afftrans, invafftrans);
				d2 = invafftrans.at<double>(0,0);
				d3 = invafftrans.at<double>(0,1);
				d1 = invafftrans.at<double>(0,2);
				d5 = invafftrans.at<double>(1,0);
				d6 = invafftrans.at<double>(1,1);
				d4 = invafftrans.at<double>(1,2);
		
	 			snew.at<double>(i,0) = d1 + (d2*s.at<double>(i,0)) + (d3*s.at<double>(i+p,0));
				snew.at<double>(i+p,0) = d4 + (d5*s.at<double>(i,0)) + (d6*s.at<double>(i+p,0));

	 			snew.at<double>(j,0) = d1 + (d2*s.at<double>(j,0)) + (d3*s.at<double>(j+p,0));
				snew.at<double>(j+p,0) = d4 + (d5*s.at<double>(j,0)) + (d6*s.at<double>(j+p,0));

	 			snew.at<double>(k,0) = d1 + (d2*s.at<double>(k,0)) + (d3*s.at<double>(k+p,0));
				snew.at<double>(k+p,0) = d4 + (d5*s.at<double>(k,0)) + (d6*s.at<double>(k+p,0));
		
				if(!avatarWarpedHead.empty() && OPENCVWarp)
				{
		
					Point2f srcpoints[3], dstpoints[3];
					srcpoints[0] = Point2f((float)s.at<double>(i,0), (float)s.at<double>(i+p,0));
					srcpoints[1] = Point2f((float)s.at<double>(j,0), (float)s.at<double>(j+p,0));
					srcpoints[2] = Point2f((float)s.at<double>(k,0), (float)s.at<double>(k+p,0));
		
					cv::Point pts[3] = {Point((int)avatarS.at<double>(i,0), (int)avatarS.at<double>(i+p,0)),
						                Point((int)avatarS.at<double>(j,0), (int)avatarS.at<double>(j+p,0)),
										Point((int)avatarS.at<double>(k,0), (int)avatarS.at<double>(k+p,0)) };
	
					float ox = 0.0; 
					float oy = 0.0;

					dstpoints[0] = Point2f((float)avatarS.at<double>(i,0)+ox, (float)avatarS.at<double>(i+p,0)+oy);
					dstpoints[1] = Point2f((float)avatarS.at<double>(j,0)+ox, (float)avatarS.at<double>(j+p,0)+oy);
					dstpoints[2] = Point2f((float)avatarS.at<double>(k,0)+ox, (float)avatarS.at<double>(k+p,0)+oy);


					Mat warp_mat = 	getAffineTransform(dstpoints,srcpoints);
		
					Mat newimage; 
					Mat mask = Mat::zeros(avatarWarpedHead.size(),CV_8UC3);
					Mat maskedimage;

					fillConvexPoly(mask, pts, 3, Scalar(255,255,255), 8, 0);
					avatarWarpedHead.copyTo(maskedimage, mask);
					warpAffine(maskedimage, newimage, warp_mat, backgroundimage.size(), 1, 0, 0);
					backgroundimage = backgroundimage | newimage;
				}
			}
		}
	}

	if(!avatarWarpedHead.empty() && OPENCVWarp)
	{	
		Mat reversemask, greybackground;

		cvtColor(backgroundimage, greybackground, CV_RGB2GRAY);
		threshold(greybackground, reversemask, 1, 255, CV_THRESH_BINARY_INV );


		Mat newcolourimage;
		colourimage.copyTo(newcolourimage,reversemask);
		add(newcolourimage, backgroundimage, newcolourimage);
		imshow("colour image", newcolourimage);
	}
	
}

//=============================================================================

void ParseToPAW(cv::Mat shape,cv::Mat localshape, cv::Mat global){
	avatarlocal = localshape;
		savatar = shape;

}

//=============================================================================

void parsecolour(Mat image){
	image.copyTo(colourimage);
}

	


//===========================================================================


void PAW::unWarpRegion(cv::Mat &mapx,cv::Mat &mapy)
{
	assert((mapx.type() == CV_32F) && (mapy.type() == CV_32F));

	if((mapx.rows != pixel_mask.rows) || (mapx.cols != pixel_mask.cols))
		map_x.create(pixel_mask.rows,pixel_mask.cols);

	if((mapy.rows != pixel_mask.rows) || (mapy.cols != pixel_mask.cols))
		map_y.create(pixel_mask.rows,pixel_mask.cols);

	int x,y,j,k=-1;
	double yi,xi,xo,yo,*a=NULL,*ap;
	

	//cv::imshow("XmapBefore", mapx);
	//cv::imshow("YmapBefore", mapy);

	cv::MatIterator_<float> xp = mapx.begin<float>();
	cv::MatIterator_<float> yp = mapy.begin<float>();
	cv::MatIterator_<uchar> mp = pixel_mask.begin();
	cv::MatIterator_<int>   tp = triangle_id.begin();
	//cout << pixel_mask << endl << pixel_mask.type();
	//CV_8U

	

	//	cout << "y min = " << min_y << ", x min = " << min_x << endl;



	for(y = 0; y < pixel_mask.rows; y++)
	{
		yi = double(y) + min_y;

	


		for(x = 0; x < pixel_mask.cols; x++)
		{
			xi = double(x) + min_x;
			j = *tp;


			if(j == -1)
			{
				*xp = -10000;
				*yp = -10000;
			}
			else{
		
				
				if(j != k)
				{
					a = coefficients.ptr<double>(j);			//a is the pointer to the coefficients
					k = j;
				}  	
				ap = a;									//ap is now the pointer to the coefficients
				xo = *ap++;								//look at the first coefficient (and increment). first coefficient is an x offset
				xo += *ap++ * xi;						//second coefficient is an x scale as a function of x
				*xp = float(xo + *ap++ * yi);			//third coefficient ap(2) is an x scale as a function of y
				yo = *ap++;								//then fourth coefficient ap(3) is a y offset
				yo += *ap++ * xi;						//fifth coeff adds coeff[4]*x to y
				*yp = float(yo + *ap++ * yi);			//final coeff adds coeff[5]*y to y

			

			}
			
			mp++; tp++; xp++; yp++;						//cycle through. All this is used as a REVERSE map, so that the (30,30) pixel in the new (small) image looks at the (30,30) pixel in mapx and mapy, ...
			//... and reads from the original image at points (mapx(30,30),mapy(30,30)). 
		}
	}
	

	 //  cv::imshow("mapdif", mapx - mapy);
}