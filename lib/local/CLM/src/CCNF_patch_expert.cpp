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

#include "CCNF_patch_expert.h"

#include "CLM_utils.h"

#include <stdio.h>
#include <iostream>
#include <highgui.h>
#include <Eigen/Dense>
#include <opencv2/core/eigen.hpp>

using Eigen::MatrixXf;

using namespace CLMTracker;

// Compute sigmas for all landmarks for a particular view and window size
void CCNF_patch_expert::ComputeSigmas(std::vector<Mat_<float> > sigma_components, int window_size)
{
	for(size_t i=0; i < window_sizes.size(); ++i)
	{
		if( window_sizes[i] == window_size)
			return;
	}
	// Each of the landmarks will have the same connections, hence constant number of sigma components
	int n_betas = sigma_components.size();

	// calculate the sigmas based on alphas and betas
	float sum_alphas = 0;

	int n_alphas = this->neurons.size();

	// sum the alphas first
	for(int a = 0; a < n_alphas; ++a)
	{
		sum_alphas = sum_alphas + this->neurons[a].alpha;
	}

	Mat_<float> q1 = sum_alphas * Mat_<float>::eye(window_size*window_size, window_size*window_size);

	Mat_<float> q2 = Mat_<float>::zeros(window_size*window_size, window_size*window_size);
	for (int b=0; b < n_betas; ++b)
	{			
		q2 = q2 + ((float)this->betas[b]) * sigma_components[b];
	}

	Mat_<float> SigmaInv = 2 * (q1 + q2);
	
	// Eigen is faster in release mode, but OpenCV in debug
	#ifdef _DEBUG
		Mat Sigma_f;
		invert(SigmaInv, Sigma_f, DECOMP_CHOLESKY);
	#else
		MatrixXf SigmaInvEig_f;
		cv2eigen(SigmaInv,SigmaInvEig_f);
		MatrixXf eye_f = MatrixXf::Identity(SigmaInv.rows, SigmaInv.cols);
		SigmaInvEig_f.llt().solveInPlace(eye_f);		
		Mat Sigma_f;
		eigen2cv(eye_f, Sigma_f);
	#endif

	window_sizes.push_back(window_size);
	Sigmas.push_back(Sigma_f);

}

//===========================================================================
void CCNF_neuron::Read(ifstream &stream)
{
	// Sanity check
	int read_type;
	stream >> read_type;
	assert(read_type == 2);

	stream >> neuron_type >> norm_weights >> bias >> alpha;
	CLMTracker::ReadMat(stream, weights); 
	weights = weights.t();

}

//===========================================================================
void CCNF_neuron::Response(Mat_<float> &im, Mat_<double> &im_dft, Mat &integral_img, Mat &integral_img_sq, Mat_<double> &resp)
{

	int h = im.rows - weights.rows + 1;
	int w = im.cols - weights.cols + 1;
	
	// the patch area on which we will calculate reponses
	Mat_<float> I;    

	if(neuron_type == 3)
	{
		// Perform normalisation across whole patch (ignoring the invalid values indicated by <= 0

		Scalar mean;
		Scalar std;
		
		// ignore missing values
		Mat_<uchar> mask = im > 0;
		meanStdDev(im, mean, std, mask);

		// if all values the same don't divide by 0
		if(std[0] != 0)
		{
			I = (im - mean[0]) / std[0];
		}
		else
		{
			I = (im - mean[0]);
		}

		I.setTo(0, mask == 0);
	}
	else
	{
		if(neuron_type == 0)
		{
			I = im;
		}
		else
		{
			printf("ERROR(%s,%d): Unsupported patch type %d!\n", __FILE__,__LINE__,neuron_type);
			abort();
		}
	}
  
	// The response from neuron before activation
	Mat_<float> neuron_response;

	if(neuron_type == 3)
	{
		// In case of depth we use per area, rather than per patch normalisation
		matchTemplate_m(I, im_dft, integral_img, integral_img_sq, weights, weights_dfts, neuron_response, CV_TM_CCOEFF); // the linear multiplication, efficient calc of response

	}
	else
	{
		matchTemplate_m(I, im_dft, integral_img, integral_img_sq, weights, weights_dfts, neuron_response, CV_TM_CCOEFF_NORMED); // the linear multiplication, efficient calc of response

	}

	// output
	resp.create(neuron_response.size());
	MatIterator_<double> p = resp.begin();

	MatIterator_<float> q1 = neuron_response.begin(); // respone for each pixel
	MatIterator_<float> q2 = neuron_response.end();

	// the logistic function (sigmoid) applied to the response
	while(q1 != q2)
	{
		*p++ = (2 * alpha) * 1.0 /(1.0 + exp( -(*q1++ * norm_weights + bias )));
	}

}

//===========================================================================
void CCNF_patch_expert::Read(ifstream &stream, vector<int> window_sizes, vector<vector<Mat_<float> > > sigma_components)
{

	// Sanity check
	int read_type;
	stream >> read_type;		
	assert(read_type == 5);

	// the number of neurons for this patch
	int num_neurons;
	stream >> width >> height >> num_neurons;

	if(num_neurons == 0)
	{
		// empty patch due to landmark being invisible at that orientation
	
		// read an empty int (due to the way things were written out)
		stream >> num_neurons;
		return;
	}

	neurons.resize(num_neurons);
	for(int i = 0; i < num_neurons; i++)
		neurons[i].Read(stream);

	int n_sigmas = window_sizes.size();

	int n_betas = 0;

	if(n_sigmas > 0)
	{
		n_betas = sigma_components[0].size();

		betas.resize(n_betas);

		for (int i=0; i < n_betas;  ++i)
		{
			stream >> betas[i];
		}
	}	

	// Read the patch confidence
	stream >> patch_confidence;

}

//===========================================================================
void CCNF_patch_expert::Response(Mat_<float> &area_of_interest, Mat_<double> &response)
{
	
	int response_height = area_of_interest.rows - height + 1;
	int response_width = area_of_interest.cols - width + 1;

	if(response.rows != response_height || response.cols != response_width)
	{
		response.create(response_height, response_width);
	}
		
	response.setTo(0);
	
	// the placeholder for the DFT of the image, the integral image, and squared integral image so they don't get recalculated for every response
	Mat_<double> area_of_interest_dft;
	Mat integral_image, integral_image_sq;
	
	Mat_<double> neuron_response;

	// responses from the neural layers
	for(size_t i = 0; i < neurons.size(); i++)
	{		
		// Do not bother with neuron response if the alpha is tiny and will not contribute much to overall result
		if(neurons[i].alpha > 1e-4)
		{
			neurons[i].Response(area_of_interest, area_of_interest_dft, integral_image, integral_image_sq, neuron_response);
			response = response + neuron_response;						
		}
	}

	int s_to_use = -1;

	// Find the matching sigma
	for(size_t i=0; i < window_sizes.size(); ++i)
	{
		if(window_sizes[i] == response_height)
		{
			// Found the correct sigma
			s_to_use = i;			
			break;
		}
	}

	Mat resp_vec = response.reshape(1, response_height * response_width);
	Mat_<float> resp_vec_f;
	resp_vec.convertTo(resp_vec_f, CV_32F);

	Mat out = Sigmas[s_to_use] * resp_vec_f;

	response = out.reshape(1, response_height);
	response.convertTo(response, CV_64F);

	// Making sure the response does not have negative numbers
	double min;

	minMaxIdx(response, &min, 0);
	if(min < 0)
	{
		response = response - min;
	}

}
