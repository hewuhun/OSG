set(OUT_NAME FeShell)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/FeShell)

SET(PROJECT_INCLUDE ${CMAKE_SOURCE_DIR}/include/FeShell)
SET(PROJECT_SRC ${CMAKE_SOURCE_DIR}/src/FeShell)

###########################################################################
#                                                                         #
#                            FESHELL                 					  #
#                                                                         #
###########################################################################
set(FESHELL_H 
	${PROJECT_INCLUDE}/Export.h
	
	${PROJECT_INCLUDE}/SystemEventHandler.h
	${PROJECT_INCLUDE}/SmartToolSys.h
	${PROJECT_INCLUDE}/EnvironmentSys.h
	${PROJECT_INCLUDE}/SystemSerializer.h
	${PROJECT_INCLUDE}/SystemManager.h
	${PROJECT_INCLUDE}/SystemService.h
	${PROJECT_INCLUDE}/LanSerializer.h
	${PROJECT_INCLUDE}/EnvironmentSerializer.h
	${PROJECT_INCLUDE}/ModelConfigSerialize.h
)
           
set(FESHELL_CPP 
	${PROJECT_SRC}/SmartToolSys.cpp
	${PROJECT_SRC}/EnvironmentSys.cpp
	${PROJECT_SRC}/SystemSerializer.cpp
	${PROJECT_SRC}/SystemManager.cpp
	${PROJECT_SRC}/SystemService.cpp
	${PROJECT_SRC}/LanSerializer.cpp
	${PROJECT_SRC}/EnvironmentSerializer.cpp
	${PROJECT_SRC}/ModelConfigSerialize.cpp
)

if(${BUILD_SHARED_LIBS} STREQUAL "On")
    add_library(
    	${OUT_NAME} SHARED 
    	${FESHELL_H}
    	${FESHELL_CPP}
		)
    ADD_DEFINITIONS(-DSHELL_LIBRARY)
else()
    add_library(
    	${OUT_NAME} STATIC 
    	${FESHELL_H}
    	${FESHELL_CPP}
		)
    ADD_DEFINITIONS(-DSHELL_LIBRARY_STATIC)
endif()
											
target_link_libraries(
    ${OUT_NAME} 
    ${OPENSCENEGRAPH_LIBRARIES} 
   	debug ${osgEarth_DEBUG_LIBRARIES} optimized ${osgEarth_RELEASE_LIBRARIES} 
	debug ${osgEarthUtil_DEBUG_LIBRARIES} optimized ${osgEarthUtil_RELEASE_LIBRARIES}  
	FeKits
	FeUtils
	FeOcean
	FeSilverlining
    )

INSTALL(TARGETS ${OUT_NAME}  DESTINATION lib)
INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)
INSTALL(FILES ${FESHELL_H} DESTINATION include/FeShell)

