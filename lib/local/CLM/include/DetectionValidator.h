///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2014, University of Southern California and University of Cambridge,
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
//       Tadas Baltrusaitis, Peter Robinson, and Louis-Philippe Morency. 3D
//       Constrained Local Model for Rigid and Non-Rigid Facial Tracking.
//       IEEE Conference on Computer Vision and Pattern Recognition (CVPR), 2012.    
//
//       Tadas Baltrusaitis, Peter Robinson, and Louis-Philippe Morency. 
//       Constrained Local Neural Fields for robust facial landmark detection in the wild.
//       in IEEE Int. Conference on Computer Vision Workshops, 300 Faces in-the-Wild Challenge, 2013.    
//
///////////////////////////////////////////////////////////////////////////////
#ifndef __DValid_h_
#define __DValid_h_

#include "PAW.h"

using namespace std;
using namespace cv;

namespace CLMTracker
{
//===========================================================================
//
// Checking if landmark detection was successful using an SVR regressor
// Using multiple validators trained add different views
// The regressor outputs -1 for ideal alignment and 1 for worst alignment
//===========================================================================
class DetectionValidator
{
		
public:    
	
	// What type of validator we're using - 0 - linear svr, 1 - feed forward neural net, 2 - convolutional neural net
	int validator_type;

	// The orientations of each of the landmark detection validator
	vector<cv::Vec3d> orientations;

	// Piecewise affine warps to the reference shape (per orientation)
	vector<PAW>     paws;

	//==========================================
	// Linear SVR

	// SVR biases
	vector<double>  bs;

	// SVR weights
	vector<Mat_<double> > ws;
	
	//==========================================
	// Neural Network

	// Neural net weights
	vector<vector<Mat_<double> > > ws_nn;

	// What type of activation or output functions are used
	// 0 - sigmoid, 1 - tanh_opt, 2 - ReLU
	vector<int> activation_fun;
	vector<int> output_fun;

	//==========================================
	// Convolutional Neural Network

	// CNN layers for each view
	// view -> layer -> input maps -> kernels
	vector<vector<vector<vector<Mat_<float> > > > > cnn_convolutional_layers;
	// Bit ugly with so much nesting, but oh well
	vector<vector<vector<vector<pair<int, Mat_<double> > > > > > cnn_convolutional_layers_dft;
	vector<vector<vector<float > > > cnn_convolutional_layers_bias;
	vector< vector<int> > cnn_subsampling_layers;
	vector< vector<Mat_<float> > > cnn_fully_connected_layers;
	vector< vector<float > > cnn_fully_connected_layers_bias;
	// 0 - convolutional, 1 - subsampling, 2 - fully connected
	vector<vector<int> > cnn_layer_types;
	
	//==========================================

	// Normalisation for face validation
	vector<Mat_<double> > mean_images;
	vector<Mat_<double> > standard_deviations;

	// Default constructor
	DetectionValidator(){;}

	// Copy constructor
	DetectionValidator(const DetectionValidator& other): orientations(other.orientations), bs(other.bs), paws(other.paws),
		cnn_subsampling_layers(other.cnn_subsampling_layers),cnn_layer_types(other.cnn_layer_types), cnn_fully_connected_layers_bias(other.cnn_fully_connected_layers_bias),
		cnn_convolutional_layers_bias(other.cnn_convolutional_layers_bias), cnn_convolutional_layers_dft(other.cnn_convolutional_layers_dft)
	{
	
		this->validator_type = other.validator_type;

		this->activation_fun = other.activation_fun;
		this->output_fun = other.output_fun;

		this->ws.resize(other.ws.size());
		for(size_t i = 0; i < other.ws.size(); ++i)
		{
			// Make sure the matrix is copied.
			this->ws[i] = other.ws[i].clone();
		}

		this->ws_nn.resize(other.ws_nn.size());
		for(size_t i = 0; i < other.ws_nn.size(); ++i)
		{
			this->ws_nn[i].resize(other.ws_nn[i].size());

			for(size_t k = 0; k < other.ws_nn[i].size(); ++k)
			{
				// Make sure the matrix is copied.
				this->ws_nn[i][k] = other.ws_nn[i][k].clone();
			}
		}

		this->cnn_convolutional_layers.resize(other.cnn_convolutional_layers.size());
		for(size_t v = 0; v < other.cnn_convolutional_layers.size(); ++v)
		{
			this->cnn_convolutional_layers[v].resize(other.cnn_convolutional_layers[v].size());

			for(size_t l = 0; l < other.cnn_convolutional_layers[v].size(); ++l)
			{
				this->cnn_convolutional_layers[v][l].resize(other.cnn_convolutional_layers[v][l].size());

				for(size_t i = 0; i < other.cnn_convolutional_layers[v][l].size(); ++i)
				{
					this->cnn_convolutional_layers[v][l][i].resize(other.cnn_convolutional_layers[v][l][i].size());

					for(size_t k = 0; k < other.cnn_convolutional_layers[v][l][i].size(); ++k)
					{
						// Make sure the matrix is copied.
						this->cnn_convolutional_layers[v][l][i][k] = other.cnn_convolutional_layers[v][l][i][k].clone();
					}
					
				}
			}
		}

		this->cnn_fully_connected_layers.resize(other.cnn_fully_connected_layers.size());
		for(size_t v = 0; v < other.cnn_fully_connected_layers.size(); ++v)
		{
			this->cnn_fully_connected_layers[v].resize(other.cnn_fully_connected_layers[v].size());

			for(size_t l = 0; l < other.cnn_fully_connected_layers[v].size(); ++l)
			{
				// Make sure the matrix is copied.
				this->cnn_fully_connected_layers[v][l] = other.cnn_fully_connected_layers[v][l].clone();
			}
		}

		this->mean_images.resize(other.mean_images.size());
		for(size_t i = 0; i < other.mean_images.size(); ++i)
		{
			// Make sure the matrix is copied.
			this->mean_images[i] = other.mean_images[i].clone();
		}

		this->standard_deviations.resize(other.standard_deviations.size());
		for(size_t i = 0; i < other.standard_deviations.size(); ++i)
		{
			// Make sure the matrix is copied.
			this->standard_deviations[i] = other.standard_deviations[i].clone();
		}
	
	}

	// Given an image, orientation and detected landmarks output the result of the appropriate regressor
	double Check(const Vec3d& orientation, const Mat_<uchar>& intensity_img, Mat_<double>& detected_landmarks);

	// Reading in the model
	void Read(string location);
			
	// Getting the closest view center based on orientation
	int GetViewId(const cv::Vec3d& orientation) const;

private:

	// The actual regressor application on the image

	// Support Vector Regression (linear kernel)
	double CheckSVR(const Mat_<double>& warped_img, int view_id);

	// Feed-forward Neural Network
	double CheckNN(const Mat_<double>& warped_img, int view_id);

	// Convolutional Neural Network
	double CheckCNN(const Mat_<double>& warped_img, int view_id);

	// A normalisation helper
	void NormaliseWarpedToVector(const Mat_<double>& warped_img, Mat_<double>& feature_vec, int view_id);

};

}
#endif
