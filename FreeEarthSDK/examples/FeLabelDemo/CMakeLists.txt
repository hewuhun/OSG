SET(OUT_NAME FeLabelDemo)
include_directories(${CMAKE_SOURCE_DIR}/examples/FeLabelDemo/include)
include_directories(${CMAKE_SOURCE_DIR}/examples/FreeSampleFrame/include)
include_directories(${CMAKE_SOURCE_DIR}/examples)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


SET(FREE_VIEWER_INCLUDE ${CMAKE_SOURCE_DIR}/examples/FeLabelDemo/include)
SET(FELABEL_DEMO_SRC ${CMAKE_SOURCE_DIR}/examples/FeLabelDemo/src)

#设置调试库的后缀
set(CMAKE_DEBUG_POSTFIX "d")
#if (UNIX)
#	set( CMAKE_BUILD_TYPE Debug)
#else()
#endif()

if(WIN32)
set(RES ${APPLICATION_RESOURCE_DIR}/label.res)
endif()

###########################################################################
#                                                                         #
#                       FELABEL_DEMO_H_CPP	                          #
#                                                                         #
###########################################################################
set(FELABEL_DEMO_H_CPP
	${FELABEL_DEMO_SRC}/main.cpp
	)

add_executable(
	${OUT_NAME} 
    ${FELABEL_DEMO_H_CPP}
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
