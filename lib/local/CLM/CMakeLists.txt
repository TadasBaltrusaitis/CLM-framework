include_directories(${BOOST_INCLUDE_DIR})

SET(SOURCE
    src/CCNF_patch_expert.cpp
	src/CLM.cpp
    src/CLM_utils.cpp
	src/CLMTracker.cpp
    src/DetectionValidator.cpp
	src/Patch_experts.cpp
	src/PAW.cpp
    src/PDM.cpp
	src/SVR_patch_expert.cpp
	src/stdafx.cpp
)

SET(HEADERS
    include/CCNF_patch_expert.h
	include/CLM.h
    include/CLM_utils.h
	include/CLMParameters.h
	include/CLMTracker.h
    include/DetectionValidator.h
	include/Patch_experts.h	
    include/PAW.h
	include/PDM.h
	include/SVR_patch_expert.h		
	include/stdafx.h
)

include_directories(./include)
include_directories(${CLM_SOURCE_DIR}/include)

add_library( CLM ${SOURCE} ${HEADERS})

install (TARGETS CLM DESTINATION bin)
install (FILES HEADERS DESTINATION include)