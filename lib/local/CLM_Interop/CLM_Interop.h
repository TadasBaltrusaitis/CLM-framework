// CLM_Interop.h

#pragma once

#pragma unmanaged

// Include all the unmanaged things we need.

#include <opencv2/core/core.hpp>
#include "opencv2/objdetect.hpp"
#include "opencv2/calib3d.hpp"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// For camera listings
#include "comet_auto_mf.h"
#include "camera_helper.h"

#pragma managed

#include <msclr\marshal.h>
#include <msclr\marshal_cppstd.h>

#include <CLM_core.h>

#include <Face_utils.h>
#include <FaceAnalyser.h>

using namespace System;
using namespace OpenCVWrappers;
using namespace System::Collections::Generic;

using namespace msclr::interop;

namespace CLM_Interop {

	public ref class CaptureFailedException : System::Exception { };
	
	public ref class Capture
	{
	private:

		// OpenCV based video capture for reading from files
		VideoCapture* vc;

		RawImage^ latestFrame;
		RawImage^ mirroredFrame;
		RawImage^ grayFrame;

		double fps;

		bool is_webcam;

	public:

		int width, height;

		Capture(int device, int width, int height)
		{
			assert(device >= 0);

			latestFrame = gcnew RawImage();
			mirroredFrame = gcnew RawImage();

			vc = new VideoCapture(device);
			vc->set(CV_CAP_PROP_FRAME_WIDTH, width);
			vc->set(CV_CAP_PROP_FRAME_HEIGHT, height);

			is_webcam = true;
			this->width = width;
			this->height = height;
		}

		Capture(System::String^ videoFile)
		{
			latestFrame = gcnew RawImage();
			mirroredFrame = gcnew RawImage();

			vc = new VideoCapture(marshal_as<std::string>(videoFile));
			fps = vc->get(CV_CAP_PROP_FPS);
			is_webcam = false;
			this->width = vc->get(CV_CAP_PROP_FRAME_WIDTH);
			this->height = vc->get(CV_CAP_PROP_FRAME_HEIGHT);
		}

		static List<Tuple<System::String^, List<Tuple<int,int>^>^, RawImage^>^>^ GetCameras()
		{

			auto managed_camera_list = gcnew List<Tuple<System::String^, List<Tuple<int,int>^>^, RawImage^>^>();

			// Using DirectShow for capturing from webcams (for MJPG as has issues with other formats)
		    comet::auto_mf auto_mf;

			std::vector<camera> cameras = camera_helper::get_all_cameras();
			
			for (size_t i = 0; i < cameras.size(); ++i)
			{
				cameras[i].activate();
				
				std::string name = cameras[i].name(); 

				// List camera media types
				auto media_types = cameras[i].media_types();

				auto resolutions = gcnew List<Tuple<int,int>^>();

				set<pair<pair<int, int>, media_type>> res_set_mjpg;
				set<pair<pair<int, int>, media_type>> res_set_rgb;

				Mat sample_img;
				RawImage^ sample_img_managed = gcnew RawImage();

				for (size_t m = 0; m < media_types.size(); ++m)
				{
					auto media_type_curr = media_types[m];		
					if(media_type_curr.format() == MediaFormat::MJPG)
					{
						res_set_mjpg.insert(pair<pair<int, int>, media_type>(pair<int,int>(media_type_curr.resolution().width, media_type_curr.resolution().height), media_type_curr));
					}
					else if(media_type_curr.format() == MediaFormat::RGB24)
					{
						res_set_rgb.insert(pair<pair<int, int>, media_type>(pair<int,int>(media_type_curr.resolution().width, media_type_curr.resolution().height), media_type_curr));
					}
				}
				
				bool found = false;

				for (auto beg = res_set_mjpg.begin(); beg != res_set_mjpg.end(); ++beg)
				{
					auto resolution = gcnew Tuple<int, int>(beg->first.first, beg->first.second);
					resolutions->Add(resolution);

					if((resolution->Item1 >= 640) && (resolution->Item2 >= 480) && !found)
					{
						found = true;
						cameras[i].set_media_type(beg->second);
						
						// read several images (to avoid overexposure)						
						for (int k = 0; k < 5; ++k)
							cameras[i].read_frame();

						// Flip horizontally
						cv::flip(cameras[i].read_frame(), sample_img, 1);
					}
				}

				// If we didn't find any MJPG resolutions revert to RGB24
				if(resolutions->Count == 0)
				{
					for (auto beg = res_set_rgb.begin(); beg != res_set_rgb.end(); ++beg)
					{
						auto resolution = gcnew Tuple<int, int>(beg->first.first, beg->first.second);
						resolutions->Add(resolution);

						if((resolution->Item1 >= 640) && (resolution->Item2 >= 480) && !found)
						{
							found = true;
							VideoCapture cap1(i);
							cap1.set(CV_CAP_PROP_FRAME_WIDTH, resolution->Item1);
							cap1.set(CV_CAP_PROP_FRAME_HEIGHT, resolution->Item2);

							for (int k = 0; k < 5; ++k)
								cap1.read(sample_img);

							// Flip horizontally
						cv::flip(sample_img, sample_img, 1);

						}
					}
				}
				sample_img.copyTo(sample_img_managed->Mat);					


				managed_camera_list->Add(gcnew Tuple<System::String^, List<Tuple<int,int>^>^, RawImage^>(gcnew System::String(name.c_str()), resolutions, sample_img_managed));
			}
			return managed_camera_list;
		}

		RawImage^ GetNextFrame()
		{
			if(vc != nullptr)
			{
				
				bool success = vc->read(mirroredFrame->Mat);

				if (!success)
					throw gcnew CaptureFailedException();

			}

			if(is_webcam)
			{
				flip(mirroredFrame->Mat, latestFrame->Mat, 1);
			}
			else
			{
				mirroredFrame->Mat.copyTo(latestFrame->Mat);
			}
			
			if (grayFrame == nullptr) {
				if (latestFrame->Width > 0) {
					grayFrame = gcnew RawImage(latestFrame->Width, latestFrame->Height, CV_8UC1);
				}
			}

			if (grayFrame != nullptr) {
				cvtColor(latestFrame->Mat, grayFrame->Mat, CV_BGR2GRAY);
			}

			return latestFrame;
		}

		bool isOpened()
		{
			if(vc != nullptr)
				return vc->isOpened();
			else
				return false;
		}

		RawImage^ GetCurrentFrameGray() {
			return grayFrame;
		}

		double GetFPS() {
			return fps;
		}
		
		// Finalizer. Definitely called before Garbage Collection,
		// but not automatically called on explicit Dispose().
		// May be called multiple times.
		!Capture()
		{
			delete vc; // Automatically closes capture object before freeing memory.			
		}

		// Destructor. Called on explicit Dispose() only.
		~Capture()
		{
			this->!Capture();
		}
	};

