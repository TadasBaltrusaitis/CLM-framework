include_directories(${BOOST_INCLUDE_DIR})

SET(SOURCE
    src/Face_utils.cpp
	src/FaceAnalyser.cpp
	src/SVM_dynamic_lin.cpp
	src/SVM_static_lin.cpp
	src/SVR_dynamic_lin_regressors.cpp
	src/SVR_static_lin_regressors.cpp
	src/GazeEstimation.cpp
)

SET(HEADERS
    include/Face_utils.h	
	include/FaceAnalyser.h
	include/SVM_dynamic_lin.h
	include/SVM_static_lin.h
	include/SVR_dynamic_lin_regressors.h
	include/SVR_static_lin_regressors.h
	include/GazeEstimation.h
)

include_directories(./include)
include_directories(${FACEANALYSER_SOURCE_DIR}/include)
include_directories(../CLM/include)

add_library( FaceAnalyser ${SOURCE} ${HEADERS})

install (TARGETS FaceAnalyser DESTINATION bin)
install (FILES HEADERS DESTINATION include)
