add_executable(SimpleCLM SimpleCLM.cpp)

# Local libraries
include_directories(${CLM_SOURCE_DIR}/include)

include_directories(../../lib/local/CLM/include)
			
target_link_libraries(SimpleCLM CLM)

target_link_libraries(SimpleCLM dlib)

if(WIN32)
	target_link_libraries(SimpleCLM ${OpenCVLibraries})
endif(WIN32)
if(UNIX)
    target_link_libraries(SimpleCLM ${OpenCV_LIBS} ${Boost_LIBRARIES} ${TBB_LIBRARIES})
endif(UNIX)

install (TARGETS SimpleCLM DESTINATION ${CMAKE_BINARY_DIR}/bin)
