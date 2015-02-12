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

		// Using DirectShow for capturing from webcams (for MJPG as has issues with other formats)
		camera* webcam;
		comet::auto_mf* this_auto_mf;

		RawImage^ latestFrame;
		RawImage^ mirroredFrame;
		RawImage^ grayFrame;

		double fps;

		bool is_webcam;

	public:


		Capture(int device, int width, int height)
		{
			assert(device >= 0);

			latestFrame = gcnew RawImage();
			mirroredFrame = gcnew RawImage();
			
			this_auto_mf = new comet::auto_mf();

			// Grab the cameras
			webcam = new camera(camera_helper::get_all_cameras()[device]);
			webcam->activate();

			auto media_types = webcam->media_types();

			bool found = false;

			for (int m = 0; m < media_types.size(); ++m)
			{
				auto media_type_curr = media_types[m];
				// For now only allow mjpeg
				if(media_type_curr.format() == MediaFormat::MJPG)
				{
					if(media_type_curr.resolution().width == width && media_type_curr.resolution().height == height)
					{
						found = true;
						webcam->set_media_type(media_type_curr);
						webcam->read_frame();
						break;
					}
				}
			}

			if(!found)
			{
				this_auto_mf->~auto_mf();
				delete this_auto_mf;
				webcam->~camera();
				delete webcam;
				vc = new VideoCapture(device);
				vc->set(CV_CAP_PROP_FRAME_WIDTH, width);
				vc->set(CV_CAP_PROP_FRAME_HEIGHT, height);
			}

			is_webcam = true;
		}

		Capture(System::String^ videoFile)
		{
			latestFrame = gcnew RawImage();
			mirroredFrame = gcnew RawImage();

			vc = new VideoCapture(marshal_as<std::string>(videoFile));
			fps = vc->get(CV_CAP_PROP_FPS);
			is_webcam = false;
		}

		static List<Tuple<System::String^, List<Tuple<int,int>^>^, RawImage^>^>^ GetCameras()
		{

			auto managed_camera_list = gcnew List<Tuple<System::String^, List<Tuple<int,int>^>^, RawImage^>^>();

		    comet::auto_mf auto_mf;

			std::vector<camera> cameras = camera_helper::get_all_cameras();
			
			for (int i = 0; i < cameras.size(); ++i)
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

				for (int m = 0; m < media_types.size(); ++m)
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
			else
			{
				webcam->read_frame().copyTo(mirroredFrame->Mat);
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
			else if(webcam != nullptr)
				return webcam->is_active();
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
			if(this_auto_mf != nullptr)
			{
				this_auto_mf->~auto_mf();
				delete this_auto_mf;				
			}
			
			if(webcam != nullptr)
			{
				webcam->~camera();
			}
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
		private:

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
	
			List<System::Windows::Point>^ CalculateLandmarks() {
				vector<Point> vecLandmarks = ::CLMTracker::CalculateLandmarks(*clm);
				
				List<System::Windows::Point>^ landmarks = gcnew List<System::Windows::Point>();
				for(Point p : vecLandmarks) {
					landmarks->Add(System::Windows::Point(p.x, p.y));
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


		};

	}
}
