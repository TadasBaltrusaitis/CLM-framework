# Local libraries
include_directories(${LandmarkDetector_SOURCE_DIR}/include)
	
include_directories(../../lib/local/LandmarkDetector/include)
			
add_executable(FaceLandmarkImg FaceLandmarkImg.cpp)
target_link_libraries(FaceLandmarkImg LandmarkDetector)
target_link_libraries(FaceLandmarkImg dlib)

target_link_libraries(FaceLandmarkImg ${OpenCV_LIBS} ${Boost_LIBRARIES} ${TBB_LIBRARIES})

install (TARGETS FaceLandmarkImg DESTINATION ${CMAKE_BINARY_DIR}/bin)
