set(OUT_NAME osgdb_osgearth_flatgis)

include_directories(${CMAKE_SOURCE_DIR}/include)
set(FLAT_GIS_H_DIR ${CMAKE_SOURCE_DIR}/include/FePlugins)
set(FLAT_GIS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/FePlugins)

set(HEADER 
	${FLAT_GIS_SOURCE_DIR}/Plugin_FlatGis/FlatGisOptions.h)
set(SOURCE 
	${FLAT_GIS_SOURCE_DIR}/Plugin_FlatGis/ReaderWriterFlatGis.cpp)


if(${BUILD_SHARED_LIBS} STREQUAL "On")
    add_library(${OUT_NAME} SHARED ${SOURCE} ${HEADER})
else()
    add_library(${OUT_NAME} STATIC ${SOURCE} ${HEADER})
endif()
set_target_properties(${OUT_NAME} PROPERTIES PREFIX "")

target_link_libraries(
    ${OUT_NAME} 
    ${OPENSCENEGRAPH_LIBRARIES} 
   	debug ${osgEarth_DEBUG_LIBRARIES} optimized ${osgEarth_RELEASE_LIBRARIES} 
	debug ${osgEarthUtil_DEBUG_LIBRARIES} optimized ${osgEarthUtil_RELEASE_LIBRARIES} 
    )

INSTALL(TARGETS ${OUT_NAME}  DESTINATION lib)
INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)
