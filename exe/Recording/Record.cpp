///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2014, University of Southern California and University of Cambridge,
// all rights reserved.
//
// THIS SOFTWARE IS PROVIDED “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY. OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Notwithstanding the license granted herein, Licensee acknowledges that certain components
// of the Software may be covered by so-called “open source” software licenses (“Open Source
// Components”), which means any software licenses approved as open source licenses by the
// Open Source Initiative or any substantially similar licenses, including without limitation any
// license that, as a condition of distribution of the software licensed under such license,
// requires that the distributor make the software available in source code format. Licensor shall
// provide a list of Open Source Components for a particular version of the Software upon
// Licensee’s request. Licensee will comply with the applicable terms of such licenses and to
// the extent required by the licenses covering Open Source Components, the terms of such
// licenses will apply in lieu of the terms of this Agreement. To the extent the terms of the
// licenses applicable to Open Source Components prohibit any of the restrictions in this
// License Agreement with respect to such Open Source Component, such restrictions will not
// apply to such Open Source Component. To the extent the terms of the licenses applicable to
// Open Source Components require Licensor to make an offer to provide source code or
// related information in connection with the Software, such offer is hereby made. Any request
// for source code or related information should be directed to cl-face-tracker-distribution@lists.cam.ac.uk
// Licensee acknowledges receipt of notices for the Open Source Components for the initial
// delivery of the Software.

//     * Any publications arising from the use of this software, including but
//       not limited to academic journal and conference publications, technical
//       reports and manuals, must cite one of the following works:
//
//       Tadas Baltrusaitis, Peter Robinson, and Louis-Philippe Morency. 3D
//       Constrained Local Model for Rigid and Non-Rigid Facial Tracking.
//       IEEE Conference on Computer Vision and Pattern Recognition (CVPR), 2012.    
//
//       Tadas Baltrusaitis, Peter Robinson, and Louis-Philippe Morency. 
//       Constrained Local Neural Fields for robust facial landmark detection in the wild.
//       in IEEE Int. Conference on Computer Vision Workshops, 300 Faces in-the-Wild Challenge, 2013.    
//
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

