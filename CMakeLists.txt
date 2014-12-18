cmake_minimum_required (VERSION 2.6)
project (CLM_framework)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# OpenCV
if(WIN32)
	include_directories(lib/3rdParty/OpenCV/include)
	include_directories(lib/3rdParty/OpenCV/include/opencv)
	link_directories( ${PROJECT_SOURCE_DIR}/lib/3rdParty/OpenCV/lib )
	set(OpenCVLibraries 
			debug opencv_core246d 
			debug opencv_calib3d246d 
			debug opencv_contrib246d 
			debug opencv_core246d 
			debug opencv_features2d246d 
			debug opencv_flann246d 
			debug opencv_gpu246d 
			debug opencv_highgui246d 
			debug opencv_imgproc246d 
			debug opencv_legacy246d 
			debug opencv_ml246d 
			debug opencv_nonfree246d 
			debug opencv_objdetect246d 
			debug opencv_photo246d 
			debug opencv_stitching246d 
			debug opencv_ts246d 
			debug opencv_video246d 
			debug opencv_videostab246d
			
			optimized opencv_core246
			optimized opencv_calib3d246
			optimized opencv_contrib246
			optimized opencv_core246
			optimized opencv_features2d246
			optimized opencv_flann246
			optimized opencv_gpu246
			optimized opencv_highgui246
			optimized opencv_imgproc246
			optimized opencv_legacy246
			optimized opencv_ml246
			optimized opencv_nonfree246
			optimized opencv_objdetect246
			optimized opencv_photo246
			optimized opencv_stitching246
			optimized opencv_ts246
			optimized opencv_video246
			optimized opencv_videostab246)

	if (MSVC)		
		file(GLOB files "lib/3rdParty/OpenCV/bin/Release/*.dll")
		foreach(file ${files})
		  file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/Release)
		endforeach()

		file(GLOB files "lib/3rdParty/OpenCV/bin/Debug/*.dll")
		foreach(file ${files})
		  file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/Debug)
		endforeach()
	else(MSVC)
		file(GLOB files "lib/3rdParty/OpenCV/bin/Release/*.dll")
		foreach(file ${files})
		  file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin)
		endforeach()

		file(GLOB files "lib/3rdParty/OpenCV/bin/Debug/*.dll")
		foreach(file ${files})
		  file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin)
		endforeach()
	endif(MSVC)
endif(WIN32)

if(UNIX)
	find_package( OpenCV 2.4.6 REQUIRED )
	find_package( Boost REQUIRED COMPONENTS filesystem system)
	MESSAGE("Boost information:") 
	MESSAGE("  Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}") 
	MESSAGE("  Boost_LIBRARIES: ${Boost_LIBRARIES}") 
	MESSAGE("  Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}") 

	INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
	INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS}/boost)
	LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})	
endif(UNIX)

find_package(TBB REQUIRED)

# Move CLM model
file(GLOB files "lib/local/CLM/model/*.txt")
foreach(file ${files})
	if (MSVC)
		file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/Debug/model)
		file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/Release/model)
	else(MSVC)
		file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/model)
	endif(MSVC)
endforeach()

file(GLOB files "lib/local/CLM/model/detection_validation/*.txt")
foreach(file ${files})
	if (MSVC)
		file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/Debug/model/detection_validation)
		file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/Release/model/detection_validation)
	else(MSVC)
		file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/model/detection_validation)
	endif(MSVC)
endforeach()

file(GLOB files "lib/local/CLM/model/patch_experts/*.txt")
foreach(file ${files})
	if (MSVC)
		file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/Debug/model/patch_experts)
		file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/Release/model/patch_experts)
	else(MSVC)
		file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/model/patch_experts)
	endif(MSVC)
endforeach()

file(GLOB files "lib/local/CLM/model/pdms/*.txt")
foreach(file ${files})
	if (MSVC)
		file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/Debug/model/pdms)
		file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/Release/model/pdms)
	else(MSVC)
		file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/model/pdms)
	endif(MSVC)
endforeach()

# Move sample videos and images classifiers
file(GLOB files "lib/3rdParty/OpenCV/classifiers/*.xml")
foreach(file ${files})
	if (MSVC)
		file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/Debug/classifiers)
		file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/Release/classifiers)
	else(MSVC)
		file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/classifiers)
	endif(MSVC)
endforeach()

# Move OpenCV classifiers
file(GLOB files "lib/3rdParty/OpenCV/classifiers/*.xml")
foreach(file ${files})
	if (MSVC)
		file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/Debug/classifiers)
		file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/Release/classifiers)
	else(MSVC)
		file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/classifiers)
	endif(MSVC)
endforeach()

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    if (GCC_VERSION VERSION_LESS 4.7)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    else ()
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif ()
else ()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif ()

# Boost
if(WIN32)
	include_directories(lib/3rdParty/boost)
	include_directories(lib/3rdParty/boost/boost)
	link_directories( ${PROJECT_SOURCE_DIR}/lib/3rdParty/boost/lib )
else()
	INCLUDE_DIRECTORIES(${BOOST_INCLUDE_DIR})
endif()

# Eigen
include_directories(lib/3rdParty/Eigen)

# dlib
include_directories(lib/3rdParty/dlib/include)

# dlib library
add_subdirectory(lib/3rdParty/dlib)

# CLM library (ordering matters)
add_subdirectory(lib/local/CLM)
add_subdirectory(lib/local/FaceAnalyser)
# executables
add_subdirectory(exe/SimpleCLMImg)
add_subdirectory(exe/SimpleCLM)
add_subdirectory(exe/MultiTrackCLM)
add_subdirectory(exe/FeatureExtraction)
