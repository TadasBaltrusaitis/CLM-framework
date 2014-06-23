
// OPEN GL

#define GLEW_STATIC

//freeglut is version freeglut 2.6.0-3.mp for MSVC
#include "glew.h"
#include "wglew.h"
#include "GL/freeglut.h"

// TODO unix compatibility
#pragma comment(lib, "opengl32.lib")
//#pragma comment(lib, "glew32s.lib")

#include "Avatar.h"
#include <cv.h>

#include <highgui.h>
#include <imgproc.hpp>

bool DISPLAYHISTOGRAMS = false;			//set to 'true' to display three histograms from the CompensateColour operation
bool KIOSKMODE = false;

bool	USESAVEDIMAGE = false;		//boolean variables for the Avatar control state-machine:
bool	CHANGEIMAGE = false;
bool	WRITETODISK = false;		//save current face image as an avatar
bool	resetexpression = false;
double	ERIstrength = 0.333f;	//the amount of ratio image that is applied to the avatar from the warped live stream

bool UNDERLAYER = true; //if the computer is fast enough, it deals with side-lighting quite well. It blends a heavily-blurred original face below the new one

// TODO these globals are very suspect
Mat alphamask;
Mat blurredmask;

GLuint framebuffer = 0;
GLuint renderbuffer;
GLenum status;

int window_height_global = 640;
int window_width_global = 640;

double aspect_ratio_img_global;

bool INIT = false;				

void sendFaceBackgroundBool(bool under)
{		//sent by the main loop to toggle background image
	UNDERLAYER = under;
}

void sendERIstrength(double texmag){	//this is called from the main simpleclm program

	ERIstrength = texmag/100.0;		//the slider is in %
};

//OpenCV -> OpenGL
// Function turn a cv::Mat into a texture, and return the texture ID as a GLuint for use
GLuint matToTexture(const cv::Mat& mat)
{

	Mat continuous_mat;
	// This makes sure we refer to a continuous blocks of pixels in a matrix
	if(mat.isContinuous())
	{
		continuous_mat = mat;
	}
	else
	{
		continuous_mat = mat.clone();
	}

	//TODO perhaps re-implement this so it goes a bit faster using GL pixel buffers, see http://www.songho.ca/opengl/gl_pbo.html 

	// Generate a number for our textureID's unique handle
	GLuint textureID;
	glGenTextures(1, &textureID);

	// Bind to our texture handle
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//use fast 4-byte alignment (default anyway) if possible
	glPixelStorei(GL_UNPACK_ALIGNMENT, (continuous_mat.step & 3) ? 1 : 4);

	//set length of one complete row in data (doesn't need to equal image.cols)
	glPixelStorei(GL_UNPACK_ROW_LENGTH, continuous_mat.step/continuous_mat.elemSize());

	int step     = continuous_mat.step;
	int texture_height   = continuous_mat.rows;
	int texture_width    = continuous_mat.cols;
	int num_channels = continuous_mat.channels();
	
	char * buffer = new char[texture_height * texture_width * num_channels];

	char * data = (char *)continuous_mat.data;;	

	for(int i = 0; i < texture_height; i++)
	{
		memcpy( &buffer[ i * texture_width * num_channels], &data[i*step], texture_width * num_channels);
	}

	if(num_channels == 3){
		glTexImage2D(GL_TEXTURE_2D,  0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	}

	else if (num_channels == 4){ 
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	}

	glGenerateMipmapEXT(GL_TEXTURE_2D);

	delete[] buffer;	

	return textureID;

}

//OpenGL-> OpenCV
//changes a screen buffer to an OpenCV matrix (so it can be post-processed or saved as a video file)
void buffertoMat(Mat &flipped)
{	
	
	Mat img(window_height_global, window_width_global, CV_8UC3);

	//use fast 4-byte alignment (default anyway) if possible
	glPixelStorei(GL_PACK_ALIGNMENT, (img.step & 3) ? 1 : 4);

	//set length of one complete row in destination data (doesn't need to equal img.cols)
	glPixelStorei(GL_PACK_ROW_LENGTH, img.step/img.elemSize());

	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, img.cols, img.rows, GL_BGR, GL_UNSIGNED_BYTE, img.data);

	if (img.empty()){
		cout << "the image seems to be empty! " << endl;
	}

	flip(img, flipped, 0);
}

