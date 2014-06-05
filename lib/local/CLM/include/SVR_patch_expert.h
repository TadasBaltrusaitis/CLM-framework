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

#ifndef __SVR_PATCH_EXPERT_h_
#define __SVR_PATCH_EXPERT_h_

#include <map>
#include <cv.h>

using namespace cv;

namespace CLMTracker
{
  //===========================================================================
  /** 
      The classes describing the SVR patch experts
  */

class SVR_patch_expert{
	public:

		// Type of data the patch expert operated on (0=raw, 1=grad)
		int     type;					

		// Logistic regression slope
		double  scaling;
		
		// Logistic regression bias
		double  bias;

		// Support vector regression weights
		Mat_<float> weights;

		// Discrete Fourier Transform of SVR weights, precalculated for speed (at different window sizes)
		std::map<int, Mat_<double> > weights_dfts;

		// Confidence of the current patch expert (used for NU_RLMS optimisation)
		double  confidence;

		SVR_patch_expert(){;}
		
		// A copy constructor
		SVR_patch_expert(const SVR_patch_expert& other): weights(other.weights.clone())
		{
			this->type = other.type;
			this->scaling = other.scaling;
			this->bias = other.bias;
			this->confidence = other.confidence;

			for(std::map<int, Mat_<double> >::const_iterator it = other.weights_dfts.begin(); it!= other.weights_dfts.end(); it++)
			{
				// Make sure the matrix is copied.
				this->weights_dfts.insert(std::pair<int, Mat>(it->first, it->second.clone()));
			}
		}

		// Reading in the patch expert
		void Read(std::ifstream &stream);

		// The actual response computation from intensity or depth (for CLM-Z)
		void Response(const Mat_<float> &area_of_interest, Mat_<double> &response);    
		void ResponseDepth(const Mat_<float> &area_of_interest, Mat_<double> &response);

};
//===========================================================================
/**
    A Multi-patch Expert that can include different patch types. Raw pixel values or image gradients
*/
class Multi_SVR_patch_expert{
	public:
		
		// Width and height of the patch expert support area
		int width;
		int height;						

		// Vector of all of the patch experts (different modalities) for this particular Multi patch expert
		std::vector<SVR_patch_expert> svr_patch_experts;	

		// Default constructor
		Multi_SVR_patch_expert(){;}
	
		// Copy constructor				
		Multi_SVR_patch_expert(const Multi_SVR_patch_expert& other): svr_patch_experts(other.svr_patch_experts)
		{
			this->width = other.width;
			this->height = other.height;
		}

		void Read(std::ifstream &stream);

		// actual response computation from intensity of depth (for CLM-Z)
		void Response(const Mat_<float> &area_of_interest, Mat_<double> &response);
		void ResponseDepth(const Mat_<float> &area_of_interest, Mat_<double> &response);

};
}
#endif
