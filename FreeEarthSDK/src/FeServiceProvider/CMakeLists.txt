set(OUT_NAME FeServiceProvider)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/FeServiceProvider)

SET(PROJECT_INCLUDE ${CMAKE_SOURCE_DIR}/include/FeServiceProvider)
SET(PROJECT_SRC ${CMAKE_SOURCE_DIR}/src/FeServiceProvider)

find_package(CURL)
include_directories(${CURL_INCLUDE_DIR})
set(FEEARTH_H 
	${PROJECT_INCLUDE}/Export.h
	${PROJECT_INCLUDE}/ServiceManager.h
	${PROJECT_INCLUDE}/ServiceProvider.h
	${PROJECT_INCLUDE}/TMSServiceProvider.h
	${PROJECT_INCLUDE}/WCSServiceProvider.h
	${PROJECT_INCLUDE}/WFSServiceProvider.h
	${PROJECT_INCLUDE}/WMSServiceProvider.h
	${PROJECT_INCLUDE}/WMTSServiceProvider.h
	${PROJECT_INCLUDE}/ServiceLayer.h
	${PROJECT_INCLUDE}/XYZServiceProvider.h
	${PROJECT_INCLUDE}/DataServiceProvider.h
	${PROJECT_INCLUDE}/CurlHelper.h
)

set(FEEARTH_CPP
	${PROJECT_SRC}/ServiceProvider.cpp
	${PROJECT_SRC}/TMSServiceProvider.cpp
	${PROJECT_SRC}/WCSServiceProvider.cpp
	${PROJECT_SRC}/WFSServiceProvider.cpp
	${PROJECT_SRC}/WMSServiceProvider.cpp
	${PROJECT_SRC}/WMTSServiceProvider.cpp
	${PROJECT_SRC}/ServiceLayer.cpp
	${PROJECT_SRC}/ServiceManager.cpp
	${PROJECT_SRC}/XYZServiceProvider.cpp
	${PROJECT_SRC}/DataServiceProvider.cpp
	${PROJECT_SRC}/CurlHelper.cpp
)


if(${BUILD_SHARED_LIBS} STREQUAL "On")
    add_library(
    	${OUT_NAME} SHARED 
    	${FEEARTH_H}
    	${FEEARTH_CPP}
		)
    ADD_DEFINITIONS(-DFESERVICEPROVIDER_LIBRARY)
else()
    add_library(
    	${OUT_NAME} STATIC 
    	${FEEARTH_H}
    	${FEEARTH_CPP}
    	)
    ADD_DEFINITIONS(-DFESERVICEPROVIDER_LIBRARY_STATIC)
endif()
			

target_link_libraries(
	${OUT_NAME} 
	FeUtils
	${CURL_LIBRARIES}
	)



INSTALL(TARGETS ${OUT_NAME}  DESTINATION lib)
INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)
INSTALL(FILES ${FEEARTH_H} DESTINATION include/FeServiceProvider)

