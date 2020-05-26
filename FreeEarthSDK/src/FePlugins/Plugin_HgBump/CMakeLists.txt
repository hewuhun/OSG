set(OUT_NAME osgdb_osgearth_hgbump)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/FePlugins)
set(HG_BUMP_H_DIR ${CMAKE_SOURCE_DIR}/include/FePlugins)
set(HG_BUMP_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/FePlugins)

set(HEADER 
	)
set(SOURCE 
	${HG_BUMP_SOURCE_DIR}/Plugin_HgBump/ReaderWriterHgBump.cpp
	)

if(${BUILD_SHARED_LIBS} STREQUAL "On")
    add_library(${OUT_NAME} SHARED ${HEADER} ${SOURCE})
else()
    add_library(${OUT_NAME} STATIC ${HEADER} ${SOURCE})
endif()
set_target_properties(${OUT_NAME} PROPERTIES PREFIX "")

target_link_libraries(
    ${OUT_NAME} 
    ${OPENSCENEGRAPH_LIBRARIES} 
   	debug ${osgEarth_DEBUG_LIBRARIES} optimized ${osgEarth_RELEASE_LIBRARIES} 
	fe_plugin_dependent
    )


INSTALL(TARGETS ${OUT_NAME}  DESTINATION lib)
INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)
