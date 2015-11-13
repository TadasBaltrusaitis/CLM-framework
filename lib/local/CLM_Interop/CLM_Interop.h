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
		public:
			::CLMTracker::CLMParameters* params;

		public:

			CLMParameters(System::String^ root)
			{
				string root_std = marshal_as<std::string>(root);
				vector<string> args;
				args.push_back(root_std);

				params = new ::CLMTracker::CLMParameters(args);

				params->track_gaze = true;
			}

			void optimiseForVideo()
			{
				params->window_sizes_small = vector<int>(4);
				params->window_sizes_init = vector<int>(4);

				// For fast tracking
				params->window_sizes_small[0] = 0;
				params->window_sizes_small[1] = 9;
				params->window_sizes_small[2] = 7;
				params->window_sizes_small[3] = 5;

				// Just for initialisation
				params->window_sizes_init.at(0) = 11;
				params->window_sizes_init.at(1) = 9;
				params->window_sizes_init.at(2) = 7;
				params->window_sizes_init.at(3) = 5;

				// For first frame use the initialisation
				params->window_sizes_current = params->window_sizes_init;

				params->sigma = 1.5;
				params->reg_factor = 25;
				params->weight_factor = 0;
			}			

			void optimiseForImages()
			{
				params->window_sizes_init = vector<int>(4);
				params->window_sizes_init[0] = 15; params->window_sizes_init[1] = 13; 
				params->window_sizes_init[2] = 11; 
				params->window_sizes_init[3] = 9;

				params->multi_view = true;

				params->sigma = 1.25;
				params->reg_factor = 35;
				params->weight_factor = 2.5;
				params->num_optimisation_iteration = 10;
			}			

			::CLMTracker::CLMParameters* getParams() {
				return params;
			}

			~CLMParameters()
			{
				delete params;
			}

		};

		public ref class CLM
		{
		public:

			::CLMTracker::CLM* clm;
			CascadeClassifier* classifier;
			

		public:

			CLM() : clm(new ::CLMTracker::CLM()) { }
			
			CLM(CLMParameters^ params)
			{				
				clm = new ::CLMTracker::CLM(params->getParams()->model_location);				
			}
			
			~CLM()
			{
				delete clm;
			}

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

			bool DetectFaceLandmarksInImage(RawImage^ image, CLMParameters^ clmParams) {
				return ::CLMTracker::DetectLandmarksInImage(image->Mat, *clm, *clmParams->getParams());
			}

			List<List<System::Tuple<double,double>^>^>^ DetectMultiFaceLandmarksInImage(RawImage^ image, CLMParameters^ clmParams) {

				List<List<System::Tuple<double,double>^>^>^ all_landmarks = gcnew List<List<System::Tuple<double,double>^>^>();

				// Detect faces in an image
				vector<Rect_<double> > face_detections;
				
				if(clmParams->params->curr_face_detector == ::CLMTracker::CLMParameters::HOG_SVM_DETECTOR)
				{
					vector<double> confidences;
		
					dlib::frontal_face_detector face_detector_hog = dlib::get_frontal_face_detector();

					::CLMTracker::DetectFacesHOG(face_detections, image->Mat, face_detector_hog, confidences);
				}
				else
				{
					if(classifier== 0)
					{
						classifier = new CascadeClassifier(clmParams->params->face_detector_location);	
					}
					::CLMTracker::DetectFaces(face_detections, image->Mat, *classifier);
				}

				// Detect landmarks around detected faces
				int face_det = 0;
				// perform landmark detection for every face detected
				for(size_t face=0; face < face_detections.size(); ++face)
				{
					Mat depth;
					// if there are multiple detections go through them
					bool success = ::CLMTracker::DetectLandmarksInImage(image->Mat, depth, face_detections[face], *clm, *clmParams->getParams());

					List<System::Tuple<double,double>^>^ landmarks_curr = gcnew List<System::Tuple<double,double>^>();
					if(clm->detected_landmarks.cols == 1)
					{
						int n = clm->detected_landmarks.rows / 2;								 
						for(int i = 0; i < n; ++i)
						{
							landmarks_curr->Add(gcnew System::Tuple<double,double>(clm->detected_landmarks.at<double>(i,0), clm->detected_landmarks.at<double>(i+n,0)));
						}
					}
					else
					{
						int n = clm->detected_landmarks.cols / 2;		
						for(int i = 0; i < clm->detected_landmarks.cols; ++i)
						{
							landmarks_curr->Add(gcnew System::Tuple<double,double>(clm->detected_landmarks.at<double>(0,i), clm->detected_landmarks.at<double>(0,i+1)));
						}
					}
					all_landmarks->Add(landmarks_curr);

				}

				return all_landmarks;
			}

			void GetCorrectedPoseCamera(List<double>^ pose, double fx, double fy, double cx, double cy) {
				auto pose_vec = ::CLMTracker::GetCorrectedPoseCamera(*clm, fx, fy, cx, cy);
				pose->Clear();
				for(int i = 0; i < 6; ++i)
				{
					pose->Add(pose_vec[i]);
				}
			}

			void GetCorrectedPoseCameraPlane(List<double>^ pose, double fx, double fy, double cx, double cy) {
				auto pose_vec = ::CLMTracker::GetCorrectedPoseCameraPlane(*clm, fx, fy, cx, cy);
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

				cv::Vec6d pose = ::CLMTracker::GetCorrectedPoseCameraPlane(*clm, fx,fy, cx, cy);

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

}
