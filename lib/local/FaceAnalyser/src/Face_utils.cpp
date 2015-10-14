///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2015, University of Cambridge,
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
//       Tadas Baltrusaitis, Marwa Mahmoud, and Peter Robinson.
//		 Cross-dataset learning and person-specific normalisation for automatic Action Unit detection
//       Facial Expression Recognition and Analysis Challenge 2015,
//       IEEE International Conference on Automatic Face and Gesture Recognition, 2015
//
///////////////////////////////////////////////////////////////////////////////
#include <Face_utils.h>

// For FHOG visualisation
#include <dlib/opencv.h>

using namespace cv;
using namespace std;

namespace FaceAnalysis
{

	// Pick only the more stable/rigid points under changes of expression
	void extract_rigid_points(Mat_<double>& source_points, Mat_<double>& destination_points)
	{
		if(source_points.rows == 68)
		{
			Mat_<double> tmp_source = source_points.clone();
			source_points = Mat_<double>();

			// Push back the rigid points (some face outline, eyes, and nose)
			source_points.push_back(tmp_source.row(1));
			source_points.push_back(tmp_source.row(2));
			source_points.push_back(tmp_source.row(3));
			source_points.push_back(tmp_source.row(4));
			source_points.push_back(tmp_source.row(12));
			source_points.push_back(tmp_source.row(13));
			source_points.push_back(tmp_source.row(14));
			source_points.push_back(tmp_source.row(15));
			source_points.push_back(tmp_source.row(27));
			source_points.push_back(tmp_source.row(28));
			source_points.push_back(tmp_source.row(29));
			source_points.push_back(tmp_source.row(31));
			source_points.push_back(tmp_source.row(32));
			source_points.push_back(tmp_source.row(33));
			source_points.push_back(tmp_source.row(34));
			source_points.push_back(tmp_source.row(35));
			source_points.push_back(tmp_source.row(36));
			source_points.push_back(tmp_source.row(39));
			source_points.push_back(tmp_source.row(40));
			source_points.push_back(tmp_source.row(41));
			source_points.push_back(tmp_source.row(42));
			source_points.push_back(tmp_source.row(45));
			source_points.push_back(tmp_source.row(46));
			source_points.push_back(tmp_source.row(47));

			Mat_<double> tmp_dest = destination_points.clone();
			destination_points = Mat_<double>();

			// Push back the rigid points
			destination_points.push_back(tmp_dest.row(1));
			destination_points.push_back(tmp_dest.row(2));
			destination_points.push_back(tmp_dest.row(3));
			destination_points.push_back(tmp_dest.row(4));
			destination_points.push_back(tmp_dest.row(12));
			destination_points.push_back(tmp_dest.row(13));
			destination_points.push_back(tmp_dest.row(14));
			destination_points.push_back(tmp_dest.row(15));
			destination_points.push_back(tmp_dest.row(27));
			destination_points.push_back(tmp_dest.row(28));
			destination_points.push_back(tmp_dest.row(29));
			destination_points.push_back(tmp_dest.row(31));
			destination_points.push_back(tmp_dest.row(32));
			destination_points.push_back(tmp_dest.row(33));
			destination_points.push_back(tmp_dest.row(34));
			destination_points.push_back(tmp_dest.row(35));
			destination_points.push_back(tmp_dest.row(36));
			destination_points.push_back(tmp_dest.row(39));
			destination_points.push_back(tmp_dest.row(40));
			destination_points.push_back(tmp_dest.row(41));
			destination_points.push_back(tmp_dest.row(42));
			destination_points.push_back(tmp_dest.row(45));
			destination_points.push_back(tmp_dest.row(46));
			destination_points.push_back(tmp_dest.row(47));
		}
	}

