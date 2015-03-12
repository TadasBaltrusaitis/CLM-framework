///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2014, University of Southern California and University of Cambridge,
// all rights reserved.
//
// THIS SOFTWARE IS PROVIDED “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY. OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
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

#include <CLM.h>
#include <CLM_utils.h>

using namespace CLMTracker;

//=============================================================================
//=============================================================================

// Constructors
// A default constructor
CLM::CLM()
{
	CLMParameters parameters;
	this->Read(parameters.model_location);
}

// Constructor from a model file
CLM::CLM(string fname)
{
	this->Read(fname);
}

// Copy constructor (makes a deep copy of CLM)
CLM::CLM(const CLM& other): pdm(other.pdm), params_local(other.params_local.clone()), params_global(other.params_global), detected_landmarks(other.detected_landmarks.clone()),
	landmark_likelihoods(other.landmark_likelihoods.clone()), patch_experts(other.patch_experts), landmark_validator(other.landmark_validator), face_detector_location(other.face_detector_location)
{
	this->detection_success = other.detection_success;
	this->tracking_initialised = other.tracking_initialised;
	this->detection_certainty = other.detection_certainty;
	this->model_likelihood = other.model_likelihood;
	this->failures_in_a_row = other.failures_in_a_row;
	
	// Load the CascadeClassifier (as it does not have a proper copy constructor)
	if(!face_detector_location.empty())
	{
		this->face_detector_HAAR.load(face_detector_location);
	}
	// Make sure the matrices are allocated properly
	this->triangulations.resize(other.triangulations.size());
	for(size_t i = 0; i < other.triangulations.size(); ++i)
	{
		// Make sure the matrix is copied.
		this->triangulations[i] = other.triangulations[i].clone();
	}

	// Make sure the matrices are allocated properly
	for(std::map<int, Mat_<float>>::const_iterator it = other.kde_resp_precalc.begin(); it!= other.kde_resp_precalc.end(); it++)
	{
		// Make sure the matrix is copied.
		this->kde_resp_precalc.insert(std::pair<int, Mat_<float>>(it->first, it->second.clone()));
	}

	this->face_detector_HOG = dlib::get_frontal_face_detector();
}

// Assignment operator for lvalues (makes a deep copy of CLM)
CLM & CLM::operator= (const CLM& other)
{
	if (this != &other) // protect against invalid self-assignment
	{
		pdm = PDM(other.pdm);
		params_local = other.params_local.clone();
		params_global = other.params_global;
		detected_landmarks = other.detected_landmarks.clone();
		
		landmark_likelihoods =other.landmark_likelihoods.clone();
		patch_experts = Patch_experts(other.patch_experts);
		landmark_validator = DetectionValidator(other.landmark_validator);
		face_detector_location = other.face_detector_location;

		this->detection_success = other.detection_success;
		this->tracking_initialised = other.tracking_initialised;
		this->detection_certainty = other.detection_certainty;
		this->model_likelihood = other.model_likelihood;
		this->failures_in_a_row = other.failures_in_a_row;

		// Load the CascadeClassifier (as it does not have a proper copy constructor)
		if(!face_detector_location.empty())
		{
			this->face_detector_HAAR.load(face_detector_location);
		}
		// Make sure the matrices are allocated properly
		this->triangulations.resize(other.triangulations.size());
		for(size_t i = 0; i < other.triangulations.size(); ++i)
		{
			// Make sure the matrix is copied.
			this->triangulations[i] = other.triangulations[i].clone();
		}

		// Make sure the matrices are allocated properly
		for(std::map<int, Mat_<float>>::const_iterator it = other.kde_resp_precalc.begin(); it!= other.kde_resp_precalc.end(); it++)
		{
			// Make sure the matrix is copied.
			this->kde_resp_precalc.insert(std::pair<int, Mat_<float>>(it->first, it->second.clone()));
		}
	}

	face_detector_HOG = dlib::get_frontal_face_detector();

	return *this;
}

// Move constructor
CLM::CLM(const CLM&& other)
{
	this->detection_success = other.detection_success;
	this->tracking_initialised = other.tracking_initialised;
	this->detection_certainty = other.detection_certainty;
	this->model_likelihood = other.model_likelihood;
	this->failures_in_a_row = other.failures_in_a_row;

	pdm = other.pdm;
	params_local = other.params_local;
	params_global = other.params_global;
	detected_landmarks = other.detected_landmarks;
	landmark_likelihoods = other.landmark_likelihoods;
	patch_experts = other.patch_experts;
	landmark_validator = other.landmark_validator;
	face_detector_location = other.face_detector_location;

	face_detector_HAAR = other.face_detector_HAAR;

	triangulations = other.triangulations;
	kde_resp_precalc = other.kde_resp_precalc;

	face_detector_HOG = dlib::get_frontal_face_detector();

}

