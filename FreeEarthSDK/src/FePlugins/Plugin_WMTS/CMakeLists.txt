SET(OUT_NAME osgdb_osgearth_wmts)

set(APPLICATION_DIR ${CMAKE_SOURCE_DIR}/src/FePlugins/Plugin_WMTS)
include_directories(${APPLICATION_DIR})

set(HEADER_FILES
	${APPLICATION_DIR}/TileSourceWMTSOptions.h
    ${APPLICATION_DIR}/ReaderWriterWMTS.h
	)

set(SOURCE_FILES
    ${APPLICATION_DIR}/ReaderWriterWMTS.cpp
	)

if(${BUILD_SHARED_LIBS} STREQUAL "On")
    add_library(
        ${OUT_NAME} SHARED 
        ${HEADER_FILES}
        ${SOURCE_FILES}
    )
else()
    add_library(
        ${OUT_NAME} STATIC 
        ${HEADER_FILES}
        ${SRC_FILES}
    )
endif()

target_link_libraries(
    ${OUT_NAME}
    ${OPENSCENEGRAPH_LIBRARIES} 
   	debug ${osgEarth_DEBUG_LIBRARIES} optimized ${osgEarth_RELEASE_LIBRARIES} 
	debug ${osgEarthUtil_DEBUG_LIBRARIES} optimized ${osgEarthUtil_RELEASE_LIBRARIES} 
)

target_link_libraries(
    ${OUT_NAME} 
    ${OPENSCENEGRAPH_LIBRARIES} 
   	debug ${osgEarth_DEBUG_LIBRARIES} optimized ${osgEarth_RELEASE_LIBRARIES} 
	debug ${osgEarthUtil_DEBUG_LIBRARIES} optimized ${osgEarthUtil_RELEASE_LIBRARIES} 
    )
	
INSTALL(TARGETS ${OUT_NAME}  DESTINATION lib)
INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)
