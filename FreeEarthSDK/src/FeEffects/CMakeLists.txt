set(OUT_NAME FeEffects)
include_directories(${CMAKE_SOURCE_DIR}/include)

SET(PROJECT_INCLUDE ${CMAKE_SOURCE_DIR}/include/FeEffects)
SET(PROJECT_SRC ${CMAKE_SOURCE_DIR}/src/FeEffects)

###########################################################################
#                                                                         #
#                                 FEEFFECT   	                          #
#                                                                         #
###########################################################################
set(FEEFFECT_H 
	${PROJECT_INCLUDE}/Export.h
	${PROJECT_INCLUDE}/FlameGeom.h
	${PROJECT_INCLUDE}/MutableCylinder.h
    ${PROJECT_INCLUDE}/SphereSegment.h
    ${PROJECT_INCLUDE}/RadarEffect.h
	${PROJECT_INCLUDE}/Corridor.h
	${PROJECT_INCLUDE}/explosion.h
	${PROJECT_INCLUDE}/UserExplosion.h
	${PROJECT_INCLUDE}/WakeFlame.h
	${PROJECT_INCLUDE}/FreeEffect.h
	${PROJECT_INCLUDE}/LineConnect.h
	${PROJECT_INCLUDE}/RadarWaveEffect.h
	${PROJECT_INCLUDE}/WakeRibbonEffect.h
	${PROJECT_INCLUDE}/HistoryTrackEffect.h
	${PROJECT_INCLUDE}/HighlightEffect.h
	${PROJECT_INCLUDE}/AnimationEffect.h
	${PROJECT_INCLUDE}/ExplosionEffect.h
	${PROJECT_INCLUDE}/WakeSmokeEffect.h
	${PROJECT_INCLUDE}/WakeFlameEffect.h
	${PROJECT_INCLUDE}/ScanRadarEffect.h
	${PROJECT_INCLUDE}/FixedStaringScanEffect.h
	${PROJECT_INCLUDE}/FixedScanEffect.h
	${PROJECT_INCLUDE}/ComplexConicEffect.h
    ${PROJECT_INCLUDE}/ConeFixedScanEffect.h    
	${PROJECT_INCLUDE}/EntityTrackEffect.h   	
    ${PROJECT_INCLUDE}/SAREffect.h
	${PROJECT_INCLUDE}/StripeEffect.h
	${PROJECT_INCLUDE}/GlareDef.h
	${PROJECT_INCLUDE}/HDRLighting.h
	${PROJECT_INCLUDE}/Scan.h
	${PROJECT_INCLUDE}/ViewAnalyseNode.h
	)
           
set(FEEFFECT_CPP 
	${PROJECT_SRC}/FlameGeom.cpp
	${PROJECT_SRC}/MutableCylinder.cpp
    ${PROJECT_SRC}/SphereSegment.cpp
    ${PROJECT_SRC}/RadarEffect.cpp
	${PROJECT_SRC}/Corridor.cpp
	${PROJECT_SRC}/explosion.cpp
	${PROJECT_SRC}/UserExplosion.cpp
	${PROJECT_SRC}/WakeFlame.cpp
	${PROJECT_SRC}/FreeEffect.cpp
	${PROJECT_SRC}/LineConnect.cpp
    ${PROJECT_SRC}/RadarWaveEffect.cpp
    ${PROJECT_SRC}/WakeRibbonEffect.cpp
    ${PROJECT_SRC}/HistoryTrackEffect.cpp
    ${PROJECT_SRC}/HighlightEffect.cpp
    ${PROJECT_SRC}/AnimationEffect.cpp
    ${PROJECT_SRC}/ExplosionEffect.cpp
    ${PROJECT_SRC}/WakeSmokeEffect.cpp
    ${PROJECT_SRC}/WakeFlameEffect.cpp
    ${PROJECT_SRC}/ScanRadarEffect.cpp
	${PROJECT_SRC}/FixedStaringScanEffect.cpp
	${PROJECT_SRC}/FixedScanEffect.cpp
	${PROJECT_SRC}/ComplexConicEffect.cpp
	${PROJECT_SRC}/ConeFixedScanEffect.cpp
	${PROJECT_SRC}/EntityTrackEffect.cpp
    ${PROJECT_SRC}/SAREffect.cpp
	${PROJECT_SRC}/StripeEffect.cpp
	${PROJECT_SRC}/GlareDef.cpp
	${PROJECT_SRC}/HDRLighting.cpp
	${PROJECT_SRC}/Scan.cpp
	${PROJECT_SRC}/ViewAnalyseNode.cpp
    )

if(${BUILD_SHARED_LIBS} STREQUAL "On")
    add_library(${OUT_NAME} SHARED ${SRC} ${FEEFFECT_H} ${FEEFFECT_CPP})
    ADD_DEFINITIONS(-DFEEFFECTS_LIBRARY)
else()
    add_library(${OUT_NAME} STATIC ${SRC} ${FEEFFECT_H} ${FEEFFECT_CPP})
    ADD_DEFINITIONS(-DFEEFFECTS_LIBRARY_STATIC)
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

INSTALL(FILES ${FEEFFECT_H} DESTINATION include/FeEffects)

