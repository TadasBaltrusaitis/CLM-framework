# Local libraries
include_directories(${CLM_SOURCE_DIR}/include)
	
include_directories(../../lib/local/CLM/include)
			
add_executable(MultiTrackCLM MultiTrackCLM.cpp)
target_link_libraries(MultiTrackCLM CLM)
target_link_libraries(SimpleCLM dlib)

if(WIN32)
	target_link_libraries(MultiTrackCLM ${OpenCVLibraries})
endif(WIN32)
if(UNIX)
	target_link_libraries(MultiTrackCLM ${OpenCV_LIBS} ${Boost_LIBRARIES})
endif(UNIX)

install (TARGETS MultiTrackCLM DESTINATION ${CMAKE_BINARY_DIR}/bin)