SET(OUT_NAME FeFuseDemo)
include_directories(${CMAKE_SOURCE_DIR}/examples/FreeSampleFrame/include)
include_directories(${CMAKE_SOURCE_DIR}/examples/FeFuseDemo/include)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


SET(FEFUSE_DEMO_INCLUDE ${CMAKE_SOURCE_DIR}/examples/FeFuseDemo/include)
SET(FEFUSE_DEMO_SRC ${CMAKE_SOURCE_DIR}/examples/FeFuseDemo/src)

if(WIN32)
set(RES ${APPLICATION_RESOURCE_DIR}/fuse.res)
endif()

###########################################################################
#                                                                         #
#                       FEFUSE_DEMO_H_CPP	                          #
#                                                                         #
###########################################################################
set(FEFUSE_DEMO_H_CPP
	${FEFUSE_DEMO_INCLUDE}/FreeControlPan.h


	${FEFUSE_DEMO_SRC}/main.cpp
	${FEFUSE_DEMO_SRC}/FreeControlPan.cpp

	)

SET(FEFUSE_DEMO_UI
	${FEFUSE_DEMO_INCLUDE}/FreeControlPan.ui
	)
SOURCE_GROUP(UI FILES ${FEFUSE_DEMO_UI})

if (Qt5Widgets_FOUND)
	qt5_wrap_ui(UICS ${FEFUSE_DEMO_UI})
else()
	qt4_wrap_ui(UICS ${FEFUSE_DEMO_UI})
endif()


set(LIB_MOC_HDRS
	${FEFUSE_DEMO_INCLUDE}/FreeControlPan.h
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
    ${FEFUSE_DEMO_H_CPP}
    ${FEFUSE_DEMO_UI}
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
		FeExtNode
		FreeSampleFrame
		FeManager
	)  

	SET_TARGET_PROPERTIES(${OUT_NAME} PROPERTIES DEBUG_OUTPUT_NAME "${OUT_NAME}${CMAKE_DEBUG_POSTFIX}")
    SET_TARGET_PROPERTIES(${OUT_NAME} PROPERTIES RELEASE_OUTPUT_NAME "${OUT_NAME}${CMAKE_RELEASE_POSTFIX}")

	INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)
		       