// Assignment operator for rvalues
CLM & CLM::operator= (const CLM&& other)
{
	this->detection_success = other.detection_success;
	this->tracking_initialised = other.tracking_initialised;
	this->detection_certainty = other.detection_certainty;
	this->model_likelihood = other.model_likelihood;
	this->failures_in_a_row = other.failures_in_a_row;

	pdm = other.pdm;
	params_local = other.params_local;
	params_global = other.params_global;
	detected_landmarks = other.detected_landmarks;
	landmark_likelihoods = other.landmark_likelihoods;
	patch_experts = other.patch_experts;
	landmark_validator = other.landmark_validator;
	face_detector_location = other.face_detector_location;

	face_detector_HAAR = other.face_detector_HAAR;

	triangulations = other.triangulations;
	kde_resp_precalc = other.kde_resp_precalc;

	face_detector_HOG = dlib::get_frontal_face_detector();

	return *this;
}


void CLM::Read_CLM(string clm_location)
{
	// Location of modules
	ifstream locations(clm_location.c_str(), ios_base::in);

	if(!locations.is_open())
	{
		cout << "Couldn't open the CLM model file aborting" << endl;
		cout.flush();
		return;
	}

	string line;
	
	vector<string> intensity_expert_locations;
	vector<string> depth_expert_locations;
	vector<string> ccnf_expert_locations;

	// The other module locations should be defined as relative paths from the main model
	boost::filesystem::path root = boost::filesystem::path(clm_location).parent_path();		

	// The main file contains the references to other files
	while (!locations.eof())
	{ 
		
		getline(locations, line);

		stringstream lineStream(line);

		string module;
		string location;

		// figure out which module is to be read from which file
		lineStream >> module;
		
		getline(lineStream, location);

		if(location.size() > 0)
			location.erase(location.begin()); // remove the first space
				
		// remove carriage return at the end for compatibility with unix systems
		if(location.size() > 0 && location.at(location.size()-1) == '\r')
		{
			location = location.substr(0, location.size()-1);
		}

		// append the lovstion to root location (boost syntax)
		location = (root / location).string();
				
		if (module.compare("PDM") == 0) 
		{            
			cout << "Reading the PDM module from: " << location << "....";
			pdm.Read(location);

			cout << "Done" << endl;
		}
		else if (module.compare("Triangulations") == 0) 
		{       
			cout << "Reading the Triangulations module from: " << location << "....";
			ifstream triangulationFile(location.c_str(), ios_base::in);

			CLMTracker::SkipComments(triangulationFile);

			int numViews;
			triangulationFile >> numViews;

			// read in the triangulations
			triangulations.resize(numViews);

			for(int i = 0; i < numViews; ++i)
			{
				CLMTracker::SkipComments(triangulationFile);
				CLMTracker::ReadMat(triangulationFile, triangulations[i]);
			}
			cout << "Done" << endl;
		}
		else if(module.compare("PatchesIntensity") == 0)
		{
			intensity_expert_locations.push_back(location);
		}
		else if(module.compare("PatchesDepth") == 0)
		{
			depth_expert_locations.push_back(location);
		}
		else if(module.compare("PatchesCCNF") == 0)
		{
			ccnf_expert_locations.push_back(location);
		}
	}
  
	// Initialise the patch experts
	patch_experts.Read(intensity_expert_locations, depth_expert_locations, ccnf_expert_locations);

	// Read in a face detector
	face_detector_HOG = dlib::get_frontal_face_detector();

}

