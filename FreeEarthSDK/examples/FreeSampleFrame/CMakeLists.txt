SET(OUT_NAME FreeSampleFrame)
include_directories(${CMAKE_SOURCE_DIR}/examples/FreeSampleFrame/include)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

SET(FREE_SAMPLE_FRAME_INCLUDE ${CMAKE_SOURCE_DIR}/examples/FreeSampleFrame/include)
SET(FREE_SAMPLE_FRAME_SRC ${CMAKE_SOURCE_DIR}/examples/FreeSampleFrame/src)
SET(FREE_SAMPLE_FRAME_RESOURCE_DIR ${CMAKE_SOURCE_DIR}/examples/FreeSampleFrame/resource)

###########################################################################
#                                                                         #
#                       FREE_SAMPLE_FRAME_H_CPP	                          #
#                                                                         #
###########################################################################
set(FREE_SAMPLE_FRAME_H_CPP
	${FREE_SAMPLE_FRAME_INCLUDE}/Export.h
	${FREE_SAMPLE_FRAME_INCLUDE}/FreeStyleSheet.h
	${FREE_SAMPLE_FRAME_INCLUDE}/FreeMainWindow.h
	${FREE_SAMPLE_FRAME_INCLUDE}/FreeControlWidget.h
	
	${FREE_SAMPLE_FRAME_SRC}/FreeStyleSheet.cpp
	${FREE_SAMPLE_FRAME_SRC}/FreeMainWindow.cpp
	${FREE_SAMPLE_FRAME_SRC}/FreeControlWidget.cpp
	#${FREE_SAMPLE_FRAME_SRC}/main.cpp
	)

SET(FREE_SAMPLE_FRAME_RESOURCE
	${FREE_SAMPLE_FRAME_RESOURCE_DIR}/images.qrc
	)
SOURCE_GROUP(Resource FILES ${FREE_SAMPLE_FRAME_RESOURCE})

if (Qt5Widgets_FOUND)
	QT5_ADD_RESOURCES(FREE_SAMPLE_FRAME_MOCS_RESOURCE
	${FREE_SAMPLE_FRAME_RESOURCE}
	)
else()
	QT4_ADD_RESOURCES(FREE_SAMPLE_FRAME_MOCS_RESOURCE
	${FREE_SAMPLE_FRAME_RESOURCE}
	)
endif()

set(FREE_SAMPLE_FRAME_UI
	)
SOURCE_GROUP(UI FILES ${FREE_SAMPLE_FRAME_UI})
	
if (Qt5Widgets_FOUND)
	qt5_wrap_ui(UICS ${FREE_SAMPLE_FRAME_UI})
else()
	qt4_wrap_ui(UICS ${FREE_SAMPLE_FRAME_UI})
endif()


set(LIB_MOC_HDRS
	${FREE_SAMPLE_FRAME_INCLUDE}/FreeMainWindow.h
	${FREE_SAMPLE_FRAME_INCLUDE}/FreeControlWidget.h
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
		${OUT_NAME} 
		${FREE_SAMPLE_FRAME_H_CPP}
		${FREE_SAMPLE_FRAME_UI}
		${FREE_SAMPLE_FRAME_RESOURCE}
		${FREE_SAMPLE_FRAME_MOCS_RESOURCE}
		${LIB_MOC_HDRS}
		${LIB_MOC_SRCS}
		)
    ADD_DEFINITIONS(-DEARTH_LIBRARY)
else()
    add_library(
		${OUT_NAME} 
		${FREE_SAMPLE_FRAME_H_CPP}
		${FREE_SAMPLE_FRAME_UI}
		${FREE_SAMPLE_FRAME_RESOURCE}
		${FREE_SAMPLE_FRAME_MOCS_RESOURCE}
		${LIB_MOC_HDRS}
		${LIB_MOC_SRCS}
    	)
    ADD_DEFINITIONS(-DEARTH_LIBRARY_STATIC)
endif()

target_link_libraries(
	${OUT_NAME}
    ${QT_LIBRARIES}
)  

SET_TARGET_PROPERTIES(${OUT_NAME} PROPERTIES DEBUG_OUTPUT_NAME "${OUT_NAME}${CMAKE_DEBUG_POSTFIX}")
SET_TARGET_PROPERTIES(${OUT_NAME} PROPERTIES RELEASE_OUTPUT_NAME "${OUT_NAME}${CMAKE_RELEASE_POSTFIX}")
	       
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

INSTALL(FILES ${FREE_SAMPLE_FRAME_H_CPP} DESTINATION include/FreeSampleFrame)