//This function crops down a camera frame of image labelled with shape points shape to just the rectangle enclosing all shape points and resizes the cropped image to output_size
//It also scales/offsets the matrix of feature points shape to make them refer to the new (cropped) image, result is stored in cropped_imag and cropped_shape
void cropFace(const cv::Mat &image, const cv::Mat_<double> &shape, const cv::Size output_size, cv::Mat &cropped_image, cv::Mat_<double>& cropped_shape)
{			
		
	int p = shape.rows/2;

	// Finding the minimum and maximum x and y values of the shape
	double min_x, min_y, max_x, max_y;
	cv::minMaxLoc(shape(Rect(0, 0, 1, p)), &min_x, &max_x);
	cv::minMaxLoc(shape(Rect(0, p, 1, p)), &min_y, &max_y);

	Rect image_b_box(1, 1, image.cols - 1, image.rows - 1);

	double width = max_x - min_x;
	double height = max_y - min_y;

	Rect face_b_box((int)min_x, (int)min_y, (int)width, (int)height);

	// Intersect image and face bounding boxes, leading to a new bounding box that is valid
	Rect to_crop = image_b_box & face_b_box;

	// Perform the operation in place
	cropped_image = Mat(image, to_crop).clone();

	// TODO this is likely where aspect ratio issues end up arising as width and height are scaled quite differently
	resize(cropped_image, cropped_image, output_size, 0, 0, INTER_NEAREST );
	
	cropped_shape = shape.clone();
	for(int i = 0; i < p; i++)
	{
		cropped_shape.at<double>(i,0) -= min_x;
		cropped_shape.at<double>(i,0) *= (output_size.width/(width));
		cropped_shape.at<double>(i+p,0) -= min_y;
		cropped_shape.at<double>(i+p,0) *= (output_size.height/(height));
	}
}

void displayHistogram(string windowname, vector<cv::Mat> bgr_planes){
	/// Establish the number of bins
	int histSize = 256;

	/// Set the ranges ( for B,G,R) )
	float range[] = { 0, 256 } ;
	const float* histRange = { range };

	bool uniform = true; bool accumulate = false;

	Mat b_hist, g_hist, r_hist;

	/// Compute the histograms:
	calcHist( &bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
	calcHist( &bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
	calcHist( &bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );

	// Draw the histograms for B, G and R
	int hist_w = 512; int hist_h = 400;
	int bin_w = cvRound( (double) hist_w/histSize );

	Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

	/// Normalize the result to [ 0, histImage.rows ]
	normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
	normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
	normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

	/// Draw for each channel
	for( int i = 1; i < histSize; i++ )
	{
		line( histImage, Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ) ,
			Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
			Scalar( 255, 0, 0), 2, 8, 0  );
		line( histImage, Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ) ,
			Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
			Scalar( 0, 255, 0), 2, 8, 0  );
		line( histImage, Point( bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1)) ) ,
			Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
			Scalar( 0, 0, 255), 2, 8, 0  );
	}

	/// Display
	imshow(windowname, histImage );
}

