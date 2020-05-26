SET(OUT_NAME FeIndoorRoamingDemo)
include_directories(${CMAKE_SOURCE_DIR}/examples/FeIndoorRoamingDemo/include)
include_directories(${CMAKE_SOURCE_DIR}/examples/FreeSampleFrame/include)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
SET(FREE_VIEWER_INCLUDE ${CMAKE_SOURCE_DIR}/examples/FeIndoorRoamingDemo/include)
SET(FREE_VIEWER_SRC ${CMAKE_SOURCE_DIR}/examples/FeIndoorRoamingDemo/src)

if(WIN32)
set(RES ${APPLICATION_RESOURCE_DIR}/roaming.res)
endif()

###########################################################################
#                                                                         #
#                       VIEWER_H_CPP	                          #
#                                                                         #
###########################################################################
set(VIEWER_H_CPP
	${FREE_VIEWER_SRC}/main.cpp
	)

add_executable(
	${OUT_NAME} 
    ${VIEWER_H_CPP}
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
		       

