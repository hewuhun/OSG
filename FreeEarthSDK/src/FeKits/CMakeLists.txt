SET(OUT_NAME FeKits)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${3rdparty_INCLUDE_DIR})

SET(KITS_INCLLUDE_DIR ${CMAKE_SOURCE_DIR}/include/FeKits)
SET(KITS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/FeKits)

set(BASE_H_CPP
	${KITS_INCLLUDE_DIR}/Export.h
	${KITS_INCLLUDE_DIR}/KitsCtrl.h
	${KITS_INCLLUDE_DIR}/ScreenKits.h

	${KITS_SOURCE_DIR}/KitsCtrl.cpp
	${KITS_SOURCE_DIR}/ScreenKits.cpp
	)
	SOURCE_GROUP( Common FILES ${BASE_H_CPP})

set(NAVIGATOR_H_CPP 
	${KITS_INCLLUDE_DIR}/navigator/BasePart.h
	${KITS_INCLLUDE_DIR}/navigator/EarthManipulatorCtrl.h
	${KITS_INCLLUDE_DIR}/navigator/Navigator.h
	${KITS_INCLLUDE_DIR}/navigator/NavigatorCtrl.h
	${KITS_SOURCE_DIR}/navigator/CompassPart.h
	${KITS_SOURCE_DIR}/navigator/MovePart.h
	${KITS_SOURCE_DIR}/navigator/NavigatorPart.h
	${KITS_SOURCE_DIR}/navigator/RotatePart.h
	${KITS_SOURCE_DIR}/navigator/ZoomPart.h

	${KITS_SOURCE_DIR}/navigator/CompassPart.cpp
	${KITS_SOURCE_DIR}/navigator/BasePart.cpp
	${KITS_SOURCE_DIR}/navigator/EarthManipulatorCtrl.cpp
	${KITS_SOURCE_DIR}/navigator/Navigator.cpp
	${KITS_SOURCE_DIR}/navigator/MovePart.cpp
	${KITS_SOURCE_DIR}/navigator/NavigatorPart.cpp
	${KITS_SOURCE_DIR}/navigator/RotatePart.cpp
	${KITS_SOURCE_DIR}/navigator/ZoomPart.cpp

	)
	SOURCE_GROUP( Navigator FILES ${NAVIGATOR_H_CPP} )


set(MANIPULATOR_H_CPP 
	${KITS_INCLLUDE_DIR}/manipulator/ManipulatorUtil.h
	${KITS_INCLLUDE_DIR}/manipulator/GroundManipulator.h
	${KITS_INCLLUDE_DIR}/manipulator/FreeEarthManipulator.h
	${KITS_INCLLUDE_DIR}/manipulator/PreEarthManipulator.h
	${KITS_INCLLUDE_DIR}/manipulator/FreeEarthManipulatorBase.h
	${KITS_INCLLUDE_DIR}/manipulator/FeNodeTrackerManipulator.h
	${KITS_INCLLUDE_DIR}/manipulator/FeFirstTrackerManipulator.h
	${KITS_INCLLUDE_DIR}/manipulator/AnimationPathManipulator.h
	${KITS_INCLLUDE_DIR}/manipulator/FirstPersonManipulator.h
	${KITS_INCLLUDE_DIR}/manipulator/FeLockNodeManipulator.h

	${KITS_SOURCE_DIR}/manipulator/ManipulatorUtil.cpp
	${KITS_SOURCE_DIR}/manipulator/GroundManipulator.cpp
	${KITS_SOURCE_DIR}/manipulator/FreeEarthManipulator.cpp
	${KITS_SOURCE_DIR}/manipulator/PreEarthManipulator.cpp
	${KITS_SOURCE_DIR}/manipulator/FreeEarthManipulatorBase.cpp
	${KITS_SOURCE_DIR}/manipulator/FeNodeTrackerManipulator.cpp
	${KITS_SOURCE_DIR}/manipulator/FeFirstTrackerManipulator.cpp
	${KITS_SOURCE_DIR}/manipulator/AnimationPathManipulator.cpp
	${KITS_SOURCE_DIR}/manipulator/FirstPersonManipulator.cpp
	${KITS_SOURCE_DIR}/manipulator/FeLockNodeManipulator.cpp
	)
	SOURCE_GROUP( Manipulator FILES ${MANIPULATOR_H_CPP} )


