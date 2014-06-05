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


// Record.cpp : A useful function for quick recording from a webcam for test purposes

#include <fstream>
#include <sstream>

#include <windows.h>

#include <cv.h>
#include <highgui.h>

#include <stdio.h>
#include <time.h>

#include <filesystem.hpp>
#include <filesystem/fstream.hpp>

#define INFO_STREAM( stream ) \
std::cout << stream << std::endl

#define WARN_STREAM( stream ) \
std::cout << "Warning: " << stream << std::endl

#define ERROR_STREAM( stream ) \
std::cout << "Error: " << stream << std::endl

static void printErrorAndAbort( const std::string & error )
{
    std::cout << error << std::endl;
    abort();
}

#define FATAL_STREAM( stream ) \
printErrorAndAbort( std::string( "Fatal error: " ) + stream )

using namespace std;
using namespace cv;

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

vector<string> get_arguments(int argc, char **argv)
{

	vector<string> arguments;

	for(int i = 1; i < argc; ++i)
	{
		arguments.push_back(string(argv[i]));
	}
	return arguments;
}

int main (int argc, char **argv)
{

	vector<string> arguments = get_arguments(argc, argv);

	// Some initial parameters that can be overriden from command line	
	string outroot, outfile;

	TCHAR NPath[200];
	GetCurrentDirectory(200, NPath);

	// By default write to same directory
	outroot = NPath;
	outroot = outroot + "/recording/";
	outfile = currentDateTime() + ".avi";

	// By default try webcam
	int device = 0;

	for (size_t i = 0; i < arguments.size(); i++)
    {
		if( strcmp( arguments[i].c_str(), "--help" ) == 0 || strcmp( arguments[i].c_str(), "-h" ) == 0 )
        {
            INFO_STREAM( "Usage is [ -r <root dir> | -dev <dev num> ] -of <out file>\n" ); // Inform the user of how to use the program
            exit( 0 );
        }
        else if( strcmp( arguments[i].c_str(), "-dev") == 0 )
        {
            std::stringstream ss;
            ss << arguments[i+1].c_str();
            ss >> device;
        }
        else if (strcmp(arguments[i].c_str(), "-r") == 0)
        {
			outroot = arguments[i+1];
        }
        else if (strcmp(arguments[i].c_str(), "-of") == 0)
        {
			outroot = arguments[i+1];
        }
        else
        {
            WARN_STREAM( "invalid argument" );
        }
        i++;
    }    		

	// Do some grabbing
	VideoCapture vCap;
	INFO_STREAM( "Attempting to capture from device: " << device );
	vCap = VideoCapture( device );

	if( !vCap.isOpened() ) FATAL_STREAM( "Failed to open video source" );
	
	Mat img;
	vCap >> img;
			
	boost::filesystem::path dir(outroot);
	boost::filesystem::create_directory(dir);

	string out_file = outroot + outfile;
	// saving the videos
	cv::VideoWriter video_writer(out_file, CV_FOURCC('D','I','V','X'), 30, img.size(), true);

	ofstream outlog;
	outlog.open((outroot + outfile + ".log").c_str());

	int frameProc = 0;
	while(!img.empty())
	{		
		
		namedWindow("rec",1);

		outlog << frameProc + 1 << " " << time(0);
		outlog << endl;
						
		vCap >> img;
		
		video_writer << img;

		imshow("rec", img);

		frameProc++;
		
		// detect key presses
		char c = cv::waitKey(1);
			
		// quit the application
		if(c=='q')
		{
			outlog.close();
				
			return(0);
		}


	}
			
	return 0;
}

