#ifndef __SVRSTATICLINREGRESSORS_h_
#define __SVRSTATICLINREGRESSORS_h_

#include <vector>
#include <string>

#include <stdio.h>
#include <iostream>

#include <cv.h>

namespace FaceAnalysis
{

// Collection of linear SVR regressors for AU prediction
class SVR_static_lin_regressors{

public:

	SVR_static_lin_regressors()
	{}

	// Predict the AU from HOG appearance of the face
	void Predict(std::vector<double>& predictions, std::vector<std::string>& names, const cv::Mat_<double>& fhog_descriptor, const cv::Mat_<double>& geom_params);

	// Reading in the model (or adding to it)
	void Read(std::ifstream& stream, const std::vector<std::string>& au_names);

private:

	// The names of Action Units this model is responsible for
	std::vector<std::string> AU_names;

	// For normalisation
	cv::Mat_<double> means;
	
	// For actual prediction
	cv::Mat_<double> support_vectors;	
	cv::Mat_<double> biases;

};
  //===========================================================================
}
#endif
