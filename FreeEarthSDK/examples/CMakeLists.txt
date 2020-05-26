SET(APPLICATION_RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resource)

add_subdirectory(FreeSampleFrame)
SET_PROPERTY(TARGET FreeSampleFrame PROPERTY FOLDER "Examples")

add_subdirectory(FeFuseDemo)
SET_PROPERTY(TARGET FeFuseDemo PROPERTY FOLDER "Examples")
add_dependencies(FeFuseDemo FeShell FeKits FeUtils FeEarth)

add_subdirectory(FeOceanDemo)
SET_PROPERTY(TARGET FeOceanDemo PROPERTY FOLDER "Examples")
add_dependencies(FeOceanDemo FeShell FeKits FeTriton FeUtils FeEarth)

add_subdirectory(FeIndoorRoamingDemo)
SET_PROPERTY(TARGET FeIndoorRoamingDemo PROPERTY FOLDER "Examples")
add_dependencies(FeIndoorRoamingDemo FeShell FeKits FeUtils FeEarth)

add_subdirectory(FeCityVisibleAnalyse)
SET_PROPERTY(TARGET FeCityVisibleAnalyse PROPERTY FOLDER "Examples")
add_dependencies(FeCityVisibleAnalyse FeShell FeKits FeUtils FeEarth)

add_subdirectory(FreeEarthQt)
SET_PROPERTY(TARGET FreeEarthQt PROPERTY FOLDER "Examples")
add_dependencies(FreeEarthQt FeShell FeKits FeEffects FeUtils FeEarth)

add_subdirectory(FeHDRLightingDemo)
SET_PROPERTY(TARGET FeHDRLightingDemo PROPERTY FOLDER "Examples")
add_dependencies(FreeEarthQt FeShell FeKits FeUtils FeEarth)

add_subdirectory(FeVolumeRenderDemo)
SET_PROPERTY(TARGET FeVolumeRenderDemo PROPERTY FOLDER "Examples")
add_dependencies(FeVolumeRenderDemo FeShell FeKits FeUtils FeEarth)

add_subdirectory(FeLabelDemo)
SET_PROPERTY(TARGET FeLabelDemo PROPERTY FOLDER "Examples")
add_dependencies(FeLabelDemo FeShell FeKits FeExtNode FeUtils FeEarth)

add_subdirectory(FeWargamingDemo)
SET_PROPERTY(TARGET FeWargamingDemo PROPERTY FOLDER "Examples")
add_dependencies(FeWargamingDemo FeShell FeKits FeUtils FeEarth)

add_subdirectory(FeSilverliningDemo)
SET_PROPERTY(TARGET FeSilverliningDemo PROPERTY FOLDER "Examples")
add_dependencies(FeSilverliningDemo FeShell FeKits FeUtils FeEarth)

add_subdirectory(FeFlightDemo)
SET_PROPERTY(TARGET FeFlightDemo PROPERTY FOLDER "Examples")
add_dependencies(FeFlightDemo FeShell FeKits FeEffects FeUtils FeEarth)

add_subdirectory(FeScanDemo)
SET_PROPERTY(TARGET FeScanDemo PROPERTY FOLDER "Examples")
add_dependencies(FeScanDemo FeShell FeKits FeUtils FeEarth)

add_subdirectory(Study01)
SET_PROPERTY(TARGET Study01 PROPERTY FOLDER "Examples")
add_dependencies(Study01 FeShell FeKits FeUtils FeEarth FeEffects FreeSampleFrame FeManager)

add_subdirectory(Study02)
SET_PROPERTY(TARGET Study02 PROPERTY FOLDER "Examples")
add_dependencies(Study02 FeShell FeKits FeUtils FeEarth FeEffects FreeSampleFrame FeManager FeExtNode)

add_subdirectory(Study03)
SET_PROPERTY(TARGET Study03 PROPERTY FOLDER "Examples")
add_dependencies(Study03 FeShell FeKits FeUtils FeEarth FeEffects FreeSampleFrame FeManager FeExtNode)

if(WIN32)
SET_PROPERTY(TARGET FeFuseDemo PROPERTY LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
SET_PROPERTY(TARGET FeOceanDemo PROPERTY LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
SET_PROPERTY(TARGET FeIndoorRoamingDemo PROPERTY LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
SET_PROPERTY(TARGET FeCityVisibleAnalyse PROPERTY LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
SET_PROPERTY(TARGET FeHDRLightingDemo PROPERTY LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
SET_PROPERTY(TARGET FeVolumeRenderDemo PROPERTY LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
SET_PROPERTY(TARGET FeLabelDemo PROPERTY LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
SET_PROPERTY(TARGET FeWargamingDemo PROPERTY LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
SET_PROPERTY(TARGET FeSilverliningDemo PROPERTY LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
SET_PROPERTY(TARGET FeFlightDemo PROPERTY LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
SET_PROPERTY(TARGET FeScanDemo PROPERTY LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()


