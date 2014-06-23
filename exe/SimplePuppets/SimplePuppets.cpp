///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012, Tadas Baltrusaitis, all rights reserved.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
//     * The software is provided under the terms of this licence stricly for
//       academic, non-commercial, not-for-profit purposes.
//     * Redistributions of source code must retain the above copyright notice, 
//       this list of conditions (licence) and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright 
//       notice, this list of conditions (licence) and the following disclaimer 
//       in the documentation and/or other materials provided with the 
//       distribution.
//     * The name of the author may not be used to endorse or promote products 
//       derived from this software without specific prior written permission.
//     * As this software depends on other libraries, the user must adhere to 
//       and keep in place any licencing terms of those libraries.
//     * Any publications arising from the use of this software, including but
//       not limited to academic journal and conference publications, technical
//       reports and manuals, must cite the following work:
//
//       Tadas Baltrusaitis, Peter Robinson, and Louis-Philippe Morency. 3D
//       Constrained Local Model for Rigid and Non-Rigid Facial Tracking.
//       IEEE Conference on Computer Vision and Pattern Recognition (CVPR), 2012.    
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED 
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO 
// EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///////////////////////////////////////////////////////////////////////////////

#include "SimplePuppets.h"


// Boost includes
#include <filesystem.hpp>
#include <filesystem/fstream.hpp>

#include <CLMTracker.h>
#include <CLMParameters.h>
#include <Avatar.h>

#include <iostream>
#include <sstream>

using namespace boost::filesystem;

using namespace std;
using namespace cv;

string oldfile;

bool GETFACE = false;		//get a new face

//called when the 'use webcam' checkbox is ticked
void use_webcam()
{			
	USEWEBCAM = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check));
	CHANGESOURCE = true;
}

void replace_face(){
	face_replace_global = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check1));
}

void recordCheckCallback()
{	
	record_global = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(record_checkbox));
}

void face_under(){
	sendFaceBackgroundBool(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check2)));
}

static gboolean time_handler( GtkWidget *widget ) {
	return TRUE;
}

gboolean expose_event_callback(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	if(opencvImage.width > 0)
	{
		pix = gdk_pixbuf_new_from_data( (guchar*)opencvImage.imageData,
			GDK_COLORSPACE_RGB, FALSE, opencvImage.depth, opencvImage.width,
			opencvImage.height, (opencvImage.widthStep), NULL, NULL);

		gdk_draw_pixbuf(widget->window,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)], pix, 0, 0, 0, 0,
			opencvImage.width, opencvImage.height, GDK_RGB_DITHER_NONE, 0, 0); /* Other possible values are  GDK_RGB_DITHER_MAX,  GDK_RGB_DITHER_NORMAL */
	}

	return TRUE;
}

// This function loads an avatar from an image
static void file_ok_sel( GtkWidget *w, GtkFileSelection *fs )
{
	oldfile = inputfile;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), 0);

	inputfile = gtk_file_selection_get_filename (GTK_FILE_SELECTION (filew));
	cout << "Loading avatar from: " << inputfile << endl;

	GETFACE = true;
	cout << "file: " << inputfile << ", oldfile: " << oldfile << endl;
	gtk_widget_destroy(filew);	
}

// This is used when loading a video from file
static void file_ok_sel_z( GtkWidget *w, GtkFileSelection *fs )
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), 0);
	USEWEBCAM = 0;
	CHANGESOURCE = true;
	inputfile = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));
	cout << "Loading Video File..... " << endl;
	g_print ("%s\n", inputfile);
	gtk_widget_destroy(filez);
}

// Video selection callback
static void selectVideoFromComboBox( GtkWidget *widget, gpointer data )
{
	int selection = gtk_combo_box_get_active(GTK_COMBO_BOX(inputchoice));	
	inputfile = default_videos[selection].first;
	CHANGESOURCE = true;
}

