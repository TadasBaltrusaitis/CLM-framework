// FaceAnalyser_Interop.h

#pragma once

#pragma unmanaged

// Include all the unmanaged things we need.

#include <opencv2/core/core.hpp>
#include "opencv2/objdetect.hpp"
#include "opencv2/calib3d.hpp"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#pragma managed

#include <msclr\marshal.h>
#include <msclr\marshal_cppstd.h>

#include <CLM_interop.h>
#include <Face_utils.h>
#include <FaceAnalyser.h>

using namespace System;
using namespace OpenCVWrappers;
using namespace System::Collections::Generic;

using namespace msclr::interop;

namespace FaceAnalyser_Interop {

public ref class FaceAnalyserManaged
{

private:

	FaceAnalysis::FaceAnalyser* face_analyser;

	// The actual descriptors (for visualisation and output)
	cv::Mat_<double>* hog_features;
	cv::Mat* aligned_face;
	cv::Mat* visualisation;
	cv::Mat* tracked_face;

	// Variables used for recording things
	std::ofstream* hog_output_file;
	std::string* align_output_dir;
	int* num_rows;
	int* num_cols;
	bool* good_frame;
	cv::VideoWriter* tracked_vid_writer;

public:

	FaceAnalyserManaged(System::String^ root) 
	{
			
		vector<Vec3d> orientation_bins;
		orientation_bins.push_back(Vec3d(0,0,0));
		double scale = 0.7;
		int width = 112;
		int height = 112;
		
		string root_std = marshal_as<std::string>(root);

		boost::filesystem::path au_loc = boost::filesystem::path(root_std) / "AU_predictors" / "AU_all_best.txt";
		boost::filesystem::path tri_loc = boost::filesystem::path(root_std) / "model" / "tris_68_full.txt";

		face_analyser = new FaceAnalysis::FaceAnalyser(orientation_bins, scale, width, height, au_loc.string(), tri_loc.string());

		hog_features = new cv::Mat_<double>();

		aligned_face = new cv::Mat();
		visualisation = new cv::Mat();
		tracked_face = new cv::Mat();

		num_rows = new int;
		num_cols = new int;

		good_frame = new bool;
			
		align_output_dir = new string();

		hog_output_file = new std::ofstream();
	}

	void SetupAlignedImageRecording(System::String^ directory)
	{
		*align_output_dir = marshal_as<std::string>(directory);			
	}

	void SetupHOGRecording(System::String^ file)
	{
		// Create the file for recording			
		hog_output_file->open(marshal_as<std::string>(file), ios_base::out | ios_base::binary);	
	}

	void SetupTrackingRecording(System::String^ file, int width, int height, double fps)
	{
		tracked_vid_writer = new cv::VideoWriter(marshal_as<std::string>(file), CV_FOURCC('D', 'I', 'V', 'X'), fps, Size(width, height));
	}

	void StopHOGRecording()
	{
		hog_output_file->close();
	}

	void StopTrackingRecording()
	{
		tracked_vid_writer->release();
	}

	void RecordAlignedFrame(int frame_num)
	{
		char name[100];
					
		// output the frame number
		sprintf(name, "frame_det_%06d.png", frame_num);
				
		string out_file = (boost::filesystem::path(*align_output_dir) / boost::filesystem::path(name)).string();
		imwrite(out_file, *aligned_face);
	}

	void RecordHOGFrame()
	{
		// Using FHOGs, hence 31 channels
		int num_channels = 31;

		hog_output_file->write((char*)(num_cols), 4);
		hog_output_file->write((char*)(num_rows), 4);
		hog_output_file->write((char*)(&num_channels), 4);

		// Not the best way to store a bool, but will be much easier to read it
		float good_frame_float;
		if(good_frame)
			good_frame_float = 1;
		else
			good_frame_float = -1;

		hog_output_file->write((char*)(&good_frame_float), 4);

		cv::MatConstIterator_<double> descriptor_it = hog_features->begin();

		for(int y = 0; y < *num_cols; ++y)
		{
			for(int x = 0; x < *num_rows; ++x)
			{
				for(unsigned int o = 0; o < 31; ++o)
				{

					float hog_data = (float)(*descriptor_it++);
					hog_output_file->write((char*)&hog_data, 4);
				}
			}
		}
		
	}

	void RecordTrackedFace()
	{
		tracked_vid_writer->write(*tracked_face);
	}

	void AddNextFrame(RawImage^ frame, CLM_Interop::CLMTracker::CLM^ clm, bool dynamic_shift, bool dynamic_scale, bool vis_hog, bool vis_tracked) {
			
		face_analyser->AddNextFrame(frame->Mat, *clm->getCLM(), 0, dynamic_shift, dynamic_scale, true);

		face_analyser->GetLatestHOG(*hog_features, *num_rows, *num_cols);
		face_analyser->GetLatestAlignedFace(*aligned_face);

		*good_frame = clm->clm->detection_success;

		if(vis_hog)
		{
			*visualisation = face_analyser->GetLatestHOGDescriptorVisualisation();
		}

		if(vis_tracked)
		{
			*tracked_face = frame->Mat.clone();
			if(clm->clm->detection_success)
			{
				::CLMTracker::Draw(*tracked_face, *clm->clm);
			}
		}
	}
		
	List<System::String^>^ GetClassActionUnitsNames()
	{
		auto names = face_analyser->GetAUClassNames();

		auto names_ret = gcnew List<System::String^>();

		for(std::string name : names)
		{
			names_ret->Add(gcnew System::String(name.c_str()));
		}

		return names_ret;

	}

	List<System::String^>^ GetRegActionUnitsNames()
	{
		auto names = face_analyser->GetAURegNames();

		auto names_ret = gcnew List<System::String^>();

		for(std::string name : names)
		{
			names_ret->Add(gcnew System::String(name.c_str()));
		}

		return names_ret;

	}

	Dictionary<System::String^, double>^ GetCurrentAUsClass()
	{
		auto classes = face_analyser->GetCurrentAUsClass();
		Dictionary<System::String^, double>^ au_classes = gcnew Dictionary<System::String^, double>();

		for(auto p: classes)
		{
			au_classes->Add(gcnew System::String(p.first.c_str()), p.second);
		}
		return au_classes;
	}

	Dictionary<System::String^, double>^ GetCurrentAUsReg()
	{
		auto preds = face_analyser->GetCurrentAUsReg();
		Dictionary<System::String^, double>^ au_preds = gcnew Dictionary<System::String^, double>();

		for(auto p: preds)
		{
			au_preds->Add(gcnew System::String(p.first.c_str()), p.second);
		}
		return au_preds;
	}

	RawImage^ GetLatestAlignedFace() {
		RawImage^ face_aligned_image = gcnew RawImage(*aligned_face);
		return face_aligned_image;
	}

	RawImage^ GetLatestHOGDescriptorVisualisation() {
		RawImage^ HOG_vis_image = gcnew RawImage(*visualisation);
		return HOG_vis_image;
	}

	void Reset()
	{
		face_analyser->Reset();
	}

	// Finalizer. Definitely called before Garbage Collection,
	// but not automatically called on explicit Dispose().
	// May be called multiple times.
	!FaceAnalyserManaged()
	{
		delete hog_features;
		delete aligned_face;
		delete visualisation;
		delete num_cols;
		delete num_rows;
		delete hog_output_file;
		delete good_frame;
		delete align_output_dir;
		delete face_analyser;
		delete tracked_face;

		if(tracked_vid_writer != 0)
		{
			delete tracked_vid_writer;
		}
	}

	// Destructor. Called on explicit Dispose() only.
	~FaceAnalyserManaged()
	{
		this->!FaceAnalyserManaged();
	}

};
}