//this is an alternative function for compensating colours, instead of compensateColours. It creates a lookup table to completely align the RGB histograms. 
//it is a little slower, and normally doesn't give results as good as compensateColours. However, in drastic lighting conditions it can give far superior results
void compensateColoursHistograms(Mat &compensator, Mat &compensated){
	if((compensator.channels() == 3) && (compensated.channels() == 3)){

		if(compensated.size() != blurredmask.size()){
			resize(compensated,compensated,blurredmask.size(),0,0,1);
		}

		vector<cv::Mat> channels, sourcechannels;
		split(compensated, channels);
		split(compensator, sourcechannels);

		Mat histMask = alphamask;

		/// Establish the number of bins
		int histSize = 256;

		/// Set the ranges ( for B,G,R) )
		float range[] = { 0, 256 } ;
		const float* histRange = { range };

		bool uniform = true; bool accumulate = false;

		Mat b_hist, g_hist, r_hist;

		Mat b_hist_before, g_hist_before, r_hist_before;
		calcHist( &channels[0], 1, 0, histMask, b_hist_before, 1, &histSize, &histRange, uniform, accumulate );
		calcHist( &channels[1], 1, 0, histMask, g_hist_before, 1, &histSize, &histRange, uniform, accumulate );
		calcHist( &channels[2], 1, 0, histMask, r_hist_before, 1, &histSize, &histRange, uniform, accumulate );

		//make cumulative
		for( int i = 1; i < histSize; i++ )
		{
			b_hist_before.at<float>(i) += b_hist_before.at<float>(i-1) ;
			g_hist_before.at<float>(i) += g_hist_before.at<float>(i-1) ;
			r_hist_before.at<float>(i) += r_hist_before.at<float>(i-1) ;
		}

		normalize(b_hist_before, b_hist_before, 0, 256, NORM_MINMAX, -1, Mat() );
		normalize(g_hist_before, g_hist_before, 0, 256, NORM_MINMAX, -1, Mat() );
		normalize(r_hist_before, r_hist_before, 0, 256, NORM_MINMAX, -1, Mat() );

		r_hist_before.convertTo(r_hist_before,CV_8U);

		equalizeHist(channels[0],channels[0]);
		equalizeHist(channels[1],channels[1]);
		equalizeHist(channels[2],channels[2]);

		/// Compute the histograms:
		calcHist( &sourcechannels[0], 1, 0, histMask, b_hist, 1, &histSize, &histRange, uniform, accumulate );
		calcHist( &sourcechannels[1], 1, 0, histMask, g_hist, 1, &histSize, &histRange, uniform, accumulate );
		calcHist( &sourcechannels[2], 1, 0, histMask, r_hist, 1, &histSize, &histRange, uniform, accumulate );

		//make cumulative
		for( int i = 1; i < histSize; i++ )
		{
			b_hist.at<float>(i) += b_hist.at<float>(i-1) ;
			g_hist.at<float>(i) += g_hist.at<float>(i-1) ;
			r_hist.at<float>(i) += r_hist.at<float>(i-1) ;
		}

		normalize(b_hist, b_hist, 0, 256, NORM_MINMAX, -1, Mat() );
		normalize(g_hist, g_hist, 0, 256, NORM_MINMAX, -1, Mat() );
		normalize(r_hist, r_hist, 0, 256, NORM_MINMAX, -1, Mat() );

		Mat bHistNew,gHistNew,rHistNew;

		b_hist.convertTo(bHistNew,CV_8U);
		g_hist.convertTo(gHistNew,CV_8U);
		r_hist.convertTo(rHistNew,CV_8U);

		Mat Bout,Rout,Gout;

		Mat bHistInv(1,256,CV_8U);
		Mat gHistInv(1,256,CV_8U);
		Mat rHistInv(1,256,CV_8U);

		for(int i = 1; i < 256; i++){
			int firstindex = bHistNew.at<uchar>(i-1);
			int secondindex = bHistNew.at<uchar>(i);
			//cout << "i: " << i << ", first index: " << firstindex << ", second index: " << secondindex << endl;
			for(int f = firstindex; f <= secondindex; f++){
				bHistInv.at<uchar>(f) = i;
				//cout << "     f: " << f << ", i: " << i << endl;
			}
		}

		for(int i = 1; i < 256; i++){
			int firstindex = gHistNew.at<uchar>(i-1);
			int secondindex = gHistNew.at<uchar>(i);
			for(int f = firstindex; f <= secondindex; f++){
				gHistInv.at<uchar>(f) = i;
			}
		}	

		for(int i = 1; i < 256; i++){
			int firstindex = rHistNew.at<uchar>(i-1);
			int secondindex = rHistNew.at<uchar>(i);
			for(int f = firstindex; f <= secondindex; f++){
				rHistInv.at<uchar>(f) = i;
			}
		}

		Mat histImage( 256, 256, CV_8UC3, Scalar( 0,0,0) );

		LUT(channels[0],bHistInv,Bout);
		LUT(channels[1],gHistInv,Gout);
		LUT(channels[2],rHistInv,Rout);

		if(!Bout.empty()){
			Bout.copyTo(channels[0]);
			Gout.copyTo(channels[1]);
			Rout.copyTo(channels[2]);
		}

		Mat b_hist_after, g_hist_after, r_hist_after;
		calcHist( &channels[0], 1, 0, histMask, b_hist_after, 1, &histSize, &histRange, uniform, accumulate );
		calcHist( &channels[1], 1, 0, histMask, g_hist_after, 1, &histSize, &histRange, uniform, accumulate );
		calcHist( &channels[2], 1, 0, histMask, r_hist_after, 1, &histSize, &histRange, uniform, accumulate );

		//make cumulative
		for( int i = 1; i < histSize; i++ )
		{
			b_hist_after.at<float>(i) += b_hist_after.at<float>(i-1) ;
			g_hist_after.at<float>(i) += g_hist_after.at<float>(i-1) ;
			r_hist_after.at<float>(i) += r_hist_after.at<float>(i-1) ;
		}

		normalize(b_hist_after, b_hist_after, 0, 256, NORM_MINMAX, -1, Mat() );
		normalize(g_hist_after, g_hist_after, 0, 256, NORM_MINMAX, -1, Mat() );
		normalize(r_hist_after, r_hist_after, 0, 256, NORM_MINMAX, -1, Mat() );


		r_hist_after.convertTo(r_hist_after,CV_8U);

		/// Draw for each channel
		for( int i = 1; i < histSize; i++ )
		{

			line( histImage, Point( i-1 , r_hist_before.at<uchar>(i-1) ) ,
				Point(i , r_hist_before.at<uchar>(i)),
				Scalar( 255, 0, 255), 2, 8, 0  );

			line( histImage, Point( i-1 , rHistNew.at<uchar>(i-1) ) ,
				Point(i , rHistNew.at<uchar>(i)),
				Scalar( 255, 0, 0), 2, 8, 0  );

			line( histImage, Point( i-1 , rHistInv.at<uchar>(i-1) ) ,
				Point(i , rHistInv.at<uchar>(i)),
				Scalar( 255, 255, 0), 2, 8, 0  );

			line( histImage, Point( i-1 , r_hist_after.at<uchar>(i-1) ) ,
				Point(i , r_hist_after.at<uchar>(i)),
				Scalar( 0, 255, 255), 2, 8, 0  );
		}

		imshow("histImage",histImage);

		merge(channels, compensated);

		split(compensated, channels);

		if(UNDERLAYER){
			channels.push_back(blurredmask / 1.3f);
		}
		else{
			channels.push_back(blurredmask);
		}

		merge(channels, compensated);

	}

}

