cmake_minimum_required (VERSION 2.6)
project (CLM_framework)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# OpenCV
if(WIN32)
	include_directories(lib/3rdParty/OpenCV3.0/include)
	include_directories(lib/3rdParty/OpenCV3.0/include/opencv)
	
	if(MSVC_VERSION == 1600)
		link_directories( ${PROJECT_SOURCE_DIR}/lib/3rdParty/OpenCV3.0/x86/v100/lib )
	endif(MSVC_VERSION)
	
	if(MSVC_VERSION == 1700)
		link_directories( ${PROJECT_SOURCE_DIR}/lib/3rdParty/OpenCV3.0/x86/v110/lib )
	endif(MSVC_VERSION)
	
	set(OpenCVLibraries 
			debug opencv_world300d 
			
			optimized opencv_world300
			)

	if (MSVC)		
		
		if(MSVC_VERSION == 1600)
			file(GLOB files "lib/3rdParty/OpenCV3.0/x86/v100/bin/*.dll")
		endif(MSVC_VERSION)
		
		if(MSVC_VERSION == 1700)
			file(GLOB files "lib/3rdParty/OpenCV3.0/x86/v110/bin/*.dll")
		endif(MSVC_VERSION)
		
		foreach(file ${files})
		  file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/Release)
		endforeach()

		foreach(file ${files})
		  file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/bin/Debug)
		endforeach()
	endif(MSVC)
endif(WIN32)

if(UNIX)
	find_package( OpenCV 3.0 REQUIRED )
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

# Move the hierarchical CLM models
file(GLOB files "lib/local/CLM/model/model*")
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

# Move OpenCV classifiers
file(GLOB files "lib/3rdParty/OpenCV3.0/classifiers/*.xml")
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
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -msse -msse2 -msse3")
    else ()
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -msse -msse2 -msse3")
    endif ()
else ()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -msse -msse2 -msse3")
endif ()

# Boost
if(WIN32)
	include_directories(lib/3rdParty/boost)
	include_directories(lib/3rdParty/boost/boost)
	link_directories( ${PROJECT_SOURCE_DIR}/lib/3rdParty/boost/lib )
else()
	INCLUDE_DIRECTORIES(${BOOST_INCLUDE_DIR})
endif()

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
