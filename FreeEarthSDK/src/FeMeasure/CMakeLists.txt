set(OUT_NAME FeMeasure)
include_directories(${CMAKE_SOURCE_DIR}/include)

SET(PROJECT_INCLUDE ${CMAKE_SOURCE_DIR}/include/FeMeasure)
SET(PROJECT_SRC ${CMAKE_SOURCE_DIR}/src/FeMeasure)

###########################################################################
#                                                                         #
#                             FEMEASURE  	                              #
#                                                                         #
###########################################################################
set(FEMEASURE_H 
	${PROJECT_INCLUDE}/Export.h
	
	${PROJECT_INCLUDE}/MeasureSys.h

	${PROJECT_INCLUDE}/Measure.h
	${PROJECT_INCLUDE}/DistMeasure.h
	${PROJECT_INCLUDE}/SurfDistMeasure.h
	${PROJECT_INCLUDE}/SpaceDistMeasure.h
	${PROJECT_INCLUDE}/HeiDistMeasure.h
	${PROJECT_INCLUDE}/SpaceAreaMeasure.h
	${PROJECT_INCLUDE}/ShadowAreaMeasure.h
	${PROJECT_INCLUDE}/RadialLineVisibleMeasure.h
	
	
	${PROJECT_INCLUDE}/ProfileAnalysisMeasure.h
	${PROJECT_INCLUDE}/FillCutAnalysisMeasure.h
	${PROJECT_INCLUDE}/FloodAnalysisMeasure.h
	${PROJECT_INCLUDE}/FloodSurface.h

	${PROJECT_INCLUDE}/BuildingsLinesMeasure.h
	${PROJECT_INCLUDE}/BillBoardDisplay.h
	${PROJECT_INCLUDE}/GradientAnalysisMeasure.h
	${PROJECT_INCLUDE}/GradientBillBoardDisplay.h
	${PROJECT_INCLUDE}/GradientMapEffect.h
	${PROJECT_INCLUDE}/RadialLineOfSightNode.h
	${PROJECT_INCLUDE}/DPLineSegmentIntersector.h
	${PROJECT_INCLUDE}/BuildingsViewMeasure.h
	${PROJECT_INCLUDE}/ViewAnalyseNode.h

	${PROJECT_INCLUDE}/MeasureFactory.h
	${PROJECT_INCLUDE}/ContourMeasure.h
    )
           
set(FEMEASURE_CPP 
	${PROJECT_SRC}/MeasureSys.cpp
	${PROJECT_SRC}/Measure.cpp
	${PROJECT_SRC}/DistMeasure.cpp
	${PROJECT_SRC}/SurfDistMeasure.cpp
	${PROJECT_SRC}/SpaceDistMeasure.cpp
	${PROJECT_SRC}/RadialLineVisibleMeasure.cpp
	${PROJECT_SRC}/HeiDistMeasure.cpp
	${PROJECT_SRC}/SpaceAreaMeasure.cpp
	${PROJECT_SRC}/ShadowAreaMeasure.cpp
	${PROJECT_SRC}/ProfileAnalysisMeasure.cpp
	${PROJECT_SRC}/FillCutAnalysisMeasure.cpp
	${PROJECT_SRC}/FloodAnalysisMeasure.cpp
	${PROJECT_SRC}/FloodSurface.cpp

	${PROJECT_SRC}/BuildingsLinesMeasure.cpp
	${PROJECT_SRC}/BillboardDisplay.cpp
	${PROJECT_SRC}/GradientAnalysisMeasure.cpp
	${PROJECT_SRC}/GradientBillboardDisplay.cpp
	${PROJECT_SRC}/GradientMapEffect.cpp
	${PROJECT_SRC}/RadialLineOfSightNode.cpp
	${PROJECT_SRC}/DPLineSegmentIntersector.cpp
	${PROJECT_SRC}/BuildingsViewMeasure.cpp
	${PROJECT_SRC}/ViewAnalyseNode.cpp

	${PROJECT_SRC}/MeasureFactory.cpp
	${PROJECT_SRC}/ContourMeasure.cpp
    )

set(LIB_MOC_HDRS
	${PROJECT_INCLUDE}/ContourMeasure.h
    )

if(${BUILD_SHARED_LIBS} STREQUAL "On")
    add_library(${OUT_NAME} SHARED ${SRC} ${FEMEASURE_H} ${FEMEASURE_CPP})
    ADD_DEFINITIONS(-DFEMEASURE_LIBRARY)
else()
    add_library(${OUT_NAME} STATIC ${SRC} ${FEMEASURE_H} ${FEMEASURE_CPP})
    ADD_DEFINITIONS(-DFEMEASURE_LIBRARY_STATIC)
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
 )

INSTALL(TARGETS ${OUT_NAME}  DESTINATION lib)
INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)

INSTALL(FILES ${FEMEASURE_H} DESTINATION include/FeMeasure)