set(COCKPIT_H_CPP 
	${KITS_INCLLUDE_DIR}/cockPit/CockPit.h
	${KITS_INCLLUDE_DIR}/cockPit/GeneralCockPit.h
	${KITS_INCLLUDE_DIR}/cockPit/meter/AdiMeter.h
	${KITS_INCLLUDE_DIR}/cockPit/meter/AltiMeter.h
	${KITS_INCLLUDE_DIR}/cockPit/meter/AoaMeter.h
	${KITS_INCLLUDE_DIR}/cockPit/meter/HsiMeter.h
	${KITS_INCLLUDE_DIR}/cockPit/meter/MeterManager.h
	${KITS_INCLLUDE_DIR}/cockPit/meter/TimeMeter.h
	${KITS_INCLLUDE_DIR}/cockPit/meter/VsiMeter.h

	${KITS_SOURCE_DIR}/cockPit/CockPit.cpp
	${KITS_SOURCE_DIR}/cockPit/GeneralCockPit.cpp
	${KITS_SOURCE_DIR}/cockPit/meter/AdiMeter.cpp
	${KITS_SOURCE_DIR}/cockPit/meter/AltiMeter.cpp
	${KITS_SOURCE_DIR}/cockPit/meter/AoaMeter.cpp
	${KITS_SOURCE_DIR}/cockPit/meter/HsiMeter.cpp
	${KITS_SOURCE_DIR}/cockPit/meter/MeterManager.cpp
	${KITS_SOURCE_DIR}/cockPit/meter/TimeMeter.cpp
	${KITS_SOURCE_DIR}/cockPit/meter/VsiMeter.cpp)
	SOURCE_GROUP( CockPit FILES ${COCKPIT_H_CPP})


set(AIRVIEW_H_CPP 
	${KITS_INCLLUDE_DIR}/airview/AirView.h
	${KITS_INCLLUDE_DIR}/airview/AirViewKits.h

	${KITS_SOURCE_DIR}/airview/AirView.cpp
	${KITS_SOURCE_DIR}/airview/AirViewKits.cpp
	)
	SOURCE_GROUP( Airview FILES ${AIRVIEW_H_CPP})


set(MAPSCALE_H_CPP 
	${KITS_INCLLUDE_DIR}/mapScale/MapScale.h

	${KITS_SOURCE_DIR}/mapScale/MapScale.cpp
	)
	SOURCE_GROUP( MapScale FILES ${MAPSCALE_H_CPP})

set(SCREENINFO_H_CPP 
	${KITS_INCLLUDE_DIR}/screenInfo/InfoMaker.h
	${KITS_INCLLUDE_DIR}/screenInfo/SceneViewInfo.h
	${KITS_INCLLUDE_DIR}/screenInfo/BottomInfoBar.h

	${KITS_SOURCE_DIR}/ScreenInfo/InfoMaker.cpp
	${KITS_SOURCE_DIR}/ScreenInfo/SceneViewInfo.cpp
	${KITS_SOURCE_DIR}/ScreenInfo/BottomInfoBar.cpp
	)
	SOURCE_GROUP( ScreenInfo FILES ${SCREENINFO_H_CPP})

set(MINILOGO_H_CPP
	${KITS_INCLLUDE_DIR}/miniLogo/MiniLogo.h

	${KITS_SOURCE_DIR}/miniLogo/MiniLogo.cpp
	)
	SOURCE_GROUP( MiniLogo FILES ${MINILOGO_H_CPP})

