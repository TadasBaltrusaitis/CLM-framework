# OpenFace: an open source facial behavior analysis toolkit

Over the past few years, there has been an increased interest in automatic facial behavior analysis and understanding. We present OpenFace – an open source tool intended for computer vision and machine learning researchers, affective computing community and people interested in building interactive applications based on facial behavior analysis. OpenFace is the ﬁrst open source tool capable of facial landmark detection, head pose estimation, facial action unit recognition, and eye-gaze estimation. The computer vision algorithms which represent the core of OpenFace demonstrate state-of-the-art results in all of the above mentioned tasks. Furthermore, our tool is capable of real-time performance and is able to run from a simple webcam without any specialist hardware.

The code was written mainly by Tadas Baltrusaitis during his time at the Language Technologies Institute at the Carnegie Mellon University; Computer Laboratory, University of Cambridge; and Institute for Creative Technologies, University of Southern California.

Special thanks goes to Louis-Philippe Morency and his MultiComp Lab at Institute for Creative Technologies for help in writing and testing the code, and Erroll Wood for the gaze estimation work.

More details about the project - http://www.cl.cam.ac.uk/research/rainbow/projects/openface/

## Table of contents

## Windows Instalation

For Windows this software comes prepackaged with all the necessary binaries and dll's for compilation of the project, you still need to compile it in order to run it. You don't need to download anything additional, just open "OpenFace.sln" using Visual Studio 2015 and compile the code. The project was built and tested on Visual Studio 2015 (can't guarantee compatibility with other versions, and you would need to find/build the appropriate dll and lib files for them yourself). Code was tested on Windows 7/8/10 and Windows Server 2008 can't guarantee compatibility with other Windows versions (but in theory it should work). 

NOTE be sure to run the project without debugger attached and in Release mode for speed (if running from Visual Studio). To run without debugger attach use CTRL + F5 instead of F5. To change from Debug mode to Release mode select Release from drop down menu in the toolbar. This can mean the difference between running at 5fps and 60fps on 320x240px videos. I also found that the x64 version seems to run faster on most machines.

## Unix Instalation

For Unix based systems and different compilers, I included Cmake files for cross-platform and cross-IDE support. For running the code on Ubuntu please see readme-ubuntu.txt for detailed instructions of how to get required libraries and build the project.

## Copyright

Copyright can be found in the Copyright.txt

You have to respect boost, TBB, dlib, and OpenCV licenses.

## Use

Explanation of the code layout and the command line arguments can be found in Readme.txt

## Windows Binaries

Coming Soon

## Citation

If you use any of the resources provided on this page in any of your publications we ask you to cite the following work and the work for a relevant submodule you used.

####Overall system

**OpenFace: an open source facial behavior analysis toolkit**
Tadas Baltrušaitis, Peter Robinson, and Louis-Philippe Morency,
in *IEEE Winter Conference on Applications of Computer Vision*, 2016  

#### Facial landmark detection and tracking

**Constrained Local Neural Fields for robust facial landmark detection in the wild**
Tadas Baltrušaitis, Peter Robinson, and Louis-Philippe Morency. 
in IEEE Int. *Conference on Computer Vision Workshops, 300 Faces in-the-Wild Challenge*, 2013.  

#### Eye gaze tracking

**Rendering of Eyes for Eye-Shape Registration and Gaze Estimation**
Erroll Wood, Tadas Baltrušaitis, Xucong Zhang, Yusuke Sugano, Peter Robinson, and Andreas Bulling 
in *IEEE International. Conference on Computer Vision (ICCV)*,  2015 

#### Facial Action Unit detection

**Cross-dataset learning and person-specific normalisation for automatic Action Unit detection**
Tadas Baltrušaitis, Marwa Mahmoud, and Peter Robinson 
in *Facial Expression Recognition and Analysis Challenge*, 
*IEEE International Conference on Automatic Face and Gesture Recognition*, 2015 
