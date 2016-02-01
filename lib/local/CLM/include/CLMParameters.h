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

//  Parameters of the CLNF, CLM-Z and CLM trackers
//
//  Tadas Baltrusaitis
//  01/05/2012
#ifndef __CLM_PARAM_H
#define __CLM_PARAM_H

using namespace cv;
using namespace std;

namespace CLMTracker
{

struct CLMParameters
{

	// A number of RLMS or NU-RLMS iterations
	int num_optimisation_iteration;
	
	// Should pose be limited to 180 degrees frontal
	bool limit_pose;
	
	// Should face validation be done
	bool validate_detections;

	// Landmark detection validator boundary for correct detection, the regressor output -1 (perfect alignment) 1 (bad alignment), 
	double validation_boundary;

	// Used when tracking is going well
	vector<int> window_sizes_small;

	// Used when initialising or tracking fails
	vector<int> window_sizes_init;
	
	// Used for the current frame
	vector<int> window_sizes_current;
	
	// How big is the tracking template that helps with large motions
	double face_template_scale;	
	bool use_face_template;

	// Where to load the model from
	string model_location;
	
	// this is used for the smooting of response maps (KDE sigma)
	double sigma;

	double reg_factor;	// weight put to regularisation
	double weight_factor; // factor for weighted least squares

	// should multiple views be considered during reinit
	bool multi_view;
	
	// How often should face detection be used to attempt reinitialisation, every n frames (set to negative not to reinit)
	int reinit_video_every;

	// Determining which face detector to use for (re)initialisation, HAAR is quicker but provides more false positives and is not goot for in-the-wild conditions
	// Also HAAR detector can detect smaller faces while HOG SVM is only capable of detecting faces at least 70px across
	enum FaceDetector{HAAR_DETECTOR, HOG_SVM_DETECTOR};

	string face_detector_location;
	FaceDetector curr_face_detector;

	// Should the results be visualised and reported to console
	bool quiet_mode;

	// Should the model be refined hierarchically (if available)
	bool refine_hierarchical;

	// Should the parameters be refined for different scales
	bool refine_parameters;

	// Using the brand new and experimental gaze tracker
	bool track_gaze;

	CLMParameters()
	{
		// initialise the default values
		init();
	}

