project(FreeViewer)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
execute_process(COMMAND ${CMAKE_COMMAND} "-Wno-dev")

#设置生成动态库和静态库
if (WIN32)
    set(BUILD_SHARED_LIBS "On")
else(UNIX)
    set(BUILD_SHARED_LIBS "On")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

#设置编译发布版本选项，发布新的版本需要开启此语句
#add_definitions(-DRELEASE_ON)

#设置调试库的后缀
SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows")
SET(CMAKE_RELEASE_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows")
SET(CMAKE_RELWITHDEBINFO_POSTFIX "rd" CACHE STRING "add a postfix, usually empty on windows")
SET(CMAKE_MINSIZEREL_POSTFIX "s" CACHE STRING "add a postfix, usually empty on windows")

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

#设置库和可执行文件的输出文件
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")
include(find_hg_FreeEarth)
include(find_hg_qwt)

#IF(WIN32 AND NOT ANDROID)

	IF(MSVC)
        OPTION(WIN32_USE_MP "Set to ON to build OpenSceneGraph with the /MP option (Visual Studio 2005 and above)." OFF)
        MARK_AS_ADVANCED(WIN32_USE_MP)
        IF(WIN32_USE_MP)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        ENDIF(WIN32_USE_MP)
	ENDIF()

	SET(3RD_PARTY_DIR "" CACHE PATH "Set to base lib install path")
	SET(FREEEARTH_DIR "" CACHE PATH "Set to base OpenSceneGraph install path")
	SET(OSG_DIR "" CACHE PATH "Set to base OpenSceneGraph install path")
	SET(OSGEARTH_DIR "" CACHE PATH "Set to OSGEARTH   install path")
    SET(QWT_DIR ${3RD_PARTY_DIR})
	
	include(find_hg_osgEarth)
	#include(find_hg_log4cpp)
	#include(find_hg_gdal)
	
	include_directories("${3RD_PARTY_DIR}/include")
	include_directories("${OSG_INCLUDE_DIR}")
	include_directories("${OSGEARTH_INCLUDE_DIR}")
	include_directories("${FREEEARTH_INCLUDE_DIR}")
	include_directories("${QWT_INCLUDE_DIR}")

	link_directories("${3RD_PARTY_DIR}/lib")
	link_directories("${OSG_DIR}/lib")
	link_directories("${OSGEARTH_DIR}/lib")
	link_directories("${FREEEARTH_DIR}/lib")
#ENDIF()

find_package(OpenGL)

find_package(OpenSceneGraph 3.2.1 REQUIRED 
	osg 
	osgGA 
	osgDB 
	osgViewer 
	osgUtil 
	osgText 
	osgQt 
	osgVolume 
	osgTerrain 
	osgSim 
	osgFX 
	osgAnimation 
	osgManipulator
	osgParticle)
	
if (WIN32)
    if (OPENSCENEGRAPH_FOUND)
        include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS})
    endif()
endif()

if (WIN32)
    if (OSGEARTH_FOUND)
        include_directories(${OSGEARTH_INCLUDE_DIR})
    endif()
endif()

#查找Qt
FIND_PACKAGE(Qt5Core)
FIND_PACKAGE(Qt5Widgets)
FIND_PACKAGE(Qt5Gui)
FIND_PACKAGE(Qt5OpenGL)
FIND_PACKAGE(Qt5Network)
FIND_PACKAGE(Qt5Test)
FIND_PACKAGE(Qt5Xml)
IF ( Qt5Core_FOUND AND Qt5Widgets_FOUND AND Qt5Gui_FOUND AND Qt5OpenGL_FOUND AND Qt5Network_FOUND AND Qt5Test_FOUND AND Qt5Xml_FOUND)
    SET(QT_INCLUDES ${Qt5Widgets_INCLUDE_DIRS} ${Qt5OpenGL_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS} ${Qt5Test_INCLUDE_DIRS} ${Qt5Xml_INCLUDE_DIRS})
ELSE()
    FIND_PACKAGE(Qt4 REQUIRED QtCore QtGui QtOpenGL QtNetwork QtTest QtXml)
    IF (QT4_FOUND)
        add_definitions(-D__QT4_USE_UNICODE)
		add_definitions(-DQT4_VERSION)
        INCLUDE(${QT_USE_FILE})
                SET(QT_INCLUDES ${QT_INCLUDES} ${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR}${QT_QTOPENGL_INCLUDE_DIR}${QT_QTNETWORK_INCLUDE_DIR} ${QT_QTTEST4_INCLUDE_DIR})
        SET(QT_ALL_LIBRARIES ${QT_LIBRARIES} ${QT_QTCORE_LIBRARY} ${QT_QTWEBKIT_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTOPENGL_LIBRARY}  ${QT_QTTEST4_LIBRARY})
    ENDIF (QT4_FOUND)
ENDIF ()

if(UNIX)
	find_library(log4Cpp_DEBUG_LIBRARIES liblog4cpp.so ENV log4Cpp_LIBRARY_DIR)
	find_library(log4Cpp_RELEASE_LIBRARIES liblog4cpp.so ENV log4Cpp_LIBRARY_DIR)

	find_library(osgEarth_RELEASE_LIBRARIES libosgEarth.so ENV osgEarth_LIBRARY_DIR)
	find_library(osgEarth_DEBUG_LIBRARIES libosgEarthd.so ENV osgEarth_LIBRARY_DIR)

	find_library(osgEarthUtil_RELEASE_LIBRARIES libosgEarthUtil.so ENV osgEarth_LIBRARY_DIR)
	find_library(osgEarthUtil_DEBUG_LIBRARIES libosgEarthUtild.so ENV osgEarth_LIBRARY_DIR)

    find_library(osgEarthAnnotation_DEBUG_LIBRARIES libosgEarthAnnotationd.so ENV osgEarthAnnotation_DEBUG_LIBRARIES)
    find_library(osgEarthAnnotation_RELEASE_LIBRARIES libosgEarthAnnotation.so ENV osgEarthAnnotation_RELEASE_LIBRARIES)

    find_library(osgEarthSymbology_DEBUG_LIBRARIES libosgEarthSymbologyd.so ENV osgEarthSymbology_DEBUG_LIBRARIES)
    find_library(osgEarthSymbology_RELEASE_LIBRARIES libosgEarthSymbology.so ENV osgEarthSymbology_RELEASE_LIBRARIES)

    find_library(osgEarthFeatures_DEBUG_LIBRARIES libosgEarthFeaturesd.so ENV osgEarthSymbology_DEBUG_LIBRARIES)
    find_library(osgEarthFeatures_RELEASE_LIBRARIES libosgEarthFeatures.so ENV osgEarthSymbology_RELEASE_LIBRARIES)
endif()

#添加子项目
add_subdirectory(src)

if(WIN32)
SET_PROPERTY(TARGET FreeViewer PROPERTY LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()
