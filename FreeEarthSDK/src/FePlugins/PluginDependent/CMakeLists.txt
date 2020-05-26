set(OUT_NAME fe_plugin_dependent)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/FePlugins)

set(PLUGIN_H_DIR ${CMAKE_SOURCE_DIR}/include/FePlugins/PluginDependent)
set(PLUGIN_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/FePlugins/PluginDependent)

set(HEADER 	
	${PLUGIN_H_DIR}/Export.h
	${PLUGIN_H_DIR}/Extent.h
	${PLUGIN_H_DIR}/GisService.h
	${PLUGIN_H_DIR}/HgSharpOptions.h
	${PLUGIN_H_DIR}/HgSharpTileSource.h
	)
	SOURCE_GROUP( Headers FILES ${HEADER})

set(SOURCE 	
	${PLUGIN_SOURCE_DIR}/Extent.cpp
	${PLUGIN_SOURCE_DIR}/GisService.cpp
	${PLUGIN_SOURCE_DIR}/HgSharpOptions.cpp
	${PLUGIN_SOURCE_DIR}/HgSharpTileSource.cpp
	)
	SOURCE_GROUP( Sources FILES ${SOURCE})
           
if(${BUILD_SHARED_LIBS} STREQUAL "On")
    add_library(
    	${OUT_NAME} SHARED 
    		${HEADER}
    		${SOURCE})
    ADD_DEFINITIONS(-DFEPLUGIN_LIBRARY)
else()
    add_library(
    	${OUT_NAME} STATIC 
    		${HEADER} 
    		${SOURCE})
    ADD_DEFINITIONS(-DFEPLUGIN_LIBRARY_STATIC)
endif()
											
target_link_libraries(
    ${OUT_NAME} 
    ${OPENSCENEGRAPH_LIBRARIES} 
   	debug ${osgEarth_DEBUG_LIBRARIES} optimized ${osgEarth_RELEASE_LIBRARIES} 
	debug ${osgEarthUtil_DEBUG_LIBRARIES} optimized ${osgEarthUtil_RELEASE_LIBRARIES}  
    )

INSTALL(TARGETS ${OUT_NAME}  DESTINATION lib)
INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)
