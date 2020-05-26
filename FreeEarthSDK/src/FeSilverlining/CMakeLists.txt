set(OUT_NAME FeSilverlining)
include_directories(${CMAKE_SOURCE_DIR}/include)

SET(PROJECT_INCLUDE ${CMAKE_SOURCE_DIR}/include/FeSilverlining)
SET(PROJECT_SRC ${CMAKE_SOURCE_DIR}/src/FeSilverlining)

###########################################################################
#                                                                         #
#                            SILVERLINING            					  #
#                                                                         #
###########################################################################
set(SILVERLINING_H
		${PROJECT_INCLUDE}/Export.h

		${PROJECT_INCLUDE}/SilverliningSys.h
		${PROJECT_INCLUDE}/SilverLiningNode.h
		${PROJECT_INCLUDE}/SilverLiningContext.h
		${PROJECT_INCLUDE}/SilverLiningCloudsDrawable.h
		${PROJECT_INCLUDE}/SilverLiningOptions.h
		${PROJECT_INCLUDE}/SilverLiningSkyDrawable.h
	)
set(SILVERLINING_CPP
		${PROJECT_SRC}/SilverliningSys.cpp
		${PROJECT_SRC}/SilverLiningNode.cpp
		${PROJECT_SRC}/SilverLiningContext.cpp
		${PROJECT_SRC}/SilverLiningCloudsDrawable.cpp
		${PROJECT_SRC}/SilverLiningSkyDrawable.cpp
		${PROJECT_SRC}/SilverLiningDriver.cpp	
	)

INCLUDE_DIRECTORIES( 
    ${SILVERLINING_INCLUDE_DIR}
	)

if(${BUILD_SHARED_LIBS} STREQUAL "On")
    add_library(${OUT_NAME} SHARED ${SRC} ${SILVERLINING_H} ${SILVERLINING_CPP})
    ADD_DEFINITIONS(-DFESILVERLINING_LIBRARY)
else()
    add_library(${OUT_NAME} STATIC ${SRC} ${SILVERLINING_H} ${SILVERLINING_CPP})
    ADD_DEFINITIONS(-DFESILVERLINING_LIBRARY_STATIC)
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
	FeSilverliningLib
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
   FeSilverliningLib
   )

endif()

INSTALL(TARGETS ${OUT_NAME}  DESTINATION lib)
INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)
INSTALL(FILES ${SILVERLINING_H} DESTINATION include/FeSilverlining)

