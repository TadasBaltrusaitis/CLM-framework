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
#include <GazeEstimation.h>

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

	// Variable storing gaze for recording

	// Absolute gaze direction
	cv::Point3f* gazeDirection0;
	cv::Point3f* gazeDirection1;

	// Gaze with respect to head rather than camera (for example if eyes are rolled up and the head is tilted or turned this will be stable)
	cv::Point3f* gazeDirection0_head;
	cv::Point3f* gazeDirection1_head;

public:

	FaceAnalyserManaged(System::String^ root, bool dynamic) 
	{
			
		vector<Vec3d> orientation_bins;
		orientation_bins.push_back(Vec3d(0,0,0));
		double scale = 0.7;
		int width = 112;
		int height = 112;
		
		string root_std = marshal_as<std::string>(root);
		
		boost::filesystem::path tri_loc = boost::filesystem::path(root_std) / "model" / "tris_68_full.txt";
		boost::filesystem::path au_loc;
		if(dynamic)
		{
			au_loc = boost::filesystem::path(root_std) / "AU_predictors" / "AU_all_best.txt";
		}
		else
		{
			au_loc = boost::filesystem::path(root_std) / "AU_predictors" / "AU_all_static.txt";
		}

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

		gazeDirection0 = new cv::Point3f();
		gazeDirection1 = new cv::Point3f();
		gazeDirection0_head = new cv::Point3f();
		gazeDirection1_head = new cv::Point3f();
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

	void AddNextFrame(RawImage^ frame, CLM_Interop::CLMTracker::CLM^ clm, double fx, double fy, double cx, double cy, bool online, bool vis_hog, bool vis_tracked) {
			
		face_analyser->AddNextFrame(frame->Mat, *clm->getCLM(), 0, online, vis_hog);

		face_analyser->GetLatestHOG(*hog_features, *num_rows, *num_cols);
		
		face_analyser->GetLatestAlignedFace(*aligned_face);

		*good_frame = clm->clm->detection_success;

		if(vis_hog)
		{
			*visualisation = face_analyser->GetLatestHOGDescriptorVisualisation();
		}

		if(vis_tracked)
		{
			if(frame->Mat.cols != tracked_face->cols && frame->Mat.rows != tracked_face->rows)
			{
				*tracked_face = frame->Mat.clone();
			}
			else
			{
				frame->Mat.clone().copyTo(*tracked_face);
			}

			if(clm->clm->detection_success)
			{
				::CLMTracker::Draw(*tracked_face, *clm->clm);
			}
			tracked_face->deallocate();
		}

		// After the AUs have been detected do some gaze estimation as well
		FaceAnalysis::EstimateGaze(*clm->getCLM(), *gazeDirection0, *gazeDirection0_head, fx, fy, cx, cy, true);
		FaceAnalysis::EstimateGaze(*clm->getCLM(), *gazeDirection1, *gazeDirection1_head, fx, fy, cx, cy, false);

	}
		
	Tuple<Tuple<double, double, double>^, Tuple<double, double, double>^>^ GetGazeCamera()
	{

		auto gaze0 = gcnew Tuple<double, double, double>(gazeDirection0->x, gazeDirection0->y, gazeDirection0->z);
		auto gaze1 = gcnew Tuple<double, double, double>(gazeDirection1->x, gazeDirection1->y, gazeDirection1->z);

		return gcnew Tuple<Tuple<double, double, double>^, Tuple<double, double, double>^>(gaze0, gaze1);

	}

	Tuple<Tuple<double, double, double>^, Tuple<double, double, double>^>^ GetGazeHead()
	{
		auto gaze0 = gcnew Tuple<double, double, double>(gazeDirection0_head->x, gazeDirection0_head->y, gazeDirection0_head->z);
		auto gaze1 = gcnew Tuple<double, double, double>(gazeDirection1_head->x, gazeDirection1_head->y, gazeDirection1_head->z);

		return gcnew Tuple<Tuple<double, double, double>^, Tuple<double, double, double>^>(gaze0, gaze1);

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

		delete gazeDirection0;
		delete gazeDirection1;
		delete gazeDirection0_head;
		delete gazeDirection1_head;

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