	// Aligning a face to a common reference frame
	void AlignFace(cv::Mat& aligned_face, const cv::Mat& frame, const CLMTracker::CLM& clm_model, bool rigid, double sim_scale, int out_width, int out_height)
	{
		// Will warp to scaled mean shape
		Mat_<double> similarity_normalised_shape = clm_model.pdm.mean_shape * sim_scale;
	
		// Discard the z component
		similarity_normalised_shape = similarity_normalised_shape(Rect(0, 0, 1, 2*similarity_normalised_shape.rows/3)).clone();

		Mat_<double> source_landmarks = clm_model.detected_landmarks.reshape(1, 2).t();
		Mat_<double> destination_landmarks = similarity_normalised_shape.reshape(1, 2).t();

		// Aligning only the more rigid points
		if(rigid)
		{
			extract_rigid_points(source_landmarks, destination_landmarks);
		}

		Matx22d scale_rot_matrix = CLMTracker::AlignShapesWithScale(source_landmarks, destination_landmarks);
		Matx23d warp_matrix;

		warp_matrix(0,0) = scale_rot_matrix(0,0);
		warp_matrix(0,1) = scale_rot_matrix(0,1);
		warp_matrix(1,0) = scale_rot_matrix(1,0);
		warp_matrix(1,1) = scale_rot_matrix(1,1);

		double tx = clm_model.params_global[4];
		double ty = clm_model.params_global[5];

		Vec2d T(tx, ty);
		T = scale_rot_matrix * T;

		// Make sure centering is correct
		warp_matrix(0,2) = -T(0) + out_width/2;
		warp_matrix(1,2) = -T(1) + out_height/2;

		cv::warpAffine(frame, aligned_face, warp_matrix, Size(out_width, out_height), INTER_LINEAR);
	}

	// Aligning a face to a common reference frame
	void AlignFaceMask(cv::Mat& aligned_face, const cv::Mat& frame, const CLMTracker::CLM& clm_model, const Mat_<int>& triangulation, bool rigid, double sim_scale, int out_width, int out_height)
	{
		// Will warp to scaled mean shape
		Mat_<double> similarity_normalised_shape = clm_model.pdm.mean_shape * sim_scale;
	
		// Discard the z component
		similarity_normalised_shape = similarity_normalised_shape(Rect(0, 0, 1, 2*similarity_normalised_shape.rows/3)).clone();

		Mat_<double> source_landmarks = clm_model.detected_landmarks.reshape(1, 2).t();
		Mat_<double> destination_landmarks = similarity_normalised_shape.reshape(1, 2).t();

		// Aligning only the more rigid points
		if(rigid)
		{
			extract_rigid_points(source_landmarks, destination_landmarks);
		}

		Matx22d scale_rot_matrix = CLMTracker::AlignShapesWithScale(source_landmarks, destination_landmarks);
		Matx23d warp_matrix;

		warp_matrix(0,0) = scale_rot_matrix(0,0);
		warp_matrix(0,1) = scale_rot_matrix(0,1);
		warp_matrix(1,0) = scale_rot_matrix(1,0);
		warp_matrix(1,1) = scale_rot_matrix(1,1);

		double tx = clm_model.params_global[4];
		double ty = clm_model.params_global[5];

		Vec2d T(tx, ty);
		T = scale_rot_matrix * T;

		// Make sure centering is correct
		warp_matrix(0,2) = -T(0) + out_width/2;
		warp_matrix(1,2) = -T(1) + out_height/2;

		cv::warpAffine(frame, aligned_face, warp_matrix, Size(out_width, out_height), INTER_LINEAR);

		// Move the destination landmarks there as well
		Matx22d warp_matrix_2d(warp_matrix(0,0), warp_matrix(0,1), warp_matrix(1,0), warp_matrix(1,1));
		
		destination_landmarks = Mat(clm_model.detected_landmarks.reshape(1, 2).t()) * Mat(warp_matrix_2d).t();

		destination_landmarks.col(0) = destination_landmarks.col(0) + warp_matrix(0,2);
		destination_landmarks.col(1) = destination_landmarks.col(1) + warp_matrix(1,2);
		
		// Move the eyebrows up to include more of upper face
		destination_landmarks.at<double>(0,1) -= 15; 
		destination_landmarks.at<double>(16,1) -= 15; 

		destination_landmarks.at<double>(17,1) -= 7; 
		destination_landmarks.at<double>(18,1) -= 7; 
		destination_landmarks.at<double>(19,1) -= 7; 
		destination_landmarks.at<double>(20,1) -= 7; 
		destination_landmarks.at<double>(21,1) -= 7; 
		destination_landmarks.at<double>(22,1) -= 7; 
		destination_landmarks.at<double>(23,1) -= 7; 
		destination_landmarks.at<double>(24,1) -= 7; 
		destination_landmarks.at<double>(25,1) -= 7; 
		destination_landmarks.at<double>(26,1) -= 7; 

		destination_landmarks = Mat(destination_landmarks.t()).reshape(1, 1).t();		

		CLMTracker::PAW paw(destination_landmarks, triangulation, 0, 0, aligned_face.cols-1, aligned_face.rows-1);
		
		vector<Mat> aligned_face_channels(aligned_face.channels());
		
		cv::split(aligned_face, aligned_face_channels);

		for(size_t i = 0; i < aligned_face_channels.size(); ++i)
		{
			aligned_face_channels[i] = aligned_face_channels[i].mul(paw.pixel_mask);
		}

		cv::merge(aligned_face_channels, aligned_face);

	}