void compensateColours(const Mat &compensator, const Mat &to_compensate, Mat &corrected_image, Mat& mask_to_compensate)
{

	if((compensator.channels() == 3) && (to_compensate.channels() == 3)){
		
		// TODO mask here is possibly incorrect

		Mat mask_compensator;
		//Mat mask_to_compensate;

		if(to_compensate.size() != blurredmask.size()){
			resize(blurredmask, mask_compensator, compensator.size());
		}

		Mat histMask = alphamask;

		Mat meanComp,stddevComp;
		Mat meanRef,stddevRef;

		meanStdDev(compensator, meanRef, stddevRef, mask_compensator);

		vector<cv::Mat> channels, refchannels;

		split(to_compensate, channels);

		if(DISPLAYHISTOGRAMS)
		{
			split(compensator, refchannels);
			displayHistogram("source",channels);
			displayHistogram("ref",refchannels);
		}

		for(int c = 0; c < 3; c++){

			channels[c].convertTo(channels[c],CV_32F);

			Mat oldmean,oldstddev;

			meanStdDev(channels[c], oldmean, oldstddev, mask_to_compensate);
			double scale = stddevRef.at<double>(c,0) / oldstddev.at<double>(0,0);

			channels[c] *= scale;

			double offset = meanRef.at<double>(c,0) - (scale*oldmean.at<double>(0,0));

			channels[c] += offset;

			channels[c].convertTo(channels[c],CV_8U);

		}

		if(DISPLAYHISTOGRAMS){
			displayHistogram("compensated",channels);
		}
		
		Mat_<uchar> underlayer_mask;

		// The values have been determined experimentaly
		if(UNDERLAYER)
		{
			cv::resize(mask_to_compensate*210, underlayer_mask, Size(64,64));
		}
		else
		{
			cv::resize(mask_to_compensate*240, underlayer_mask, Size(64,64));
		}

		// Blur the mask so it is smooth at the borders
		GaussianBlur(underlayer_mask, underlayer_mask, Size(0,0), 11, 11, BORDER_CONSTANT);

		// Manually correct the underlayer mask so that eyebrow region has a lower value as otherwise there is a visible line where a face is warped in		
		int steps = 5;
		double scaling = 1.0/(double)steps;
		for(int i = 0; i < steps; ++i)
		{
			underlayer_mask.row(i) = underlayer_mask.row(i) * (scaling * (double)i);
			underlayer_mask.col(i) = underlayer_mask.col(i) * (scaling * (double)i);
			underlayer_mask.col(underlayer_mask.cols - i - 1) = underlayer_mask.col(underlayer_mask.cols - i - 1) * (scaling * (double)i);
		}

		cv::resize(underlayer_mask, underlayer_mask, mask_to_compensate.size());

		imshow("underlayer_mask", underlayer_mask);

		channels.push_back(underlayer_mask);
		
		merge(channels, corrected_image);
		
	}


}