// Our callback.
static void callback( GtkWidget *widget, gpointer data )
{

	string command = string((char *)data);
	
	// TODO need proper avatar saving? Just an image would do
	if(command.compare("save avatar") == 0)
	{
		write_to_file_global = true;
	}

	if(command.compare("load avatar") == 0)
	{

		/* Create a new file selection widget */
		filew = gtk_file_selection_new ("File selection");

		g_signal_connect (GTK_FILE_SELECTION (filew)->ok_button,
			"clicked", G_CALLBACK (file_ok_sel), (gpointer) filew);

		/* Connect the cancel_button to destroy the widget */
		g_signal_connect_swapped (GTK_FILE_SELECTION (filew)->cancel_button,
			"clicked", G_CALLBACK (gtk_widget_destroy),filew);

		/* Lets set the filename, as if this were a save dialog, and we are giving
		a default filename */
		gtk_file_selection_set_filename (GTK_FILE_SELECTION(filew), "../" );

		gtk_widget_show (filew);

	}

	if(command.compare("reset eri") == 0)
	{
		reset_neutral_global = true;
	}

	if(command.compare("load video") == 0)
	{

		/* Create a new file selection widget */
		filez = gtk_file_selection_new ("File selection");

		g_signal_connect (GTK_FILE_SELECTION (filez)->ok_button, "clicked", G_CALLBACK (file_ok_sel_z), (gpointer) filez);

		/* Connect the cancel_button to destroy the widget */
		g_signal_connect_swapped (GTK_FILE_SELECTION (filez)->cancel_button, "clicked", G_CALLBACK (gtk_widget_destroy), filez);

		/* Lets set the filename, as if this were a save dialog, and we are giving
		a default filename */
		gtk_file_selection_set_filename (GTK_FILE_SELECTION(filez), "../videos/");

		gtk_widget_show (filez);

	}

	if(command.compare("reset tracking") == 0)
	{
		// TODO a way to do this
		//clm_model.Reset();
	}
}

/* This callback quits the program */
static gboolean delete_event( GtkWidget *widget, GdkEvent  *event, gpointer   data )
{
	quitmain=true;
	return 0;
}

static void printErrorAndAbort( const std::string & error )
{
	std::cout << error << std::endl;
	abort();
}

#define FATAL_STREAM( stream ) \
	printErrorAndAbort( std::string( "Fatal error: " ) + stream )


vector<string> get_arguments(int argc, char **argv)
{

	vector<string> arguments;

	for(int i = 1; i < argc; ++i)
	{
		arguments.push_back(string(argv[i]));
	}
	return arguments;
}

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d-%H-%M", &tstruct);

    return buf;
}

// Reading in triangle files
void readTriangles(int num_landmarks, Mat_<int>& face_triangles, Mat_<int>& mouth_triangles, Mat_<int>& eye_triangles)
{

	string mouth_file;
	string face_file;
	if(num_landmarks == 66)
	{
		mouth_file = "./avatars/mouth_eyes_tris_66.yml";
		face_file = "./avatars/face_tris_66.yml";
	}
	else if(num_landmarks == 68)
	{
		mouth_file = "./avatars/mouth_eyes_tris_68.yml";
		face_file = "./avatars/face_tris_68.yml";
	}
	else
	{
		// TODO proper error message here
		cout << "Unsupported number of landmarks detected" << endl;
	}

	cout << "Reading mouth and eyes triangles from " << mouth_file << endl;
	FileStorage fsc(mouth_file, FileStorage::READ);		
	fsc["eyestri"] >> eye_triangles;
	fsc["mouthtri"] >> mouth_triangles;
	fsc.release();
	
	cout << "Reading face triangles from " << face_file << endl;
	FileStorage fsc_face(face_file, FileStorage::READ);		
	fsc_face["facetri"] >> face_triangles;
	fsc_face.release();
}