void CLM::Read(string main_location)
{

	cout << "Reading the CLM landmark detector/tracker from: " << main_location << endl;
	
	ifstream locations(main_location.c_str(), ios_base::in);
	if(!locations.is_open())
	{
		cout << "Couldn't open the model file, aborting" << endl;
		return;
	}
	string line;
	
	// The other module locations should be defined as relative paths from the main model
	boost::filesystem::path root = boost::filesystem::path(main_location).parent_path();	

	// The main file contains the references to other files
	while (!locations.eof())
	{ 
		getline(locations, line);

		stringstream lineStream(line);

		string module;
		string location;

		// figure out which module is to be read from which file
		lineStream >> module;
				
		getline(lineStream, location);

		if(location.size() > 0)
			location.erase(location.begin()); // remove the first space
						
		// remove carriage return at the end for compatibility with unix systems
		if(location.size() > 0 && location.at(location.size()-1) == '\r')
		{
			location = location.substr(0, location.size()-1);
		}

		// append to root
		location = (root / location).string();
		if (module.compare("CLM") == 0) 
		{ 
			cout << "Reading the CLM module from: " << location << endl;

			// The CLM module includes the PDM and the patch experts
			Read_CLM(location);
		}
		else if (module.compare("DetectionValidator") == 0) // Don't do face checking atm, as a new one needs to be trained
		{            
			cout << "Reading the landmark validation module....";
			landmark_validator.Read(location);
			cout << "Done" << endl;
		}
	}
 
	detected_landmarks.create(2 * pdm.NumberOfPoints(), 1);
	detected_landmarks.setTo(0);

	detection_success = false;
	tracking_initialised = false;
	model_likelihood = -10; // very low
	detection_certainty = 1; // very uncertain

	// Initialising default values for the rest of the variables

	// local parameters (shape)
	params_local.create(pdm.NumberOfModes(), 1);
	params_local.setTo(0.0);

	// global parameters (pose) [scale, euler_x, euler_y, euler_z, tx, ty]
	params_global = Vec6d(1, 0, 0, 0, 0, 0);

	failures_in_a_row = -1;

}

// Resetting the model (for a new video, or complet reinitialisation
void CLM::Reset()
{
	detected_landmarks.setTo(0);

	detection_success = false;
	tracking_initialised = false;
	model_likelihood = -10;  // very low
	detection_certainty = 1; // very uncertain

	// local parameters (shape)
	params_local.setTo(0.0);

	// global parameters (pose) [scale, euler_x, euler_y, euler_z, tx, ty]
	params_global = Vec6d(1, 0, 0, 0, 0, 0);

	failures_in_a_row = -1;
	face_template = Mat_<uchar>();
}

// Resetting the model, choosing the face nearest (x,y)
void CLM::Reset(double x, double y)
{

	// First reset the model overall
	this->Reset();

	// Now in the following frame when face detection takes place this is the point at which it will be preffered
	this->preference_det.x = x;
	this->preference_det.y = y;

}

// The main internal landmark detection call (should not be used externally?)
bool CLM::DetectLandmarks(const Mat_<uchar> &image, const Mat_<float> &depth, CLMParameters& params)
{

	// Fits from the current estimate of local and global parameters in clm_model
	bool fit_success = Fit(image, depth, params.window_sizes_current, params);

	// Store the landmarks converged on in detected_landmarks
	pdm.CalcShape2D(detected_landmarks, params_local, params_global);	
	
	// Check detection correctness
	if(params.validate_detections && fit_success)
	{
		Vec3d orientation(params_global[1], params_global[2], params_global[3]);

		detection_certainty = landmark_validator.Check(orientation, image, detected_landmarks);

		detection_success = detection_certainty < params.validation_boundary;
	}
	else
	{
		detection_success = fit_success;
		if(fit_success)
		{
			detection_certainty = -1;
		}
		else
		{
			detection_certainty = 1;
		}

	}

	return detection_success;
}

