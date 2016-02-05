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
#include "stdafx.h"

#include <CLM_utils.h>

using namespace boost::filesystem;

using namespace cv;
using namespace std;

namespace CLMTracker
{

// Useful utility for creating directories for storing the output files
void create_directory_from_file(string output_path)
{

	// Creating the right directory structure
	
	// First get rid of the file
	auto p = path(path(output_path).parent_path());

	if(!p.empty() && !boost::filesystem::exists(p))		
	{
		bool success = boost::filesystem::create_directories(p);
		if(!success)
		{
			cout << "Failed to create a directory... " << p.string() << endl;
		}
	}
}

// Useful utility for creating directories for storing the output files
void create_directories(string output_path)
{

	// Creating the right directory structure
	
	// First get rid of the file
	auto p = path(output_path);

	if(!p.empty() && !boost::filesystem::exists(p))		
	{
		bool success = boost::filesystem::create_directories(p);
		if(!success)
		{
			cout << "Failed to create a directory... " << p.string() << endl;
		}
	}
}

// Extracting the following command line arguments -f, -fd, -op, -of, -ov (and possible ordered repetitions)
void get_video_input_output_params(vector<string> &input_video_files, vector<string> &depth_dirs,
	vector<string> &output_pose_files, vector<string> &output_video_files, vector<string> &output_2d_landmark_files, vector<string> &output_3D_landmark_files, bool& world_coordinates, vector<string> &arguments)
{
	bool* valid = new bool[arguments.size()];

	for(size_t i = 0; i < arguments.size(); ++i)
	{
		valid[i] = true;
	}

	// By default use rotation with respect to camera (not world coordinates)
	world_coordinates = false;

	string root = "";
	// First check if there is a root argument (so that videos and outputs could be defined more easilly)
	for(size_t i = 0; i < arguments.size(); ++i)
	{
		if (arguments[i].compare("-root") == 0) 
		{                    
			root = arguments[i + 1];
			// Do not discard root as it might be used in other later steps
			i++;
		}		
	}

	for(size_t i = 0; i < arguments.size(); ++i)
	{
		if (arguments[i].compare("-f") == 0) 
		{                    
			input_video_files.push_back(root + arguments[i + 1]);			
			valid[i] = false;
			valid[i+1] = false;			
			i++;
		}		
		else if (arguments[i].compare("-fd") == 0) 
		{                    
			depth_dirs.push_back(root + arguments[i + 1]);
			valid[i] = false;
			valid[i+1] = false;		
			i++;
		} 
		else if (arguments[i].compare("-op") == 0)
		{
			output_pose_files.push_back(root + arguments[i + 1]);
			create_directory_from_file(root + arguments[i + 1]);
			valid[i] = false;
			valid[i+1] = false;
			i++;
		} 
		else if (arguments[i].compare("-of") == 0)
		{
			output_2d_landmark_files.push_back(root + arguments[i + 1]);
			create_directory_from_file(root + arguments[i + 1]);
			valid[i] = false;
			valid[i+1] = false;
			i++;
		} 
		else if (arguments[i].compare("-of3D") == 0)
		{
			output_3D_landmark_files.push_back(root + arguments[i + 1]);
			create_directory_from_file(root + arguments[i + 1]);
			valid[i] = false;
			valid[i+1] = false;
			i++;
		} 
		else if (arguments[i].compare("-ov") == 0)
		{
			output_video_files.push_back(root + arguments[i + 1]);
			create_directory_from_file(root + arguments[i + 1]);
			valid[i] = false;
			valid[i+1] = false;
			i++;
		}		
		else if (arguments[i].compare("-world_coord") == 0)
		{
			world_coordinates = true;
		}
		else if (arguments[i].compare("-help") == 0)
		{
			cout << "Input output files are defined as: -f <infile> -fd <indepthdir> -op <outpose> -of <outfeatures> -ov <outvideo>\n"; // Inform the user of how to use the program				
		}
	}

	for(int i=arguments.size()-1; i >= 0; --i)
	{
		if(!valid[i])
		{
			arguments.erase(arguments.begin()+i);
		}
	}

}

void get_camera_params(int &device, float &fx, float &fy, float &cx, float &cy, vector<string> &arguments)
{
	bool* valid = new bool[arguments.size()];

	for(size_t i=0; i < arguments.size(); ++i)
	{
		valid[i] = true;
		if (arguments[i].compare("-fx") == 0) 
		{                   
			stringstream data(arguments[i+1]);
			data >> fx;
			valid[i] = false;
			valid[i+1] = false;			
			i++;
		}		
		else if (arguments[i].compare("-fy") == 0) 
		{
			stringstream data(arguments[i+1]);
			data >> fy;
			valid[i] = false;
			valid[i+1] = false;		
			i++;
		} 
		else if (arguments[i].compare("-cx") == 0)
		{
			stringstream data(arguments[i+1]);
			data >> cx;
			valid[i] = false;
			valid[i+1] = false;
			i++;
		} 
		else if (arguments[i].compare("-cy") == 0)
		{
			stringstream data(arguments[i+1]);
			data >> cy;
			valid[i] = false;
			valid[i+1] = false;
			i++;
		}
		else if (arguments[i].compare("-device") == 0)
		{
			stringstream data(arguments[i+1]);
			data >> device;
			valid[i] = false;
			valid[i+1] = false;
			i++;
		}
		else if (arguments[i].compare("-help") == 0)
		{
			cout << "Camera parameters are defined as: -device <webcam number> -fx <float focal length x> -fy <float focal length y> -cx <float optical center x> -cy <float optical center y> "  << endl; // Inform the user of how to use the program				
		}
	}

	for(int i=arguments.size()-1; i >= 0; --i)
	{
		if(!valid[i])
		{
			arguments.erase(arguments.begin()+i);
		}
	}
}

void get_image_input_output_params(vector<string> &input_image_files, vector<string> &input_depth_files, vector<string> &output_feature_files, vector<string> &output_pose_files, vector<string> &output_image_files,
		vector<Rect_<double>> &input_bounding_boxes, vector<string> &arguments)
{
	bool* valid = new bool[arguments.size()];
	
	string out_pts_dir, out_pose_dir, out_img_dir;

	for(size_t i = 0; i < arguments.size(); ++i)
	{
		valid[i] = true;
		if (arguments[i].compare("-f") == 0) 
		{                    
			input_image_files.push_back(arguments[i + 1]);
			valid[i] = false;
			valid[i+1] = false;			
			i++;
		}		
		else if (arguments[i].compare("-fd") == 0) 
		{                    
			input_depth_files.push_back(arguments[i + 1]);
			valid[i] = false;
			valid[i+1] = false;		
			i++;
		}
		else if (arguments[i].compare("-fdir") == 0) 
		{                    

			// parse the -fdir directory by reading in all of the .png and .jpg files in it
			path image_directory (arguments[i+1]); 

			try
			{
				 // does the file exist and is it a directory
				if (exists(image_directory) && is_directory(image_directory))   
				{

					vector<path> file_in_directory;                                
					copy(directory_iterator(image_directory), directory_iterator(), back_inserter(file_in_directory));
					
					// Sort the images in the directory first
					sort(file_in_directory.begin(), file_in_directory.end()); 

					for (vector<path>::const_iterator file_iterator (file_in_directory.begin()); file_iterator != file_in_directory.end(); ++file_iterator)
					{
						// Possible image extension .jpg and .png
						if(file_iterator->extension().string().compare(".jpg") == 0 || file_iterator->extension().string().compare(".png") == 0 || file_iterator->extension().string().compare(".bmp") == 0)
						{
							
								
							input_image_files.push_back(file_iterator->string());
								
							// If there exists a .txt file corresponding to the image, it is assumed that it contains a bounding box definition for a face
							// [minx, miny, maxx, maxy]
							path current_file = *file_iterator;
							path bbox = current_file.replace_extension("txt");

							// If there is a bounding box file push it to the list of bounding boxes
							if(exists(bbox))
							{

								std::ifstream in_bbox(bbox.string().c_str(), ios_base::in);

								double min_x, min_y, max_x, max_y;

								in_bbox >> min_x >> min_y >> max_x >> max_y;

								in_bbox.close();

								input_bounding_boxes.push_back(Rect_<double>(min_x, min_y, max_x - min_x, max_y - min_y));
							}
						}
					}
				}
			}
			catch (const filesystem_error& ex)
			{
				cout << ex.what() << '\n';
			}

			valid[i] = false;
			valid[i+1] = false;		
			i++;
		}
		else if (arguments[i].compare("-ofdir") == 0) 
		{
			out_pts_dir = arguments[i + 1];
			create_directories(out_pts_dir);
			valid[i] = false;
			valid[i+1] = false;
			i++;
		}
		else if (arguments[i].compare("-opdir") == 0)
		{
			out_pose_dir = arguments[i + 1];
			create_directories(out_pose_dir);
			valid[i] = false;
			valid[i + 1] = false;
			i++;
		}
		else if (arguments[i].compare("-oidir") == 0)
		{
			out_img_dir = arguments[i + 1];
			create_directories(out_img_dir);
			valid[i] = false;
			valid[i+1] = false;
			i++;
		}
		else if (arguments[i].compare("-op") == 0)
		{
			output_pose_files.push_back(arguments[i + 1]);
			valid[i] = false;
			valid[i + 1] = false;
			i++;
		}
		else if (arguments[i].compare("-of") == 0)
		{
			output_feature_files.push_back(arguments[i + 1]);
			valid[i] = false;
			valid[i+1] = false;
			i++;
		} 
		else if (arguments[i].compare("-oi") == 0)
		{
			output_image_files.push_back(arguments[i + 1]);
			valid[i] = false;
			valid[i+1] = false;
			i++;
		} 
		else if (arguments[i].compare("-help") == 0)
		{
			cout << "Input output files are defined as: -f <infile (can have multiple ones)> -of <where detected landmarks should be stored(can have multiple ones)> -oi <where should images with detected landmarks should be written (can have multiple ones)> -fdir <the directory containing .png and .jpg files to be processed (with optional .txt files corresponding to EACH image containing the bounding boxes> " << endl; // Inform the user of how to use the program				
		}
	}

	// If any output directories are defined populate them based on image names
	if(!out_img_dir.empty())
	{
		for(size_t i=0; i < input_image_files.size(); ++i)
		{
			path image_loc(input_image_files[i]);

			path fname = image_loc.filename();
			fname = fname.replace_extension("jpg");
			output_image_files.push_back(out_img_dir + "/" + fname.string());
			
		}
		if(!input_image_files.empty())
		{
			create_directory_from_file(output_image_files[0]);
		}
	}

	if(!out_pts_dir.empty())
	{
		for(size_t i=0; i < input_image_files.size(); ++i)
		{
			path image_loc(input_image_files[i]);

			path fname = image_loc.filename();
			fname = fname.replace_extension("pts");
			output_feature_files.push_back(out_pts_dir + "/" + fname.string());			
		}
		create_directory_from_file(output_feature_files[0]);
	}

	if (!out_pose_dir.empty())
	{
		for (size_t i = 0; i < input_image_files.size(); ++i)
		{
			path image_loc(input_image_files[i]);

			path fname = image_loc.filename();
			fname = fname.replace_extension("pose");
			output_pose_files.push_back(out_pose_dir + "/" + fname.string());
		}
		create_directory_from_file(output_pose_files[0]);
	}

	// Make sure the same number of images and bounding boxes is present, if any bounding boxes are defined
	if(input_bounding_boxes.size() > 0)
	{
		assert(input_bounding_boxes.size() == input_image_files.size());
	}

	// Clear up the argument list
	for(int i=arguments.size()-1; i >= 0; --i)
	{
		if(!valid[i])
		{
			arguments.erase(arguments.begin()+i);
		}
	}

}

//===========================================================================
// Fast patch expert response computation (linear model across a ROI) using normalised cross-correlation
//===========================================================================

void crossCorr_m( const Mat_<float>& img, Mat_<double>& img_dft, const Mat_<float>& _templ, map<int, cv::Mat_<double> >& _templ_dfts, Mat_<float>& corr)
{
	// Our model will always be under min block size so can ignore this
    //const double blockScale = 4.5;
    //const int minBlockSize = 256;

	int maxDepth = CV_64F;

	Size dftsize;
	
    dftsize.width = getOptimalDFTSize(corr.cols + _templ.cols - 1);
    dftsize.height = getOptimalDFTSize(corr.rows + _templ.rows - 1);

    // Compute block size
    Size blocksize;
    blocksize.width = dftsize.width - _templ.cols + 1;
    blocksize.width = MIN( blocksize.width, corr.cols );
    blocksize.height = dftsize.height - _templ.rows + 1;
    blocksize.height = MIN( blocksize.height, corr.rows );
	
	cv::Mat_<double> dftTempl;

	// if this has not been precomputed, precompute it, otherwise use it
	if(_templ_dfts.find(dftsize.width) == _templ_dfts.end())
	{
		dftTempl.create(dftsize.height, dftsize.width);

		cv::Mat_<float> src = _templ;

		// TODO simplify no need for rect?
		cv::Mat_<double> dst(dftTempl, cv::Rect(0, 0, dftsize.width, dftsize.height));
		
		cv::Mat_<double> dst1(dftTempl, cv::Rect(0, 0, _templ.cols, _templ.rows));
			
		if( dst1.data != src.data )
			src.convertTo(dst1, dst1.depth());

		if( dst.cols > _templ.cols )
		{
			cv::Mat_<double> part(dst, cv::Range(0, _templ.rows), cv::Range(_templ.cols, dst.cols));
			part.setTo(0);
		}

		// Perform DFT of the template
		dft(dst, dst, 0, _templ.rows);
		
		_templ_dfts[dftsize.width] = dftTempl;

	}
	else
	{
		// use the precomputed version
		dftTempl = _templ_dfts.find(dftsize.width)->second;
	}

	Size bsz(std::min(blocksize.width, corr.cols), std::min(blocksize.height, corr.rows));
	Mat src;

	Mat cdst(corr, Rect(0, 0, bsz.width, bsz.height));
	
	cv::Mat_<double> dftImg;

	if(img_dft.empty())
	{
		dftImg.create(dftsize);
		dftImg.setTo(0.0);

		Size dsz(bsz.width + _templ.cols - 1, bsz.height + _templ.rows - 1);

		int x2 = std::min(img.cols, dsz.width);
		int y2 = std::min(img.rows, dsz.height);

		Mat src0(img, Range(0, y2), Range(0, x2));
		Mat dst(dftImg, Rect(0, 0, dsz.width, dsz.height));
		Mat dst1(dftImg, Rect(0, 0, x2, y2));

		src = src0;
		
		if( dst1.data != src.data )
			src.convertTo(dst1, dst1.depth());

		dft( dftImg, dftImg, 0, dsz.height );
		img_dft = dftImg.clone();
	}

	Mat dftTempl1(dftTempl, Rect(0, 0, dftsize.width, dftsize.height));
	mulSpectrums(img_dft, dftTempl1, dftImg, 0, true);
	dft( dftImg, dftImg, DFT_INVERSE + DFT_SCALE, bsz.height );

	src = dftImg(Rect(0, 0, bsz.width, bsz.height));

	src.convertTo(cdst, CV_32F);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////

void matchTemplate_m(  const Mat_<float>& input_img, Mat_<double>& img_dft, cv::Mat& _integral_img, cv::Mat& _integral_img_sq, const Mat_<float>&  templ, map<int, Mat_<double> >& templ_dfts, Mat_<float>& result, int method )
{

        int numType = method == CV_TM_CCORR || method == CV_TM_CCORR_NORMED ? 0 :
                  method == CV_TM_CCOEFF || method == CV_TM_CCOEFF_NORMED ? 1 : 2;
    bool isNormed = method == CV_TM_CCORR_NORMED ||
                    method == CV_TM_SQDIFF_NORMED ||
                    method == CV_TM_CCOEFF_NORMED;
	
	// Assume result is defined properly
	if(result.empty())
	{
		Size corrSize(input_img.cols - templ.cols + 1, input_img.rows - templ.rows + 1);
		result.create(corrSize);
	}
    CLMTracker::crossCorr_m( input_img, img_dft, templ, templ_dfts, result);

    if( method == CV_TM_CCORR )
        return;

    double invArea = 1./((double)templ.rows * templ.cols);

    Mat sum, sqsum;
    Scalar templMean, templSdv;
    double *q0 = 0, *q1 = 0, *q2 = 0, *q3 = 0;
    double templNorm = 0, templSum2 = 0;

    if( method == CV_TM_CCOEFF )
    {
		// If it has not been precomputed compute it now
		if(_integral_img.empty())
		{
			integral(input_img, _integral_img, CV_64F);
		}
		sum = _integral_img;

        templMean = mean(templ);
    }
    else
    {
		// If it has not been precomputed compute it now
		if(_integral_img.empty())
		{
			integral(input_img, _integral_img, _integral_img_sq, CV_64F);			
		}

		sum = _integral_img;
		sqsum = _integral_img_sq;

        meanStdDev( templ, templMean, templSdv );

        templNorm = templSdv[0]*templSdv[0] + templSdv[1]*templSdv[1] + templSdv[2]*templSdv[2] + templSdv[3]*templSdv[3];

        if( templNorm < DBL_EPSILON && method == CV_TM_CCOEFF_NORMED )
        {
			result.setTo(1.0);
            return;
        }

        templSum2 = templNorm + templMean[0]*templMean[0] + templMean[1]*templMean[1] + templMean[2]*templMean[2] + templMean[3]*templMean[3];

        if( numType != 1 )
        {
            templMean = Scalar::all(0);
            templNorm = templSum2;
        }

        templSum2 /= invArea;
        templNorm = std::sqrt(templNorm);
        templNorm /= std::sqrt(invArea); // care of accuracy here

        q0 = (double*)sqsum.data;
        q1 = q0 + templ.cols;
        q2 = (double*)(sqsum.data + templ.rows*sqsum.step);
        q3 = q2 + templ.cols;
    }

    double* p0 = (double*)sum.data;
    double* p1 = p0 + templ.cols;
    double* p2 = (double*)(sum.data + templ.rows*sum.step);
    double* p3 = p2 + templ.cols;

    int sumstep = sum.data ? (int)(sum.step / sizeof(double)) : 0;
    int sqstep = sqsum.data ? (int)(sqsum.step / sizeof(double)) : 0;

    int i, j;

    for( i = 0; i < result.rows; i++ )
    {
        float* rrow = result.ptr<float>(i);
        int idx = i * sumstep;
        int idx2 = i * sqstep;

        for( j = 0; j < result.cols; j++, idx += 1, idx2 += 1 )
        {
            double num = rrow[j], t;
            double wndMean2 = 0, wndSum2 = 0;

            if( numType == 1 )
            {

                t = p0[idx] - p1[idx] - p2[idx] + p3[idx];
                wndMean2 += t*t;
                num -= t*templMean[0];

                wndMean2 *= invArea;
            }

            if( isNormed || numType == 2 )
            {

                t = q0[idx2] - q1[idx2] - q2[idx2] + q3[idx2];
                wndSum2 += t;

                if( numType == 2 )
                {
                    num = wndSum2 - 2*num + templSum2;
                    num = MAX(num, 0.);
                }
            }

            if( isNormed )
            {
                t = std::sqrt(MAX(wndSum2 - wndMean2,0))*templNorm;
                if( fabs(num) < t )
                    num /= t;
                else if( fabs(num) < t*1.125 )
                    num = num > 0 ? 1 : -1;
                else
                    num = method != CV_TM_SQDIFF_NORMED ? 0 : 1;
            }

            rrow[j] = (float)num;
        }
    }
}


//===========================================================================
// Point set and landmark manipulation functions
//===========================================================================
// Using Kabsch's algorithm for aligning shapes
//This assumes that align_from and align_to are already mean normalised
Matx22d AlignShapesKabsch2D(const Mat_<double>& align_from, const Mat_<double>& align_to )
{

	cv::SVD svd(align_from.t() * align_to);
    
	// make sure no reflection is there
	// corr ensures that we do only rotaitons and not reflections
	double d = cv::determinant(svd.vt.t() * svd.u.t());

	cv::Matx22d corr = cv::Matx22d::eye();
	if(d > 0)
	{
		corr(1,1) = 1;
	}
	else
	{
		corr(1,1) = -1;
	}

    Matx22d R;
	Mat(svd.vt.t()*cv::Mat(corr)*svd.u.t()).copyTo(R);

	return R;
}

//=============================================================================
// Basically Kabsch's algorithm but also allows the collection of points to be different in scale from each other
Matx22d AlignShapesWithScale(cv::Mat_<double>& src, cv::Mat_<double> dst)
{
	int n = src.rows;

	// First we mean normalise both src and dst
	double mean_src_x = cv::mean(src.col(0))[0];
	double mean_src_y = cv::mean(src.col(1))[0];

	double mean_dst_x = cv::mean(dst.col(0))[0];
	double mean_dst_y = cv::mean(dst.col(1))[0];

	Mat_<double> src_mean_normed = src.clone();
	src_mean_normed.col(0) = src_mean_normed.col(0) - mean_src_x;
	src_mean_normed.col(1) = src_mean_normed.col(1) - mean_src_y;

	Mat_<double> dst_mean_normed = dst.clone();
	dst_mean_normed.col(0) = dst_mean_normed.col(0) - mean_dst_x;
	dst_mean_normed.col(1) = dst_mean_normed.col(1) - mean_dst_y;

	// Find the scaling factor of each
	Mat src_sq;
	cv::pow(src_mean_normed, 2, src_sq);

	Mat dst_sq;
	cv::pow(dst_mean_normed, 2, dst_sq);

	double s_src = sqrt(cv::sum(src_sq)[0]/n);
	double s_dst = sqrt(cv::sum(dst_sq)[0]/n);

	src_mean_normed = src_mean_normed / s_src;
	dst_mean_normed = dst_mean_normed / s_dst;

	double s = s_dst / s_src;

	// Get the rotation
	Matx22d R = AlignShapesKabsch2D(src_mean_normed, dst_mean_normed);
		
	Matx22d	A;
	Mat(s * R).copyTo(A);

	Mat_<double> aligned = (Mat(Mat(A) * src.t())).t();
    Mat_<double> offset = dst - aligned;

	double t_x =  cv::mean(offset.col(0))[0];
	double t_y =  cv::mean(offset.col(1))[0];
    
	return A;

}


//===========================================================================
// Visualisation functions
//===========================================================================
void Project(Mat_<double>& dest, const Mat_<double>& mesh, double fx, double fy, double cx, double cy)
{
	dest = Mat_<double>(mesh.rows,2, 0.0);

	int num_points = mesh.rows;

	double X, Y, Z;


	Mat_<double>::const_iterator mData = mesh.begin();
	Mat_<double>::iterator projected = dest.begin();

	for(int i = 0;i < num_points; i++)
	{
		// Get the points
		X = *(mData++);
		Y = *(mData++);
		Z = *(mData++);
			
		double x;
		double y;

		// if depth is 0 the projection is different
		if(Z != 0)
		{
			x = ((X * fx / Z) + cx);
			y = ((Y * fy / Z) + cy);
		}
		else
		{
			x = X;
			y = Y;
		}

		// Project and store in dest matrix
		(*projected++) = x;
		(*projected++) = y;
	}

}

void DrawBox(Mat image, Vec6d pose, Scalar color, int thickness, float fx, float fy, float cx, float cy)
{
	double boxVerts[] = {-1, 1, -1,
						1, 1, -1,
						1, 1, 1,
						-1, 1, 1,
						1, -1, 1,
						1, -1, -1,
						-1, -1, -1,
						-1, -1, 1};

	vector<std::pair<int,int>> edges;
	edges.push_back(pair<int,int>(0,1));
	edges.push_back(pair<int,int>(1,2));
	edges.push_back(pair<int,int>(2,3));
	edges.push_back(pair<int,int>(0,3));
	edges.push_back(pair<int,int>(2,4));
	edges.push_back(pair<int,int>(1,5));
	edges.push_back(pair<int,int>(0,6));
	edges.push_back(pair<int,int>(3,7));
	edges.push_back(pair<int,int>(6,5));
	edges.push_back(pair<int,int>(5,4));
	edges.push_back(pair<int,int>(4,7));
	edges.push_back(pair<int,int>(7,6));

	// The size of the head is roughly 200mm x 200mm x 200mm
	Mat_<double> box = Mat(8, 3, CV_64F, boxVerts).clone() * 100;

	Matx33d rot = CLMTracker::Euler2RotationMatrix(Vec3d(pose[3], pose[4], pose[5]));
	Mat_<double> rotBox;
	
	// Rotate the box
	rotBox = Mat(rot) * box.t();
	rotBox = rotBox.t();

	// Move the bounding box to head position
	rotBox.col(0) = rotBox.col(0) + pose[0];
	rotBox.col(1) = rotBox.col(1) + pose[1];
	rotBox.col(2) = rotBox.col(2) + pose[2];

	// draw the lines
	Mat_<double> rotBoxProj;
	Project(rotBoxProj, rotBox, fx, fy, cx, cy);

	Rect image_rect(0,0,image.cols, image.rows);
	
	for (size_t i = 0; i < edges.size(); ++i)
	{
		Mat_<double> begin;
		Mat_<double> end;
	
		rotBoxProj.row(edges[i].first).copyTo(begin);
		rotBoxProj.row(edges[i].second).copyTo(end);

		Point p1((int)begin.at<double>(0), (int)begin.at<double>(1));
		Point p2((int)end.at<double>(0), (int)end.at<double>(1));
		
		// Only draw the line if one of the points is inside the image
		if(p1.inside(image_rect) || p2.inside(image_rect))
		{
			cv::line(image, p1, p2, color, thickness);
		}
		
	}

}

vector<std::pair<Point,Point>> CalculateBox(Vec6d pose, float fx, float fy, float cx, float cy)
{
	double boxVerts[] = {-1, 1, -1,
						1, 1, -1,
						1, 1, 1,
						-1, 1, 1,
						1, -1, 1,
						1, -1, -1,
						-1, -1, -1,
						-1, -1, 1};

	vector<std::pair<int,int>> edges;
	edges.push_back(pair<int,int>(0,1));
	edges.push_back(pair<int,int>(1,2));
	edges.push_back(pair<int,int>(2,3));
	edges.push_back(pair<int,int>(0,3));
	edges.push_back(pair<int,int>(2,4));
	edges.push_back(pair<int,int>(1,5));
	edges.push_back(pair<int,int>(0,6));
	edges.push_back(pair<int,int>(3,7));
	edges.push_back(pair<int,int>(6,5));
	edges.push_back(pair<int,int>(5,4));
	edges.push_back(pair<int,int>(4,7));
	edges.push_back(pair<int,int>(7,6));

	// The size of the head is roughly 200mm x 200mm x 200mm
	Mat_<double> box = Mat(8, 3, CV_64F, boxVerts).clone() * 100;

	Matx33d rot = CLMTracker::Euler2RotationMatrix(Vec3d(pose[3], pose[4], pose[5]));
	Mat_<double> rotBox;
	
	// Rotate the box
	rotBox = Mat(rot) * box.t();
	rotBox = rotBox.t();

	// Move the bounding box to head position
	rotBox.col(0) = rotBox.col(0) + pose[0];
	rotBox.col(1) = rotBox.col(1) + pose[1];
	rotBox.col(2) = rotBox.col(2) + pose[2];

	// draw the lines
	Mat_<double> rotBoxProj;
	Project(rotBoxProj, rotBox, fx, fy, cx, cy);

	vector<std::pair<Point,Point>> lines;
	
	for (size_t i = 0; i < edges.size(); ++i)
	{
		Mat_<double> begin;
		Mat_<double> end;
	
		rotBoxProj.row(edges[i].first).copyTo(begin);
		rotBoxProj.row(edges[i].second).copyTo(end);

		Point p1((int)begin.at<double>(0), (int)begin.at<double>(1));
		Point p2((int)end.at<double>(0), (int)end.at<double>(1));
		
		lines.push_back(pair<Point, Point>(p1,p2));
		
	}

	return lines;
}

void DrawBox(vector<pair<Point, Point>> lines, Mat image, Scalar color, int thickness)
{
	Rect image_rect(0,0,image.cols, image.rows);
	
	for (size_t i = 0; i < lines.size(); ++i)
	{
		Point p1 = lines.at(i).first;
		Point p2 = lines.at(i).second;
		// Only draw the line if one of the points is inside the image
		if(p1.inside(image_rect) || p2.inside(image_rect))
		{
			cv::line(image, p1, p2, color, thickness);
		}
		
	}

}

// Computing landmarks (to be drawn later possibly)
vector<Point2d> CalculateLandmarks(const Mat_<double>& shape2D, Mat_<int>& visibilities)
{
	int n = shape2D.rows/2;
	vector<Point2d> landmarks;

	for( int i = 0; i < n; ++i)
	{		
		if(visibilities.at<int>(i))
		{
			Point2d featurePoint(shape2D.at<double>(i), shape2D.at<double>(i +n));

			landmarks.push_back(featurePoint);
		}
	}

	return landmarks;
}

// Computing landmarks (to be drawn later possibly)
vector<Point2d> CalculateLandmarks(cv::Mat img, const Mat_<double>& shape2D)
{
	
	int n;
	vector<Point2d> landmarks;
	
	if(shape2D.cols == 2)
	{
		n = shape2D.rows;
	}
	else if(shape2D.cols == 1)
	{
		n = shape2D.rows/2;
	}

	for( int i = 0; i < n; ++i)
	{		
		Point2d featurePoint;
		if(shape2D.cols == 1)
		{
			featurePoint = Point2d(shape2D.at<double>(i), shape2D.at<double>(i +n));
		}
		else
		{
			featurePoint = Point2d(shape2D.at<double>(i, 0), shape2D.at<double>(i, 1));
		}

		landmarks.push_back(featurePoint);
	}
	
	return landmarks;
}

// Computing landmarks (to be drawn later possibly)
vector<cv::Point2d> CalculateLandmarks(CLM& clm_model)
{

	int idx = clm_model.patch_experts.GetViewIdx(clm_model.params_global, 0);

	// Because we only draw visible points, need to find which points patch experts consider visible at a certain orientation
	return CalculateLandmarks(clm_model.detected_landmarks, clm_model.patch_experts.visibilities[0][idx]);

}

// Drawing landmarks on a face image
void Draw(cv::Mat img, const Mat_<double>& shape2D, const Mat_<int>& visibilities)
{
	int n = shape2D.rows/2;

	// Drawing feature points
	if(n >= 66)
	{
		for( int i = 0; i < n; ++i)
		{		
			if(visibilities.at<int>(i))
			{
				Point featurePoint((int)shape2D.at<double>(i), (int)shape2D.at<double>(i +n));

				// A rough heuristic for drawn point size
				int thickness = (int)std::ceil(3.0* ((double)img.cols) / 640.0);
				int thickness_2 = (int)std::ceil(1.0* ((double)img.cols) / 640.0);

				cv::circle(img, featurePoint, 1, Scalar(0,0,255), thickness);
				cv::circle(img, featurePoint, 1, Scalar(255,0,0), thickness_2);
			}
		}
	}
	else if(n == 28) // drawing eyes
	{
		for( int i = 0; i < n; ++i)
		{		
			Point featurePoint((int)shape2D.at<double>(i), (int)shape2D.at<double>(i +n));

			// A rough heuristic for drawn point size
			int thickness = 1.0;
			int thickness_2 = 1.0;

			int next_point = i + 1;
			if(i == 7)
				next_point = 0;
			if(i == 19)
				next_point = 8;
			if(i == 27)
				next_point = 20;

			Point nextFeaturePoint((int)shape2D.at<double>(next_point), (int)shape2D.at<double>(next_point+n));
			if( i < 8 || i > 19)
				cv::line(img, featurePoint, nextFeaturePoint, Scalar(255, 0, 0), thickness_2);
			else
				cv::line(img, featurePoint, nextFeaturePoint, Scalar(0, 0, 255), thickness_2);

			//cv::circle(img, featurePoint, 1, Scalar(0,255,0), thickness);
			//cv::circle(img, featurePoint, 1, Scalar(0,0,255), thickness_2);
			

		}
	}
	else if(n == 6)
	{
		for( int i = 0; i < n; ++i)
		{		
			Point featurePoint((int)shape2D.at<double>(i), (int)shape2D.at<double>(i +n));

			// A rough heuristic for drawn point size
			int thickness = 1.0;
			int thickness_2 = 1.0;

			//cv::circle(img, featurePoint, 1, Scalar(0,255,0), thickness);
			//cv::circle(img, featurePoint, 1, Scalar(0,0,255), thickness_2);
			
			int next_point = i + 1;
			if(i == 5)
				next_point = 0;

			Point nextFeaturePoint((int)shape2D.at<double>(next_point), (int)shape2D.at<double>(next_point+n));
			cv::line(img, featurePoint, nextFeaturePoint, Scalar(255, 0, 0), thickness_2);
		}
	}
}

// Drawing landmarks on a face image
void Draw(cv::Mat img, const Mat_<double>& shape2D)
{
	
	int n;
	
	if(shape2D.cols == 2)
	{
		n = shape2D.rows;
	}
	else if(shape2D.cols == 1)
	{
		n = shape2D.rows/2;
	}

	for( int i = 0; i < n; ++i)
	{		
		Point featurePoint;
		if(shape2D.cols == 1)
		{
			featurePoint = Point((int)shape2D.at<double>(i), (int)shape2D.at<double>(i +n));
		}
		else
		{
			featurePoint = Point((int)shape2D.at<double>(i, 0), (int)shape2D.at<double>(i, 1));
		}
		// A rough heuristic for drawn point size
		int thickness = (int)std::ceil(5.0* ((double)img.cols) / 640.0);
		int thickness_2 = (int)std::ceil(1.5* ((double)img.cols) / 640.0);

		cv::circle(img, featurePoint, 1, Scalar(0,0,255), thickness);
		cv::circle(img, featurePoint, 1, Scalar(255,0,0), thickness_2);

	}
	
}

// Drawing detected landmarks on a face image
void Draw(cv::Mat img, const CLM& clm_model)
{

	int idx = clm_model.patch_experts.GetViewIdx(clm_model.params_global, 0);

	// Because we only draw visible points, need to find which points patch experts consider visible at a certain orientation
	Draw(img, clm_model.detected_landmarks, clm_model.patch_experts.visibilities[0][idx]);

	// If the model has hierarchical updates draw those too
	for(size_t i = 0; i < clm_model.hierarchical_models.size(); ++i)
	{
		if(clm_model.hierarchical_models[i].pdm.NumberOfPoints() != clm_model.hierarchical_mapping[i].size())
		{
			Draw(img, clm_model.hierarchical_models[i]);
		}
	}
}

void DrawLandmarks(cv::Mat img, vector<Point> landmarks)
{
	for(Point p : landmarks)
	{		
		// A rough heuristic for drawn point size
		int thickness = (int)std::ceil(5.0* ((double)img.cols) / 640.0);
		int thickness_2 = (int)std::ceil(1.5* ((double)img.cols) / 640.0);

		cv::circle(img, p, 1, Scalar(0,0,255), thickness);
		cv::circle(img, p, 1, Scalar(255,0,0), thickness_2);
	}
	
}

//===========================================================================
// Angle representation conversion helpers
//===========================================================================

// Using the XYZ convention R = Rx * Ry * Rz, left-handed positive sign
Matx33d Euler2RotationMatrix(const Vec3d& eulerAngles)
{
	Matx33d rotation_matrix;

	double s1 = sin(eulerAngles[0]);
	double s2 = sin(eulerAngles[1]);
	double s3 = sin(eulerAngles[2]);

	double c1 = cos(eulerAngles[0]);
	double c2 = cos(eulerAngles[1]);
	double c3 = cos(eulerAngles[2]);

	rotation_matrix(0,0) = c2 * c3;
	rotation_matrix(0,1) = -c2 *s3;
	rotation_matrix(0,2) = s2;
	rotation_matrix(1,0) = c1 * s3 + c3 * s1 * s2;
	rotation_matrix(1,1) = c1 * c3 - s1 * s2 * s3;
	rotation_matrix(1,2) = -c2 * s1;
	rotation_matrix(2,0) = s1 * s3 - c1 * c3 * s2;
	rotation_matrix(2,1) = c3 * s1 + c1 * s2 * s3;
	rotation_matrix(2,2) = c1 * c2;

	return rotation_matrix;
}

// Using the XYZ convention R = Rx * Ry * Rz, left-handed positive sign
Vec3d RotationMatrix2Euler(const Matx33d& rotation_matrix)
{
	double q0 = sqrt( 1 + rotation_matrix(0,0) + rotation_matrix(1,1) + rotation_matrix(2,2) ) / 2.0;
	double q1 = (rotation_matrix(2,1) - rotation_matrix(1,2)) / (4.0*q0) ;
	double q2 = (rotation_matrix(0,2) - rotation_matrix(2,0)) / (4.0*q0) ;
	double q3 = (rotation_matrix(1,0) - rotation_matrix(0,1)) / (4.0*q0) ;

	double t1 = 2.0 * (q0*q2 + q1*q3);

	double yaw  = asin(2.0 * (q0*q2 + q1*q3));
	double pitch= atan2(2.0 * (q0*q1-q2*q3), q0*q0-q1*q1-q2*q2+q3*q3); 
	double roll = atan2(2.0 * (q0*q3-q1*q2), q0*q0+q1*q1-q2*q2-q3*q3);
    
	return Vec3d(pitch, yaw, roll);
}

Vec3d Euler2AxisAngle(const Vec3d& euler)
{
	Matx33d rotMatrix = CLMTracker::Euler2RotationMatrix(euler);
	Vec3d axis_angle;
	cv::Rodrigues(rotMatrix, axis_angle);
	return axis_angle;
}

Vec3d AxisAngle2Euler(const Vec3d& axis_angle)
{
	Matx33d rotation_matrix;
	cv::Rodrigues(axis_angle, rotation_matrix);
	return RotationMatrix2Euler(rotation_matrix);
}

Matx33d AxisAngle2RotationMatrix(const Vec3d& axis_angle)
{
	Matx33d rotation_matrix;
	cv::Rodrigues(axis_angle, rotation_matrix);
	return rotation_matrix;
}

Vec3d RotationMatrix2AxisAngle(const Matx33d& rotation_matrix)
{
	Vec3d axis_angle;
	cv::Rodrigues(rotation_matrix, axis_angle);
	return axis_angle;
}

//===========================================================================

//============================================================================
// Face detection helpers
//============================================================================
bool DetectFaces(vector<Rect_<double> >& o_regions, const Mat_<uchar>& intensity)
{
	CascadeClassifier classifier("./classifiers/haarcascade_frontalface_alt.xml");
	if(classifier.empty())
	{
		cout << "Couldn't load the Haar cascade classifier" << endl;
		return false;
	}
	else
	{
		return DetectFaces(o_regions, intensity, classifier);
	}

}

bool DetectFaces(vector<Rect_<double> >& o_regions, const Mat_<uchar>& intensity, CascadeClassifier& classifier)
{
		
	vector<Rect> face_detections;
	classifier.detectMultiScale(intensity, face_detections, 1.2, 2, 0, Size(50, 50)); 		

	// Convert from int bounding box do a double one with corrections
	o_regions.resize(face_detections.size());

	for( size_t face = 0; face < o_regions.size(); ++face)
	{
		// OpenCV is overgenerous with face size and y location is off
		// CLM expect the bounding box to encompass from eyebrow to chin in y, and from cheeck outline to cheeck outline in x, so we need to compensate

		// The scalings were learned using the Face Detections on LFPW, Helen, AFW and iBUG datasets, using ground truth and detections from openCV

		// Correct for scale
		o_regions[face].width = face_detections[face].width * 0.8924; 
		o_regions[face].height = face_detections[face].height * 0.8676;

		// Move the face slightly to the right (as the width was made smaller)
		o_regions[face].x = face_detections[face].x + 0.0578 * face_detections[face].width;
		// Shift face down as OpenCV Haar Cascade detects the forehead as well, and we're not interested
		o_regions[face].y = face_detections[face].y + face_detections[face].height * 0.2166;
		
		
	}
	return o_regions.size() > 0;
}

bool DetectSingleFace(Rect_<double>& o_region, const Mat_<uchar>& intensity_image, CascadeClassifier& classifier, cv::Point preference)
{
	// The tracker can return multiple faces
	vector<Rect_<double> > face_detections;
				
	bool detect_success = CLMTracker::DetectFaces(face_detections, intensity_image, classifier);
					
	if(detect_success)
	{
		
		bool use_preferred = (preference.x != -1) && (preference.y != -1);

		if(face_detections.size() > 1)
		{
			// keep the closest one if preference point not set
			double best = -1;
			int bestIndex = -1;
			for( size_t i = 0; i < face_detections.size(); ++i)
			{
				double dist;
				bool better;

				if(use_preferred)
				{
					dist = sqrt((preference.x) * (face_detections[i].width/2 + face_detections[i].x) + 
								(preference.y) * (face_detections[i].height/2 + face_detections[i].y));
					better = dist < best;
				}
				else
				{
					dist = face_detections[i].width;
					better = face_detections[i].width > best;
				}

				// Pick a closest face to preffered point or the biggest face
				if(i == 0 || better)
				{
					bestIndex = i;	
					best = dist;
				}									
			}

			o_region = face_detections[bestIndex];

		}
		else
		{	
			o_region = face_detections[0];
		}				
	
	}
	else
	{
		// if not detected
		o_region = Rect_<double>(0,0,0,0);
	}
	return detect_success;
}

bool DetectFacesHOG(vector<Rect_<double> >& o_regions, const Mat_<uchar>& intensity, std::vector<double>& confidences)
{
	dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();

	return DetectFacesHOG(o_regions, intensity, detector, confidences);

}

bool DetectFacesHOG(vector<Rect_<double> >& o_regions, const Mat_<uchar>& intensity, dlib::frontal_face_detector& detector, std::vector<double>& o_confidences)
{
		
	Mat_<uchar> upsampled_intensity;

	double scaling = 1.3;

	cv::resize(intensity, upsampled_intensity, cv::Size((int)(intensity.cols * scaling), (int)(intensity.rows * scaling)));

	dlib::cv_image<uchar> cv_grayscale(upsampled_intensity);

	std::vector<dlib::full_detection> face_detections;
	detector(cv_grayscale, face_detections, -0.2);

	// Convert from int bounding box do a double one with corrections
	o_regions.resize(face_detections.size());
	o_confidences.resize(face_detections.size());

	for( size_t face = 0; face < o_regions.size(); ++face)
	{
		// CLM expect the bounding box to encompass from eyebrow to chin in y, and from cheeck outline to cheeck outline in x, so we need to compensate

		// The scalings were learned using the Face Detections on LFPW and Helen using ground truth and detections from the HOG detector

		// Move the face slightly to the right (as the width was made smaller)
		o_regions[face].x = (face_detections[face].rect.get_rect().tl_corner().x() + 0.0389 * face_detections[face].rect.get_rect().width())/scaling;
		// Shift face down as OpenCV Haar Cascade detects the forehead as well, and we're not interested
		o_regions[face].y = (face_detections[face].rect.get_rect().tl_corner().y() + 0.1278 * face_detections[face].rect.get_rect().height())/scaling;

		// Correct for scale
		o_regions[face].width = (face_detections[face].rect.get_rect().width() * 0.9611)/scaling; 
		o_regions[face].height = (face_detections[face].rect.get_rect().height() * 0.9388)/scaling;

		o_confidences[face] = face_detections[face].detection_confidence;
		
		
	}
	return o_regions.size() > 0;
}

bool DetectSingleFaceHOG(Rect_<double>& o_region, const Mat_<uchar>& intensity_img, dlib::frontal_face_detector& detector, double& confidence, cv::Point preference)
{
	// The tracker can return multiple faces
	vector<Rect_<double> > face_detections;
	vector<double> confidences;

	bool detect_success = CLMTracker::DetectFacesHOG(face_detections, intensity_img, detector, confidences);
					
	if(detect_success)
	{

		bool use_preferred = (preference.x != -1) && (preference.y != -1);

		// keep the most confident one or the one closest to preference point if set
		double best_so_far;
		if(use_preferred)
		{			
			best_so_far = sqrt((preference.x - (face_detections[0].width/2 + face_detections[0].x)) * (preference.x - (face_detections[0].width/2 + face_detections[0].x)) + 
							   (preference.y - (face_detections[0].height/2 + face_detections[0].y)) * (preference.y - (face_detections[0].height/2 + face_detections[0].y)));
		}
		else
		{
			best_so_far = confidences[0];
		}
		int bestIndex = 0;

		for( size_t i = 1; i < face_detections.size(); ++i)
		{

			double dist;
			bool better;

			if(use_preferred)
			{
				dist = sqrt((preference.x - (face_detections[0].width/2 + face_detections[0].x)) * (preference.x - (face_detections[0].width/2 + face_detections[0].x)) + 
							   (preference.y - (face_detections[0].height/2 + face_detections[0].y)) * (preference.y - (face_detections[0].height/2 + face_detections[0].y)));
				better = dist < best_so_far;
			}
			else
			{
				dist = confidences[i];
				better = dist > best_so_far;
			}

			// Pick a closest face
			if(better)
			{
				best_so_far = dist;
				bestIndex = i;
			}									
		}

		o_region = face_detections[bestIndex];
		confidence = confidences[bestIndex];
	}
	else
	{
		// if not detected
		o_region = Rect_<double>(0,0,0,0);
		// A completely unreliable detection (shouldn't really matter what is returned here)
		confidence = -2;		
	}
	return detect_success;
}

//============================================================================
// Matrix reading functionality
//============================================================================

// Reading in a matrix from a stream
void ReadMat(std::ifstream& stream, Mat &output_mat)
{
	// Read in the number of rows, columns and the data type
	int row,col,type;
	
	stream >> row >> col >> type;

	output_mat = cv::Mat(row, col, type);
	
	switch(output_mat.type())
	{
		case CV_64FC1: 
		{
			cv::MatIterator_<double> begin_it = output_mat.begin<double>();
			cv::MatIterator_<double> end_it = output_mat.end<double>();
			
			while(begin_it != end_it)
			{
				stream >> *begin_it++;
			}
		}
		break;
		case CV_32FC1:
		{
			cv::MatIterator_<float> begin_it = output_mat.begin<float>();
			cv::MatIterator_<float> end_it = output_mat.end<float>();

			while(begin_it != end_it)
			{
				stream >> *begin_it++;
			}
		}
		break;
		case CV_32SC1:
		{
			cv::MatIterator_<int> begin_it = output_mat.begin<int>();
			cv::MatIterator_<int> end_it = output_mat.end<int>();
			while(begin_it != end_it)
			{
				stream >> *begin_it++;
			}
		}
		break;
		case CV_8UC1:
		{
			cv::MatIterator_<uchar> begin_it = output_mat.begin<uchar>();
			cv::MatIterator_<uchar> end_it = output_mat.end<uchar>();
			while(begin_it != end_it)
			{
				stream >> *begin_it++;
			}
		}
		break;
		default:
			printf("ERROR(%s,%d) : Unsupported Matrix type %d!\n", __FILE__,__LINE__,output_mat.type()); abort();


	}
}

void ReadMatBin(std::ifstream& stream, Mat &output_mat)
{
	// Read in the number of rows, columns and the data type
	int row, col, type;
	
	stream.read ((char*)&row, 4);
	stream.read ((char*)&col, 4);
	stream.read ((char*)&type, 4);
	
	output_mat = cv::Mat(row, col, type);
	int size = output_mat.rows * output_mat.cols * output_mat.elemSize();
	stream.read((char *)output_mat.data, size);

}

// Skipping lines that start with # (together with empty lines)
void SkipComments(std::ifstream& stream)
{	
	while(stream.peek() == '#' || stream.peek() == '\n'|| stream.peek() == ' ' || stream.peek() == '\r')
	{
		std::string skipped;
		std::getline(stream, skipped);
	}
}

}
