# Local libraries
include_directories(${LandmarkDetector_SOURCE_DIR}/include)
	
include_directories(../../lib/local/LandmarkDetector/include)
			
add_executable(FaceLandmarkVidMulti FaceLandmarkVidMulti.cpp)
target_link_libraries(FaceLandmarkVidMulti LandmarkDetector)
target_link_libraries(FaceLandmarkVidMulti dlib)

target_link_libraries(FaceLandmarkVidMulti ${OpenCV_LIBS} ${Boost_LIBRARIES} ${TBB_LIBRARIES})

install (TARGETS FaceLandmarkVidMulti DESTINATION ${CMAKE_BINARY_DIR}/bin)