//=============================================================================
bool CLM::Fit(const Mat_<uchar>& im, const Mat_<float>& depthImg, const std::vector<int>& window_sizes, const CLMParameters& clm_parameters)
{
	// Making sure it is a single channel image
	assert(im.channels() == 1);	
	
	// Placeholder for the landmarks
	Mat_<double> current_shape(2 * pdm.NumberOfPoints() , 1, 0.0);

	int n = pdm.NumberOfPoints(); 
	
	Mat_<float> depth_img_no_background;	
	
	// Background elimination from the depth image
	if(!depthImg.empty())
	{
		bool success = RemoveBackground(depth_img_no_background, depthImg);

		// The attempted background removal can fail leading to tracking failure
		if(!success)
		{
			return false;
		}
	}

	double curr_scale = params_global[0];

	// Find the closest depth and colour patch scales, and start window_size below, this will make sure that the last iteration is done at the best scale available
	int scale = -1;

	double minDist;
	for( size_t i = 0; i < patch_experts.patch_scaling.size(); ++i)
	{
		if(i==0 || std::abs(patch_experts.patch_scaling[i] - curr_scale) < minDist)
		{
			minDist = std::abs(patch_experts.patch_scaling[i] - curr_scale);
			scale = i + 1;
		}

	}
	
	scale = scale - window_sizes.size();

	if(scale < 0)
		scale = 0;

	int num_scales = patch_experts.patch_scaling.size();

	// Storing the patch expert response maps
	vector<Mat_<float> > patch_expert_responses(n);

	// Converting from image space to patch expert space (normalised for rotation and scale)
	Matx22f sim_ref_to_img;
	Matx22d sim_img_to_ref;

	// Optimise the model across a number of areas of interest (usually in descending window size and ascending scale size)
	for(size_t witer = 0; witer < window_sizes.size(); witer++)
	{

		int window_size = window_sizes[witer];

		// The patch expert response computation
		if(witer != window_sizes.size() - 1)
		{
			patch_experts.Response(patch_expert_responses, sim_ref_to_img, sim_img_to_ref, im, depth_img_no_background, pdm, params_global, params_local, window_size, scale);
		}
		else
		{
			// Do not use depth for the final iteration as it is not as accurate
			patch_experts.Response(patch_expert_responses, sim_ref_to_img, sim_img_to_ref, im, Mat(), pdm, params_global, params_local, window_size, scale);
		}
		
		// Get the current landmark locations
		pdm.CalcShape2D(current_shape, params_local, params_global);

		// Get the view used by patch experts
		int view_id = patch_experts.GetViewIdx(params_global, scale);

		// the actual optimisation step
		this->NU_RLMS(params_global, params_local, patch_expert_responses, Vec6d(params_global), params_local.clone(), current_shape, sim_img_to_ref, sim_ref_to_img, window_size, view_id, true, scale, this->landmark_likelihoods, clm_parameters);

		// non-rigid optimisation
		this->model_likelihood = this->NU_RLMS(params_global, params_local, patch_expert_responses, Vec6d(params_global), params_local.clone(), current_shape, sim_img_to_ref, sim_ref_to_img, window_size, view_id, false, scale, this->landmark_likelihoods, clm_parameters);
		
		// If there are more scales to go, and we don't need to upscale too much move to next scale level
		if(scale < num_scales - 1 && 0.9 * patch_experts.patch_scaling[scale] < params_global[0])
		{
			scale++;			
		}
		else
		{
			// If we can't go up a scale just break, no point doing same scale over again
			break;
		}
		// Can't track very small images reliably (less than ~30px across)
		if(params_global[0] < 0.25)
		{
			cout << "Detection too small for CLM" << endl;
			return false;
		}
	}

	return true;
}

