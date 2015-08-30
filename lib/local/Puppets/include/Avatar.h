

#ifndef __AVATAR_h_
#define __AVATAR_h_

#include <GL/glut.h>
#include <cv.h>
#include <CLM_core.h>

using namespace std;

using namespace cv;

void sendOptions(bool writeto, bool usedsave, string avatar);

// Cropping the face
void cropFace(const cv::Mat &image, const cv::Mat_<double> &shape, const cv::Size output_size, cv::Mat &cropped_image, cv::Mat_<double>& cropped_shape);
 
void initGL(int argc, char* argv[], double aspect_ratio);

void sendERIstrength(double texmag);

// Converting an OpenCV matrix to texture (stores the result in GPU memory)
GLuint matToTexture(const cv::Mat& mat);

void compensateColoursAndAddAlpha(const Mat &compensator, const Mat &to_compensate, Mat &corrected_image, Mat& mask_to_compensate, double alpha_strength);

void addAlphaMask(Mat &threechannel);

void extractResultImage(const Mat_<double> &destination_shape, Mat &result_image);

void buffertoMat(Mat &flipped, Rect roi = Rect());

void sendReplace_Face(bool replace);

void sendFaceBackgroundBool(bool under);

// Drawing texture helper
void drawTexture(GLuint texture_index, const Mat_<double>& shape_in_texture, const Size& texture_size, const Mat_<double>& shape_in_output, const Size& output_size, const Mat_<int>& triangles, bool cull_faces);



// New functions
void faceReplace(const Mat& original_image_bgr, const Mat_<double>& shape_original_image, const Mat& avatar_image, const Mat_<double>& avatar_shape, const Mat& neutral_face_warped, const Mat_<double>& shape_destination, 
	const cv::Mat_<int>& face_triangles, const cv::Mat_<int>& mouth_triangles, const cv::Mat_<int>& eye_triangles, CLMTracker::PAW& paw, bool ERI, Mat& result_image, bool record, double alpha_strength, double eri_multiplier);
void drawFaceReplace(const cv::Mat& background_image, const cv::Mat_<double>& background_shape, const cv::Mat& underlayer_image, const cv::Mat_<double>& underlayer_shape, 
	const cv::Mat& avatar_image, const cv::Mat_<double>& avatar_shape, const cv::Mat_<double>& destination_shape, const cv::Mat_<int>& face_triangles, const cv::Mat_<int>& mouth_triangles, const cv::Mat_<int>& eye_triangles);

void faceAnimate(const Mat& original_image_bgr, const Mat_<double>& shape_original_image, const Mat& avatar_image, const Mat_<double>& avatar_shape, const Mat& neutral_face_warped, const Mat_<double>& shape_destination,
	const cv::Mat_<int>& face_triangles, const cv::Mat_<int>& mouth_triangles, const cv::Mat_<int>& eye_triangles, CLMTracker::PAW& paw, bool ERI, Mat& result_image, bool record);
void drawFaceAnimate(const cv::Mat& background_image, const cv::Mat_<double>& background_shape, const cv::Mat& avatar_image, const cv::Mat_<double>& avatar_shape,
	const cv::Mat_<double>& destination_shape, const cv::Mat_<int>& face_triangles, const cv::Mat_<int>& mouth_triangles, const cv::Mat_<int>& eye_triangles);

#endif


