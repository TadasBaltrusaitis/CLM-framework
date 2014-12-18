add_executable(FeatureExtraction FeatureExtraction.cpp)

# Local libraries
include_directories(${CLM_SOURCE_DIR}/include)

include_directories(../../lib/local/CLM/include)
			
target_link_libraries(FeatureExtraction CLM)
target_link_libraries(FeatureExtraction dlib)

if(WIN32)
	target_link_libraries(FeatureExtraction ${OpenCVLibraries})
endif(WIN32)
if(UNIX)
    FIND_LIBRARY(TBB_LIB TBB)
    target_link_libraries(FeatureExtraction ${OpenCV_LIBS} ${Boost_LIBRARIES} ${TBB_LIB})
endif(UNIX)

install (TARGETS FeatureExtraction DESTINATION ${CMAKE_BINARY_DIR}/bin)