void CLM::NonVectorisedMeanShift_precalc_kde(Mat_<float>& out_mean_shifts, const vector<Mat_<float> >& patch_expert_responses, const Mat_<float> &dxs, const Mat_<float> &dys, int resp_size, float a, int scale, int view_id, map<int, Mat_<float> >& kde_resp_precalc)
{
	
	int n = dxs.rows;
	
	Mat_<float> kde_resp;
	float step_size = 0.1;

	// if this has not been precomputer, precompute it, otherwise use it
	if(kde_resp_precalc.find(resp_size) == kde_resp_precalc.end())
	{		
		kde_resp = Mat_<float>((int)((resp_size / step_size)*(resp_size/step_size)), resp_size * resp_size);
		MatIterator_<float> kde_it = kde_resp.begin();

		for(int x = 0; x < resp_size/step_size; x++)
		{
			float dx = x * step_size;
			for(int y = 0; y < resp_size/step_size; y++)
			{
				float dy = y * step_size;

				int ii,jj;
				float v,vx,vy;
			
				for(ii = 0; ii < resp_size; ii++)
				{
					vx = (dy-ii)*(dy-ii);
					for(jj = 0; jj < resp_size; jj++)
					{
						vy = (dx-jj)*(dx-jj);

						// the KDE evaluation of that point
						v = exp(a*(vx+vy));
						
						*kde_it++ = v;
					}
				}
			}
		}

		kde_resp_precalc[resp_size] = kde_resp.clone();
	}
	else
	{
		// use the precomputed version
		kde_resp = kde_resp_precalc.find(resp_size)->second;
	}

	// for every point (patch) calculating mean-shift
	for(int i = 0; i < n; i++)
	{
		if(patch_experts.visibilities[scale][view_id].at<int>(i,0) == 0)
		{
			out_mean_shifts.at<float>(i,0) = 0;
			out_mean_shifts.at<float>(i+n,0) = 0;
			continue;
		}

		// indices of dx, dy
		float dx = dxs.at<float>(i);
		float dy = dys.at<float>(i);

		// Ensure that we are within bounds (important for precalculation)
		if(dx < 0)
			dx = 0;
		if(dy < 0)
			dy = 0;
		if(dx > resp_size - step_size)
			dx = resp_size - step_size;
		if(dy > resp_size - step_size)
			dy = resp_size - step_size;
		
		// Pick the row from precalculated kde that approximates the current dx, dy best		
		int closest_col = (int)(dy /step_size + 0.5); // Plus 0.5 is there, as C++ rounds down with int cast
		int closest_row = (int)(dx /step_size + 0.5); // Plus 0.5 is there, as C++ rounds down with int cast
		
		int idx = closest_row * ((int)(resp_size/step_size + 0.5)) + closest_col; // Plus 0.5 is there, as C++ rounds down with int cast

		MatIterator_<float> kde_it = kde_resp.begin() + kde_resp.cols*idx;
		
		float mx=0.0;
		float my=0.0;
		float sum=0.0;

		// Iterate over the patch responses here
		MatConstIterator_<float> p = patch_expert_responses[i].begin();
			
		for(int ii = 0; ii < resp_size; ii++)
		{
			for(int jj = 0; jj < resp_size; jj++)
			{

				// the KDE evaluation of that point multiplied by the probability at the current, xi, yi
				float v = (*p++) * (*kde_it++);

				sum += v;

				// mean shift in x and y
				mx += v*jj;
				my += v*ii; 

			}
		}
		
		float msx = (mx/sum - dx);
		float msy = (my/sum - dy);

		out_mean_shifts.at<float>(i,0) = msx;
		out_mean_shifts.at<float>(i+n,0) = msy;

	}

}

void CLM::GetWeightMatrix(Mat_<float>& WeightMatrix, int scale, int view_id, const CLMParameters& parameters)
{
	int n = pdm.NumberOfPoints();  

	// Is the weight matrix needed at all
	if(parameters.weight_factor > 0)
	{
		WeightMatrix = Mat_<float>::zeros(n*2, n*2);

		for (int p=0; p < n; p++)
		{
			if(!patch_experts.ccnf_expert_intensity.empty())
			{

				// for the x dimension
				WeightMatrix.at<float>(p,p) = WeightMatrix.at<float>(p,p)  + patch_experts.ccnf_expert_intensity[scale][view_id][p].patch_confidence;
				
				// for they y dimension
				WeightMatrix.at<float>(p+n,p+n) = WeightMatrix.at<float>(p,p);

			}
			else
			{
				// Across the modalities add the confidences
				for(size_t pc=0; pc < patch_experts.svr_expert_intensity[scale][view_id][p].svr_patch_experts.size(); pc++)
				{
					// for the x dimension
					WeightMatrix.at<float>(p,p) = WeightMatrix.at<float>(p,p)  + patch_experts.svr_expert_intensity[scale][view_id][p].svr_patch_experts.at(pc).confidence;
				}	
				// for the y dimension
				WeightMatrix.at<float>(p+n,p+n) = WeightMatrix.at<float>(p,p);
			}
		}
		WeightMatrix = parameters.weight_factor * WeightMatrix;
	}
	else
	{
		WeightMatrix = Mat_<float>::eye(n*2, n*2);
	}

}

