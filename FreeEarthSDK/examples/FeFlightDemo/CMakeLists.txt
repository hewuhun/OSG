SET(OUT_NAME FeFlightDemo)
include_directories(${CMAKE_SOURCE_DIR}/examples/FeFlightDemo/include)
include_directories(${CMAKE_SOURCE_DIR}/examples/FreeSampleFrame/include)
include_directories(${CMAKE_SOURCE_DIR}/examples)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
SET(FREE_VIEWER_INCLUDE ${CMAKE_SOURCE_DIR}/examples/FeFlightDemo/include)
SET(FREE_VIEWER_SRC ${CMAKE_SOURCE_DIR}/examples/FeFlightDemo/src)

#设置调试库的后缀
set(CMAKE_DEBUG_POSTFIX "d")
#if (UNIX)
#	set( CMAKE_BUILD_TYPE Debug)
#else()
#endif()

if(WIN32)
set(RES ${APPLICATION_RESOURCE_DIR}/flight.res)
endif()

###########################################################################
#                                                                         #
#                       VIEWER_H_CPP	                          #
#                                                                         #
###########################################################################
set(VIEWER_H_CPP
	${FREE_VIEWER_INCLUDE}/FlightWidget.h
	${FREE_VIEWER_INCLUDE}/FlightDemo.h

	${FREE_VIEWER_SRC}/main.cpp
	${FREE_VIEWER_SRC}/FlightWidget.cpp
	${FREE_VIEWER_SRC}/FlightDemo.cpp
	)

set(LIB_MOC_HDRS
	${FREE_VIEWER_INCLUDE}/FlightWidget.h
	${FREE_VIEWER_INCLUDE}/FlightDemo.h
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


SET(FEOCEAN_DEMO_UI
	${FREE_VIEWER_SRC}/FlightControlPan.ui
	)

if (Qt5Widgets_FOUND)
	qt5_wrap_ui(UICS ${FEOCEAN_DEMO_UI})
else()
	qt4_wrap_ui(UICS ${FEOCEAN_DEMO_UI})
endif()

SOURCE_GROUP(UI FILES ${FEOCEAN_DEMO_UI})


add_executable(
	${OUT_NAME} 
    ${VIEWER_H_CPP}
	${FEOCEAN_DEMO_UI}
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
	FeMeasure  
	FeEffects
	FeEarth
	FeAlg
	FreeSampleFrame
    FeSilverlining
	FeManager
	FeExtNode
)  

	SET_TARGET_PROPERTIES(${OUT_NAME} PROPERTIES DEBUG_OUTPUT_NAME "${OUT_NAME}${CMAKE_DEBUG_POSTFIX}")
    SET_TARGET_PROPERTIES(${OUT_NAME} PROPERTIES RELEASE_OUTPUT_NAME "${OUT_NAME}${CMAKE_RELEASE_POSTFIX}")
		       
	INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)
