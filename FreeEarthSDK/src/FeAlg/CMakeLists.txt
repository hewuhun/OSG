set(OUT_NAME FeAlg)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/FeAlg)

SET(FE_NODE_INCLUDE ${CMAKE_SOURCE_DIR}/include/FeAlg)
SET(FE_NODE_SRC ${CMAKE_SOURCE_DIR}/src/FeAlg)


###########################################################################
#                                                                         #
#                                 FEALG   	                              #
#                                                                         #
###########################################################################
set(FEALG_H
		${FE_NODE_INCLUDE}/Export.h

		${FE_NODE_INCLUDE}/CoordConverter.h
		${FE_NODE_INCLUDE}/MotionStateInfo.h
		${FE_NODE_INCLUDE}/MotionAlgorithm.h
		${FE_NODE_INCLUDE}/FlightMotionAlgorithm.h
		${FE_NODE_INCLUDE}/LandMotionAlgorithm.h
		${FE_NODE_INCLUDE}/BSplineAlgorithm.h
	)

set(FEALG_CPP
		${FE_NODE_SRC}/CoordConverter.cpp
		${FE_NODE_SRC}/MotionStateInfo.cpp
		${FE_NODE_SRC}/MotionAlgorithm.cpp
		${FE_NODE_SRC}/FlightMotionAlgorithm.cpp
		${FE_NODE_SRC}/LandMotionAlgorithm.cpp
		${FE_NODE_SRC}/BSplineAlgorithm.cpp
	)
#SOURCE_GROUP(Common FILES ${FEALG_H} {FEALG_CPP})

if(${BUILD_SHARED_LIBS} STREQUAL "On")
    add_library(
	${OUT_NAME} SHARED 
	${FEALG_H} 
	${FEALG_CPP} 
	)
    ADD_DEFINITIONS(-DFEALG_LIBRARY)
else()
    add_library(
	${OUT_NAME} STATIC 
	${FEALG_H} 
	${FEALG_CPP} 
	)
    ADD_DEFINITIONS(-DFEALG_LIBRARY_STATIC)
endif()


target_link_libraries(
    ${OUT_NAME} 
    ${OPENSCENEGRAPH_LIBRARIES} 
    debug ${osgEarth_DEBUG_LIBRARIES} optimized ${osgEarth_RELEASE_LIBRARIES} 
	FeUtils
 )

INSTALL(TARGETS ${OUT_NAME}  DESTINATION lib)
INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)
INSTALL(FILES ${FEALG_H} DESTINATION include/FeAlg)
