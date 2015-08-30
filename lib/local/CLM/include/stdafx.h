// Precompiled headers stuff

// OpenCV stuff
#include <opencv2/core/core.hpp>
#include "opencv2/objdetect.hpp"
#include "opencv2/calib3d.hpp"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// IplImage stuff (get rid of it? TODO)
#include <opencv2/core/core_c.h>
#include <opencv2/imgproc/imgproc_c.h>

// C++ stuff
#include <stdio.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include <vector>
#include <map>

#define _USE_MATH_DEFINES
#include <math.h>

// dlib stuff
// Used for face detection
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/opencv.h>

// Boost stuff
#include <filesystem.hpp>
#include <filesystem/fstream.hpp>