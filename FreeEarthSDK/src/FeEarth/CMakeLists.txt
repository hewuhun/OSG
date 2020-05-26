set(OUT_NAME FeEarth)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/FeEarth)

SET(PROJECT_INCLUDE ${CMAKE_SOURCE_DIR}/include/FeEarth)
SET(PROJECT_SRC ${CMAKE_SOURCE_DIR}/src/FeEarth)

###########################################################################
#                                                                         #
#                                 FEEARTH   	                          #
#                                                                         #
###########################################################################
set(FEEARTH_H 
	${PROJECT_INCLUDE}/Export.h
	${PROJECT_INCLUDE}/FreeEarth.h
	${PROJECT_INCLUDE}/3DSceneWidget.h
)

set(FEEARTH_CPP
	${PROJECT_SRC}/FreeEarth.cpp
	${PROJECT_SRC}/3DSceneWidget.cpp
)

set(LIB_MOC_HDRS
	${PROJECT_INCLUDE}/3DSceneWidget.h
	)

IF(Qt5Widgets_FOUND)
    SET(LIB_MOC_SRCS)
    FOREACH( LIB_MOC_HDR ${LIB_MOC_HDRS} )
        GET_FILENAME_COMPONENT( LIB_MOC_HDR_ABS ${LIB_MOC_HDR} ABSOLUTE )
        QT5_WRAP_CPP( LIB_MOC_SRC ${LIB_MOC_HDR} OPTIONS "-f${LIB_MOC_HDR_ABS}" )
        LIST( APPEND LIB_MOC_SRCS ${LIB_MOC_SRC} )
    ENDFOREACH()
ELSE()
    INCLUDE( ${QT_USE_FILE} )
    QT4_WRAP_CPP( LIB_MOC_SRCS ${LIB_MOC_HDRS} OPTIONS "-f" )
ENDIF()
           
if(${BUILD_SHARED_LIBS} STREQUAL "On")
    add_library(
    	${OUT_NAME} SHARED 
    	${FEEARTH_H}
    	${FEEARTH_CPP}
		${LIB_MOC_HDRS}
		${LIB_MOC_SRCS}
		)
    ADD_DEFINITIONS(-DEARTH_LIBRARY)
else()
    add_library(
    	${OUT_NAME} STATIC 
    	${FEEARTH_H}
    	${FEEARTH_CPP}
    	)
    ADD_DEFINITIONS(-DEARTH_LIBRARY_STATIC)
endif()
			
IF(Qt5Widgets_FOUND)	
	target_link_libraries(
		${OUT_NAME} 
		${Qt5Core_LIBRARIES}  ${Qt5Widgets_LIBRARIES}  ${Qt5OpenGL_LIBRARIES}
		${OPENSCENEGRAPH_LIBRARIES} 
		  debug ${osgEarth_DEBUG_LIBRARIES} optimized ${osgEarth_RELEASE_LIBRARIES} 
		debug ${osgEarthUtil_DEBUG_LIBRARIES} optimized ${osgEarthUtil_RELEASE_LIBRARIES}  
		FeKits
		FeUtils
		FeShell 
		)							
ELSE()
	target_link_libraries(
		${OUT_NAME} 
		${QT_LIBRARIES}
		${OPENSCENEGRAPH_LIBRARIES} 
		  debug ${osgEarth_DEBUG_LIBRARIES} optimized ${osgEarth_RELEASE_LIBRARIES} 
		debug ${osgEarthUtil_DEBUG_LIBRARIES} optimized ${osgEarthUtil_RELEASE_LIBRARIES}  
		FeKits
		FeUtils
		FeShell 
		)
ENDIF()

INSTALL(TARGETS ${OUT_NAME}  DESTINATION lib)
INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)
INSTALL(FILES ${FEEARTH_H} DESTINATION include/FeEarth)

