project(FeLayers)
set(OUT_NAME FeLayers)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/FeLayers)
include_directories(${CMAKE_SOURCE_DIR}/include/FeLayers/shaders)

SET(PROJECT_INCLUDE ${CMAKE_SOURCE_DIR}/include/FeLayers)
SET(PROJECT_SRC ${CMAKE_SOURCE_DIR}/src/FeLayers)

###########################################################################
#                                                                         #
#                          生成内置的Shader的方法  	                      #
#                                                                         #
###########################################################################
SET(CMAKE_SHADERS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/include/FeLayers/shaders)

set(SHADERS_CPP
    "${CMAKE_CURRENT_BINARY_DIR}/AutoGenShaders.cpp")

set(TARGET_GLSL
    NormalMapEffect.vert.glsl
    NormalMapEffect.frag.glsl
    )

set(TARGET_IN
    ${CMAKE_SHADERS_SOURCE_DIR}/Shaders.cpp.in)

configure_shaders(
    Shaders.cpp.in
    ${SHADERS_CPP}
    ${TARGET_GLSL}
    )

SOURCE_GROUP("Shader Files" FILES ${TARGET_GLSL})
SOURCE_GROUP("Template Files" FILES ${TARGET_IN})

###########################################################################
#                                                                         #
#                                 FELAYER   	                          #
#                                                                         #
###########################################################################
set(FELAYER_H
    ${PROJECT_INCLUDE}/Export.h

    ${PROJECT_INCLUDE}/Shaders.h

    ${PROJECT_INCLUDE}/LayerSys.h
    ${PROJECT_INCLUDE}/FeEffectLayer.h
    ${PROJECT_INCLUDE}/NormalMapEffect.h

    ${PROJECT_INCLUDE}/FeatureReaderWriter.h
    ${PROJECT_INCLUDE}/CustomLayerReaderWriter.h
    #${PROJECT_INCLUDE}/ContourReaderWriter.h
    ${PROJECT_INCLUDE}/STFColorFilter.h
    
    ${PROJECT_INCLUDE}/IFeLayer.h
    ${PROJECT_INCLUDE}/FeLayerConfig.h
    ${PROJECT_INCLUDE}/IFeLayerFactory.h
    ${PROJECT_INCLUDE}/FeLayerImage.h
    ${PROJECT_INCLUDE}/FeLayerModel.h
    ${PROJECT_INCLUDE}/FeLayerElevation.h
    ${PROJECT_INCLUDE}/FeLayerSerialize.h
    ${PROJECT_INCLUDE}/FeLayerValid.h
    
)

set(FELAYER_H_INSTALL
    ${PROJECT_INCLUDE}/Export.h
    ${PROJECT_INCLUDE}/Shaders.h
    ${PROJECT_INCLUDE}/LayerSys.h
    ${PROJECT_INCLUDE}/FeEffectLayer.h
    ${PROJECT_INCLUDE}/NormalMapEffect.h
    ${PROJECT_INCLUDE}/FeatureReaderWriter.h
    ${PROJECT_INCLUDE}/CustomLayerReaderWriter.h
    ${PROJECT_INCLUDE}/STFColorFilter.h
    
    ${PROJECT_INCLUDE}/IFeLayer.h
    ${PROJECT_INCLUDE}/FeLayerConfig.h
    ${PROJECT_INCLUDE}/IFeLayerFactory.h
)

set(FELAYER_CPP
    ${PROJECT_SRC}/LayerSys.cpp
    ${PROJECT_SRC}/FeEffectLayer.cpp
    ${PROJECT_SRC}/NormalMapEffect.cpp

    ${PROJECT_SRC}/FeatureReaderWriter.cpp
    ${PROJECT_SRC}/CustomLayerReaderWriter.cpp
    #${PROJECT_SRC}/ContourReaderWriter.cpp
    ${PROJECT_SRC}/STFColorFilter.cpp
    
    ${PROJECT_SRC}/FeLayerConfig.cpp
    ${PROJECT_SRC}/FeLayerFactory.cpp
    ${PROJECT_SRC}/FeLayerImage.cpp
    ${PROJECT_SRC}/FeLayerModel.cpp
    ${PROJECT_SRC}/FeLayerElevation.cpp
    ${PROJECT_SRC}/FeLayerSerialize.cpp
    ${PROJECT_SRC}/FeLayerValid.cpp

    ${SHADERS_CPP}
)

if(${BUILD_SHARED_LIBS} STREQUAL "On")
    add_library(
        ${OUT_NAME} SHARED
        ${FELAYER_H}
        ${FELAYER_CPP}
        #${TARGET_GLSL}
        #${TARGET_IN}
        )
    ADD_DEFINITIONS(-DSHELL_LIBRARY)
else()
    add_library(
        ${OUT_NAME} STATIC
        ${FELAYER_H}
        ${FELAYER_CPP}
        #${TARGET_GLSL}
        #${TARGET_IN}
		)
    ADD_DEFINITIONS(-DSHELL_LIBRARY_STATIC)
endif()

target_link_libraries(
    ${OUT_NAME}
    ${OPENSCENEGRAPH_LIBRARIES}
    debug ${osgEarth_DEBUG_LIBRARIES} optimized ${osgEarth_RELEASE_LIBRARIES}
    debug ${osgEarthUtil_DEBUG_LIBRARIES} optimized ${osgEarthUtil_RELEASE_LIBRARIES}
    debug ${osgEarthSymbology_DEBUG_LIBRARIES} optimized ${osgEarthSymbology_RELEASE_LIBRARIES}
    debug ${osgEarthFeatures_DEBUG_LIBRARIES} optimized ${osgEarthFeatures_RELEASE_LIBRARIES}
    FeUtils
    )

INSTALL(TARGETS ${OUT_NAME}  DESTINATION lib)
INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)

INSTALL(FILES ${FELAYER_H} DESTINATION include/FeLayers)

