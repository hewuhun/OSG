set(OUT_NAME FeOcean)
include_directories(${CMAKE_SOURCE_DIR}/include)

SET(PROJECT_INCLUDE ${CMAKE_SOURCE_DIR}/include/FeOcean)
SET(PROJECT_SRC ${CMAKE_SOURCE_DIR}/src/FeOcean)

###########################################################################
#                                                                         #
#                             FEOCEAN    	                              #
#                                                                         #
###########################################################################
set(FEOCEAN_H
		${PROJECT_INCLUDE}/OceanSys.h

		${PROJECT_INCLUDE}/OceanNode.h
		${PROJECT_INCLUDE}/TritonContext.h
		${PROJECT_INCLUDE}/TritonDrawable.h
		${PROJECT_INCLUDE}/TritonNode.h
		${PROJECT_INCLUDE}/Export.h
		${PROJECT_INCLUDE}/WakeGeneratorCallback.h
		${PROJECT_INCLUDE}/RotorWashCallback.h
	)
	

set(FEOCEAN_CPP
		${PROJECT_SRC}/OceanSys.cpp

		${PROJECT_SRC}/OceanNode.cpp
		${PROJECT_SRC}/TritonContext.cpp
		${PROJECT_SRC}/TritonDrawable.cpp
		${PROJECT_SRC}/TritonNode.cpp
		${PROJECT_SRC}/WakeGeneratorCallback.cpp
		${PROJECT_SRC}/RotorWashCallback.cpp
	)

if(${BUILD_SHARED_LIBS} STREQUAL "On")
    add_library(${OUT_NAME} SHARED ${SRC} ${FEOCEAN_H} ${FEOCEAN_CPP})
    ADD_DEFINITIONS(-DOCEAN_LIBRARY)
else()
    add_library(${OUT_NAME} STATIC ${SRC} ${FEOCEAN_H} ${FEOCEAN_CPP})
    ADD_DEFINITIONS(-DOCEAN_LIBRARY_STATIC)
endif()

if(WIN32)
target_link_libraries(
	${OUT_NAME} 
	${OPENSCENEGRAPH_LIBRARIES} 
   	debug ${osgEarth_DEBUG_LIBRARIES} optimized ${osgEarth_RELEASE_LIBRARIES} 
	debug ${osgEarthUtil_DEBUG_LIBRARIES} optimized ${osgEarthUtil_RELEASE_LIBRARIES}  
	debug ${osgEarthSymbology_DEBUG_LIBRARIES} optimized ${osgEarthSymbology_RELEASE_LIBRARIES}
	debug ${osgEarthFeatures_DEBUG_LIBRARIES} optimized ${osgEarthFeatures_RELEASE_LIBRARIES}
	debug ${osgEarthAnnotation_DEBUG_LIBRARIES} optimized ${osgEarthAnnotation_RELEASE_LIBRARIES}
	${OPENGL_LIBRARIES}
	FeUtils
	FeTriton
	)
else()
target_link_libraries(
    ${OUT_NAME}
    ${OPENSCENEGRAPH_LIBRARIES}
    debug ${osgEarth_DEBUG_LIBRARIES} optimized ${osgEarth_RELEASE_LIBRARIES}
    debug ${osgEarthUtil_DEBUG_LIBRARIES} optimized ${osgEarthUtil_RELEASE_LIBRARIES}
    debug ${osgEarthSymbology_DEBUG_LIBRARIES} optimized ${osgEarthSymbology_RELEASE_LIBRARIES}
    debug ${osgEarthFeatures_DEBUG_LIBRARIES} optimized ${osgEarthFeatures_RELEASE_LIBRARIES}
    debug ${osgEarthAnnotation_DEBUG_LIBRARIES} optimized ${osgEarthAnnotation_RELEASE_LIBRARIES}
    ${OPENGL_LIBRARIES}
    FeUtils
   FeTriton
    )

endif()

INSTALL(TARGETS ${OUT_NAME}  DESTINATION lib)
INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)

INSTALL(FILES ${FEOCEAN_H} DESTINATION include/FeOcean)

