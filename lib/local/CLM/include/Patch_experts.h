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
#ifndef __Patch_experts_h_
#define __Patch_experts_h_

#include "SVR_patch_expert.h"
#include "CCNF_patch_expert.h"
#include "PDM.h"

namespace CLMTracker
{
//===========================================================================
/** 
    Combined class for all of the patch experts
*/
class Patch_experts
{

public:

	// The collection of SVR patch experts (for intensity/grayscale images), the experts are laid out scale->view->landmark
	vector<vector<vector<Multi_SVR_patch_expert> > >	svr_expert_intensity;
	 
	// The collection of SVR patch experts (for depth/range images), the experts are laid out scale->view->landmark
	vector<vector<vector<Multi_SVR_patch_expert> > >	svr_expert_depth;

	// The collection of LNF (CCNF) patch experts (for intensity images), the experts are laid out scale->view->landmark
	vector<vector<vector<CCNF_patch_expert> > >			ccnf_expert_intensity;

	// The node connectivity for CCNF experts, at different window sizes and corresponding to separate edge features
	vector<vector<cv::Mat_<float> > >					sigma_components;

	// The available scales for intensity patch experts
	vector<double>							patch_scaling;

	// The available views for the patch experts at every scale (in radians)
	vector<vector<cv::Vec3d> >               centers;

	// Landmark visibilities for each scale and view
    vector<vector<cv::Mat_<int> > >          visibilities;

	// A default constructor
	Patch_experts(){;}

	// A copy constructor
	Patch_experts(const Patch_experts& other): patch_scaling(other.patch_scaling), centers(other.centers), svr_expert_intensity(other.svr_expert_intensity), svr_expert_depth(other.svr_expert_depth), ccnf_expert_intensity(other.ccnf_expert_intensity)
	{

		// Make sure the matrices are allocated properly
		this->sigma_components.resize(other.sigma_components.size());
		for(size_t i = 0; i < other.sigma_components.size(); ++i)
		{
			this->sigma_components[i].resize(other.sigma_components[i].size());

			for(size_t j = 0; j < other.sigma_components[i].size(); ++j)
			{
				// Make sure the matrix is copied.
				this->sigma_components[i][j] = other.sigma_components[i][j].clone();
			}
		}

		// Make sure the matrices are allocated properly
		this->visibilities.resize(other.visibilities.size());
		for(size_t i = 0; i < other.visibilities.size(); ++i)
		{
			this->visibilities[i].resize(other.visibilities[i].size());

			for(size_t j = 0; j < other.visibilities[i].size(); ++j)
			{
				// Make sure the matrix is copied.
				this->visibilities[i][j] = other.visibilities[i][j].clone();
			}
		}
	}

	// Returns the patch expert responses given a grayscale and an optional depth image.
	// Additionally returns the transform from the image coordinates to the response coordinates (and vice versa).
	// The computation also requires the current landmark locations to compute response around, the PDM corresponding to the desired model, and the parameters describing its instance
	// Also need to provide the size of the area of interest and the desired scale of analysis
	void Response(vector<cv::Mat_<float> >& patch_expert_responses, Matx22f& sim_ref_to_img, Matx22d& sim_img_to_ref, const Mat_<uchar>& grayscale_image, const Mat_<float>& depth_image,
							 const PDM& pdm, const Vec6d& params_global, const Mat_<double>& params_local, int window_size, int scale);

	// Getting the best view associated with the current orientation
	int GetViewIdx(const Vec6d& params_global, int scale) const;

	// The number of views at a particular scale
	inline int nViews(int scale = 0) const { return centers[scale].size(); };

	// Reading in all of the patch experts
	void Read(vector<string> intensity_svr_expert_locations, vector<string> depth_svr_expert_locations, vector<string> intensity_ccnf_expert_locations);


   

private:
	void Read_SVR_patch_experts(string expert_location, std::vector<cv::Vec3d>& centers, std::vector<cv::Mat_<int> >& visibility, std::vector<std::vector<Multi_SVR_patch_expert> >& patches, double& scale);
	void Read_CCNF_patch_experts(string patchesFileLocation, std::vector<cv::Vec3d>& centers, std::vector<cv::Mat_<int> >& visibility, std::vector<std::vector<CCNF_patch_expert> >& patches, double& patchScaling);
	

};
 
}
#endif
