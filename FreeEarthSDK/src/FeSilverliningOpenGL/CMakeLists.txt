set(OUT_NAME FeSilverliningOpenGL)
include_directories(${CMAKE_SOURCE_DIR}/include)

SET(PROJECT_INCLUDE ${CMAKE_SOURCE_DIR}/include/FeSilverliningOpenGL)
SET(PROJECT_SRC ${CMAKE_SOURCE_DIR}/src/FeSilverliningOpenGL)


###########################################################################
#                                                                         #
#                            SILVERLINING_OPENGL    					  #
#                                                                         #
###########################################################################
set(SILVERLINING_OPENGL_H
	${PROJECT_INCLUDE}/StdAfx.h

	${PROJECT_INCLUDE}/SilverLiningOpenGL.h
	${PROJECT_INCLUDE}/TextureLoader.h
	${PROJECT_INCLUDE}/TextureRenderer.h
	)
set(SILVERLINING_OPENGL_CPP
	${PROJECT_SRC}/StdAfx.cpp

	${PROJECT_SRC}/SilverLiningOpenGL.cpp
	${PROJECT_SRC}/TextureLoader.cpp
	${PROJECT_SRC}/TextureRenderer.cpp
	)

if(${BUILD_SHARED_LIBS} STREQUAL "On")
    add_library(${OUT_NAME} SHARED ${SRC} ${SILVERLINING_OPENGL_H} ${SILVERLINING_OPENGL_CPP})
    if (WIN32)
    ADD_DEFINITIONS(-DFESILVERLININGOPENGL_LIBRARY)
    endif()
else()
    add_library(${OUT_NAME} STATIC ${SRC} ${SILVERLINING_OPENGL_H} ${SILVERLINING_OPENGL_CPP})
    if (WIN32)
    ADD_DEFINITIONS(-DFESILVERLININGOPENGL_LIBRARY_STATIC)
    endif()
endif()

if(WIN32)
target_link_libraries(
	${OUT_NAME} 
	${OPENGL_LIBRARIES}
	FeSilverliningLib
	)
else()
target_link_libraries(
    ${OUT_NAME}
    ${OPENGL_LIBRARIES}
	FeSilverliningLib
    )

endif()

INSTALL(TARGETS ${OUT_NAME}  DESTINATION lib)
INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)

