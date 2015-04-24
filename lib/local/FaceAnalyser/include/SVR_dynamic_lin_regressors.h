#ifndef __SVRDYNAMICLINREGRESSORS_h_
#define __SVRDYNAMICLINREGRESSORS_h_

#include <vector>
#include <string>

#include <stdio.h>
#include <iostream>

#include <cv.h>

namespace FaceAnalysis
{

// Collection of linear SVR regressors for AU prediction that uses per person face nomalisation with the help of a running median
class SVR_dynamic_lin_regressors{

public:

	SVR_dynamic_lin_regressors()
	{}

	// Predict the AU from HOG appearance of the face
	void Predict(std::vector<double>& predictions, std::vector<std::string>& names, const cv::Mat_<double>& descriptor, const cv::Mat_<double>& geom_params, const cv::Mat_<double>& running_median, const cv::Mat_<double>& running_median_geom);

	// Reading in the model (or adding to it)
	void Read(std::ifstream& stream, const std::vector<std::string>& au_names);

	std::vector<std::string> GetAUNames()
	{
		return AU_names;
	}

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