// A helper function for drawing textures
void drawTexture(GLuint texture_index, const Mat_<double>& shape_in_texture, const Size& texture_size, const Mat_<double>& shape_in_output, const Size& output_size, const Mat_<int>& triangles, bool cull_faces)
{
	glBindTexture(GL_TEXTURE_2D, texture_index);		

	int p = (shape_in_texture.rows/2);

	if(cull_faces)
	{
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);		
	}

	glBegin(GL_TRIANGLES);
	
	for(int l = 0; l < triangles.rows; l++){

		//read from the eyestri matrix - copy the eyes from the avatar image (still active texture)
		int	i = triangles.at<int>(l,0);		
		int	j = triangles.at<int>(l,2);
		int	k = triangles.at<int>(l,1);
		
		glTexCoord2d((shape_in_texture.at<double>(i,0)+1)/texture_size.width, (shape_in_texture.at<double>(i+p,0)+1)/texture_size.height);
		glVertex3d(shape_in_output.at<double>(i,0)/output_size.width, shape_in_output.at<double>(i+p,0)/output_size.height,0.0f);

		glTexCoord2d((shape_in_texture.at<double>(j,0)+1)/texture_size.width, (shape_in_texture.at<double>(j+p,0)+1)/texture_size.height);
		glVertex3d(shape_in_output.at<double>(j,0)/output_size.width, shape_in_output.at<double>(j+p,0)/output_size.height,0.0f);

		glTexCoord2d((shape_in_texture.at<double>(k,0)+1)/texture_size.width, (shape_in_texture.at<double>(k+p,0)+1)/texture_size.height);
		glVertex3d(shape_in_output.at<double>(k,0)/output_size.width, shape_in_output.at<double>(k+p,0)/output_size.height,0.0f);

	}

	glEnd();

}

