include_directories(${BOOST_INCLUDE_DIR})

SET(SOURCE
    src/Face_utils.cpp
)

SET(HEADERS
    include/Face_utils.h	
)

include_directories(./include)
include_directories(${FACEANALYSER_SOURCE_DIR}/include)
include_directories(../CLM/include)

add_library( FaceAnalyser ${SOURCE} ${HEADERS})

install (TARGETS FaceAnalyser DESTINATION bin)
install (FILES HEADERS DESTINATION include)
