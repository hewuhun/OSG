SET(OUT_NAME Study03)

include_directories(${CMAKE_SOURCE_DIR}/examples)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


add_executable(${OUT_NAME} main.cpp)
target_link_libraries(${OUT_NAME} FeShell FeEarth FeExtNode FeEffects FreeSampleFrame FeManager )