//=============================================================================
double CLM::NU_RLMS(Vec6d& final_global, Mat_<double>& final_local, const vector<Mat_<float> >& patch_expert_responses, const Vec6d& initial_global, const Mat_<double>& initial_local,
		          const Mat_<double>& base_shape, const Matx22d& sim_img_to_ref, const Matx22f& sim_ref_to_img, int resp_size, int view_id, bool rigid, int scale, Mat_<double>& landmark_lhoods,
				  const CLMParameters& parameters)
{
	
	int n = pdm.NumberOfPoints();  
	
	// Mean, eigenvalues, eigenvectors
	Mat_<double> M = this->pdm.mean_shape;
	Mat_<double> E = this->pdm.eigen_values;
	//Mat_<double> V = this->pdm.princ_comp;

	int m = pdm.NumberOfModes();
	
	Vec6d current_global(initial_global);

	Mat_<float> current_local;
	initial_local.convertTo(current_local, CV_32F);

	Mat_<double> current_shape;
	Mat_<double> previous_shape;

	// Pre-calculate the regularisation term
	Mat_<float> regTerm;

	if(rigid)
	{
		regTerm = Mat_<float>::zeros(6,6);
	}
	else
	{
		Mat_<double> regularisations = Mat_<double>::zeros(1, 6 + m);

		// Setting the regularisation to the inverse of eigenvalues
		Mat(parameters.reg_factor / E).copyTo(regularisations(Rect(6, 0, m, 1)));			
		Mat_<double> regTerm_d = Mat::diag(regularisations.t());
		regTerm_d.convertTo(regTerm, CV_32F);
	}	

	Mat_<float> WeightMatrix;
	GetWeightMatrix(WeightMatrix, scale, view_id, parameters);

	Mat_<float> dxs, dys;
	
	// The preallocated memory for the mean shifts
	Mat_<float> mean_shifts(2 * pdm.NumberOfPoints(), 1, 0.0);

	// Number of iterations
	for(int iter = 0; iter < parameters.num_optimisation_iteration; iter++)
	{

		// get the current estimates of x
		pdm.CalcShape2D(current_shape, current_local, current_global);
		
		if(iter > 0)
		{
			// if the shape hasn't changed terminate
			if(norm(current_shape, previous_shape) < 0.01)
			{				
				break;
			}
		}

		current_shape.copyTo(previous_shape);
		
		// Jacobian, and transposed weighted jacobian
		Mat_<float> J, J_w_t;

		// calculate the appropriate Jacobians in 2D, even though the actual behaviour is in 3D, using small angle approximation and oriented shape
		if(rigid)
		{
			pdm.ComputeRigidJacobian(current_local, current_global, J, WeightMatrix, J_w_t);
		}
		else
		{
			pdm.ComputeJacobian(current_local, current_global, J, WeightMatrix, J_w_t);
		}
		
		// useful for mean shift calculation
		float a = -0.5/(parameters.sigma * parameters.sigma);

		Mat_<double> current_shape_2D = current_shape.reshape(1, 2).t();
		Mat_<double> base_shape_2D = base_shape.reshape(1, 2).t();

		Mat_<float> offsets;
		Mat((current_shape_2D - base_shape_2D) * Mat(sim_img_to_ref).t()).convertTo(offsets, CV_32F);
		
		dxs = offsets.col(0) + (resp_size-1)/2;
		dys = offsets.col(1) + (resp_size-1)/2;
		
		NonVectorisedMeanShift_precalc_kde(mean_shifts, patch_expert_responses, dxs, dys, resp_size, a, scale, view_id, kde_resp_precalc);

		// Now transform the mean shifts to the the image reference frame, as opposed to one of ref shape (object space)
		Mat_<float> mean_shifts_2D = (mean_shifts.reshape(1, 2)).t();
		
		mean_shifts_2D = mean_shifts_2D * Mat(sim_ref_to_img).t();
		mean_shifts = Mat(mean_shifts_2D.t()).reshape(1, n*2);		

		// remove non-visible observations
		for(int i = 0; i < n; ++i)
		{
			// if patch unavailable for current index
			if(patch_experts.visibilities[scale][view_id].at<int>(i,0) == 0)
			{				
				Mat Jx = J.row(i);
				Jx = cvScalar(0);
				Mat Jy = J.row(i+n);
				Jy = cvScalar(0);
				mean_shifts.at<float>(i,0) = 0.0f;
				mean_shifts.at<float>(i+n,0) = 0.0f;
			}
		}

		// projection of the meanshifts onto the jacobians (using the weighted Jacobian, see Baltrusaitis 2013)
		Mat_<float> J_w_t_m = J_w_t * mean_shifts;

		// Add the regularisation term
		if(!rigid)
		{
			J_w_t_m(Rect(0,6,1, m)) = J_w_t_m(Rect(0,6,1, m)) - regTerm(Rect(6,6, m, m)) * current_local;
		}

		// Calculating the Hessian approximation
		Mat_<float> Hessian = J_w_t * J;

		// Add the Tikhonov regularisation
		Hessian = Hessian + regTerm;

		// Solve for the parameter update (from Baltrusaitis 2013 based on eq (36) Saragih 2011)
		Mat_<float> param_update;
		solve(Hessian, J_w_t_m, param_update, CV_CHOLESKY);
		
		// update the reference
		pdm.UpdateModelParameters(param_update, current_local, current_global);		
		
		// clamp to the local parameters for valid expressions
		pdm.Clamp(current_local, current_global, parameters);

	}

	// compute the log likelihood
	double loglhood = 0;
	
	landmark_lhoods = Mat_<double>(n, 1, -1e8);
	
	for(int i = 0; i < n; i++)
	{

		if(patch_experts.visibilities[scale][view_id].at<int>(i,0) == 0 )
		{
			continue;
		}
		float dx = dxs.at<float>(i);
		float dy = dys.at<float>(i);

		int ii,jj;
		float v,vx,vy,sum=0.0;

		// Iterate over the patch responses here
		MatConstIterator_<float> p = patch_expert_responses[i].begin();
			
		for(ii = 0; ii < resp_size; ii++)
		{
			vx = (dy-ii)*(dy-ii);
			for(jj = 0; jj < resp_size; jj++)
			{
				vy = (dx-jj)*(dx-jj);

				// the probability at the current, xi, yi
				v = *p++;

				// the KDE evaluation of that point
				v *= exp(-0.5*(vx+vy)/(parameters.sigma * parameters.sigma));

				sum += v;
			}
		}
		landmark_lhoods.at<double>(i,0) = (double)sum;

		// the offset is there for numerical stability
		loglhood += log(sum + 1e-8);

	}	
	loglhood = loglhood/sum(patch_experts.visibilities[scale][view_id])[0];

	final_global = current_global;
	final_local = current_local;

	return loglhood;

}


