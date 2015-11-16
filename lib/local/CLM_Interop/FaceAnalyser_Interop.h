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
#include <RapportAnalyser.h>

using namespace System;
using namespace OpenCVWrappers;
using namespace System::Collections::Generic;

using namespace msclr::interop;

namespace FaceAnalyser_Interop {

public ref class FaceAnalyserManaged
{

private:

	FaceAnalysis::FaceAnalyser* face_analyser;
	FaceAnalysis::RapportAnalyser* rapport_analyser;

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

	cv::Point3f* pupil_left;
	cv::Point3f* pupil_right;

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
		rapport_analyser = new FaceAnalysis::RapportAnalyser();

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

		pupil_left = new cv::Point3f();
		pupil_right = new cv::Point3f();
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

	void AddNextFrame(RawImage^ frame, CLM_Interop::CLMTracker::CLM^ clm, double time_ms, double fx, double fy, double cx, double cy, bool online, bool vis_hog, bool vis_tracked) {
			
		face_analyser->AddNextFrame(frame->Mat, *clm->getCLM(), time_ms, online, vis_hog);

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

		// Grab pupil locations
		int part_left = -1;
		int part_right = -1;
		for (size_t i = 0; i < clm->getCLM()->hierarchical_models.size(); ++i)
		{
			if (clm->getCLM()->hierarchical_model_names[i].compare("left_eye_28") == 0)
			{
				part_left = i;
			}
			if (clm->getCLM()->hierarchical_model_names[i].compare("right_eye_28") == 0)
			{
				part_right = i;
			}
		}

		cv::Mat_<double> eyeLdmks3d_left = clm->getCLM()->hierarchical_models[part_left].GetShape(fx, fy, cx, cy);
		Point3f pupil_left_h = FaceAnalysis::GetPupilPosition(eyeLdmks3d_left);
		pupil_left->x = pupil_left_h.x; pupil_left->y = pupil_left_h.y; pupil_left->z = pupil_left_h.z;

		cv::Mat_<double> eyeLdmks3d_right = clm->getCLM()->hierarchical_models[part_right].GetShape(fx, fy, cx, cy);
		Point3f pupil_right_h = FaceAnalysis::GetPupilPosition(eyeLdmks3d_right);
		pupil_right->x = pupil_right_h.x; pupil_right->y = pupil_right_h.y; pupil_right->z = pupil_right_h.z;

		// Do the rapport prediction
		rapport_analyser->AddObservation(*clm->getCLM(), *face_analyser, *gazeDirection0, *gazeDirection1, fx, fy, cx, cy);

	}
		
	double GetRapport()
	{
		return rapport_analyser->GetRapportEstimate();
	}

	double GetAttention()
	{
		return rapport_analyser->GetAttentionEstimate();
	}

	double GetValence()
	{
		return rapport_analyser->GetValenceEstimate();
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

	List<Tuple<System::Windows::Point, System::Windows::Point>^>^ CalculateGazeLines(float fx, float fy, float cx, float cy) 
	{
		
		cv::Mat_<double> cameraMat = (cv::Mat_<double>(3, 3) << fx, 0, cx, 0, fy, cy, 0, 0, 0);

		vector<Point3d> points_left;
		points_left.push_back(Point3d(*pupil_left));
		points_left.push_back(Point3d(*pupil_left + *gazeDirection0*40.0));

		vector<Point3d> points_right;
		points_right.push_back(Point3d(*pupil_right));
		points_right.push_back(Point3d(*pupil_right + *gazeDirection1*40.0));

		vector<Point2d> imagePoints_left;
		projectPoints(points_left, Mat::eye(3, 3, DataType<double>::type), Mat::zeros(1, 3, DataType<double>::type), cameraMat, Mat::zeros(4, 1, DataType<double>::type), imagePoints_left);
		
		vector<Point2d> imagePoints_right;
		projectPoints(points_right, Mat::eye(3, 3, DataType<double>::type), Mat::zeros(1, 3, DataType<double>::type), cameraMat, Mat::zeros(4, 1, DataType<double>::type), imagePoints_right);
		
		auto lines = gcnew List<Tuple<System::Windows::Point, System::Windows::Point>^>();
		lines->Add(gcnew Tuple<System::Windows::Point, System::Windows::Point>(System::Windows::Point(imagePoints_left[0].x, imagePoints_left[0].y), System::Windows::Point(imagePoints_left[1].x, imagePoints_left[1].y)));
		lines->Add(gcnew Tuple<System::Windows::Point, System::Windows::Point>(System::Windows::Point(imagePoints_right[0].x, imagePoints_right[0].y), System::Windows::Point(imagePoints_right[1].x, imagePoints_right[1].y)));

		return lines;
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

		delete pupil_left;
		delete pupil_right;

		if(tracked_vid_writer != 0)
		{
			delete tracked_vid_writer;
		}

		delete rapport_analyser;
	}

	// Destructor. Called on explicit Dispose() only.
	~FaceAnalyserManaged()
	{
		this->!FaceAnalyserManaged();
	}

};
}
