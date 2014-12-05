// OpenCVWrappers.h

#pragma once
#pragma unmanaged

#include "cv.h"

#pragma managed

#pragma make_public(cv::Mat)

using namespace System;
using namespace System::Windows;
using namespace System::Windows::Threading;
using namespace System::Windows::Media;
using namespace System::Windows::Media::Imaging;

namespace OpenCVWrappers {

	public ref class RawImage : IDisposable
	{

	private:

		cv::Mat* mat;

		static int refCount;


	public:

		static int PixelFormatToType(PixelFormat fmt)
		{
			if (fmt == PixelFormats::Gray8)
				return CV_8UC1;
			else if(fmt == PixelFormats::Bgr24)
				return CV_8UC3;
			else if(fmt == PixelFormats::Bgra32)
				return CV_8UC4;
			else if(fmt == PixelFormats::Gray32Float)
				return CV_32FC1;
			else
				throw gcnew Exception("Unsupported pixel format");
		}

		static PixelFormat TypeToPixelFormat(int type)
		{
			switch (type) {
			case CV_8UC1:
				return PixelFormats::Gray8;
			case CV_8UC3:
				return PixelFormats::Bgr24;
			case CV_8UC4:
				return PixelFormats::Bgra32;
			case CV_32FC1:
				return PixelFormats::Gray32Float;
			default:
				throw gcnew Exception("Unsupported image type");
			}
		}

		static property int RefCount {
			int get() { return refCount; }
		}

		RawImage() 
		{
			mat = new cv::Mat();
			refCount++;
		}

		RawImage(const cv::Mat& m)
		{
			mat = new cv::Mat(m);
			refCount++;
		}

		RawImage(RawImage^ img)
		{
			mat = new cv::Mat(img->Mat.clone());
			refCount++;
		}

		RawImage(int width, int height, int type)
		{
			mat = new cv::Mat(height, width, type);
			refCount++;
		}

		RawImage(int width, int height, PixelFormat format)
		{
			int type = RawImage::PixelFormatToType(format);
			mat = new cv::Mat(height, width, type);
			refCount++;
		}

		// Finalizer. Definitely called before Garbage Collection,
		// but not automatically called on explicit Dispose().
		// May be called multiple times.
		!RawImage()
		{
			if (mat)
			{
				delete mat;
				mat = NULL;
				refCount--;
			}
		}

		// Destructor. Called on explicit Dispose() only.
		~RawImage()
		{
			this->!RawImage();
		}


		property int Width
		{
			int get()
			{
				return mat->cols;
			}
		}

		property int Height
		{
			int get()
			{
				return mat->rows;
			}
		}

		property int Stride
		{

			int get()
			{
				return mat->step;
			}
		}

		property PixelFormat Format
		{
			PixelFormat get()
			{
				return TypeToPixelFormat(mat->type());
			}
		}

		property cv::Mat& Mat
		{
			cv::Mat& get()
			{
				return *mat;
			}
		}

		property bool IsEmpty
		{
			bool get()
			{
				return !mat || mat->empty();
			}
		}

		bool UpdateWriteableBitmap(WriteableBitmap^ bitmap)
		{
			if (bitmap == nullptr || bitmap->PixelWidth != Width || bitmap->PixelHeight != Height || bitmap->Format != Format)
				return false;
			else {
				if (mat->data == NULL) {
					cv::Mat zeros(bitmap->PixelHeight, bitmap->PixelWidth, PixelFormatToType(bitmap->Format), 0);
					bitmap->WritePixels(Int32Rect(0, 0, Width, Height), IntPtr(zeros.data), Stride * Height * (Format.BitsPerPixel / 8), Stride, 0, 0);
				}
				else {
					bitmap->WritePixels(Int32Rect(0, 0, Width, Height), IntPtr(mat->data), Stride * Height * (Format.BitsPerPixel / 8), Stride, 0, 0);
				}
				return true;
			}
		}

		WriteableBitmap^ CreateWriteableBitmap()
		{
			return gcnew WriteableBitmap(Width, Height, 72, 72, Format, nullptr);
		}

	};
}
