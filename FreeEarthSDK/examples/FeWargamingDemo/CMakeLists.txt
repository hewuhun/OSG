SET(OUT_NAME FeWargamingDemo)
include_directories(${CMAKE_SOURCE_DIR}/examples/FeWargamingDemo)
include_directories(${CMAKE_SOURCE_DIR}/examples/FreeSampleFrame/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)

SET(FREE_FLIGHT_INCLUDE ${CMAKE_SOURCE_DIR}/examples/FeWargamingDemo)
SET(FREE_FLIGHT_SRC ${CMAKE_SOURCE_DIR}/examples/FeWargamingDemo)

#设置调试库的后缀
set(CMAKE_DEBUG_POSTFIX "d")
#if (UNIX)
#	set( CMAKE_BUILD_TYPE Debug)
#else()
#endif()

if(WIN32)
set(RES ${APPLICATION_RESOURCE_DIR}/wargaming.res)
endif()

###########################################################################
#                                                                         #
#                       FLIGHT_H_CPP	                          #
#                                                                         #
###########################################################################
set(FLIGHT_H
	)

set(FLIGHT_CPP
	${FREE_FLIGHT_SRC}/main.cpp
	)
	
add_executable(
	${OUT_NAME} 
    ${FLIGHT_H}
    ${FLIGHT_CPP}
  )
	target_link_libraries(
		${OUT_NAME}
		${RES}
        ${QT_LIBRARIES}
		${OPENSCENEGRAPH_LIBRARIES}
        debug ${osgEarth_DEBUG_LIBRARIES}     optimized ${osgEarth_RELEASE_LIBRARIES}
        debug ${osgEarthUtil_DEBUG_LIBRARIES} optimized ${osgEarthUtil_RELEASE_LIBRARIES}
        FeUtils
		FeShell
		FeLayers
		FeEarth
		FreeSampleFrame
		FeManager
		FeExtNode
		FeEffects
	)  

	SET_TARGET_PROPERTIES(${OUT_NAME} PROPERTIES DEBUG_OUTPUT_NAME "${OUT_NAME}${CMAKE_DEBUG_POSTFIX}")
    SET_TARGET_PROPERTIES(${OUT_NAME} PROPERTIES RELEASE_OUTPUT_NAME "${OUT_NAME}${CMAKE_RELEASE_POSTFIX}")
		       
	INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)
