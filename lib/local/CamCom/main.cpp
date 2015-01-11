#include <iostream>

#include <cv.h>
#include <highgui.h>

#include "comet_auto_mf.h"
#include "camera_helper.h"

int main(int argc, char *argv[])
{
    // Initialises COM for cameras, deinitialises in destructor
    comet::auto_mf auto_mf;

	std::vector<camera> cameras = camera_helper::get_all_cameras();

    if (cameras.size() == 0) {
        std::cerr << "No cameras" << std::endl;
        return 1;
    }

    for (camera cam : cameras) {
        // Cameras need to be "activated" before they are used.
		//cam().activate();
		//camera().activate();
		cam.activate();

        std::cout << cam.name() << std::endl;
        std::cout << cam.symlink() << std::endl;

        // List camera media types
        auto media_types = cam.media_types();
        for (const auto& media_type : media_types)
        {
            std::cout << "  " << media_type.resolution().width << "x" << media_type.resolution().height
                << " (" << media_type.framerate() << " fps) - " << to_string(media_type.format()) << std::endl;
        }

        // Set the resolution by changing the camera's current media type
        // camera.set_media_type(media_types[x]);
    
        for (const auto& prop : camera::list_properties())
        {
            if (cam.has(prop))
            {
                auto range = cam.get_range(prop);
                std::cout << "  " << camera::property_name(prop) << ": " << range << std::endl;
            }
        }
    }

	// Assuming the cameras are listed the same way?

    //while(cv::waitKey(10) != -1) {
        for (camera cam : cameras) {
            // Can check camera.symlink() to uniquely identify the camera
			cam.activate();
			auto media_type = cam.media_types()[38];
			cam.set_media_type(media_type);
			
			media_type = cam.get_media_type();
			//media_types[0].format.resolution
			std::cout << "  " << media_type.resolution().width << "x" << media_type.resolution().height
                << " (" << media_type.framerate() << " fps) - " << to_string(media_type.format()) << std::endl;

			//cam.set_media_type(media_types[0]);
			std::cout << cam.is_active() << std::endl;
			std::cout << cam.symlink() << std::endl;
			
            try {
				while(true)
				{
					cv::Mat m = cam.read_frame();
					cv::imshow(cam.name(), m);
					cv::waitKey(5);
				}
            } catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
                return 1;
            }
        }
    //}
	int a;
	std::cin >> a;
}