set(WEATHER_H_CPP 
	${KITS_INCLLUDE_DIR}/weather/WeatherEffectNode.h
	${KITS_INCLLUDE_DIR}/weather/WeatherHandler.h
    ${KITS_INCLLUDE_DIR}/weather/GlobalFog.h

	${KITS_SOURCE_DIR}/weather/WeatherEffectNode.cpp
	${KITS_SOURCE_DIR}/weather/WeatherHandler.cpp
    ${KITS_SOURCE_DIR}/weather/GlobalFog.cpp)
	SOURCE_GROUP( Weather FILES ${WEATHER_H_CPP})

set(FREESKY_H_CPP
	${KITS_INCLLUDE_DIR}/sky/FreeSky.h
	${KITS_SOURCE_DIR}/sky/FreeSky.cpp
	${KITS_INCLLUDE_DIR}/sky/HgNebula.h
	${KITS_SOURCE_DIR}/sky/HgNebula.cpp
	)
	SOURCE_GROUP( Sky FILES ${FREESKY_H_CPP})


set(DROP_SCREEN_H_CPP
	${KITS_INCLLUDE_DIR}/dropScreen/DropScreen.h

	${KITS_SOURCE_DIR}/dropScreen/DropScreen.cpp
	)
	SOURCE_GROUP( DropScreen FILES ${DROP_SCREEN_H_CPP})
	
set(PARALLE_SPHERE_H_CPP
	${KITS_INCLLUDE_DIR}/parallelSphere/ParallelSphere.h
	${KITS_SOURCE_DIR}/parallelSphere/ParallelSphere.cpp
	)
	SOURCE_GROUP( ParallelSphere FILES ${PARALLE_SPHERE_H_CPP})
	
set(IR_H_CPP
	${KITS_INCLLUDE_DIR}/IR/SulTypes.h
	${KITS_INCLLUDE_DIR}/IR/SulFilterPass.h
	${KITS_INCLLUDE_DIR}/IR/SulGeode.h
	${KITS_INCLLUDE_DIR}/IR/SulGeomQuad.h
	${KITS_INCLLUDE_DIR}/IR/SulPass.h
	${KITS_INCLLUDE_DIR}/IR/SulPostFilterBloom.h
	${KITS_INCLLUDE_DIR}/IR/SulPostFilterBase.h
	${KITS_INCLLUDE_DIR}/IR/SulScreenAlignedQuad.h
	${KITS_INCLLUDE_DIR}/IR/SulShaderBrightPass.h
	${KITS_INCLLUDE_DIR}/IR/SulShaderCombine.h
	${KITS_INCLLUDE_DIR}/IR/SulShaderGaussH.h
	${KITS_INCLLUDE_DIR}/IR/SulShaderGaussV.h
	${KITS_INCLLUDE_DIR}/IR/SulShaderIntensity.h
	${KITS_INCLLUDE_DIR}/IR/SulTexCam.h
	${KITS_INCLLUDE_DIR}/IR/SulString.h
	${KITS_INCLLUDE_DIR}/IR/SulShaderBase.h
	${KITS_INCLLUDE_DIR}/IR/SulTransScreenAlign.h
	${KITS_INCLLUDE_DIR}/IR/SulIR.h
	${KITS_INCLLUDE_DIR}/IR/SulShaderStreak.h
	${KITS_INCLLUDE_DIR}/IR/SulGeom.h
	${KITS_INCLLUDE_DIR}/IR/SulPostFilterBrightPass.h
	${KITS_INCLLUDE_DIR}/IR/SulPostFilterNightVision.h

	${KITS_SOURCE_DIR}/IR/SulFilterPass.cpp
	${KITS_SOURCE_DIR}/IR/SulGeode.cpp
	${KITS_SOURCE_DIR}/IR/SulGeom.cpp
	${KITS_SOURCE_DIR}/IR/SulGeomQuad.cpp
	${KITS_SOURCE_DIR}/IR/SulPass.cpp
	${KITS_SOURCE_DIR}/IR/SulPostFilterBase.cpp
	${KITS_SOURCE_DIR}/IR/SulPostFilterBloom.cpp
	${KITS_SOURCE_DIR}/IR/SulPostFilterBrightPass.cpp
	${KITS_SOURCE_DIR}/IR/SulPostFilterNightVision.cpp
	${KITS_SOURCE_DIR}/IR/SulScreenAlignedQuad.cpp
	${KITS_SOURCE_DIR}/IR/SulShaderBrightPass.cpp
	${KITS_SOURCE_DIR}/IR/SulShaderCombine.cpp
	${KITS_SOURCE_DIR}/IR/SulShaderGaussH.cpp
	${KITS_SOURCE_DIR}/IR/SulShaderGaussV.cpp
	${KITS_SOURCE_DIR}/IR/SulShaderIntensity.cpp
	${KITS_SOURCE_DIR}/IR/SulString.cpp
	${KITS_SOURCE_DIR}/IR/SulTexCam.cpp
	${KITS_SOURCE_DIR}/IR/SulTransScreenAlign.cpp
	${KITS_SOURCE_DIR}/IR/SulShaderBase.cpp
	${KITS_SOURCE_DIR}/IR/SulShaderStreak.cpp
	

	${KITS_SOURCE_DIR}/IR/SulIR.cpp
	)
	SOURCE_GROUP( IR FILES ${IR_H_CPP})
	
    set(GRATICULE_H_CPP 
	${KITS_INCLLUDE_DIR}/graticule/FeGraticule.h

	${KITS_SOURCE_DIR}/graticule/FeGraticule.cpp
	)
	SOURCE_GROUP( Graticule FILES ${GRATICULE_H_CPP})
    