// warped_avatar_image is only used when considering ERI as it is only needed then for the ratio computation
void drawFaceReplace(const cv::Mat& background_image, const cv::Mat_<double>& background_shape, const cv::Mat& underlayer_image, const cv::Mat_<double>& underlayer_shape, 
	const cv::Mat& avatar_image, const cv::Mat_<double>& avatar_shape, const cv::Mat_<double>& destination_shape, const cv::Mat_<int>& face_triangles, const cv::Mat_<int>& mouth_triangles, const cv::Mat_<int>& eye_triangles)
{

	int height = background_image.rows;
	int width = background_image.cols;

	int avatar_height = avatar_image.rows;
	int avatar_width = avatar_image.cols;

	// TODO rem
	//Mat disp;
	//cvtColor(background_image, disp, CV_BGR2RGB);
	//CLMTracker::Draw(disp, background_shape);
	//cv::imshow("background_shape", disp);

	////disp = avatar_image.clone();
	//cvtColor(avatar_image, disp, CV_BGRA2RGBA);
	//CLMTracker::Draw(disp, avatar_shape);
	//cv::imshow("avatar_image shape", disp);
	
	GLuint underlayer_texture;
	// If underlayer is used
	if(!underlayer_image.empty())
	{		
		// TODO rem
		//cvtColor(underlayer_image, disp, CV_BGRA2RGBA);
		//CLMTracker::Draw(disp, underlayer_shape);
		//cv::imshow("underlayer shape", disp);

		underlayer_texture = matToTexture(underlayer_image);
	}
			
	
	// Set the avatar texture
	GLuint avatar_texture = matToTexture(avatar_image);
	
	// Set the background texture
	GLuint background_texture = matToTexture(background_image);
		
	// Clear the screen and depth buffer, and reset the ModelView matrix to identity
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// Move things onto the screen
	glTranslatef(-1.0f, +1.0f, -0.0f);

	//this is because opengl's pixel order is upside-down
	glScalef(2.0f, -2.0f ,1.0f);							

	glEnable(GL_TEXTURE_2D);
	
	int p = (background_shape.rows/2);

	glEnable (GL_BLEND);
	
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	// This moves the whole background image to be mapped on
	glBindTexture(GL_TEXTURE_2D, background_texture);	
			
	glDisable(GL_CULL_FACE);	

	glBegin(GL_TRIANGLES);

	float xmax = 1.0;
	float ymax = 1.0;

	glTexCoord2f(0.0f,0.0f);
	glVertex3f(0.0f,0.0f,0.0f);

	glTexCoord2f(1.0f,0.0f);
	glVertex3f(xmax,0.0f,0.0f);

	glTexCoord2f(0.0f,1.0f);
	glVertex3f(0.0f,ymax,0.0f);

	glTexCoord2f(0.0f,1.0f);
	glVertex3f(0.0f,ymax,0.0f);

	glTexCoord2f(1.0f,0.0f);
	glVertex3f(xmax,0.0f,0.0f);

	glTexCoord2f(1.0f,1.0f);
	glVertex3f(xmax,ymax,0.0f);

	glEnd();
	
	// Do the mouth filling in from background texture to destination
	drawTexture(background_texture, background_shape, background_image.size(), destination_shape, background_image.size(), mouth_triangles, false);

	// Potentially draw the underlayer texture
	if(!underlayer_image.empty())
	{
		drawTexture(underlayer_texture, underlayer_shape, underlayer_image.size(), destination_shape, background_image.size(), face_triangles, true);
	}

	// Drawing the actual avatar (TODO culling needs to be sorted here) (triangles need to be rearranged properly during read?)
	drawTexture(avatar_texture, avatar_shape, avatar_image.size(), destination_shape, background_image.size(), face_triangles, true);
	
	// Drawing the eyes from the avatar texture, we're not sure which way around the triangles go, hence no culling
	drawTexture(avatar_texture, avatar_shape, avatar_image.size(), destination_shape, background_image.size(), eye_triangles, true);

	glDisable(GL_TEXTURE_2D);

	// Delete the assigned textures	
	glDeleteTextures( 1, &avatar_texture );			
	glDeleteTextures(1, &background_texture);	

	if(!underlayer_image.empty())
	{
		glDeleteTextures(1, &underlayer_texture);
	}

	glFinish();
}

// warped_avatar_image is only used when considering ERI as it is only needed then for the ratio computation
void drawFaceAnimate(const cv::Mat& background_image, const cv::Mat_<double>& background_shape, const cv::Mat& avatar_image, const cv::Mat_<double>& avatar_shape, const cv::Mat_<double>& destination_shape,
	const cv::Mat_<int>& face_triangles, const cv::Mat_<int>& mouth_triangles, const cv::Mat_<int>& eye_triangles)
{
	// TODO this should be set somewhere else? or always resize the background image to this representation
	int height = background_image.rows;
	int width = background_image.cols;

	int avatar_height = avatar_image.rows;
	int avatar_width = avatar_image.cols;

	//Mat disp;
	//cvtColor(background_image, disp, CV_BGR2RGB);
	//CLMTracker::Draw(disp, background_shape);
	//cv::imshow("background_shape", disp);

	////disp = avatar_image.clone();
	//cvtColor(avatar_image, disp, CV_BGRA2RGBA);
	//CLMTracker::Draw(disp, avatar_shape);
	//cv::imshow("avatar_image shape", disp);	
	
	// Set the avatar texture
	GLuint avatar_texture = matToTexture(avatar_image);
	
	// Set the background texture
	GLuint background_texture = matToTexture(background_image);
		
	// Clear the screen and depth buffer, and reset the ModelView matrix to identity
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// Move things onto the screen
	glTranslatef(-1.0f, +1.0f, -0.0f);

	//this is because opengl's pixel order is upside-down
	glScalef(2.0f, -2.0f ,1.0f);							

	glEnable(GL_TEXTURE_2D);
	
	int p = (background_shape.rows/2);

	glEnable (GL_BLEND);
	
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	// This moves the whole background image to be mapped on
	glBindTexture(GL_TEXTURE_2D, background_texture);	
	
	// Do the mouth filling in from background texture to destination
	drawTexture(background_texture, background_shape, background_image.size(), destination_shape, background_image.size(), mouth_triangles, false);

	// Drawing the actual avatar, TODO culling needs to be sorted here as triangles can be in a weird order
	drawTexture(avatar_texture, avatar_shape, avatar_image.size(), destination_shape, background_image.size(), face_triangles, true);
	
	// Drawing the eyes from the avatar texture, we're not sure which way around the triangles go, hence no culling
	drawTexture(avatar_texture, avatar_shape, avatar_image.size(), destination_shape, background_image.size(), eye_triangles, false);

	// Delete the assigned textures	
	glDeleteTextures( 1, &avatar_texture );			
	glDeleteTextures(1, &background_texture);	

	glFinish();
}