// This is the main loop
void doFaceTracking(int argc, char **argv)
{
	
	Mat_<double> avatar_shape;
	Mat avatar_image;

	// Getting input arguments
	vector<string> arguments = get_arguments(argc, argv);
	
	CLMTracker::CLMParameters clm_parameters(arguments);
	
	// Initialise our tracker
	CLMTracker::CLM clm_model = CLMTracker::CLM(clm_parameters.model_location);
		
	// Create a new CLM model for avatars
	CLMTracker::CLM clm_model_avatar(clm_model);

	// Avatar selection is performed through a combo box, determine the initial combo box selection
	int avatar_selection = gtk_combo_box_get_active(GTK_COMBO_BOX(avatarchoice));

	// This is the Piecewise affine warp that will be used to map to the avatar
	CLMTracker::PAW paw;

	// Read in the triangles that will be used by the avatar
	Mat_<int> face_triangles;
	Mat_<int> mouth_triangles;
	Mat_<int> eye_triangles;

	int num_landmarks = clm_model.pdm.NumberOfPoints();

	readTriangles(num_landmarks, face_triangles, mouth_triangles, eye_triangles);

	// The object for capturing videos from webcam or files
	VideoCapture video_capture;

	// We will be resizing our videos to this
	int drawing_area_width;
	int drawing_area_height;
	gdk_window_get_size(gtk_widget_get_window(drawing_area), &drawing_area_width, &drawing_area_height);

	// Object for recording videos
	VideoWriter video_writer;

	// The main loop
	while( true )
	{

		// Useful when dealing with avatar for colour normalisation, ERI etc.

		// Face mask includes the inner part of the face without eyes and the mouth
		Mat_<uchar> mask_avatar_face;

		// Eye mask includes just the eyes
		Mat_<uchar> mask_avatar_eyes;

		// This will store a normalised version of the face for computing ERI
		Mat neutral_face_reference;


		while(gtk_events_pending ())
		{
			gtk_main_iteration ();
		}

		cout << "Button pressed:" << gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check)) << endl;

		// Some initial parameters that can be overriden from command line	
		vector<string> files, depth_dirs, outposes, outvideos, outfeatures;

		// cx and cy aren't always half dimx or half dimy, so need to be able to override it (start with unit vals and init them if none specified)
		float fx = 500, fy = 500, cx = 0, cy = 0;

		// Get the input output file parameters
		bool camera_plane_pose;

		CLMTracker::get_video_input_output_params(files, depth_dirs, outposes, outvideos, outfeatures, camera_plane_pose, arguments);
		
		// Get camera parameters
		int device = 0;
		CLMTracker::get_camera_params(device, fx, fy, cx, cy, arguments);

		int f_n = -1;

		// We might specify multiple video files as arguments
		if(files.size() > 0)
		{
			f_n++;			
			inputfile = files[f_n];
		}
				
		// Do some grabbing
		if(USEWEBCAM)
		{
			INFO_STREAM( "Attempting to capture from device: " << device );
			video_capture = VideoCapture( device );

			// Attempt highish res video capture
			video_capture.set(CV_CAP_PROP_FRAME_WIDTH, 960);
			video_capture.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

			if( !video_capture.isOpened() ) 
			{
				GtkWidget *dialog;
				dialog = gtk_message_dialog_new (GTK_WINDOW (window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Error loading Webcam. Please load video file");

				/* Destroy the dialog when the user responds to it (e.g. clicks a button) */
				g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
				gtk_widget_show (dialog);

				// Change to reading a file
				USEWEBCAM = false;
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), 0);
				CHANGESOURCE = true;
				cout << "Not using Webcam. " << endl;
			}
		}
		else
		{

			if(inputfile.size() > 0 )
			{
				INFO_STREAM( "Attempting to read from file: " << inputfile );
				video_capture = VideoCapture( inputfile );
			}
			else 
			{
				INFO_STREAM("No file specified. Please use webcam or load file manually");
				USEWEBCAM = 1;
			}
		}

		Mat read_img;
		video_capture >> read_img;

		// If optical centers are not defined just use center of image
		if(cx == 0 || cy == 0)
		{
			cx = read_img.cols / 2.0f;
			cy = read_img.rows / 2.0f;
		}
		
		int frame_processed = 0;
				
		// For measuring the timings
		int64 t1,t0 = cv::getTickCount();
		double fps = 10;

		Mat read_image_bgr;

		CHANGESOURCE = false;

		//todo: fix bug with crash when selecting video file to play under webcam mode (disable video select button?)
		//also occasionally opencv error when changing between different sizes of video input/webcam owing to shape going outside boundries. 
		
		while(!read_img.empty() && !CHANGESOURCE)
		{		
			
			// Grabbing the avatar image
			if(avatar_image.empty() || avatar_selection != gtk_combo_box_get_active(GTK_COMBO_BOX(avatarchoice)))
			{
				avatar_selection = gtk_combo_box_get_active(GTK_COMBO_BOX(avatarchoice));
				string avatar_file = avatar_files[avatar_selection].first;
	
				clm_model_avatar.Reset();
				
				avatar_image = imread( avatar_file);

				Mat_<uchar> grayscale_avatar_image;
				if(avatar_image.channels() == 3)
				{
					cv::cvtColor(avatar_image, grayscale_avatar_image, CV_RGB2GRAY);					
				}
				else
				{
					grayscale_avatar_image = avatar_image.clone();
				}

				vector<string> argumens;
				arguments.push_back("-clmwild");
				CLMTracker::CLMParameters params(arguments);
				bool detection_success = CLMTracker::DetectLandmarksInImage(grayscale_avatar_image, clm_model_avatar, params);
						
				if(detection_success)
				{
					//avatar_shape = clm_model_avatar.detected_landmarks;
				
					Mat_<double> shape_in_image = clm_model_avatar.detected_landmarks;

					double avatar_scale = clm_model_avatar.params_global[0];

					// If avatar is too big resize it for efficiency
					if(avatar_scale > 2)
					{
						avatar_scale = 2;
					}

					// Rotate the avatar shape to a neutral that we will warp to
					clm_model.pdm.CalcShape2D(avatar_shape, clm_model_avatar.params_local, Vec6d(clm_model_avatar.params_global[0], 0, 0, 0, 0, 0));								
					
					// This is an opportunity to create a piecewise affine warp
					Mat_<int> triangles_face_with_eyes;
					cv::vconcat(face_triangles, eye_triangles, triangles_face_with_eyes);

					paw = CLMTracker::PAW(avatar_shape, triangles_face_with_eyes);

					// The triangle ID's of eyes will be above number of face triangles
					mask_avatar_eyes = paw.triangle_id >= face_triangles.rows;

					// Face mask does not include the eyes
					mask_avatar_face = paw.pixel_mask & (mask_avatar_eyes == 0);
					
					// Warp from current shape in image to the normalised avatar image
					paw.Warp(avatar_image, avatar_image, shape_in_image);

					// Correct avatar location from the PAW
					for(int i = 0; i < num_landmarks; i++)
					{
						avatar_shape.at<double>(i, 0) -= paw.min_x;
						avatar_shape.at<double>(i + num_landmarks, 0) -= paw.min_y;
					}
					
					cv::imshow("Avatar used", avatar_image);

					// Convert RGB to BGR as that is what OpenGL expects
					if(avatar_image.channels() == 3)
					{
						cvtColor(avatar_image, avatar_image, CV_RGB2BGR);	

					}
					else
					{
						cvtColor(avatar_image, avatar_image, CV_GRAY2BGR);
					}
				}
				else
				{
					// empty the image if failed to construct an avatar
					avatar_image = Mat();
					avatar_shape = Mat_<double>();

					// Inform the user that the face could not be detected
					GtkWidget *dialog;
					dialog = gtk_message_dialog_new (GTK_WINDOW (window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Can't detect a face in an avatar image, aborting");

					/* Destroy the dialog when the user responds to it (e.g. clicks a button) */
					g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
					gtk_widget_show (dialog);
				}

			}

			Mat_<uchar> gray;
			cvtColor(read_img, gray, CV_RGB2GRAY);
			cvtColor(read_img, read_image_bgr, CV_RGB2BGR);

			// For display purposes			
			Mat disp;
			cvtColor(read_img, disp, CV_BGR2RGB);

			if(GRAYSCALE)
			{
				cvtColor(gray, read_image_bgr, CV_GRAY2RGB);
			}

			Vec6d poseEstimateHaar;
			Matx66d poseEstimateHaarUncertainty;

			Rect faceRegion;

			// The actual facial landmark detection / tracking
			bool detection_success = CLMTracker::DetectLandmarksInVideo(gray, Mat_<float>(), clm_model, clm_parameters);
			
			Vec6d pose_estimate_to_draw = CLMTracker::GetCorrectedPoseCameraPlane(clm_model, fx, fy, cx, cy, clm_parameters);

			if(detection_success)			
			{

				// drawing the facial features on the face if tracking is successful
				CLMTracker::Draw(disp, clm_model);
				
				// Draw it in reddish if uncertain, blueish if certain
				CLMTracker::DrawBox(disp, pose_estimate_to_draw, Scalar(0, 0, 255.0), 3, fx, fy, cx, cy);	
			}

			if(frame_processed % 10 == 0)
			{      
				t1 = cv::getTickCount();
				fps = 10.0 / (double(t1-t0)/cv::getTickFrequency()); 
				t0 = t1;
			}
			
			Mat animation_result;

			// The actual animation and displaying step
			if(!avatar_image.empty() && clm_model.detection_certainty < -0.2)
			{
				Mat_<double> local_params_corrected;
				Vec6d global_params_corrected;
		
				// Get expression exaggeration attenuation values
				double mouth = double(gtk_adjustment_get_value(gtk_range_get_adjustment( GTK_RANGE(hscale))));
				double eyebrows = double(gtk_adjustment_get_value(gtk_range_get_adjustment( GTK_RANGE(hscale2))));
				double smile = double(gtk_adjustment_get_value(gtk_range_get_adjustment( GTK_RANGE(hscale3))));		//weight of expression parameters

				// Get head motion attenuation or exaggeration values
				double head_motion = double(gtk_adjustment_get_value(gtk_range_get_adjustment( GTK_RANGE(hscale5))))/100.0;

				clm_model.params_local.copyTo(local_params_corrected);
				global_params_corrected = clm_model.params_global;

				// Exaggeration or attenuation of rotation
				global_params_corrected[1] *= head_motion;
				global_params_corrected[2] *= head_motion;
				global_params_corrected[3] *= head_motion;

				// Exaggeration or attenuation of expression
				local_params_corrected.at<double>(0,0) *= (mouth/100.0);
				local_params_corrected.at<double>(1,0) *= (eyebrows/100.0);
				local_params_corrected.at<double>(2,0) *= (smile/100.0);
		
				// Compute and exaggerated or attenuated shape
				Mat_<double> destination_shape;		
				clm_model.pdm.CalcShape2D(destination_shape, local_params_corrected, global_params_corrected); //calculate new shape

				// TODO if rotation too extreme don't do ERI

		
				if(face_replace_global)
				{
					faceReplace(read_image_bgr, clm_model.detected_landmarks, avatar_image, avatar_shape, destination_shape, face_triangles, mouth_triangles, eye_triangles, paw, false, animation_result, record_global);
				}
				else
				{
					faceAnimate(read_image_bgr, clm_model.detected_landmarks, avatar_image, avatar_shape, destination_shape, face_triangles, mouth_triangles, eye_triangles, paw, false, animation_result, record_global);
				}
			}

			// Recording the animations
			if(video_writer.isOpened())
			{

				if(record_global)
				{
					cv::resize(animation_result, animation_result, Size(read_img.cols, read_img.rows));
					video_writer << animation_result;
				}
				else
				{
					video_writer.release();
					cout << video_writer.isOpened() << endl;
				}

			}
			else if(record_global)
			{
				string time = currentDateTime();
				string recording_root = "./recorded/";

				boost::filesystem::path dir(recording_root);
				boost::filesystem::create_directory(recording_root);

				stringstream ss;
				ss << recording_root << "recording_" << time << ".avi";

				int fps = (int)video_capture.get(CV_CAP_PROP_FPS);

				if(fps == 0)
				{
					fps = 30;
				}
				video_writer.open(ss.str(), CV_FOURCC('D', 'I', 'V', 'X'), fps, Size(read_img.cols, read_img.rows), true);
			}

			frame_processed++;

			char fpsC[255];
			_itoa((int)fps, fpsC, 10);
			string fpsSt("FPS:");
			fpsSt += fpsC;
			cv::putText(disp, fpsSt, cv::Point(10,20), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(255,0,0));
			
			// Setting the image to draw

			// Resize the image appropriately
			double aspect_ratio_drawing_area = ((double)drawing_area_height) / (double)drawing_area_width;
			double aspect_ratio_image = ((double)disp.rows) / (double)disp.cols;

			// Set the viewport to reflect the aspect ratio to avoid stretching	
			if(aspect_ratio_drawing_area > aspect_ratio_image)
			{
				int new_height = (int)((double)drawing_area_width * aspect_ratio_image);
				cv::resize(disp, disp, Size(drawing_area_width, new_height));
			}
			else
			{
				int new_width = (int)((double)drawing_area_height / aspect_ratio_image);
				cv::resize(disp, disp, Size(new_width, drawing_area_height));
			}

			opencvImage = disp;

			gtk_widget_draw(GTK_WIDGET(drawing_area), NULL);
			
			// Saving the avatar from current image feed (TODO rename getface)			
			if(write_to_file_global || GETFACE)
			{
				string image_loc;
				string image_name;
				if(GETFACE)
				{
					image_loc = inputfile;
					image_name = path(inputfile).filename().stem().string();
					read_img = imread(image_loc);

					// Reset the flag
					GETFACE = false;
				}
				else
				{
					// The new avatar name will be a date?
					string time = currentDateTime();

					stringstream ss_file;
					ss_file << "./avatars/From_Video_" << time << ".png";
					image_loc = ss_file.str();

					stringstream ss_name;
					ss_name << "From Video " << time;
					image_name = ss_name.str();
				}
				imwrite(image_loc, read_img);

				avatar_files.push_back(pair<string,string>(image_loc, image_name));

				// add to the combo box selection
				gtk_combo_box_append_text(GTK_COMBO_BOX(avatarchoice), image_name.c_str());

				// and select it
				gtk_combo_box_set_active(GTK_COMBO_BOX(avatarchoice), avatar_files.size() - 1);

				write_to_file_global = false;
			}

			video_capture >> read_img;

			sendERIstrength(double(gtk_adjustment_get_value(gtk_range_get_adjustment( GTK_RANGE(hscale4)))));
			
			while(gtk_events_pending ()){
				gtk_main_iteration ();
			}
			
			if(quitmain==1){
				cout << "Quit." << endl;
				return;
			}

		}			

		clm_model.Reset();
		
	}

}

