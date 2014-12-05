// CLM_Interop.h

#pragma once

#pragma unmanaged

// Include all the unmanaged things we need.

#include "cv.h"
#include "highgui.h"

#pragma managed

#include <msclr\marshal.h>
#include <msclr\marshal_cppstd.h>

#include <CLM.h>
#include <CLMTracker.h>
#include <CLMParameters.h>
#include <CLM_utils.h>

using namespace System;
using namespace OpenCVWrappers;
using namespace System::Collections::Generic;

using namespace msclr::interop;

namespace CLM_Interop {

	public ref class CaptureFailedException : System::Exception { };

	public ref class Capture
	{
	private:

		VideoCapture* vc;

		RawImage^ latestFrame;
		RawImage^ mirroredFrame;
		RawImage^ grayFrame;

		double fps;

	public:
		
		Capture(int device)
		{
			latestFrame = gcnew RawImage();
			mirroredFrame = gcnew RawImage();

			vc = new VideoCapture(device);
			fps = vc->get(CV_CAP_PROP_FPS);
		}

		Capture(System::String^ videoFile)
		{
			latestFrame = gcnew RawImage();
			mirroredFrame = gcnew RawImage();

			vc = new VideoCapture(marshal_as<std::string>(videoFile));
			fps = vc->get(CV_CAP_PROP_FPS);
		}

		RawImage^ GetNextFrame()
		{
			bool success = vc->read(mirroredFrame->Mat);

			if (!success)
				throw gcnew CaptureFailedException();

			flip(mirroredFrame->Mat, latestFrame->Mat, 1);

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
			return vc->isOpened();
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
