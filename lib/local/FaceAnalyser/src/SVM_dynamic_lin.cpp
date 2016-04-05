///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016, Carnegie Mellon University and University of Cambridge,
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
//       reports and manuals, must cite at least one of the following works:
//
//       OpenFace: an open source facial behavior analysis toolkit
//       Tadas Baltrušaitis, Peter Robinson, and Louis-Philippe Morency
//       in IEEE Winter Conference on Applications of Computer Vision, 2016  
//
//       Rendering of Eyes for Eye-Shape Registration and Gaze Estimation
//       Erroll Wood, Tadas Baltrušaitis, Xucong Zhang, Yusuke Sugano, Peter Robinson, and Andreas Bulling 
//       in IEEE International. Conference on Computer Vision (ICCV),  2015 
//
//       Cross-dataset learning and person-speci?c normalisation for automatic Action Unit detection
//       Tadas Baltrušaitis, Marwa Mahmoud, and Peter Robinson 
//       in Facial Expression Recognition and Analysis Challenge, 
//       IEEE International Conference on Automatic Face and Gesture Recognition, 2015 
//
//       Constrained Local Neural Fields for robust facial landmark detection in the wild.
//       Tadas Baltrušaitis, Peter Robinson, and Louis-Philippe Morency. 
//       in IEEE Int. Conference on Computer Vision Workshops, 300 Faces in-the-Wild Challenge, 2013.    
//
///////////////////////////////////////////////////////////////////////////////

#include "SVM_dynamic_lin.h"

#include "LandmarkCoreIncludes.h"

using namespace FaceAnalysis;

void SVM_dynamic_lin::Read(std::ifstream& stream, const std::vector<std::string>& au_names)
{

	if(this->means.empty())
	{
		LandmarkDetector::ReadMatBin(stream, this->means);
	}
	else
	{
		cv::Mat_<double> m_tmp;
		LandmarkDetector::ReadMatBin(stream, m_tmp);
		if(cv::norm(m_tmp - this->means > 0.00001))
		{
			cout << "Something went wrong with the SVR dynamic regressors" << endl;
		}
	}

	cv::Mat_<double> support_vectors_curr;
	LandmarkDetector::ReadMatBin(stream, support_vectors_curr);

	double bias;
	stream.read((char *)&bias, 8);

	// Read in positive or negative class
	double pos_class;	
	stream.read((char *)&pos_class, 8);

	double neg_class;
	stream.read((char *)&neg_class, 8);


	// Add a column vector to the matrix of support vectors (each column is a support vector)
	if(!this->support_vectors.empty())
	{	
		cv::transpose(this->support_vectors, this->support_vectors);
		cv::transpose(support_vectors_curr, support_vectors_curr);
		this->support_vectors.push_back(support_vectors_curr);
		cv::transpose(this->support_vectors, this->support_vectors);

		cv::transpose(this->biases, this->biases);
		this->biases.push_back(cv::Mat_<double>(1, 1, bias));
		cv::transpose(this->biases, this->biases);

	}
	else
	{
		this->support_vectors.push_back(support_vectors_curr);
		this->biases.push_back(cv::Mat_<double>(1, 1, bias));
	}

	this->pos_classes.push_back(pos_class);
	this->neg_classes.push_back(neg_class);
	
	for(size_t i=0; i < au_names.size(); ++i)
	{
		this->AU_names.push_back(au_names[i]);
	}
}

// Prediction using the HOG descriptor
void SVM_dynamic_lin::Predict(std::vector<double>& predictions, std::vector<std::string>& names, const cv::Mat_<double>& fhog_descriptor, const cv::Mat_<double>& geom_params,  const cv::Mat_<double>& running_median,  const cv::Mat_<double>& running_median_geom)
{
	if(AU_names.size() > 0)
	{
		cv::Mat_<double> preds;
		if(fhog_descriptor.cols ==  this->means.cols)
		{
			preds = (fhog_descriptor - this->means - running_median) * this->support_vectors + this->biases;
		}
		else
		{
			cv::Mat_<double> input;
			cv::hconcat(fhog_descriptor, geom_params, input);

			cv::Mat_<double> run_med;
			cv::hconcat(running_median, running_median_geom, run_med);

			preds = (input - this->means - run_med) * this->support_vectors + this->biases;
		}

		for(int i = 0; i < preds.cols; ++i)
		{		
			if(preds.at<double>(i) > 0)
			{
				predictions.push_back(pos_classes[i]);
			}
			else
			{
				predictions.push_back(neg_classes[i]);
			}
		}

		names = this->AU_names;
	}
}