add_subdirectory(FeExtNode)
SET_PROPERTY(TARGET FeExtNode PROPERTY FOLDER "FeCore")
add_dependencies(FeExtNode FeUtils )

add_subdirectory(FeManager)
SET_PROPERTY(TARGET FeManager PROPERTY FOLDER "FeCore")
add_dependencies(FeManager FeUtils )

add_subdirectory(FePlots)
SET_PROPERTY(TARGET FePlots PROPERTY FOLDER "FeCore")
add_dependencies(FePlots FeExtNode FeUtils )

add_subdirectory(FeLayers)
SET_PROPERTY(TARGET FeLayers PROPERTY FOLDER "FeCore")
add_dependencies(FeLayers FeUtils )

add_subdirectory(FeUtils)
SET_PROPERTY(TARGET FeUtils PROPERTY FOLDER "FeCore")

add_subdirectory(FeKits)
add_dependencies(FeKits FeUtils)
SET_PROPERTY(TARGET FeKits PROPERTY FOLDER "FeCore")

add_subdirectory(FeShell)
add_dependencies(FeShell FeUtils FeKits FeLayers)
SET_PROPERTY(TARGET FeShell PROPERTY FOLDER "FeCore")

add_subdirectory(FeAlg)
add_dependencies(FeAlg FeUtils)
SET_PROPERTY(TARGET FeAlg PROPERTY FOLDER "FeCore")

add_subdirectory(FeMeasure)
add_dependencies(FeMeasure FeUtils)
SET_PROPERTY(TARGET FeMeasure PROPERTY FOLDER "FeCore")

add_subdirectory(FeEffects)
add_dependencies(FeEffects FeUtils)
SET_PROPERTY(TARGET FeEffects PROPERTY FOLDER "FeCore")

add_subdirectory(FeEarth)
add_dependencies(FeEarth FeUtils FeKits FeEffects FeShell FeTriton FeMeasure )
SET_PROPERTY(TARGET FeEarth PROPERTY FOLDER "FeCore")

add_subdirectory(FeSilverlining)
add_dependencies(FeSilverlining FeUtils)
SET_PROPERTY(TARGET FeSilverlining PROPERTY FOLDER "FeExtern")

add_subdirectory(FeSilverliningLib)
SET_PROPERTY(TARGET FeSilverliningLib PROPERTY FOLDER "FeExtern")

add_subdirectory(FeSilverliningOpenGL)
SET_PROPERTY(TARGET FeSilverliningOpenGL PROPERTY FOLDER "FeExtern")

add_subdirectory(FeOcean)
add_dependencies(FeOcean FeUtils FeAlg)
SET_PROPERTY(TARGET FeOcean PROPERTY FOLDER "FeExtern")

add_subdirectory(FeTriton)
SET_PROPERTY(TARGET FeTriton PROPERTY FOLDER "FeExtern")

add_subdirectory(FeServiceProvider)
SET_PROPERTY(TARGET FeServiceProvider PROPERTY FOLDER "FeExtern")

if(WIN32)
if(CUDA_VERSION)
add_subdirectory(FeTritonCUDA)
SET_PROPERTY(TARGET FeTritonCUDA PROPERTY FOLDER "FeExtern")
endif()
endif()

add_subdirectory(FePlugins)
