set(OUT_NAME FeUtils)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/FeUtils)

SET(FE_NODE_INCLUDE ${CMAKE_SOURCE_DIR}/include/FeUtils)
SET(FE_NODE_SRC ${CMAKE_SOURCE_DIR}/src/FeUtils)

if (WIN32)
    set(mplib opengl32)
else()
    set(mplib osgdb_osgearth_engine_mp)
endif()

OPTION(RELEASE_VERSION "this version is release" OFF)
IF   (RELEASE_VERSION)
    add_definitions(-DRELEASE_ON)
ENDIF(RELEASE_VERSION)

###########################################################################
#                                                                         #
#                            COMMON    									  #
#                                                                         #
###########################################################################
set(COMMON_H
        ${FE_NODE_INCLUDE}/Export.h

		${FE_NODE_INCLUDE}/StrUtil.h 
		${FE_NODE_INCLUDE}/CoordConverter.h
		${FE_NODE_INCLUDE}/UtilityGeom.h
		${FE_NODE_INCLUDE}/RenderContext.h
		${FE_NODE_INCLUDE}/TimeUtil.h
		${FE_NODE_INCLUDE}/MathUtil.h
		${FE_NODE_INCLUDE}/IntersectUtil.h
		${FE_NODE_INCLUDE}/NodeUtils.h
		${FE_NODE_INCLUDE}/PathRegistry.h
		${FE_NODE_INCLUDE}/MiniAnimation.h
		${FE_NODE_INCLUDE}/MiniAnimationBuilder.h
		${FE_NODE_INCLUDE}/FreeViewPoint.h
		${FE_NODE_INCLUDE}/EnvironmentVariableReader.h
		${FE_NODE_INCLUDE}/SceneSysCall.h
		${FE_NODE_INCLUDE}/GeneratorID.h
		${FE_NODE_INCLUDE}/EventListener.h
		${FE_NODE_INCLUDE}/MiniAnimationCtrl.h
		${FE_NODE_INCLUDE}/DrawTexture.h
		${FE_NODE_INCLUDE}/SystemInfo.h
		${FE_NODE_INCLUDE}/ShpAndKMLTypeParser.h
		${FE_NODE_INCLUDE}/FlightMotionAlgorithm.h
		${FE_NODE_INCLUDE}/TerrainMask.h
		
	)

	set(COMMON_CPP
        ${FE_NODE_SRC}/StrUtil.cpp
		${FE_NODE_SRC}/RenderContext.cpp
		${FE_NODE_SRC}/CoordConverter.cpp
		${FE_NODE_SRC}/UtilityGeom.cpp
		${FE_NODE_SRC}/PathRegistry.cpp
		${FE_NODE_SRC}/MathUtil.cpp
		${FE_NODE_SRC}/IntersectUtil.cpp
		${FE_NODE_SRC}/TimeUtil.cpp
		${FE_NODE_SRC}/NodeUtils.cpp
		${FE_NODE_SRC}/FreeViewPoint.cpp
		${FE_NODE_SRC}/MiniAnimation.cpp
		${FE_NODE_SRC}/MiniAnimationBuilder.cpp
		${FE_NODE_SRC}/EnvironmentVariableReader.cpp
		${FE_NODE_SRC}/SceneSysCall.cpp
		${FE_NODE_SRC}/GeneratorID.cpp
		${FE_NODE_SRC}/EventListener.cpp
		${FE_NODE_SRC}/MiniAnimationCtrl.cpp
		${FE_NODE_SRC}/DrawTexture.cpp
		${FE_NODE_SRC}/SystemInfo.cpp
		${FE_NODE_SRC}/ShpAndKMLTypeParser.cpp
		${FE_NODE_SRC}/FlightMotionAlgorithm.cpp
		${FE_NODE_SRC}/TerrainMask.cpp
	)
SOURCE_GROUP(Common FILES ${COMMON_H} ${COMMON_CPP})


###########################################################################
#                                                                         #
#                            TINYXML  									  #
#                                                                         #
###########################################################################
set(TINYXML_H
	${FE_NODE_INCLUDE}/tinyXml/tinystr.h
	${FE_NODE_INCLUDE}/tinyXml/tinyxml.h
	${FE_NODE_INCLUDE}/tinyXml/tinyutil.h
	)

set(TINYXML_CPP 
	${FE_NODE_SRC}/tinyXml/tinystr.cpp
	${FE_NODE_SRC}/tinyXml/tinyxml.cpp
	${FE_NODE_SRC}/tinyXml/tinyutil.cpp
	${FE_NODE_SRC}/tinyXml/tinyxmlerror.cpp
	${FE_NODE_SRC}/tinyXml/tinyxmlparser.cpp
	)
SOURCE_GROUP( TinyXml FILES ${TINYXML_H} ${TINYXML_CPP})

###########################################################################
#                                                                         #
#                            LOGGER  									  #
#                                                                         #
###########################################################################
set(LOGGER_H  
	${FE_NODE_INCLUDE}/logger/Singleton.h
	${FE_NODE_INCLUDE}/logger/Logger.h
	${FE_NODE_INCLUDE}/logger/LoggerDef.h
	#${FE_NODE_INCLUDE}/logger/ConsoleAdapter.h
	)
	
set(LOGGER_CPP  
	${FE_NODE_SRC}/logger/Logger.cpp
	)
	
SOURCE_GROUP( Logger FILES ${LOGGER_H} ${LOGGER_CPP})


if(${BUILD_SHARED_LIBS} STREQUAL "On")
    add_library(
	${OUT_NAME} SHARED 
	${COMMON_H} 
	${COMMON_CPP} 
	${TINYXML_H}
	${TINYXML_CPP}
	${LOGGER_H}
	${LOGGER_CPP}
	)
    ADD_DEFINITIONS(-DFEUTILS_LIBRARY)
else()
    add_library(
	${OUT_NAME} STATIC 
	${COMMON_H} 
	${COMMON_CPP} 
	${TINYXML_H}
	${TINYXML_CPP}
	${LOGGER_H}
	${LOGGER_CPP}
	)
    ADD_DEFINITIONS(-DFEUTILS_LIBRARY_STATIC)
endif()

find_package(GDAL)
target_link_libraries(
    ${OUT_NAME} 
    ${OPENSCENEGRAPH_LIBRARIES} 
    ${mplib}
	debug ${log4Cpp_DEBUG_LIBRARIES} optimized ${log4Cpp_RELEASE_LIBRARIES}
    debug ${osgEarth_DEBUG_LIBRARIES} optimized ${osgEarth_RELEASE_LIBRARIES} 
    debug ${osgEarthUtil_DEBUG_LIBRARIES} optimized ${osgEarthUtil_RELEASE_LIBRARIES}
	${GDAL_LIBRARIES}
 )

INSTALL(TARGETS ${OUT_NAME}  DESTINATION lib)
INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)

INSTALL(FILES ${COMMON_H} DESTINATION include/FeUtils)
INSTALL(FILES ${TINYXML_H} DESTINATION include/FeUtils/tinyXml)
INSTALL(FILES ${LOGGER_H} DESTINATION include/FeUtils/logger)
