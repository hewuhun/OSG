set(OUT_NAME FeTriton)
include_directories(${CMAKE_SOURCE_DIR}/include)

SET(PROJECT_INCLUDE ${CMAKE_SOURCE_DIR}/include/FeTriton)
SET(PROJECT_SRC ${CMAKE_SOURCE_DIR}/src/FeTriton)

###########################################################################
#                                                                         #
#                            TRITON    									  #
#                                                                         #
###########################################################################
set(TRITON_H 
	${PROJECT_INCLUDE}/ComplexNumber.h
	${PROJECT_INCLUDE}/Configuration.h
	${PROJECT_INCLUDE}/Decal.h
	${PROJECT_INCLUDE}/DecalManager.h
	${PROJECT_INCLUDE}/DecalManagerOpenGL.h
	${PROJECT_INCLUDE}/DecalOpenGL.h
	${PROJECT_INCLUDE}/DefaultRandomNumberGenerator.h
	${PROJECT_INCLUDE}/Environment.h
	${PROJECT_INCLUDE}/FFT.h
	${PROJECT_INCLUDE}/FFTComputeShader.h
	${PROJECT_INCLUDE}/FFTCPU.h
	${PROJECT_INCLUDE}/FFTCUDA.h
	${PROJECT_INCLUDE}/FFTIPP.h
	${PROJECT_INCLUDE}/FFTWaterModel.h
	${PROJECT_INCLUDE}/FFTWaterModelOpenGL.h
	${PROJECT_INCLUDE}/Frustum.h
	${PROJECT_INCLUDE}/GodRays.h
	${PROJECT_INCLUDE}/GodRaysOpenGL.h
	${PROJECT_INCLUDE}/IFFT.h
	${PROJECT_INCLUDE}/IFFTComputeShader.h
	${PROJECT_INCLUDE}/IFFTCUDA.h
	${PROJECT_INCLUDE}/IFFTIPP.h
	${PROJECT_INCLUDE}/Impact.h
	${PROJECT_INCLUDE}/LicenseManager.h
	${PROJECT_INCLUDE}/Matrix3.h
	${PROJECT_INCLUDE}/Matrix4.h
	${PROJECT_INCLUDE}/MemAlloc.h
	${PROJECT_INCLUDE}/Ocean.h
	${PROJECT_INCLUDE}/OceanLightingModel.h
	${PROJECT_INCLUDE}/OceanLightingModelOpenGL.h
	${PROJECT_INCLUDE}/OrientedBoundingBox.h
	${PROJECT_INCLUDE}/ParticleSystem.h
	${PROJECT_INCLUDE}/ParticleSystemOpenGL.h
	${PROJECT_INCLUDE}/Plane.h
	${PROJECT_INCLUDE}/ProjectedGrid.h
	${PROJECT_INCLUDE}/ProjectedGridOpenGL.h
	${PROJECT_INCLUDE}/RandomNumberGenerator.h
	${PROJECT_INCLUDE}/ResourceLoader.h
	${PROJECT_INCLUDE}/RotorWash.h
	${PROJECT_INCLUDE}/SprayManager.h
	${PROJECT_INCLUDE}/SprayManagerOpenGL.h
	${PROJECT_INCLUDE}/TGALoader.h
	${PROJECT_INCLUDE}/TidalStreamWake.h
	${PROJECT_INCLUDE}/Triton.h
	${PROJECT_INCLUDE}/TritonCommon.h
	${PROJECT_INCLUDE}/TritonOpenGL.h
	${PROJECT_INCLUDE}/Vector3.h
	${PROJECT_INCLUDE}/Vector4.h
	${PROJECT_INCLUDE}/WakeGenerator.h
	${PROJECT_INCLUDE}/WakeManager.h
	${PROJECT_INCLUDE}/WakeManagerOpenGL.h
	${PROJECT_INCLUDE}/WaterModel.h
	${PROJECT_INCLUDE}/WindFetch.h
	${PROJECT_INCLUDE}/Export.h
    )
           
set(TRITON_CPP 
	${PROJECT_SRC}/Configuration.cpp
	${PROJECT_SRC}/Decal.cpp
	${PROJECT_SRC}/DecalManager.cpp
	${PROJECT_SRC}/DecalManagerOpenGL.cpp
	${PROJECT_SRC}/DecalOpenGL.cpp
	${PROJECT_SRC}/DefaultRandomNumberGenerator.cpp
	${PROJECT_SRC}/DynamicLibrary.cpp
	${PROJECT_SRC}/Environment.cpp
	${PROJECT_SRC}/FFT.cpp
	${PROJECT_SRC}/FFTComputeShader.cpp
	${PROJECT_SRC}/FFTCPU.cpp
	${PROJECT_SRC}/FFTCUDA.cpp
	${PROJECT_SRC}/FFTIPP.cpp
	${PROJECT_SRC}/FFTWaterModel.cpp
	${PROJECT_SRC}/FFTWaterModelOpenGL.cpp
	${PROJECT_SRC}/GodRays.cpp
	${PROJECT_SRC}/GodRaysOpenGL.cpp
	${PROJECT_SRC}/Impact.cpp
	${PROJECT_SRC}/LicenseManager.cpp
	${PROJECT_SRC}/Matrix3.cpp
	${PROJECT_SRC}/Matrix4.cpp
	${PROJECT_SRC}/MemAlloc.cpp
	${PROJECT_SRC}/Ocean.cpp
	${PROJECT_SRC}/OceanLightingModel.cpp
	${PROJECT_SRC}/OceanLightingModelOpenGL.cpp
	${PROJECT_SRC}/OrientedBoundingBox.cpp
	${PROJECT_SRC}/ParticleSystem.cpp
	${PROJECT_SRC}/ParticleSystemOpenGL.cpp
	${PROJECT_SRC}/ProjectedGrid.cpp
	${PROJECT_SRC}/ProjectedGridOpenGL.cpp
	${PROJECT_SRC}/ResourceLoader.cpp
	${PROJECT_SRC}/RotorWash.cpp
	${PROJECT_SRC}/SprayManager.cpp
	${PROJECT_SRC}/SprayManagerOpenGL.cpp
	${PROJECT_SRC}/TGALoader.cpp
	${PROJECT_SRC}/TidalStreamWake.cpp
	${PROJECT_SRC}/TritonCommon.cpp
	${PROJECT_SRC}/TritonOpenGL.cpp
	${PROJECT_SRC}/WakeGenerator.cpp
	${PROJECT_SRC}/WakeManager.cpp
	${PROJECT_SRC}/WakeManagerOpenGL.cpp
	${PROJECT_SRC}/WaterModel.cpp
	${PROJECT_SRC}/WindFetch.cpp
    )

if(${BUILD_SHARED_LIBS} STREQUAL "On")
    add_library(${OUT_NAME} SHARED ${SRC} ${TRITON_H} ${TRITON_CPP})
    ADD_DEFINITIONS(-DTRITON_LIBRARY)
else()
    add_library(${OUT_NAME} STATIC ${SRC} ${TRITON_H} ${TRITON_CPP})
    ADD_DEFINITIONS(-DTRITON_LIBRARY_STATIC)
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
add_definitions(-D_DEBUG)
endif()

target_link_libraries(  
    ${OUT_NAME} 
    ${OPENSCENEGRAPH_LIBRARIES} 

        fftss
        ${OPENGL_LIBRARIES}
    )

INSTALL(TARGETS ${OUT_NAME}  DESTINATION lib)
INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)

INSTALL(FILES ${TRITON_H} DESTINATION include/FeTriton)

