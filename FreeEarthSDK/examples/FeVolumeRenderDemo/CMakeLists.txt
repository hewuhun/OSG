SET(OUT_NAME FeVolumeRenderDemo)
include_directories(${CMAKE_SOURCE_DIR}/examples/FeVolumeRenderDemo/include)
include_directories(${CMAKE_SOURCE_DIR}/examples/FreeSampleFrame/include)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


SET(FEVOLUME_RENDER_DEMO_INCLUDE ${CMAKE_SOURCE_DIR}/examples/FeVolumeRenderDemo/include)
SET(FEVOLUME_RENDER_DEMO_SRC ${CMAKE_SOURCE_DIR}/examples/FeVolumeRenderDemo/src)

#设置调试库的后缀
set(CMAKE_DEBUG_POSTFIX "d")
#if (UNIX)
#	set( CMAKE_BUILD_TYPE Debug)
#else()
#endif()

if(WIN32)
set(RES ${APPLICATION_RESOURCE_DIR}/volumeRender.res)
endif()

###########################################################################
#                                                                         #
#                       FEVOLUME_RENDER_DEMO_H_CPP	                      #
#                                                                         #
###########################################################################
set(FEVOLUME_RENDER_DEMO_H_CPP
    ${FEVOLUME_RENDER_DEMO_INCLUDE}/VolumeRendering.h
    ${FEVOLUME_RENDER_DEMO_INCLUDE}/FeVolumeControlWidget.h

    ${FEVOLUME_RENDER_DEMO_SRC}/VolumeRendering.cpp
    ${FEVOLUME_RENDER_DEMO_SRC}/FeVolumeControlWidget.cpp
	${FEVOLUME_RENDER_DEMO_SRC}/main.cpp
	)
	
set(LIB_MOC_HDRS
    ${FEVOLUME_RENDER_DEMO_INCLUDE}/FeVolumeControlWidget.h
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


add_executable(
	${OUT_NAME} 
    ${FEVOLUME_RENDER_DEMO_H_CPP}
	${LIB_MOC_HDRS}
    ${LIB_MOC_SRCS}
  )
	target_link_libraries(
		${OUT_NAME}
		${RES}
        ${QT_LIBRARIES}
		${OPENSCENEGRAPH_LIBRARIES}
        debug ${osgEarth_DEBUG_LIBRARIES}     optimized ${osgEarth_RELEASE_LIBRARIES}
        debug ${osgEarthUtil_DEBUG_LIBRARIES} optimized ${osgEarthUtil_RELEASE_LIBRARIES}
        FeUtils
        FeKits
		FeShell 
		FeEarth
		FreeSampleFrame
	)  

	SET_TARGET_PROPERTIES(${OUT_NAME} PROPERTIES DEBUG_OUTPUT_NAME "${OUT_NAME}${CMAKE_DEBUG_POSTFIX}")
    SET_TARGET_PROPERTIES(${OUT_NAME} PROPERTIES RELEASE_OUTPUT_NAME "${OUT_NAME}${CMAKE_RELEASE_POSTFIX}")
		       
	INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)
