// Camera_Interop.h

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

using namespace System;
using namespace OpenCVWrappers;
using namespace System::Collections::Generic;

using namespace msclr::interop;

namespace Camera_Interop {

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

}
