#include "SVM_dynamic_lin.h"

#include "CLM_core.h"

using namespace FaceAnalysis;

void SVM_dynamic_lin::Read(std::ifstream& stream, const std::vector<std::string>& au_names)
{

	if(this->means.empty())
	{
		CLMTracker::ReadMatBin(stream, this->means);
	}
	else
	{
		Mat_<double> m_tmp;
		CLMTracker::ReadMatBin(stream, m_tmp);
		if(cv::norm(m_tmp - this->means > 0.00001))
		{
			cout << "Something went wrong with the SVR dynamic regressors" << endl;
		}
	}

	Mat_<double> support_vectors_curr;
	CLMTracker::ReadMatBin(stream, support_vectors_curr);

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
		Mat_<double> preds;
		if(fhog_descriptor.cols ==  this->means.cols)
		{
			preds = (fhog_descriptor - this->means - running_median) * this->support_vectors + this->biases;
		}
		else
		{
			Mat_<double> input;
			cv::hconcat(fhog_descriptor, geom_params, input);

			Mat_<double> run_med;
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