if(${BUILD_SHARED_LIBS} STREQUAL "On")
    add_library(
	${OUT_NAME} SHARED 
	${BASE_H_CPP}
	${NAVIGATOR_H_CPP}
	${COCKPIT_H_CPP}
	${AIRVIEW_H_CPP}
	${MAPSCALE_H_CPP}
	${SCREENINFO_H_CPP}
	${MINILOGO_H_CPP}
	${PARALLELSPHERE_H_CPP}
	${FREESKY_H_CPP}
	${WEATHER_H_CPP}
	${DROP_SCREEN_H_CPP}
	${PARALLE_SPHERE_H_CPP}
	${SATELITE_ORBIT_H_CPP}
	${EXPLOSION_H_CPP}
	${IR_H_CPP}
    ${GRATICULE_H_CPP}
	${MANIPULATOR_H_CPP}
	)
    ADD_DEFINITIONS(-DFEKIT_LIBRARY)
else()
    add_library(
	${OUT_NAME} STATIC 
	${BASE_H_CPP}
	${NAVIGATOR_H_CPP}
	${COCKPIT_H_CPP}
	${AIRVIEW_H_CPP}
	${MAPSCALE_H_CPP}
	${SCREENINFO_H_CPP}
	${MINILOGO_H_CPP}
	${PARALLELSPHERE_H_CPP}
	${FREESKY_H_CPP}
	${WEATHER_H_CPP}
	${DROP_SCREEN_H_CPP}
	${PARALLE_SPHERE_H_CPP}
	${SATELITE_ORBIT_H_CPP}
	${EXPLOSION_H_CPP}
	${IR_H_CPP}
    ${GRATICULE_H_CPP}
	${MANIPULATOR_H_CPP}
	)
    ADD_DEFINITIONS(-DFEKIT_LIBRARY_STATIC)
endif()