	public ref class Vec6d {

	private:
		cv::Vec6d* vec;

	public:

		Vec6d(cv::Vec6d vec): vec(new cv::Vec6d(vec)) { }
		
		cv::Vec6d* getVec() { return vec; }
	};

	namespace CLMTracker {

		public ref class CLMParameters
		{
		private:
			::CLMTracker::CLMParameters* params;

		public:

			CLMParameters() : params(new ::CLMTracker::CLMParameters()) { }

			::CLMTracker::CLMParameters* getParams() {
				return params;
			}
		};

		public ref class CLM
		{
		public:

			::CLMTracker::CLM* clm;

		public:

			CLM() : clm(new ::CLMTracker::CLM()) { }

			::CLMTracker::CLM* getCLM() {
				return clm;
			}

			void Reset() {
				clm->Reset();
			}

			void Reset(double x, double y) {
				clm->Reset(x, y);
			}


			double GetConfidence()
			{
				return clm->detection_certainty;
			}

			bool DetectLandmarksInVideo(RawImage^ image, CLMParameters^ clmParams) {
				return ::CLMTracker::DetectLandmarksInVideo(image->Mat, *clm, *clmParams->getParams());
			}

			void GetCorrectedPoseCamera(List<double>^ pose, double fx, double fy, double cx, double cy, CLMParameters^ clmParams) {
				auto pose_vec = ::CLMTracker::GetCorrectedPoseCamera(*clm, fx, fy, cx, cy, *clmParams->getParams());
				pose->Clear();
				for(int i = 0; i < 6; ++i)
				{
					pose->Add(pose_vec[i]);
				}
			}

			void GetCorrectedPoseCameraPlane(List<double>^ pose, double fx, double fy, double cx, double cy, CLMParameters^ clmParams) {
				auto pose_vec = ::CLMTracker::GetCorrectedPoseCameraPlane(*clm, fx, fy, cx, cy, *clmParams->getParams());
				pose->Clear();
				for(int i = 0; i < 6; ++i)
				{
					pose->Add(pose_vec[i]);
				}
			}
	
			List<System::Tuple<double,double>^>^ CalculateLandmarks() {
				vector<Point2d> vecLandmarks = ::CLMTracker::CalculateLandmarks(*clm);
				
				List<Tuple<double,double>^>^ landmarks = gcnew List<Tuple<double,double>^>();
				for(Point2d p : vecLandmarks) {
					landmarks->Add(gcnew Tuple<double,double>(p.x, p.y));
				}

				return landmarks;
			}

			List<System::Windows::Media::Media3D::Point3D>^ Calculate3DLandmarks(double fx, double fy, double cx, double cy) {
				
				Mat_<double> shape3D = clm->GetShape(fx, fy, cx, cy);
				
				List<System::Windows::Media::Media3D::Point3D>^ landmarks_3D = gcnew List<System::Windows::Media::Media3D::Point3D>();
				
				for(int i = 0; i < shape3D.cols; ++i) 
				{
					landmarks_3D->Add(System::Windows::Media::Media3D::Point3D(shape3D.at<double>(0, i), shape3D.at<double>(1, i), shape3D.at<double>(2, i)));
				}

				return landmarks_3D;
			}


			// Static functions from the CLMTracker namespace.
			void DrawLandmarks(RawImage^ img, List<System::Windows::Point>^ landmarks) {

				vector<Point> vecLandmarks;

				for(int i = 0; i < landmarks->Count; i++) {
					System::Windows::Point p = landmarks[i];
					vecLandmarks.push_back(Point(p.X, p.Y));
				}

				::CLMTracker::DrawLandmarks(img->Mat, vecLandmarks);
			}

			List<Tuple<System::Windows::Point, System::Windows::Point>^>^ CalculateBox(float fx, float fy, float cx, float cy) {
				::CLMTracker::CLMParameters params = ::CLMTracker::CLMParameters();
				cv::Vec6d pose = ::CLMTracker::GetCorrectedPoseCameraPlane(*clm, fx,fy, cx, cy, params);

				vector<pair<Point, Point>> vecLines = ::CLMTracker::CalculateBox(pose, fx, fy, cx, cy);

				List<Tuple<System::Windows::Point, System::Windows::Point>^>^ lines = gcnew List<Tuple<System::Windows::Point,System::Windows::Point>^>();

				for(pair<Point, Point> line : vecLines) {
					lines->Add(gcnew Tuple<System::Windows::Point, System::Windows::Point>(System::Windows::Point(line.first.x, line.first.y), System::Windows::Point(line.second.x, line.second.y)));
				}

				return lines;
			}

			void DrawBox(System::Collections::Generic::List<System::Tuple<System::Windows::Point, System::Windows::Point>^>^ lines, RawImage^ image, double r, double g, double b, int thickness) {
				cv::Scalar color = cv::Scalar(r,g,b,1);

				vector<pair<Point, Point>> vecLines;

				for(int i = 0; i < lines->Count; i++) {
					System::Tuple<System::Windows::Point, System::Windows::Point>^ points = lines[i];
					vecLines.push_back(pair<Point,Point>(Point(points->Item1.X, points->Item1.Y), Point(points->Item2.X, points->Item2.Y)));
				}

				::CLMTracker::DrawBox(vecLines, image->Mat, color, thickness);
			}

			int GetNumPoints()
			{
				return clm->pdm.NumberOfPoints();
			}

			int GetNumModes()
			{
				return clm->pdm.NumberOfModes();
			}

			// Getting the non-rigid shape parameters describing the facial expression
			List<double>^ GetNonRigidParams()
			{
				List<double>^ non_rigid_params = gcnew List<double>();

				for (int i = 0; i < clm->params_local.rows; ++i)
				{
					non_rigid_params->Add(clm->params_local.at<double>(i));
				}

				return non_rigid_params;
			}

			// Getting the rigid shape parameters describing face scale rotation and translation (scale,rotx,roty,rotz,tx,ty)
			List<double>^ GetRigidParams()
			{
				List<double>^ rigid_params = gcnew List<double>();

				for (size_t i = 0; i < 6; ++i)
				{
					rigid_params->Add(clm->params_global[i]);
				}
				return rigid_params;
			}

			// Rigid params followed by non-rigid ones
			List<double>^ GetParams()
			{
				List<double>^ all_params = GetRigidParams();
				all_params->AddRange(GetNonRigidParams());
				return all_params;
			}

		};

	}

