
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

bool DISPLAYHISTOGRAMS = false;			//set to 'true' to display three histograms from the CompensateColour operation
bool KIOSKMODE = false;

// TODO rem as well
 //if the computer is fast enough, it deals with side-lighting quite well. It blends a heavily-blurred original face below the new one
bool UNDERLAYER = true;

int window_height_global = 480;
int window_width_global = 640;

double aspect_ratio_img_global;

bool INIT = false;				

void sendFaceBackgroundBool(bool under)
{		//sent by the main loop to toggle background image
	UNDERLAYER = under;
}

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

// This function extracts the openGL image and stores it in the result image (which should be of required size and already contain the desired background)
void extractResultImage(const Mat_<double> &destination_shape, Mat &result_image)
{
	double min_x, min_y, max_x, max_y;
	int num_points = destination_shape.rows / 2;

	cv::minMaxLoc(destination_shape(Rect(0,0,1,num_points)), &min_x, &max_x);
	cv::minMaxLoc(destination_shape(Rect(0,num_points,1,num_points)), &min_y, &max_y);

	// Need to determine actual viewport sizes for reading
	double scale;
	double aspect_ratio_window = (double)window_height_global / (double)window_width_global;

	int window_height_dest = result_image.rows;
	int window_width_dest = result_image.cols;

	if(aspect_ratio_window > aspect_ratio_img_global)
	{
		scale = window_width_global / (double)window_width_dest;		
	}
	else
	{
		scale = window_height_global / (double)window_height_dest;
	}

	// Image boundaries
	Rect image_b_box(0, 0, window_width_dest, window_height_dest);

	double width = max_x - min_x;
	double height = max_y - min_y;

	Rect face_b_box_ocv((int)min_x, (int)min_y, (int)width, (int)height);

	// Make sure roi is within image
	face_b_box_ocv = face_b_box_ocv & image_b_box;

	Rect face_b_box_gl( face_b_box_ocv);
	face_b_box_gl.x = (int)(face_b_box_gl.x  * scale);
	face_b_box_gl.y = (int)(face_b_box_gl.y * scale);
	face_b_box_gl.width = (int)(face_b_box_gl.width * scale);
	face_b_box_gl.height = (int)(face_b_box_gl.height * scale);

	// Inverting the y (as openGL starts y at the bottom and openCV at the top)
	int gl_min_y = window_height_global - face_b_box_gl.y - (int)face_b_box_gl.height;
	face_b_box_gl.y = gl_min_y;

	Mat sub_result;
	buffertoMat(sub_result, face_b_box_gl);	
	resize(sub_result, sub_result, Size(face_b_box_ocv.width, face_b_box_ocv.height));
	
	sub_result.copyTo(result_image(face_b_box_ocv));
}