target_link_libraries(
	${OUT_NAME} 
	${OPENGL_LIBRARIES}
	${OPENSCENEGRAPH_LIBRARIES} 
   	debug ${osgEarth_DEBUG_LIBRARIES} optimized ${osgEarth_RELEASE_LIBRARIES} 
	debug ${osgEarthUtil_DEBUG_LIBRARIES} optimized ${osgEarthUtil_RELEASE_LIBRARIES}  
	debug ${osgEarthSymbology_DEBUG_LIBRARIES} optimized ${osgEarthSymbology_RELEASE_LIBRARIES}
	debug ${osgEarthFeatures_DEBUG_LIBRARIES} optimized ${osgEarthFeatures_RELEASE_LIBRARIES}
	debug ${osgEarthAnnotation_DEBUG_LIBRARIES} optimized ${osgEarthAnnotation_RELEASE_LIBRARIES}
	FeUtils
	)
#############################################################################################################################################################################
set(H_INS 
	${KITS_INCLLUDE_DIR}/Export.h
	${KITS_INCLLUDE_DIR}/KitsCtrl.h
	${KITS_INCLLUDE_DIR}/ScreenKits.h

)

set(
	H_INS1
	${KITS_INCLLUDE_DIR}/navigator/BasePart.h
	${KITS_INCLLUDE_DIR}/navigator/EarthManipulatorCtrl.h
	${KITS_INCLLUDE_DIR}/navigator/Navigator.h
	${KITS_INCLLUDE_DIR}/navigator/NavigatorCtrl.h
	${KITS_SOURCE_DIR}/navigator/CompassPart.h
	${KITS_SOURCE_DIR}/navigator/MovePart.h
	${KITS_SOURCE_DIR}/navigator/NavigatorPart.h
	${KITS_SOURCE_DIR}/navigator/RotatePart.h
	${KITS_SOURCE_DIR}/navigator/ZoomPart.h
)
set(
	H_INS2
	${KITS_INCLLUDE_DIR}/cockPit/CockPit.h
	${KITS_INCLLUDE_DIR}/cockPit/GeneralCockPit.h
	${KITS_INCLLUDE_DIR}/cockPit/meter/AdiMeter.h
	${KITS_INCLLUDE_DIR}/cockPit/meter/AltiMeter.h
	${KITS_INCLLUDE_DIR}/cockPit/meter/AoaMeter.h
	${KITS_INCLLUDE_DIR}/cockPit/meter/HsiMeter.h
	${KITS_INCLLUDE_DIR}/cockPit/meter/MeterManager.h
	${KITS_INCLLUDE_DIR}/cockPit/meter/TimeMeter.h
	${KITS_INCLLUDE_DIR}/cockPit/meter/VsiMeter.h
)

set(
	H_INS3
	${KITS_INCLLUDE_DIR}/airview/AirView.h
	${KITS_INCLLUDE_DIR}/airview/AirViewKits.h
)

set(
	H_INS4
	${KITS_INCLLUDE_DIR}/mapScale/MapScale.h
)

set(
	H_INS5
	${KITS_INCLLUDE_DIR}/screenInfo/InfoMaker.h
	${KITS_INCLLUDE_DIR}/screenInfo/SceneViewInfo.h
	${KITS_INCLLUDE_DIR}/screenInfo/BottomInfoBar.h
)
set(
	H_INS6
	${KITS_INCLLUDE_DIR}/miniLogo/MiniLogo.h
)

set(
	H_INS9
	${KITS_INCLLUDE_DIR}/weather/WeatherEffectNode.h
	${KITS_INCLLUDE_DIR}/weather/WeatherHandler.h
    ${KITS_INCLLUDE_DIR}/weather/GlobalFog.h
)
set(
	H_INS10
	${KITS_INCLLUDE_DIR}/sky/FreeSky.h
	${KITS_INCLLUDE_DIR}/sky/HgNebula.h
)

set(
	H_INS11
	${KITS_INCLLUDE_DIR}/dropScreen/DropScreen.h
)

set(
	H_INS12
	${KITS_INCLLUDE_DIR}/parallelSphere/ParallelSphere.h
)