// Making sure the image fits when resizing (while also keeping the correct aspect ratio)
void changeSize(int w, int h)
{

	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);

        // Reset Matrix
	glLoadIdentity();

	double aspect_ratio_window = ((double)h) / (double)w;

	// Set the viewport to reflect the aspect ratio to avoid stretching	
	if(aspect_ratio_window > aspect_ratio_img_global)
	{
		h = (int)((double)w * aspect_ratio_img_global);
		
		window_height_global = h;
		window_width_global = w;

		glViewport(0, 0, w, h);
	}
	else
	{
		w = (int)((double)h / aspect_ratio_img_global);
		
		window_height_global = h;
		window_width_global = w;

		glViewport(0, 0, w, h);
	}
	// Change to the projection matrix and set our viewing volume
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// ----- OpenGL settings -----
	// Specify depth function to use
	glDepthFunc(GL_LEQUAL);		

	// Enable the depth buffer
	glEnable(GL_DEPTH_TEST);    

	// Switch to ModelView matrix and reset
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Set our clear colour to black
}

// Needed for changeSize to work
void renderScene(void)
{

}

void initGL(int argc, char* argv[], double aspect_ratio)
{

	// Initialise glfw
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE);

	int width = (int) window_height_global / aspect_ratio;

	glutInitWindowSize(width, window_height_global); 

	window_width_global = width;

	glutCreateWindow("Puppets");

	glewInit();

	// Creation of the mickey mouse alpha mask (TODO move out somewhere else?
	alphamask = Mat::zeros(256,256, CV_8UC1);
	circle(alphamask, Point(64,64), 40, Scalar(200), -1, 8, 0); 
	circle(alphamask, Point(192,64), 40, Scalar(200), -1, 8, 0); 
	circle(alphamask, Point(128,128), 110, Scalar(255), -1, 8, 0); 
	GaussianBlur(alphamask,blurredmask,Size(0,0),11,11,4);

	resize(alphamask, alphamask, Size(128,128),0,0,1);
	resize(blurredmask, blurredmask, Size(128,128),0,0,1);

	// Setup our viewport to be the entire size of the window
	// TODO dealing with resizing
	glViewport(0, 0, width, window_height_global);
	
	// Change to the projection matrix and set our viewing volume
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// ----- OpenGL settings -----
	// Specify depth function to use
	glDepthFunc(GL_LEQUAL);		

	// Enable the depth buffer
	glEnable(GL_DEPTH_TEST);    

	// Switch to ModelView matrix and reset
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Set our clear colour to black
	if(KIOSKMODE)
	{
		 glutHideWindow();
	}

	// Dealing with window resizing	
	glutReshapeFunc(changeSize);
	glutDisplayFunc(renderScene);

}