//OpenGL-> OpenCV
//changes a screen buffer to an OpenCV matrix (so it can be post-processed or saved as a video file)
// Only read the bit of interest for speed (and to store everything in the same output location)
void buffertoMat(Mat &flipped, Rect roi)
{	
	
	// If roi defined use that for reading
	int width = roi.width == 0 ? window_width_global : roi.width;
	int height = roi.width == 0 ? window_height_global : roi.height;

	Mat img(height, width, CV_8UC3);

	//use fast 4-byte alignment (default anyway) if possible
	glPixelStorei(GL_PACK_ALIGNMENT, (img.step & 3) ? 1 : 4);

	//set length of one complete row in destination data (doesn't need to equal img.cols)
	glPixelStorei(GL_PACK_ROW_LENGTH, img.step/img.elemSize());

	glReadBuffer(GL_FRONT);
	if(roi.width == 0)
	{
		glReadPixels(0, 0, img.cols, img.rows, GL_BGR, GL_UNSIGNED_BYTE, img.data);
	}
	else
	{
		glReadPixels(roi.x, roi.y, roi.width, roi.height, GL_BGR, GL_UNSIGNED_BYTE, img.data);
	}

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

void compensateColoursAndAddAlpha(const Mat &compensator, const Mat &to_compensate, Mat &corrected_image, Mat& mask_smooth, double alpha_strength)
{

	if((compensator.channels() == 3) && (to_compensate.channels() == 3)){
		
		// Only use the reliable features (this allows us to ignore eyes and mouth to some extent)
		Mat_<uchar> mask_sharp = mask_smooth > 200;

		Mat meanComp,stddevComp;
		Mat meanRef,stddevRef;

		meanStdDev(compensator, meanRef, stddevRef, mask_sharp);

		vector<cv::Mat> channels, refchannels;

		split(to_compensate, channels);

		if(DISPLAYHISTOGRAMS)
		{
			split(compensator, refchannels);
			displayHistogram("source",channels);
			displayHistogram("ref",refchannels);
		}

		for(int c = 0; c < 3; c++){

			channels[c].convertTo(channels[c], CV_32F);

			Mat oldmean,oldstddev;

			meanStdDev(channels[c], oldmean, oldstddev, mask_sharp);
			double scale = stddevRef.at<double>(c,0) / oldstddev.at<double>(0,0);

			channels[c] *= scale;

			double offset = meanRef.at<double>(c,0) - (scale*oldmean.at<double>(0,0));

			channels[c] += offset;

			channels[c].convertTo(channels[c],CV_8U);

		}

		if(DISPLAYHISTOGRAMS){
			displayHistogram("compensated",channels);
		}		

		channels.push_back(mask_smooth * (alpha_strength / 100));

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
	drawTexture(avatar_texture, avatar_shape, avatar_image.size(), destination_shape, background_image.size(), eye_triangles, false);

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

	int width = (int) ((double)window_height_global / aspect_ratio);

	glutInitWindowSize(width, window_height_global); 

	window_width_global = width;

	glutCreateWindow("Puppets");

	glewInit();
	
	// Setup our viewport to be the entire size of the window
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

void computeSmoothMask(const Mat_<uchar>& pixel_mask, Mat_<uchar>& smooth_mask)
{
	Size small_size = Size(64,64);
	// Do the convolutions in resized image for size independence and speed
	resize(pixel_mask*255, smooth_mask, small_size);

	circle(smooth_mask, Point(32, 0), 10, Scalar(125), -1);	

	// Add the mickey mouse ear thing

	int border_size = 10;

	// Add border to the image for Gaussian blurring
	hconcat(smooth_mask, Mat_<uchar>(smooth_mask.rows, border_size, (uchar)0), smooth_mask);
	hconcat( Mat_<uchar>(smooth_mask.rows, border_size, (uchar)0), smooth_mask, smooth_mask);

	cv::vconcat(smooth_mask, Mat_<uchar>(border_size, smooth_mask.cols, (uchar)0), smooth_mask);
	cv::vconcat(Mat_<uchar>(border_size, smooth_mask.cols, (uchar)0), smooth_mask, smooth_mask);

	Mat_<float> smooth_mask_float;
	smooth_mask.convertTo(smooth_mask_float, CV_32F);
	
	// This ensure that at the edge of the mask we have 0, ensuring nice and smooth blending
	smooth_mask_float.setTo(-255, smooth_mask == 0);
		
		// Blur the mask so it is smooth at the borders
	GaussianBlur(smooth_mask_float, smooth_mask_float, Size(0, 0), 2, 2, BORDER_CONSTANT);

	// smooth less around the mouth area TODO

	// Remove the negative values
	smooth_mask_float.setTo(0, smooth_mask_float < 0);

	smooth_mask_float.convertTo(smooth_mask, CV_8U);

	// Take the valid region
	smooth_mask = smooth_mask(Rect(border_size, border_size, 64, 64));

	// Smoothing the left and right sides a bit (for better blending in)
	int steps = 5;
	double step_size = 1.0/steps;
	for(int i = 0; i < steps; ++i)
	{
		smooth_mask.col(i) *= step_size*i;
		smooth_mask.col(small_size.width - 1 - i) *= step_size*i;
	}

	// Resize to original size
	cv::resize(smooth_mask, smooth_mask, pixel_mask.size());

	//imshow("smooth_mask", smooth_mask);

}

// TODO cleanup
// Constructing an ERI corrected image from a neutral face, current face and an avatar face (all warped to same basis)
void constructERI(const Mat& current, const Mat& neutral_expression, const Mat& avatar_image, Mat& eri_image, double eri_multiplier)
{
	
	//imshow("warped_to_neutral_original", warped_to_neutral_original);
	//imshow("neutral_face_warped", neutral_face_warped);
			
	// Convert to floats in order to do proper ratio computation
	
	Mat current_float;
	current.convertTo(current_float, CV_32FC3);

	Mat neutral_expression_float;	
	neutral_expression.convertTo(neutral_expression_float, CV_32FC3);

	// convert image to YCrCb color space (as we only want to modify Y
	Mat current_float_ycrcb, neutral_expression_float_ycrcb;
	cvtColor(current_float, current_float_ycrcb, CV_BGR2YCrCb);
	cvtColor(neutral_expression_float, neutral_expression_float_ycrcb, CV_BGR2YCrCb);

	// Picking just the first plane for ratio
	vector<Mat> current_expression_planes, neutral_expression_planes;
	split(current_float_ycrcb, current_expression_planes);
	split(neutral_expression_float_ycrcb, neutral_expression_planes);

	// Compute ratio between current expression Y and neutral expression Y
	Mat ratio_image_float;
	divide(current_expression_planes[0], neutral_expression_planes[0], ratio_image_float);

	// Blur the mask for smoother results
	Mat ratio_image_float_blur;
	GaussianBlur( ratio_image_float, ratio_image_float_blur, Size( 5, 5 ), 0, 0 );

	// Normalise the ratio image (to be around 0)
	ratio_image_float -= Scalar(1.0f);

	// use only regions that become darker (wrinkles and shadows)
	ratio_image_float.setTo(0, ratio_image_float_blur >= 0.95);

	// Scale it using our ERI multiplier
	ratio_image_float *= eri_multiplier;

	// Renormalise it
	ratio_image_float += Scalar(1.0f);
	
	// Apply the eri scaling on the actual avatar image
	Mat avatar_face_yuv;

	cvtColor(avatar_image, avatar_face_yuv, CV_BGR2YCrCb);
	vector<Mat> avatar_face_channels;
	split(avatar_face_yuv, avatar_face_channels);

	avatar_face_channels[0].convertTo(avatar_face_channels[0], CV_32F);

	Mat avatar_with_eri;

	multiply(avatar_face_channels[0], ratio_image_float, avatar_with_eri);

	avatar_face_channels[0] = avatar_with_eri;

	avatar_face_channels[0].convertTo(avatar_face_channels[0], CV_8U);

	// now merge the results back
	merge(avatar_face_channels, avatar_with_eri);

	// and produce the output RGB image
	cvtColor(avatar_with_eri, avatar_with_eri, CV_YCrCb2BGR);

	eri_image = avatar_with_eri;

	//imshow("eri_image", eri_image);
	//imshow("avatar_image", avatar_image);

}

void faceReplace(const Mat& original_image_bgr, const Mat_<double>& shape_original_image, const Mat& avatar_image, const Mat_<double>& avatar_shape, const Mat& neutral_face_warped, const Mat_<double>& shape_destination,
	const cv::Mat_<int>& face_triangles, const cv::Mat_<int>& mouth_triangles, const cv::Mat_<int>& eye_triangles, CLMTracker::PAW& paw, bool ERI, Mat& result_image, bool record, double alpha_strength, double eri_multiplier)
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

	int num_points = shape_original_image.rows/2;

	// First warp the original image to the avatar location (this will be useful for both ERI computation, underlayer creation, and colour compensation)
	Mat warped_to_neutral_original;
	paw.Warp(original_image_bgr, warped_to_neutral_original, shape_original_image);

	Mat eri_image;
	constructERI(warped_to_neutral_original, neutral_face_warped, avatar_image, eri_image, eri_multiplier);

	Mat_<uchar> smooth_mask;

	// Correct the avatar image using colour correction and masking
	Mat_<uchar> mask = paw.pixel_mask;

	computeSmoothMask(mask, smooth_mask);
	
	Mat_<double> underlayer_shape;
	Mat underlayer_image;
		
	if(UNDERLAYER)
	{

		// First crop, then blur, upsample, then warp, and mask be just regular mask
		Mat cropped_resized_original;
		Mat_<double> cropped_original_shape;

		Size small_size(64,64);

		cropFace(original_image_bgr, shape_original_image, small_size, cropped_resized_original, cropped_original_shape);

		GaussianBlur(cropped_resized_original, underlayer_image, Size(0,0), 1.5, 1.5);

		paw.Warp(underlayer_image, underlayer_image, cropped_original_shape);
				
		underlayer_shape = paw.destination_landmarks.clone();

		vector<Mat> channels;
		split(underlayer_image, channels);

		// Add an underlayer mask
		channels.push_back(smooth_mask * 0.8);
		
		merge(channels, underlayer_image);
		
	}		
		
	// Perform colour compensation	
	Mat avatar_image_corrected;
	compensateColoursAndAddAlpha(warped_to_neutral_original, eri_image, avatar_image_corrected, smooth_mask, alpha_strength);

	// The actual model drawing
	drawFaceReplace(original_image_bgr, shape_original_image, underlayer_image, underlayer_shape, avatar_image_corrected, avatar_shape, shape_destination, face_triangles, mouth_triangles, eye_triangles);

	// Displaying on the screen
	glutSwapBuffers();

	// For recording and display purposes
	
	cvtColor(original_image_bgr, result_image, CV_BGR2RGB);

	if(record)
	{
		extractResultImage(shape_original_image, result_image);
	}

}

void faceAnimate(const Mat& original_image_bgr, const Mat_<double>& shape_original_image, const Mat& avatar_image, const Mat_<double>& avatar_shape, const Mat& neutral_face_warped, const Mat_<double>& shape_destination, 
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
	
	int num_points = shape_original_image.rows/2;

	drawFaceAnimate(original_image_bgr, shape_original_image, avatar_image, avatar_shape, shape_destination, face_triangles, mouth_triangles, eye_triangles);

	glutSwapBuffers();

	// Only read the relevant bit of an image	
	result_image = Mat::zeros(original_image_bgr.rows, original_image_bgr.cols, original_image_bgr.type());

	extractResultImage(shape_original_image, result_image);	

}