bool CLM::RemoveBackground(Mat_<float>& out_depth_image, const Mat_<float>& depth_image)
{
	// use the current estimate of the face location to determine what is foreground and background
	double tx = this->params_global[4];
	double ty = this->params_global[5];

	// if we are too close to the edge fail
	if(tx - 9 <= 0 || ty - 9 <= 0 || tx + 9 >= depth_image.cols || ty + 9 >= depth_image.rows)
	{
		cout << "Face estimate is too close to the edge, tracking failed" << endl;
		return false;
	}

	Mat_<double> current_shape;

	pdm.CalcShape2D(current_shape, params_local, params_global);

	double min_x, max_x, min_y, max_y;

	int n = this->pdm.NumberOfPoints();

	minMaxLoc(current_shape(Range(0, n),Range(0,1)), &min_x, &max_x);
	minMaxLoc(current_shape(Range(n, n*2),Range(0,1)), &min_y, &max_y);

	// the area of interest: size of face with some scaling ( these scalings are fairly ad-hoc)
	double width = 3 * (max_x - min_x); 
	double height = 2.5 * (max_y - min_y); 

	// getting the region of interest from the depth image,
	// so we don't get other objects lying at same depth as head in the image but away from it
	Rect_<int> roi((int)(tx-width/2), (int)(ty - height/2), (int)width, (int)height);

	// clamp it if it does not lie fully in the image
	if(roi.x < 0) roi.x = 0;
	if(roi.y < 0) roi.y = 0;
	if(roi.width + roi.x >= depth_image.cols) roi.x = depth_image.cols - roi.width;
	if(roi.height + roi.y >= depth_image.rows) roi.y = depth_image.rows - roi.height;
		
	if(width > depth_image.cols)
	{
		roi.x = 0; roi.width = depth_image.cols;
	}
	if(height > depth_image.rows)
	{
		roi.y = 0; roi.height = depth_image.rows;
	}

	if(roi.width == 0) roi.width = depth_image.cols;
	if(roi.height == 0) roi.height = depth_image.rows;

	if(roi.x >= depth_image.cols) roi.x = 0;
	if(roi.y >= depth_image.rows) roi.y = 0;

	// Initialise the mask
	Mat_<uchar> mask(depth_image.rows, depth_image.cols, (uchar)0);

	Mat_<uchar> valid_pixels = depth_image > 0;

	// check if there is any depth near the estimate
	if(sum(valid_pixels(Rect((int)tx - 8, (int)ty - 8, 16, 16))/255)[0] > 0)
	{
		double Z = mean(depth_image(Rect((int)tx - 8, (int)ty - 8, 16, 16)), valid_pixels(Rect((int)tx - 8, (int)ty - 8, 16, 16)))[0]; // Z offset from the surface of the face
				
		// Only operate within region of interest of the depth image
		Mat dRoi = depth_image(roi);

		Mat mRoi = mask(roi);

		// Filter all pixels further than 20cm away from the current pose depth estimate
		inRange(dRoi, Z - 200, Z + 200, mRoi);
		
		// Convert to be either 0 or 1
		mask = mask / 255;
		
		Mat_<float> maskF;
		mask.convertTo(maskF, CV_32F);

		//Filter the depth image
		out_depth_image = depth_image.mul(maskF);
	}
	else
	{
		cout << "No depth signal found in foreground, tracking failed" << endl;
		return false;
	}
	return true;
}

