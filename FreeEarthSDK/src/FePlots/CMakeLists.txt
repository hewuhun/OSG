set(OUT_NAME FePlots)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/FePlots)

SET(PROJECT_INCLUDE ${CMAKE_SOURCE_DIR}/include/FePlots)
SET(PROJECT_SRC ${CMAKE_SOURCE_DIR}/src/FePlots)

###########################################################################
#                                                                         #
#                                 FEPLOTS   	                          #
#                                                                         #
###########################################################################
set(FEPLOTS_H
		${PROJECT_INCLUDE}/Export.h
		${PROJECT_INCLUDE}/ExternPlotsVisitor.h
		${PROJECT_INCLUDE}/PolygonPlot.h
		${PROJECT_INCLUDE}/LinePlot.h
		${PROJECT_INCLUDE}/StraightArrow.h
		${PROJECT_INCLUDE}/DovetailDiagonalArrow.h
		${PROJECT_INCLUDE}/DovetailDiagonalMoreArrow.h
		#${PROJECT_INCLUDE}/CurveFlag.h
		#${PROJECT_INCLUDE}/TriangleFlag.h
		#${PROJECT_INCLUDE}/RectFlag.h
		${PROJECT_INCLUDE}/BezierCurveArrow.h
		${PROJECT_INCLUDE}/CardinalCurveArrow.h
		${PROJECT_INCLUDE}/DiagonalArrow.h
		${PROJECT_INCLUDE}/DoubleArrow.h
		${PROJECT_INCLUDE}/DoveTailStraightArrow.h
		${PROJECT_INCLUDE}/ParallelSearch.h
		${PROJECT_INCLUDE}/PolylineArrow.h
		${PROJECT_INCLUDE}/SectorSearch.h
		${PROJECT_INCLUDE}/CloseCurve.h
		${PROJECT_INCLUDE}/DiagonalMoreArrow.h
		${PROJECT_INCLUDE}/DoveTailStraightMoreArrow.h
		#${PROJECT_INCLUDE}/Freeline.h
		#${PROJECT_INCLUDE}/FreePolygon.h
		${PROJECT_INCLUDE}/GatheringPlace.h
		${PROJECT_INCLUDE}/RoundedRect.h
		${PROJECT_INCLUDE}/StraightMoreArrow.h
		${PROJECT_INCLUDE}/StraightLineArrow.h
)

set(FEPLOTS_CPP 
		${PROJECT_SRC}/ExternPlotsVisitor.cpp
		${PROJECT_SRC}/PolygonPlot.cpp
		${PROJECT_SRC}/LinePlot.cpp
		${PROJECT_SRC}/StraightArrow.cpp
		${PROJECT_SRC}/DovetailDiagonalArrow.cpp
		${PROJECT_SRC}/DovetailDiagonalMoreArrow.cpp
		#${PROJECT_SRC}/CurveFlag.cpp
		#${PROJECT_SRC}/RectFlag.cpp
		#${PROJECT_SRC}/TriangleFlag.cpp
		${PROJECT_SRC}/BezierCurveArrow.cpp
		${PROJECT_SRC}/CardinalCurveArrow.cpp
		${PROJECT_SRC}/DiagonalArrow.cpp
		${PROJECT_SRC}/DoubleArrow.cpp
		${PROJECT_SRC}/DoveTailStraightArrow.cpp
		${PROJECT_SRC}/ParallelSearch.cpp
		${PROJECT_SRC}/PolylineArrow.cpp
		${PROJECT_SRC}/SectorSearch.cpp
		${PROJECT_SRC}/CloseCurve.cpp
		${PROJECT_SRC}/DiagonalMoreArrow.cpp
		${PROJECT_SRC}/DoveTailStraightMoreArrow.cpp
		#${PROJECT_SRC}/Freeline.cpp
		#${PROJECT_SRC}/FreePolygon.cpp
		${PROJECT_SRC}/GatheringPlace.cpp
		${PROJECT_SRC}/RoundedRect.cpp
		${PROJECT_SRC}/StraightMoreArrow.cpp
		${PROJECT_SRC}/StraightLineArrow.cpp
		)

if(${BUILD_SHARED_LIBS} STREQUAL "On")
    add_library(${OUT_NAME} SHARED ${FEPLOTS_H} ${FEPLOTS_CPP})
    ADD_DEFINITIONS(-DFEPLOTS_LIBRARY)
else()
    add_library(${OUT_NAME} STATIC ${FEPLOTS_H} ${FEPLOTS_CPP})
    ADD_DEFINITIONS(-DFEPLOTS_LIBRARY_STATIC)
endif()


target_link_libraries(
    ${OUT_NAME} 
    ${OPENSCENEGRAPH_LIBRARIES} 
    debug ${osgEarth_DEBUG_LIBRARIES} optimized ${osgEarth_RELEASE_LIBRARIES} 
    debug ${osgEarthUtil_DEBUG_LIBRARIES} optimized ${osgEarthUtil_RELEASE_LIBRARIES}
	debug ${osgEarthSymbology_DEBUG_LIBRARIES} optimized ${osgEarthSymbology_RELEASE_LIBRARIES}
	debug ${osgEarthFeatures_DEBUG_LIBRARIES} optimized ${osgEarthFeatures_RELEASE_LIBRARIES}
	debug ${osgEarthAnnotation_DEBUG_LIBRARIES} optimized ${osgEarthAnnotation_RELEASE_LIBRARIES}
	FeUtils
	FeAlg
	FeExtNode
 )

INSTALL(TARGETS ${OUT_NAME}  DESTINATION lib)
INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)
INSTALL(FILES ${FEPLOTS_H} DESTINATION include/FePlots)

