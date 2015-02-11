This requires cmake, OpenCV 3.0.0 (or newer), tbb and boost.

Need to do the following:

1. Get newest GCC, done using: sudo apt-get install build-essential

2. Cmake: sudo apt-get install cmake


4. OpenCV 3.0.0, Based on tutorial from http://docs.opencv.org/trunk/doc/tutorials/introduction/linux_install/linux_install.html
4.1 Install OpenCV dependencies:
	sudo apt-get install libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
    sudo apt-get install python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev checkinstall
4.2 Download OpenCV 3.0.0 https://github.com/Itseez/opencv/archive/3.0.0-beta.zip
4.3 Unzip it and create a build folder there:
	
	unzip 3.0.0-beta.zip
	cd opencv-3.0.0-beta
	mkdir build
	cd build

4.4 Build it using: 
	cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D WITH_TBB=ON -D BUILD_SHARED_LIBS=OFF ..
	make -j2
	sudo make install
	
	//sudo checkinstall (and follow the instructions)
	//sudo sh -c 'echo "/usr/local/lib" > /etc/ld.so.conf.d/opencv.conf'
	//sudo ldconfig
	
5. Get Boost: sudo apt-get install libboost1.53-all-dev
	alternatively: sudo apt-get install libboost-all-dev

6. Make the actual CLM-framework and compile it using
	cd CLM-framework	
	cmake .
	make

7. Test it with 

for videos:
	cd bin
	./SimpleCLM -f "../videos/changeLighting.wmv" -f "../videos/0188_03_021_al_pacino.avi" -f "../videos/0217_03_006_alanis_morissette.avi" -f "../videos/0244_03_004_anderson_cooper.avi" -f "../videos/0294_02_004_angelina_jolie.avi" -f "../videos/0417_02_003_bill_clinton.avi" -f "../videos/0490_03_007_bill_gates.avi" -f "../videos/0686_02_003_gloria_estefan.avi" -f "../videos/1034_03_006_jet_li.avi" -f "../videos/1192_01_006_julia_roberts.avi" -f "../videos/1461_01_021_noam_chomsky.avi" -f "../videos/1804_03_006_sylvester_stallone.avi" -f "../videos/1815_01_008_tony_blair.avi" -f "../videos/1869_03_009_victoria_beckham.avi" -f "../videos/1878_01_002_vladimir_putin.avi"
	
for images (slow due to dlib face detection in Unix being slow):
	cd bin
	./SimpleCLMImg -fdir "../videos/" -ofdir "./demo_img/" -oidir "./demo_img/"

for multiple faces (face detection slows it a bit):
	cd bin
	./MultiTrackCLM -f ../videos/multi_face.avi

for feature extraction (i.e. HOG and similarity aligned faces):
	cd bin
	./FeatureExtraction -rigid  -verbose  -f "../videos/default.wmv" -simalign "./output_features/default" -hogalign "./output_features/default.hog"

8. (optional)
	You might experience a problem with "cannon connect to X server" when trying to execute the tracker, a solution can be found here http://askubuntu.com/questions/64820/wkhtmltopdf-wkhtmltoimage-cannot-connect-to-x-server

	run:
	apt-get install xvfb