set(
	H_INS14
	${KITS_INCLLUDE_DIR}/IR/SulTypes.h
	${KITS_INCLLUDE_DIR}/IR/SulFilterPass.h
	${KITS_INCLLUDE_DIR}/IR/SulGeode.h
	${KITS_INCLLUDE_DIR}/IR/SulGeomQuad.h
	${KITS_INCLLUDE_DIR}/IR/SulPass.h
	${KITS_INCLLUDE_DIR}/IR/SulPostFilterBloom.h
	${KITS_INCLLUDE_DIR}/IR/SulPostFilterBase.h
	${KITS_INCLLUDE_DIR}/IR/SulScreenAlignedQuad.h
	${KITS_INCLLUDE_DIR}/IR/SulShaderBrightPass.h
	${KITS_INCLLUDE_DIR}/IR/SulShaderCombine.h
	${KITS_INCLLUDE_DIR}/IR/SulShaderGaussH.h
	${KITS_INCLLUDE_DIR}/IR/SulShaderGaussV.h
	${KITS_INCLLUDE_DIR}/IR/SulShaderIntensity.h
	${KITS_INCLLUDE_DIR}/IR/SulTexCam.h
	${KITS_INCLLUDE_DIR}/IR/SulString.h
	${KITS_INCLLUDE_DIR}/IR/SulShaderBase.h
	${KITS_INCLLUDE_DIR}/IR/SulTransScreenAlign.h
	${KITS_INCLLUDE_DIR}/IR/SulIR.h
	${KITS_INCLLUDE_DIR}/IR/SulShaderStreak.h
	${KITS_INCLLUDE_DIR}/IR/SulGeom.h
	${KITS_INCLLUDE_DIR}/IR/SulPostFilterBrightPass.h
	${KITS_INCLLUDE_DIR}/IR/SulPostFilterNightVision.h
)

set(
	H_INS15
	${KITS_INCLLUDE_DIR}/graticule/FeGraticule.h
)

set(H_INS16
	${KITS_INCLLUDE_DIR}/manipulator/ManipulatorUtil.h
	${KITS_INCLLUDE_DIR}/manipulator/GroundManipulator.h
	${KITS_INCLLUDE_DIR}/manipulator/PreEarthManipulator.h
	${KITS_INCLLUDE_DIR}/manipulator/FreeEarthManipulator.h
	${KITS_INCLLUDE_DIR}/manipulator/FeNodeTrackerManipulator.h
	${KITS_INCLLUDE_DIR}/manipulator/FreeEarthManipulatorBase.h
	${KITS_INCLLUDE_DIR}/manipulator/FeFirstTrackerManipulator.h
	${KITS_INCLLUDE_DIR}/manipulator/AnimationPathManipulator.h
	${KITS_INCLLUDE_DIR}/manipulator/FeLockNodeManipulator.h
	)

INSTALL(TARGETS ${OUT_NAME}  DESTINATION lib)
INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)

INSTALL(FILES ${H_INS} DESTINATION include/FeKits)
INSTALL(FILES ${H_INS1} DESTINATION include/FeKits/navigator)
INSTALL(FILES ${H_INS2} DESTINATION include/FeKits/cockPit)
INSTALL(FILES ${H_INS3} DESTINATION include/FeKits/airview)
INSTALL(FILES ${H_INS4} DESTINATION include/FeKits/mapScale)
INSTALL(FILES ${H_INS5} DESTINATION include/FeKits/screenInfo)
INSTALL(FILES ${H_INS6} DESTINATION include/FeKits/miniLogo)
INSTALL(FILES ${H_INS9} DESTINATION include/FeKits/weather)
INSTALL(FILES ${H_INS10} DESTINATION include/FeKits/sky)
INSTALL(FILES ${H_INS11} DESTINATION include/FeKits/dropScreen)
INSTALL(FILES ${H_INS12} DESTINATION include/FeKits/parallelSphere)
INSTALL(FILES ${H_INS14} DESTINATION include/FeKits/IR)
INSTALL(FILES ${H_INS16} DESTINATION include/FeKits/manipulator)
INSTALL(FILES ${H_INS15} DESTINATION include/FeKits/graticule)