// Getting a 3D shape model from the current detected landmarks (in camera space)
Mat_<double> CLM::GetShape(double fx, double fy, double cx, double cy)
{
	int n = this->detected_landmarks.rows/2;

	Mat_<double> shape3d(n*3, 1);

	this->pdm.CalcShape3D(shape3d, this->params_local);
	
	// Need to rotate the shape to get the actual 3D representation
	
	// get the rotation matrix from the euler angles
	Matx33d R = CLMTracker::Euler2RotationMatrix(Vec3d(params_global[1], params_global[2], params_global[3]));

	shape3d = shape3d.reshape(1, 3);

	shape3d = shape3d.t() * Mat(R).t();
	
	// from the weak perspective model can determine the average depth of the object
	double Zavg = fx / params_global[0];	

	Mat_<double> outShape(n,3,0.0);

	// this is described in the paper in section 3.4 (equation 10) (of the CLM-Z paper)
	for(int i = 0; i < n; i++)
	{
		double Z = Zavg + shape3d.at<double>(i,2);

		double X = Z * ((this->detected_landmarks.at<double>(i) - cx)/fx);
		double Y = Z * ((this->detected_landmarks.at<double>(i + n) - cy)/fy);

		outShape.at<double>(i,0) = (double)X;
		outShape.at<double>(i,1) = (double)Y;
		outShape.at<double>(i,2) = (double)Z;

	}

	// The format is 3 rows - n cols
	return outShape.t();
	
}

// A utility bounding box function
Rect_<double> CLM::GetBoundingBox() const
{
	Mat_<double> xs = this->detected_landmarks(Rect(0,0,1,this->detected_landmarks.rows/2));
	Mat_<double> ys = this->detected_landmarks(Rect(0,this->detected_landmarks.rows/2, 1, this->detected_landmarks.rows/2));

	double min_x, max_x;
	double min_y, max_y;
	cv::minMaxLoc(xs, &min_x, &max_x);
	cv::minMaxLoc(ys, &min_y, &max_y);

	// See if the detections intersect
	Rect_<double> model_rect(min_x, min_y, max_x - min_x, max_y - min_y);
	return model_rect;
}

// Legacy function not used at the moment
void CLM::NonVectorisedMeanShift(Mat_<double>& out_mean_shifts, const vector<Mat_<float> >& patch_expert_responses, const Mat_<double> &dxs, const Mat_<double> &dys, int resp_size, double a, int scale, int view_id)
{
	
	int n = dxs.rows;
	
	for(int i = 0; i < n; i++)
	{

		if(patch_experts.visibilities[scale][view_id].at<int>(i,0) == 0  || sum(patch_expert_responses[i])[0] == 0)
		{
			out_mean_shifts.at<double>(i,0) = 0;
			out_mean_shifts.at<double>(i+n,0) = 0;
			continue;
		}

		// indices of dx, dy
		double dx = dxs.at<double>(i);
		double dy = dys.at<double>(i);

		int ii,jj;
		double v,vx,vy,mx=0.0,my=0.0,sum=0.0;

		// Iterate over the patch responses here
		MatConstIterator_<float> p = patch_expert_responses[i].begin();
			
		for(ii = 0; ii < resp_size; ii++)
		{
			vx = (dy-ii)*(dy-ii);
			for(jj = 0; jj < resp_size; jj++)
			{
				vy = (dx-jj)*(dx-jj);

				// the probability at the current, xi, yi
				v = *p++;

				// the KDE evaluation of that point
				double kd = exp(a*(vx+vy));
				v *= kd;

				sum += v;

				// mean shift in x and y
				mx += v*jj;
				my += v*ii; 

			}
		}

		// setting the actual mean shift update
		double msx = (mx/sum - dx);
		double msy = (my/sum - dy);

		out_mean_shifts.at<double>(i, 0) = msx;
		out_mean_shifts.at<double>(i + n, 0) = msy;
			
	}
}