	void Visualise_FHOG(const cv::Mat_<double>& descriptor, int num_rows, int num_cols, cv::Mat& visualisation)
	{

		// First convert to dlib format
		dlib::array2d<dlib::matrix<float,31,1> > hog(num_rows, num_cols);
		
		cv::MatConstIterator_<double> descriptor_it = descriptor.begin();
		for(int y = 0; y < num_cols; ++y)
		{
			for(int x = 0; x < num_rows; ++x)
			{
				for(unsigned int o = 0; o < 31; ++o)
				{
					hog[y][x](o) = *descriptor_it++;
				}
			}
		}

		// Draw the FHOG to OpenCV format
		auto fhog_vis = dlib::draw_fhog(hog);
		visualisation = dlib::toMat(fhog_vis).clone();
	}

	// Create a row vector Felzenszwalb HOG descriptor from a given image
	void Extract_FHOG_descriptor(cv::Mat_<double>& descriptor, const cv::Mat& image, int& num_rows, int& num_cols, int cell_size)
	{
		
		dlib::array2d<dlib::matrix<float,31,1> > hog;
		if(image.channels() == 1)
		{
			dlib::cv_image<uchar> dlib_warped_img(image);
			dlib::extract_fhog_features(dlib_warped_img, hog, cell_size);
		}
		else
		{
			dlib::cv_image<dlib::bgr_pixel> dlib_warped_img(image);
			dlib::extract_fhog_features(dlib_warped_img, hog, cell_size);
		}

		// Convert to a usable format
		num_cols = hog.nc();
		num_rows = hog.nr();

		descriptor = Mat_<double>(1, num_cols * num_rows * 31);
		cv::MatIterator_<double> descriptor_it = descriptor.begin();
		for(int y = 0; y < num_cols; ++y)
		{
			for(int x = 0; x < num_rows; ++x)
			{
				for(unsigned int o = 0; o < 31; ++o)
				{
					*descriptor_it++ = (double)hog[y][x](o);
				}
			}
		}
	}

	// Extract summary statistics (mean, stdev, min, max) from each dimension of a descriptor, each row is a descriptor
	void ExtractSummaryStatistics(const cv::Mat_<double>& descriptors, cv::Mat_<double>& sum_stats, bool use_mean, bool use_stdev, bool use_max_min)
	{
		// Using four summary statistics at the moment 
		// Means, stds, mins, maxs
		int num_stats = 0;

		if(use_mean)
			num_stats++;

		if(use_stdev)
			num_stats++;

		if(use_max_min)
			num_stats++;

		sum_stats = Mat_<double>(1, descriptors.cols * num_stats, 0.0);
		for(int i = 0; i < descriptors.cols; ++i)
		{
			Scalar mean, stdev;
			cv::meanStdDev(descriptors.col(i), mean, stdev);

			int add = 0;

			if(use_mean)
			{
				sum_stats.at<double>(0, i*num_stats + add) = mean[0];
				add++;
			}

			if(use_stdev)
			{
				sum_stats.at<double>(0, i*num_stats + add) = stdev[0];
				add++;
			}

			if(use_max_min)
			{
				double min, max;
				cv::minMaxIdx(descriptors.col(i), &min, &max);
				sum_stats.at<double>(0, i*num_stats + add) = max - min;
				add++;
			}
		}		
	}

	void AddDescriptor(cv::Mat_<double>& descriptors, cv::Mat_<double> new_descriptor, int curr_frame, int num_frames_to_keep)
	{
		if(descriptors.empty())
		{
			descriptors = Mat_<double>(num_frames_to_keep, new_descriptor.cols, 0.0);
		}

		int row_to_change = curr_frame % num_frames_to_keep;

		new_descriptor.copyTo(descriptors.row(row_to_change));
	}	

}