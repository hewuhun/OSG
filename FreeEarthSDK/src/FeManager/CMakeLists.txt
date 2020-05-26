set(OUT_NAME FeManager)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/FeManager)

SET(PROJECT_INCLUDE ${CMAKE_SOURCE_DIR}/include/FeManager)
SET(PROJECT_SRC ${CMAKE_SOURCE_DIR}/src/FeManager)

###########################################################################
#                                                                         #
#                                 FEMANAGER   	                          #
#                                                                         #
###########################################################################
set(FEMANAGER_H
	${PROJECT_INCLUDE}/Export.h
	${PROJECT_INCLUDE}/FreeMarkSys.h
	${PROJECT_INCLUDE}/SceneNodeSerialize.h
	
)

set(FEMANAGER_CPP 
	${PROJECT_SRC}/FreeMarkSys.cpp
	${PROJECT_SRC}/SceneNodeSerialize.cpp
	
	)

if(${BUILD_SHARED_LIBS} STREQUAL "On")
    add_library(${OUT_NAME} SHARED ${FEMANAGER_H} ${FEMANAGER_CPP})
    ADD_DEFINITIONS(-DFEMANAGER_LIBRARY)
else()
    add_library(${OUT_NAME} STATIC ${FEMANAGER_H} ${FEMANAGER_CPP})
    ADD_DEFINITIONS(-DFEMANAGER_LIBRARY_STATIC)
endif()

target_link_libraries(
    ${OUT_NAME} 
    ${OPENSCENEGRAPH_LIBRARIES} 
    debug ${osgEarth_DEBUG_LIBRARIES} optimized ${osgEarth_RELEASE_LIBRARIES} 
    debug ${osgEarthUtil_DEBUG_LIBRARIES} optimized ${osgEarthUtil_RELEASE_LIBRARIES}
	debug ${osgEarthSymbology_DEBUG_LIBRARIES} optimized ${osgEarthSymbology_RELEASE_LIBRARIES}
	debug ${osgEarthFeatures_DEBUG_LIBRARIES} optimized ${osgEarthFeatures_RELEASE_LIBRARIES}
	debug ${osgEarthAnnotation_DEBUG_LIBRARIES} optimized ${osgEarthAnnotation_RELEASE_LIBRARIES}
	FeUtils
	FeExtNode
	FePlots
 )

INSTALL(TARGETS ${OUT_NAME}  DESTINATION lib)
INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)
INSTALL(FILES ${FEMANAGER_H} DESTINATION include/FeManager)

