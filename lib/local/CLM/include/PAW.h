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
#ifndef __PAW_h_
#define __PAW_h_

using namespace cv;

namespace CLMTracker
{
  //===========================================================================
  /** 
      A Piece-wise Affine Warp
	  The ideas for this piece-wise affine triangular warping are taken from the
	  Active appearance models revisited by Iain Matthews and Simon Baker in IJCV 2004
	  This is used for both validation of landmark detection, and for avatar animation

	  The code is based on the CLM tracker by Jason Saragih et al.
  */	

class PAW{
public:    
	// Number of pixels after the warping to neutral shape
    int     number_of_pixels; 

	// Minimum x coordinate in destination
    double  min_x;

	// minimum y coordinate in destination
    double  min_y;

	// Destination points (landmarks to be warped to)
    Mat_<double> destination_landmarks;

	// Destination points (landmarks to be warped from)
    Mat_<double> source_landmarks;

	// Triangulation, each triangle is warped using an affine transform
    Mat_<int> triangulation;    

	// Triangle index, indicating which triangle each of destination pixels lies in
    Mat_<int> triangle_id;  

	// Indicating if the destination warped pixels is valid (lies within a face)
	Mat_<uchar> pixel_mask;

	// A number of precomputed coefficients that are helpful for quick warping
	
	// affine coefficients for all triangles (see Matthews and Baker 2004)
	// 6 coefficients for each triangle (are computed from alpha and beta)
	// This is computed during each warp based on source landmarks
    Mat_<double> coefficients;

	// matrix of (c,x,y) coeffs for alpha
    Mat_<double> alpha;  

	// matrix of (c,x,y) coeffs for alpha
    Mat_<double> beta;   

	// x-source of warped points
    Mat_<float> map_x;   

	// y-source of warped points
    Mat_<float> map_y;   

	// Default constructor
    PAW(){;}

	// Construct a warp from a destination shape and triangulation
	PAW(const Mat_<double>& destination_shape, const Mat_<int>& triangulation);

	// The final optional argument allows for optimisation if the triangle indices from previous frame are known (for tracking in video)
	PAW(const Mat_<double>& destination_shape, const Mat_<int>& triangulation, double in_min_x, double in_min_y, double in_max_x, double in_max_y);

	// Copy constructor
	PAW(const PAW& other): destination_landmarks(other.destination_landmarks.clone()), source_landmarks(other.source_landmarks.clone()), triangulation(other.triangulation.clone()),
		triangle_id(other.triangle_id.clone()), pixel_mask(other.pixel_mask.clone()), coefficients(other.coefficients.clone()), alpha(other.alpha.clone()), beta(other.beta.clone()), map_x(other.map_x.clone()), map_y(other.map_y.clone())
	{
		this->number_of_pixels = other.number_of_pixels; 
		this->min_x = other.min_x;
		this->min_y = other.min_y;
	}

	void Read(std::ifstream &s);

	// The actual warping
    void Warp(const Mat& image_to_warp, Mat& destination_image, const Mat_<double>& landmarks_to_warp);
	
	// Compute coefficients needed for warping
    void CalcCoeff();

	// Perform the actual warping
    void WarpRegion(Mat_<float>& map_x, Mat_<float>& map_y);

    inline int NumberOfLandmarks() const {return destination_landmarks.rows/2;} ;
    inline int NumberOfTriangles() const {return triangulation.rows;} ;

	// The width and height of the warped image
    inline int constWidth() const {return pixel_mask.cols;}
    inline int Height() const {return pixel_mask.rows;}
    
private:

	int findTriangle(const cv::Point_<double>& point, const std::vector<std::vector<double>>& control_points, int guess = -1) const;

  };
  //===========================================================================
}
#endif
