This requires cmake, OpenCV 2.4.6 (or newer) and boost.

Need to do the following:

1. Get newest GCC, done using: sudo apt-get install build-essential

2. Cmake: sudo apt-get install cmake

3. Get checkinstall: sudo apt-get install checkinstall

4. OpenCV 2.4.6, Based on tutorial from http://docs.opencv.org/doc/tutorials/introduction/linux_install/linux_install.html
4.1 Install OpenCV dependencies: sudo apt-get install git libgtk2.0-dev pkg-config python-dev python-numpy libtbb-dev libavcodec-dev libavformat-dev libswscale-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libqt4-dev
4.2 Download OpenCV 2.4.6 from https://github.com/itseez/opencv/archive/2.4.6.zip
4.3 Unzip it and create a build folder there:
	
	unzip 2.4.6.zip
	cd opencv-2.4.6
	mkdir build
	cd build

4.4 Build it using: 
	cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D WITH_TBB=ON -D BUILD_NEW_PYTHON_SUPPORT=ON -D WITH_V4L=ON -D INSTALL_C_EXAMPLES=ON -D WITH_QT=ON -D INSTALL_PYTHON_EXAMPLES=ON -D BUILD_EXAMPLES=ON -D WITH_OPENGL=ON ..
	make -j2
	sudo checkinstall (and follow the instructions)
	sudo sh -c 'echo "/usr/local/lib" > /etc/ld.so.conf.d/opencv.conf'
	sudo ldconfig
	
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
	
for images:
	cd bin
	./SimpleCLMImg -fdir "../videos/" -ofdir "../matlab_runners/demo_img/" -oidir "../matlab_runners/demo_img/"

for multiple faces:
	cd bin
	./MultiTrackCLM -f ../videos/multi_face.avi

8. (optional)
	You might experience a problem with "cannon connect to X server" when trying to execute the tracker, a solution can be found here http://askubuntu.com/questions/64820/wkhtmltopdf-wkhtmltoimage-cannot-connect-to-x-server

	run:
	apt-get install xvfb