void faceReplace(const Mat& original_image_bgr, const Mat_<double>& shape_original_image, const Mat& avatar_image, const Mat_<double>& avatar_shape, const Mat_<double>& shape_destination,
	const cv::Mat_<int>& face_triangles, const cv::Mat_<int>& mouth_triangles, const cv::Mat_<int>& eye_triangles, CLMTracker::PAW& paw, bool ERI, Mat& result_image, bool record)
{

	aspect_ratio_img_global = ((double)original_image_bgr.rows) / (double)original_image_bgr.cols;

	// Initialising openGL
	if(INIT != true ) {

		char *myargv [1];
		int myargc=1;
		myargv [0]=_strdup ("Myappname");
		initGL(1, myargv, aspect_ratio_img_global);
		INIT = true;
	}
	
	// First warp the original image to the avatar location (this will be useful for both ERI computation and underlayer creation)
	Mat warped_to_neutral_original;
	paw.Warp(original_image_bgr, warped_to_neutral_original, shape_original_image);
	Mat underlayer_image;

	int num_points = shape_original_image.rows/2;

	Mat_<double> underlayer_shape;

	if(UNDERLAYER)
	{
		// TODO do I care about alphas in underlayer going down? Try without (prob should care as now looks odd, also does underlayer have mouth)

		//Mat_<uchar> pixel_mask = paw.pixel_mask;

		// First crop, then blur, upsample, then warp, and mask be just regular mask
		Mat cropped_resized_original;
		Mat_<double> cropped_original_shape;

		Size small_size(64,64);

		cropFace(original_image_bgr, shape_original_image, small_size, cropped_resized_original, cropped_original_shape);

		GaussianBlur(cropped_resized_original, underlayer_image, Size(0,0), 7, 7);
		cv::resize(underlayer_image, underlayer_image, Size(warped_to_neutral_original.cols, warped_to_neutral_original.rows));	

		underlayer_shape = cropped_original_shape.clone();

		double scaling_x = (double)warped_to_neutral_original.cols / (double)small_size.width;
		double scaling_y = (double)warped_to_neutral_original.rows / (double)small_size.height;

		for(int i = 0; i < num_points; ++i)
		{
			underlayer_shape.at<double>(i) = underlayer_shape.at<double>(i) * scaling_x;
			underlayer_shape.at<double>(i + num_points) = underlayer_shape.at<double>(i + num_points) * scaling_y;
		}
		
		vector<Mat> channels;
		split(underlayer_image, channels);

		// The mask needs to be extended up and down as well		
		//Mat_<uchar> underlayer_mask(small_size, 100);
		Mat_<uchar> underlayer_mask(small_size, 240);

		// TODO the mask needs to be smooth around the edges of the triangle, maybe polyfill?
		int steps = 5;
		double scaling = 1.0/(double)steps;
		for(int i = 0; i < steps; ++i)
		{
			underlayer_mask.row(i) = underlayer_mask.row(i) * (scaling * (double)i);			
			underlayer_mask.col(i) = underlayer_mask.col(i) * (scaling * (double)i);			
			underlayer_mask.col(small_size.width-1-i) = underlayer_mask.col(small_size.width-1-i) * (scaling * (double)i);			
		}
		resize(underlayer_mask, underlayer_mask, underlayer_image.size());

		channels.push_back(underlayer_mask);
		
		merge(channels, underlayer_image);

	}		
	
	// TODO col correct should only be on the face area?

	// Crop the face from the background image and use that for colour correction
	Mat_<double> shape_original_cropped;
	Mat original_image_bgr_cropped;

	// The size of 128 x 128 seems suitable for colour correction
	cropFace(original_image_bgr, shape_original_image, Size(128,128), original_image_bgr_cropped, shape_original_cropped);
	
	// Correct the avatar image using colour correction and masking
	Mat avatar_image_corrected;// = avatar_image.clone();
	compensateColours(original_image_bgr_cropped, avatar_image, avatar_image_corrected, paw.pixel_mask);

	drawFaceReplace(original_image_bgr, shape_original_image, underlayer_image, underlayer_shape, avatar_image_corrected, avatar_shape, shape_destination, face_triangles, mouth_triangles, eye_triangles);

	glutSwapBuffers();

	if(record)
	{
		buffertoMat(result_image);	
	}

}

void faceAnimate(const Mat& original_image_bgr, const Mat_<double>& shape_original_image, const Mat& avatar_image, const Mat_<double>& avatar_shape, const Mat_<double>& shape_destination, 
	const cv::Mat_<int>& face_triangles, const cv::Mat_<int>& mouth_triangles, const cv::Mat_<int>& eye_triangles, CLMTracker::PAW& paw, bool ERI, Mat& result_image, bool record)
{
	
	aspect_ratio_img_global = ((double)original_image_bgr.rows) / (double)original_image_bgr.cols;

	// Initialising openGL
	if(INIT != true ) {
		char *myargv [1];
		int myargc=1;
		myargv [0]=_strdup ("Myappname");
		initGL(1, myargv, aspect_ratio_img_global);
		INIT = true;
	}		

	drawFaceAnimate(original_image_bgr, shape_original_image, avatar_image, avatar_shape, shape_destination, face_triangles, mouth_triangles, eye_triangles);

	glutSwapBuffers();

	if(record)
	{
		buffertoMat(result_image);	
	}
}