vector<pair<string,string> > CollectFiles(string directory, vector<string> extensions)
{

	path image_directory(directory);
	vector<pair<string, string>> result;

	// does the file exist and is it a directory
	if (exists(image_directory) && is_directory(image_directory))   
	{

		vector<path> file_in_directory;                                
		copy(directory_iterator(image_directory), directory_iterator(), back_inserter(file_in_directory));

		for (vector<path>::const_iterator file_iterator (file_in_directory.begin()); file_iterator != file_in_directory.end(); ++file_iterator)
		{
			// check if the extension matches the provided list of extensions
			if(std::find(extensions.begin(), extensions.end(), file_iterator->extension().string()) != extensions.end())
			{
				// First in the pair we store the actual location second we store a parsed name
								
				// Parse the name
				string name = file_iterator->filename().stem().string();
				
				// Replace underscores with spaces
				std::replace(name.begin(), name.end(), '_', ' ');

				result.push_back(pair<string,string>(file_iterator->string(), name));
				
			}
		}
	}
	return result;
}

void startGTK(int argc, char **argv)
{

	gtk_init (&argc, &argv);

	/* Create a new window */
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	
	/* Set the window title */
	gtk_window_set_title (GTK_WINDOW (window), "Puppets Control Panel");

	gtk_window_set_resizable(GTK_WINDOW (window), false);

	/* Set a handler for delete_event that immediately
	* exits GTK. */
	g_signal_connect (window, "delete-event", G_CALLBACK (delete_event), NULL);

	/* Sets the border width of the window. */
	gtk_container_set_border_width (GTK_CONTAINER (window), 20);

	/* Create an n x m table */
	table = gtk_table_new(12,5,TRUE);

	/* Put the table in the main window */
	gtk_container_add (GTK_CONTAINER (window), table);

	/* Create first button */
	button = gtk_button_new_with_label ("Save Avatar");

	/* When the button is clicked, we call the "callback" function
	* with a pointer to "save avatar" as its argument */
	g_signal_connect (button, "clicked", G_CALLBACK (callback), (gpointer) "save avatar");

	/* Insert button 1 into the upper left quadrant of the table */
	gtk_table_attach_defaults (GTK_TABLE (table), button, 0, 1, 0, 1);

	gtk_widget_show (button);

	/* Create a recording checkbox*/
		
	record_checkbox = gtk_check_button_new_with_label ("Record");
	gtk_signal_connect(GTK_OBJECT (record_checkbox), "pressed", recordCheckCallback, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(record_checkbox), 0);
	gtk_table_attach_defaults (GTK_TABLE (table), record_checkbox, 1, 2, 0, 1);
	gtk_widget_show(record_checkbox);

	/* Create "Quit" button */
	button = gtk_button_new_with_label ("Quit");

	/* When the button is clicked, we call the "delete-event" function
	* and the program exits */
	g_signal_connect (button, "clicked", G_CALLBACK (delete_event), NULL);

	/* Create third button */
	button = gtk_button_new_with_label ("Load Avatar Image");

	/* When the button is clicked, we call the "callback" function
	* with a pointer to "load avatar" as its argument */
	g_signal_connect (button, "clicked", G_CALLBACK (callback), (gpointer) "load avatar");

	/* Insert button 3 into the table */
	gtk_table_attach_defaults (GTK_TABLE (table), button, 1, 2, 3, 4);

	gtk_widget_show (button);

	/* Create fourth button */
	button = gtk_button_new_with_label ("Reset ERI Mapping");

	/* When the button is clicked, we call the "callback" function
	* with a pointer to "reset eri" as its argument */
	g_signal_connect (button, "clicked",
		G_CALLBACK (callback), (gpointer) "reset eri");

	/* Insert button 4 into the upper left quadrant of the table */
	gtk_table_attach_defaults (GTK_TABLE (table), button, 0, 1, 3, 4);
	gtk_widget_show (button);

	/* Create fifth button: load video file */

	button = gtk_button_new_with_label ("Load Video File");

	/* When the button is clicked, we call the "callback" function
	* with a pointer to "load video" as its argument */
	g_signal_connect (button, "clicked", G_CALLBACK (callback), (gpointer) "load video");
	/* Insert button 3 into the table */
	gtk_table_attach_defaults (GTK_TABLE (table), button, 0, 1, 1, 2);

	gtk_widget_show (button);

	/* Create sizth button: reset tracking */

	button = gtk_button_new_with_label ("Reset Tracking");

	/* When the button is clicked, we call the "callback" function
	* with a pointer to "reset tracking" as its argument */
	g_signal_connect (button, "clicked",
		G_CALLBACK (callback), (gpointer) "reset tracking");
	/* Insert button 3 into the table */
	gtk_table_attach_defaults (GTK_TABLE (table), button, 1,2, 10,11);

	gtk_widget_show (button);

	/* Add a check button to select the webcam by default */
	check = gtk_check_button_new_with_label ("Use Webcam");
	gtk_signal_connect(GTK_OBJECT (check), "pressed",use_webcam, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), 0);
	gtk_table_attach_defaults (GTK_TABLE (table), check, 0, 1, 10,11);
	gtk_widget_show(check);


	/* Add a check button for face replacement functionality */
	check1 = gtk_check_button_new_with_label ("Face Replacement");
	gtk_signal_connect(GTK_OBJECT (check1), "pressed", replace_face, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check1), 1);
	gtk_table_attach_defaults (GTK_TABLE (table), check1, 0, 1,  11, 12);
	gtk_widget_show(check1);

	/* Add a check button to select the webcam by default */
	check2 = gtk_check_button_new_with_label ("Face Undercoat");
	gtk_signal_connect(GTK_OBJECT (check2), "pressed",face_under, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check2), 1);
	gtk_table_attach_defaults (GTK_TABLE (table), check2, 1, 2, 11,12);
	gtk_widget_show(check2);

	/* Create "Quit" button */
	button = gtk_button_new_with_label ("Quit");


	/* When the button is clicked, we call the "delete-event" function
	* and the program exits */
	g_signal_connect (button, "clicked", G_CALLBACK (delete_event), NULL);

	/* Now add a child widget to the aspect frame */
	drawing_area = gtk_drawing_area_new();

	/* Ask for a window   */
	//gtk_widget_set_size_request(drawing_area, opencvImage.width, opencvImage.height); (do the size properly)
	gtk_table_attach_defaults (GTK_TABLE (table), drawing_area, 2, 5, 0, 12);
	
	g_signal_connect(G_OBJECT (drawing_area), "expose_event", G_CALLBACK (expose_event_callback), NULL);
	gtk_widget_show(drawing_area);

	time_handler(window);

	adj1 = gtk_adjustment_new (100.0, 0.0, 301.0, 0.1, 1.0, 1.0);
	adj2 = gtk_adjustment_new (100.0, 0.0, 301.0, 0.1, 1.0, 1.0);
	adj3 = gtk_adjustment_new (100.0, 0.0, 301.0, 0.1, 1.0, 1.0);
	adj4 = gtk_adjustment_new (33.3, 0.0, 101.0, 0.1, 1.0, 1.0);
	adj5 = gtk_adjustment_new (100.0, 0.0, 301.0, 0.1, 1.0, 1.0);

	avatarchoice = gtk_combo_box_entry_new_text();
	inputchoice = gtk_combo_box_entry_new_text();

	// Find the image files in the avatars folder and construct a combo box based on this
	vector<string> avatar_extensions;
	avatar_extensions.push_back(".jpg");
	avatar_extensions.push_back(".png");

	avatar_files = CollectFiles("./avatars", avatar_extensions);

	for(vector<pair<string,string> >::iterator it = avatar_files.begin(); it != avatar_files.end(); it++)
	{
		gtk_combo_box_append_text(GTK_COMBO_BOX(avatarchoice), it->second.c_str());
	}

	gtk_combo_box_set_active(GTK_COMBO_BOX(avatarchoice), 0);	
	gtk_table_attach_defaults (GTK_TABLE (table), avatarchoice, 0, 2, 2, 3);

	// For selecting default videos (quick access)
	vector<string> video_extensions;
	video_extensions.push_back(".avi");
	video_extensions.push_back(".wmv");

	default_videos = CollectFiles("../videos", video_extensions);

	for(vector<pair<string,string> >::iterator it = default_videos.begin(); it != default_videos.end(); it++)
	{
		gtk_combo_box_append_text(GTK_COMBO_BOX(inputchoice), it->second.c_str());
	}
		
	// Adding a listener for selection
	g_signal_connect (inputchoice, "changed", G_CALLBACK (selectVideoFromComboBox), NULL);

	gtk_widget_show(avatarchoice);

	gtk_table_attach_defaults (GTK_TABLE (table), inputchoice, 0, 2, 4, 5);
	gtk_widget_show(inputchoice);

	hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj1));
	gtk_widget_set_size_request (GTK_WIDGET (hscale), 200, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), hscale, 1, 2, 5,6);

	gtk_range_set_update_policy( GTK_RANGE(hscale), GTK_UPDATE_DELAYED);
	gtk_widget_show (hscale);

	label1 = gtk_label_new("Mouth open (%)");
	label2 = gtk_label_new("Eyebrows (%)");
	label3 = gtk_label_new("Smile (%)");
	label4 = gtk_label_new("ERI (Texture) Mapping (%)");
	label5 = gtk_label_new("Head Rotation (%)");

	gtk_table_attach_defaults (GTK_TABLE (table), label1, 0, 1, 5, 6);
	gtk_table_attach_defaults (GTK_TABLE (table), label2, 0, 1, 6, 7);
	gtk_table_attach_defaults (GTK_TABLE (table), label3, 0, 1, 7, 8);
	gtk_table_attach_defaults (GTK_TABLE (table), label4, 0, 1, 8, 9);
	gtk_table_attach_defaults (GTK_TABLE (table), label5, 0, 1, 9, 10);

	gtk_widget_show (label1);
	gtk_widget_show (label2);
	gtk_widget_show (label3);
	gtk_widget_show (label4);
	gtk_widget_show (label5);

	hscale2 = gtk_hscale_new (GTK_ADJUSTMENT (adj2));
	gtk_widget_set_size_request (GTK_WIDGET (hscale2), 200, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), hscale2, 1, 2,6,7);

	gtk_range_set_update_policy( GTK_RANGE(hscale2), GTK_UPDATE_DELAYED);
	gtk_widget_show (hscale2);

	hscale3 = gtk_hscale_new (GTK_ADJUSTMENT (adj3));
	gtk_widget_set_size_request (GTK_WIDGET (hscale3), 200, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), hscale3, 1, 2, 7,8);

	gtk_range_set_update_policy( GTK_RANGE(hscale3), GTK_UPDATE_DELAYED);
	gtk_widget_show (hscale3);

	hscale4 = gtk_hscale_new (GTK_ADJUSTMENT (adj4));
	gtk_widget_set_size_request (GTK_WIDGET (hscale4), 200, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), hscale4, 1, 2, 8,9);

	gtk_range_set_update_policy( GTK_RANGE(hscale4), GTK_UPDATE_DELAYED);
	gtk_widget_show (hscale4);

	hscale5 = gtk_hscale_new (GTK_ADJUSTMENT (adj5));
	gtk_widget_set_size_request (GTK_WIDGET (hscale5), 200, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), hscale5, 1, 2, 9,10);

	gtk_range_set_update_policy( GTK_RANGE(hscale5), GTK_UPDATE_DELAYED);
	gtk_widget_show (hscale5);

	/* Insert the quit button into the both 
	* lower quadrants of the table */
	gtk_table_attach_defaults (GTK_TABLE (table), button, 1, 2, 1, 2);

	gtk_widget_show (button);

	gtk_widget_show (table);
	gtk_widget_show (window);
		
	// Adding a listener for window resizing (so that drawing area is resized properly)
	// TODO rem
	//g_signal_connect(G_OBJECT(window), "configure-event", G_CALLBACK(resize_callback), NULL);

}

int main (int argc, char **argv)
{

	startGTK(argc,argv);
	
	//TODO: fix bug with crash when selecting video file to play under webcam mode (disable video select button?)
	doFaceTracking(argc,argv);

	return 0;
}