	// TODO split into separate managing files
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

		FaceAnalyserManaged() 
		{
			
			vector<Vec3d> orientation_bins;
			orientation_bins.push_back(Vec3d(0,0,0));
			double scale = 0.7;
			int width = 112;
			int height = 112;

			// TODO relative paths?
			std::string au_location("AU_predictors/AU_all_best.txt");
			std::string tri_location("model/tris_68_full.txt");

			face_analyser = new FaceAnalysis::FaceAnalyser(orientation_bins, scale, width, height, au_location, tri_location);

			hog_features = new cv::Mat_<double>();

			aligned_face = new cv::Mat();
			visualisation = new cv::Mat();
			tracked_face = new cv::Mat();

			num_rows = new int[0];
			num_cols = new int[0];

			good_frame = new bool[0];
			
			align_output_dir = new string();

			hog_output_file = new std::ofstream();
		}

		void SetupAlignedImageRecording(System::String^ directory)
		{
			*align_output_dir = marshal_as<std::string>(directory);

			// TODO create the directory?

			
		}

		void SetupHOGRecording(System::String^ file)
		{
			// First make sure the directory is there, TODO

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

		void AddNextFrame(RawImage^ frame, CLMTracker::CLM^ clm, bool dynamic_shift, bool dynamic_scale) {
			
			face_analyser->AddNextFrame(frame->Mat, *clm->getCLM(), 0, dynamic_shift, dynamic_scale, true);

			face_analyser->GetLatestHOG(*hog_features, *num_rows, *num_cols);
			face_analyser->GetLatestAlignedFace(*aligned_face);

			// TODO make optional
			*visualisation = face_analyser->GetLatestHOGDescriptorVisualisation();

			*good_frame = clm->clm->detection_success;

			*tracked_face = frame->Mat.clone();
			if(clm->clm->detection_success)
			{
				::CLMTracker::Draw(*tracked_face, *clm->clm);
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
			delete tracked_vid_writer;
		}

		// Destructor. Called on explicit Dispose() only.
		~FaceAnalyserManaged()
		{
			this->!FaceAnalyserManaged();
		}

	};
}