	// possible parameters are -clm 'file' which specifies the default location of main clm root
	CLMParameters(vector<string> &arguments)
	{
		// initialise the default values
	    init(); 

		// First element is reserved for the executable location (useful for finding relative model locs)
		boost::filesystem::path root = boost::filesystem::path(arguments[0]).parent_path();

		bool* valid = new bool[arguments.size()];
		valid[0] = true;

		for(size_t i = 1; i < arguments.size(); ++i)
		{
			valid[i] = true;

			if (arguments[i].compare("-mloc") == 0) 
			{                    
				string model_loc = arguments[i + 1];
				model_location = model_loc;
				valid[i] = false;
				valid[i+1] = false;
				i++;

			}
			if (arguments[i].compare("-clm_sigma") == 0) 
			{                    
				stringstream data(arguments[i + 1]);
				data >> sigma;
				valid[i] = false;
				valid[i+1] = false;
				i++;
			}
			else if(arguments[i].compare("-w_reg") == 0)
			{
				stringstream data(arguments[i + 1]);
				data >> weight_factor;
				valid[i] = false;
				valid[i+1] = false;
				i++;
			}
			else if(arguments[i].compare("-reg") == 0)
			{
				stringstream data(arguments[i + 1]);
				data >> reg_factor;
				valid[i] = false;
				valid[i+1] = false;
				i++;
			}
			else if (arguments[i].compare("-multi_view") == 0) 
			{                    

				stringstream data(arguments[i + 1]);
				int m_view;
				data >> m_view;

				multi_view = (bool)(m_view != 0);
				valid[i] = false;
				valid[i+1] = false;
				i++;
			}
			else if(arguments[i].compare("-validate_detections") == 0)
			{
				stringstream data(arguments[i + 1]);
				int v_det;
				data >> v_det;

				validate_detections = (bool)(v_det != 0);
				valid[i] = false;
				valid[i+1] = false;
				i++;
			}
			else if(arguments[i].compare("-n_iter") == 0)
			{
				stringstream data(arguments[i + 1]);											
				data >> num_optimisation_iteration;

				valid[i] = false;
				valid[i+1] = false;
				i++;
			}
			else if (arguments[i].compare("-gaze") == 0)
			{
				track_gaze = true;

				valid[i] = false;
				i++;
			}
			else if (arguments[i].compare("-q") == 0) 
			{                    

				quiet_mode = true;

				valid[i] = false;
			}
			else if (arguments[i].compare("-clmwild") == 0) 
			{                    
				// For in the wild fitting these parameters are suitable
				window_sizes_init = vector<int>(4);
				window_sizes_init[0] = 15; window_sizes_init[1] = 13; window_sizes_init[2] = 11; window_sizes_init[3] = 9;

				sigma = 1.25;
				reg_factor = 35;
				weight_factor = 2.5;
				num_optimisation_iteration = 10;

				valid[i] = false;

				// For in-the-wild images use an in-the wild detector				
				curr_face_detector = HOG_SVM_DETECTOR;

			}
			else if (arguments[i].compare("-help") == 0)
			{
				cout << "CLM parameters are defined as follows: -mloc <location of model file> -pdm_loc <override pdm location> -w_reg <weight term for patch rel.> -reg <prior regularisation> -clm_sigma <float sigma term> -fcheck <should face checking be done 0/1> -n_iter <num EM iterations> -clwild (for in the wild images) -q (quiet mode)" << endl; // Inform the user of how to use the program				
			}
		}

		for(int i=arguments.size()-1; i >= 0; --i)
		{
			if(!valid[i])
			{
				arguments.erase(arguments.begin()+i);
			}
		}

		// Make sure model_location is valid
		if(!boost::filesystem::exists(boost::filesystem::path(model_location)))
		{
			model_location = (root / model_location).string();
			if(!boost::filesystem::exists(boost::filesystem::path(model_location)))
			{
				std::cout << "Could not find the landmark detection model to load" << std::endl;
			}
		}

	}

	private:
		void init()
		{

			// number of iterations that will be performed at each clm scale
			num_optimisation_iteration = 5;
			
			// using an external face checker based on SVM
			validate_detections = true;

			// Using hierarchical refinement by default (can be turned off)
			refine_hierarchical = true;

			// Refining parameters by default
			refine_parameters = true;

			window_sizes_small = vector<int>(4);
			window_sizes_init = vector<int>(4);

			// For fast tracking
			window_sizes_small[0] = 0;
			window_sizes_small[1] = 9;
			window_sizes_small[2] = 7;
			window_sizes_small[3] = 5;

			// Just for initialisation
			window_sizes_init.at(0) = 11;
			window_sizes_init.at(1) = 9;
			window_sizes_init.at(2) = 7;
			window_sizes_init.at(3) = 5;
			
			face_template_scale = 0.3;
			// Off by default (as it might lead to some slight inaccuracies in slowly moving faces)
			use_face_template = false;

			// For first frame use the initialisation
			window_sizes_current = window_sizes_init;

			model_location = "model/main_ccnf_general.txt";

			sigma = 1.5;
			reg_factor = 25;
			weight_factor = 0; // By default do not use NU-RLMS for videos as it does not work as well for them

			validation_boundary = -0.45;

			limit_pose = true;
			multi_view = false;

			reinit_video_every = 4;
						
			// Face detection
			#if OS_UNIX
				face_detector_location = "classifiers/haarcascade_frontalface_alt.xml";
			#else
				face_detector_location = "classifiers/haarcascade_frontalface_alt.xml";
			#endif

			quiet_mode = false;

			// By default use HOG SVM
			curr_face_detector = HOG_SVM_DETECTOR;

			// The gaze tracking has to be explicitly initialised
			track_gaze = false;
		}
};

}

#endif // __CLM_PARAM_H
