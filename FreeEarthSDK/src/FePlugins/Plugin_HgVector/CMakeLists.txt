set(OUT_NAME osgdb_osgearth_hgvector)

include_directories(${3RD_PARTY_DIR}/include)
include_directories(${JPEG_INCLUDE_DIR})
include_directories(${PNG_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src/FePlugins/Plugin_HgVector)
set(GOOGLE_H_DIR ${CMAKE_SOURCE_DIR}/include/FePlugins)
set(GOOGLE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/FePlugins)

set(HEADER 
	${GOOGLE_SOURCE_DIR}/Plugin_HgVector/HgVectorImageDataSource.h
	)
set(SOURCE 
	${GOOGLE_SOURCE_DIR}/Plugin_HgVector/HgVectorImageDataSource.cpp
	${GOOGLE_SOURCE_DIR}/Plugin_HgVector/ReaderWriterHgVector.cpp
	${GOOGLE_SOURCE_DIR}/Plugin_HgVector/ImageDecode.h
	${GOOGLE_SOURCE_DIR}/Plugin_HgVector/ImageDecode.cpp
	)


if(${BUILD_SHARED_LIBS} STREQUAL "On")
    add_library(${OUT_NAME} SHARED ${SOURCE} ${HEADER})
else()
    add_library(${OUT_NAME} STATIC ${SOURCE} ${HEADER})
endif()
set_target_properties(${OUT_NAME} PROPERTIES PREFIX "")

target_link_libraries(
    ${OUT_NAME} 
    ${OPENSCENEGRAPH_LIBRARIES} 
	${JPEG_LIBRARY}
	${CURL_LIBRARY}
	${ZLIB_LIBRARY}
	${PNG_LIBRARY}
   	debug ${osgEarth_DEBUG_LIBRARIES} optimized ${osgEarth_RELEASE_LIBRARIES} 
	debug ${osgEarthUtil_DEBUG_LIBRARIES} optimized ${osgEarthUtil_RELEASE_LIBRARIES} 

    )

INSTALL(TARGETS ${OUT_NAME}  DESTINATION lib)
INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)
