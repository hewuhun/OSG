SET(OUT_NAME FreeViewer)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

SET(APPLICATION_DIR ${CMAKE_SOURCE_DIR}/src)
SET(APPLICATION_RESOURCE_DIR ${CMAKE_SOURCE_DIR}/src/resource)

###########################################################################
#                                                                         #
#                            MAIN    									  #
#                                                                         #
###########################################################################
SET(MAIN_WINDOW_H_CPP
	${APPLICATION_DIR}/mainWindow/BootScreenFrame.h
	${APPLICATION_DIR}/mainWindow/BootScreenFrame.cpp
	${APPLICATION_DIR}/mainWindow/BootScreenRoundBar.h
	${APPLICATION_DIR}/mainWindow/BootScreenRoundBar.cpp
	${APPLICATION_DIR}/mainWindow/BootScreenBallPath.h
	${APPLICATION_DIR}/mainWindow/BootScreenBallPath.cpp

	${APPLICATION_DIR}/mainWindow/UIObserver.h
	${APPLICATION_DIR}/mainWindow/UIObserver.cpp
	
	${APPLICATION_DIR}/mainWindow/FreeUtil.h
	${APPLICATION_DIR}/mainWindow/FreeUtil.cpp
	
	${APPLICATION_DIR}/mainWindow/FolderWidget.h
	${APPLICATION_DIR}/mainWindow/FolderWidget.cpp

	${APPLICATION_DIR}/mainWindow/FreeFramelessWidget.h
	${APPLICATION_DIR}/mainWindow/FreeFramelessWidget.cpp
	${APPLICATION_DIR}/mainWindow/FreeMainWindow.h
	${APPLICATION_DIR}/mainWindow/FreeMainWindow.cpp
	${APPLICATION_DIR}/mainWindow/FreeSceneWidget.h
	${APPLICATION_DIR}/mainWindow/FreeSceneWidget.cpp
	${APPLICATION_DIR}/mainWindow/FreeMenuWidget.h
	${APPLICATION_DIR}/mainWindow/FreeMenuWidget.cpp
	${APPLICATION_DIR}/mainWindow/FreeServiceDockTree.h
	${APPLICATION_DIR}/mainWindow/FreeServiceButton.h
	${APPLICATION_DIR}/mainWindow/FreeToolBar.h
	${APPLICATION_DIR}/mainWindow/FreeToolBar.cpp
	${APPLICATION_DIR}/mainWindow/FreeToolButton.h
	${APPLICATION_DIR}/mainWindow/FreeToolButton.cpp
	${APPLICATION_DIR}/mainWindow/FreeStatusBar.h
	${APPLICATION_DIR}/mainWindow/FreeStatusBar.cpp

	${APPLICATION_DIR}/mainWindow/FreeDockFrame.h
	${APPLICATION_DIR}/mainWindow/FreeDockFrame.cpp
	${APPLICATION_DIR}/mainWindow/FreeDockWidget.h
	${APPLICATION_DIR}/mainWindow/FreeDockWidget.cpp
	${APPLICATION_DIR}/mainWindow/FreeTreeWidget.h
	${APPLICATION_DIR}/mainWindow/FreeTreeWidget.cpp
	${APPLICATION_DIR}/mainWindow/CustomPushButton.h
	${APPLICATION_DIR}/mainWindow/CustomPushButton.cpp
	${APPLICATION_DIR}/mainWindow/Free3DDockTreeWidget.h
	${APPLICATION_DIR}/mainWindow/Free3DDockTreeWidget.cpp

	${APPLICATION_DIR}/mainWindow/FreeServiceDockTree.h
	${APPLICATION_DIR}/mainWindow/FreeServiceDockTree.cpp
	${APPLICATION_DIR}/mainWindow/FreeServiceButton.h
	${APPLICATION_DIR}/mainWindow/FreeServiceButton.cpp
	
	${APPLICATION_DIR}/mainWindow/FreeDialog.h
	${APPLICATION_DIR}/mainWindow/FreeDialog.cpp

	${APPLICATION_DIR}/mainWindow/FreeLineEdit.h
	${APPLICATION_DIR}/mainWindow/FreeLineEdit.cpp

	${APPLICATION_DIR}/mainWindow/SimicircleMenu.h
	${APPLICATION_DIR}/mainWindow/SimicircleMenu.cpp
	
	${APPLICATION_DIR}/mainWindow/main.cpp
	)
SOURCE_GROUP(MainWindow FILES ${MAIN_WINDOW_H_CPP})


SET(MAIN_WINDOW_RESOURCE
	${APPLICATION_RESOURCE_DIR}/images.qrc
	)
SOURCE_GROUP(Resource FILES ${MAIN_WINDOW_RESOURCE})

if (Qt5Widgets_FOUND)
	QT5_ADD_RESOURCES(MAIN_WINDOW_MOCS_RESOURCE
	${MAIN_WINDOW_RESOURCE}
	)
else()
	QT4_ADD_RESOURCES(MAIN_WINDOW_MOCS_RESOURCE
	${MAIN_WINDOW_RESOURCE}
	)
endif()


SET(MAIN_WINDOW_UI
	${APPLICATION_DIR}/mainWindow/BootScreenFrame.ui
	#${APPLICATION_DIR}/mainWindow/BootScreenWidget.ui
	${APPLICATION_DIR}/mainWindow/FreeMenuWidget.ui
	${APPLICATION_DIR}/mainWindow/FreeDockFrame.ui
	${APPLICATION_DIR}/mainWindow/FreeDockWidget.ui
	${APPLICATION_DIR}/mainWindow/FolderWidget.ui
	${APPLICATION_DIR}/mainWindow/FreeServiceButton.ui
	)

if (Qt5Widgets_FOUND)
	qt5_wrap_ui(UICS ${MAIN_WINDOW_UI})
else()
	qt4_wrap_ui(UICS ${MAIN_WINDOW_UI})
endif()

SOURCE_GROUP(MainWindow FILES ${MAIN_WINDOW_UI})

	
###########################################################################
#                                                                         #
#                            SYSTEM									      #
#                                                                         #
###########################################################################
SET(SYSTEM_H_CPP
	${APPLICATION_DIR}/system/SystemPluginInterface.h
	${APPLICATION_DIR}/system/SystemPluginInterface.cpp
	)
SOURCE_GROUP(System FILES ${SYSTEM_H_CPP})


###########################################################################
#                                                                         #
#                            VIEWS									      #
#                                                                         #
###########################################################################
SET(VIEW_H_CPP
	${APPLICATION_DIR}/view/ViewPluginInterface.h
	${APPLICATION_DIR}/view/ViewPluginInterface.cpp
	${APPLICATION_DIR}/view/FreeStatusCallback.h
	${APPLICATION_DIR}/view/FreeStatusCallback.cpp
	)
SOURCE_GROUP(View FILES ${VIEW_H_CPP})

###########################################################################
#                                                                         #
#                            Service    								  #
#                                                                         #
###########################################################################
SET(SERVICE_H_CPP
	${APPLICATION_DIR}/service/ServicePluginInterface.h
	${APPLICATION_DIR}/service/ServicePluginInterface.cpp
	${APPLICATION_DIR}/service/OGCServiceWidget.h
	${APPLICATION_DIR}/service/OGCServiceWidget.cpp
	${APPLICATION_DIR}/service/TileServiceWidget.h
	${APPLICATION_DIR}/service/TileServiceWidget.cpp
	${APPLICATION_DIR}/service/DataServiceWidget.h
	${APPLICATION_DIR}/service/DataServiceWidget.cpp
	${APPLICATION_DIR}/service/OGCServicePropertyWidget.h
	${APPLICATION_DIR}/service/OGCServicePropertyWidget.cpp
	${APPLICATION_DIR}/service/OGCLayerPropertyWidget.h
	${APPLICATION_DIR}/service/OGCLayerPropertyWidget.cpp
	${APPLICATION_DIR}/service/ServiceLayerPropertyWidget.h
	${APPLICATION_DIR}/service/ServiceLayerPropertyWidget.cpp
	${APPLICATION_DIR}/service/LocalDiskTree.h
	${APPLICATION_DIR}/service/LocalDiskTree.cpp
	)
SOURCE_GROUP(Service FILES ${SERVICE_H_CPP})
	
SET(SERVICE_UI
	${APPLICATION_DIR}/service/OGCServiceWidget.ui
	${APPLICATION_DIR}/service/TileServiceWidget.ui
	${APPLICATION_DIR}/service/DataServiceWidget.ui
	${APPLICATION_DIR}/service/OGCServicePropertyWidget.ui
	${APPLICATION_DIR}/service/OGCLayerPropertyWidget.ui
	${APPLICATION_DIR}/service/ServiceLayerPropertyWidget.ui
	)
if (Qt5Widgets_FOUND)
	qt5_wrap_ui(UICS ${SERVICE_UI})
else()
	qt4_wrap_ui(UICS ${SERVICE_UI})
endif()
SOURCE_GROUP(Service FILES ${SERVICE_UI})


###########################################################################
#                                                                         #
#                            LAYER									      #
#                                                                         #
###########################################################################
SET(LAYER_H_CPP
	${APPLICATION_DIR}/layer/SystemLayerTree.h
	${APPLICATION_DIR}/layer/SystemLayerTree.cpp
	${APPLICATION_DIR}/layer/ImageLayerAttributeDialog.h
	${APPLICATION_DIR}/layer/ImageLayerAttributeDialog.cpp
	${APPLICATION_DIR}/layer/CustomLayerTree.h
	${APPLICATION_DIR}/layer/CustomLayerTree.cpp
	${APPLICATION_DIR}/layer/ImageLayerDialog.h
	${APPLICATION_DIR}/layer/ImageLayerDialog.cpp
	${APPLICATION_DIR}/layer/ElevationLayerDialog.h
	${APPLICATION_DIR}/layer/ElevationLayerDialog.cpp
	${APPLICATION_DIR}/layer/VectorLayerDialog.h
	${APPLICATION_DIR}/layer/VectorLayerDialog.cpp
	${APPLICATION_DIR}/layer/RGBColorFilterDialog.h
	${APPLICATION_DIR}/layer/RGBColorFilterDialog.cpp

	
	
	${APPLICATION_DIR}/layer/PropertyImageLayerDialog.h
	${APPLICATION_DIR}/layer/PropertyImageLayerDialog.cpp

	)
SOURCE_GROUP(Layer FILES ${LAYER_H_CPP})


SET(LAYER_UI
	${APPLICATION_DIR}/layer/ImageLayerDialog.ui
	${APPLICATION_DIR}/layer/ElevationLayerDialog.ui
	${APPLICATION_DIR}/layer/VectorLayerDialog.ui
	${APPLICATION_DIR}/layer/ImageLayerAttributeDialog.ui
	${APPLICATION_DIR}/layer/RGBColorFilterDialog.ui
	${APPLICATION_DIR}/layer/PropertyImageLayerDialog.ui
	)

if (Qt5Widgets_FOUND)
	qt5_wrap_ui(UICS ${LAYER_UI})
else()
	qt4_wrap_ui(UICS ${LAYER_UI})
endif()


SOURCE_GROUP(Layer FILES ${LAYER_UI})



###########################################################################
#                                                                         #
#                            ENVIRONMENT							      #
#                                                                         #
###########################################################################
SET(ENVIRONMENT_H_CPP
	${APPLICATION_DIR}/environment/EnvironmentPluginInterface.h
	${APPLICATION_DIR}/environment/EnvironmentPluginInterface.cpp
	#${APPLICATION_DIR}/environment/SkyCtrlBar.h
	#${APPLICATION_DIR}/environment/SkyCtrlBar.cpp
	${APPLICATION_DIR}/environment/RainSetWidget.h
	${APPLICATION_DIR}/environment/RainSetWidget.cpp
	${APPLICATION_DIR}/environment/SnowSetWidget.h
	${APPLICATION_DIR}/environment/SnowSetWidget.cpp	
	${APPLICATION_DIR}/environment/OceanSetWidget.h
	${APPLICATION_DIR}/environment/OceanSetWidget.cpp
	${APPLICATION_DIR}/environment/LightSetWidget.h
	${APPLICATION_DIR}/environment/LightSetWidget.cpp
	${APPLICATION_DIR}/environment/TimeSetWidget.h
	${APPLICATION_DIR}/environment/TimeSetWidget.cpp
	${APPLICATION_DIR}/environment/SettingWidget.h
	${APPLICATION_DIR}/environment/SettingWidget.cpp
    #${APPLICATION_DIR}/environment/StructSetData.h
    #${APPLICATION_DIR}/environment/StructSetData.cpp
	${APPLICATION_DIR}/environment/SilverLiningSetWidget.h
	${APPLICATION_DIR}/environment/SilverLiningSetWidget.cpp
	${APPLICATION_DIR}/environment/FogSetWidget.h
	${APPLICATION_DIR}/environment/FogSetWidget.cpp
	)
SOURCE_GROUP(Environment FILES ${ENVIRONMENT_H_CPP})



SET(ENVIRONMENT_UI
	#${APPLICATION_DIR}/environment/SkyCtrlBar.ui
	${APPLICATION_DIR}/environment/RainSetWidget.ui
	${APPLICATION_DIR}/environment/SnowSetWidget.ui
	${APPLICATION_DIR}/environment/OceanSetWidget.ui
	${APPLICATION_DIR}/environment/SettingWidget.ui
	${APPLICATION_DIR}/environment/TimeSetWidget.ui
	${APPLICATION_DIR}/environment/LightSetWidget.ui
	${APPLICATION_DIR}/environment/SilverLiningSetWidget.ui
	${APPLICATION_DIR}/environment/FogSetWidget.ui
	)
	
if (Qt5Widgets_FOUND)
	qt5_wrap_ui(UICS ${ENVIRONMENT_UI})
else()
	qt4_wrap_ui(UICS ${ENVIRONMENT_UI})
endif()


SOURCE_GROUP(Environment FILES ${ENVIRONMENT_UI})


###########################################################################
#                                                                         #
#                            TOOL									      #
#                                                                         #
###########################################################################
SET(TOOL_H_CPP
	${APPLICATION_DIR}/tool/ToolPluginInterface.h
	${APPLICATION_DIR}/tool/ToolPluginInterface.cpp
	${APPLICATION_DIR}/tool/LanguageWidget.h
	${APPLICATION_DIR}/tool/LanguageWidget.cpp
	#${APPLICATION_DIR}/tool/LocateWidget.h
	#${APPLICATION_DIR}/tool/LocateWidget.cpp
	${APPLICATION_DIR}/tool/BlackBoard.h
	${APPLICATION_DIR}/tool/BlackBoard.cpp
	${APPLICATION_DIR}/tool/DropScreen.h
	${APPLICATION_DIR}/tool/DropScreen.cpp
	${APPLICATION_DIR}/tool/DrawUnit.h
	${APPLICATION_DIR}/tool/DrawUnit.cpp
	${APPLICATION_DIR}/tool/BlackBoardControlBtns.h
	${APPLICATION_DIR}/tool/BlackBoardControlBtns.cpp
	)
SOURCE_GROUP(Tool FILES ${TOOL_H_CPP})


SET(TOOL_UI
	${APPLICATION_DIR}/tool/LanguageWidget.ui
	#${APPLICATION_DIR}/tool/LocateWidget.ui
	${APPLICATION_DIR}/tool/BlackBoard.ui
	${APPLICATION_DIR}/tool/BlackBoardControlBtns.ui
	)

if (Qt5Widgets_FOUND)
	qt5_wrap_ui(UICS ${TOOL_UI})
else()
	qt4_wrap_ui(UICS ${TOOL_UI})
endif()

SOURCE_GROUP(Tool FILES ${TOOL_UI})

###########################################################################
#                                                                         #
#                            MARK									      #
#                                                                         #
###########################################################################
SET(MARK_H_CPP
	${APPLICATION_DIR}/mark/Common.h
	${APPLICATION_DIR}/mark/Common.cpp
	#${APPLICATION_DIR}/mark/MarkMgtWidget.h
	#${APPLICATION_DIR}/mark/MarkMgtWidget.cpp
	${APPLICATION_DIR}/mark/PropertyWidget.h
	${APPLICATION_DIR}/mark/PropertyWidget.cpp
	${APPLICATION_DIR}/mark/MarkPropertyBuilder.h
	${APPLICATION_DIR}/mark/MarkPropertyBuilder.cpp
	${APPLICATION_DIR}/mark/BaseStyleWidget.h
	${APPLICATION_DIR}/mark/BaseStyleWidget.cpp
	${APPLICATION_DIR}/mark/ViewPointStyleWidget.h
	${APPLICATION_DIR}/mark/ViewPointStyleWidget.cpp 
	#${APPLICATION_DIR}/mark/VectorMarkWidget.h
	#${APPLICATION_DIR}/mark/VectorMarkWidget.cpp
	${APPLICATION_DIR}/mark/DescribeStyleWidget.h
	${APPLICATION_DIR}/mark/DescribeStyleWidget.cpp
	${APPLICATION_DIR}/mark/PositionStyleWidget.h
	${APPLICATION_DIR}/mark/PositionStyleWidget.cpp
	${APPLICATION_DIR}/mark/ModelStyleWidget.h
	${APPLICATION_DIR}/mark/ModelStyleWidget.cpp
	${APPLICATION_DIR}/mark/LineStyleWidget.h
	${APPLICATION_DIR}/mark/LineStyleWidget.cpp
	${APPLICATION_DIR}/mark/BillboardStyleWidget.h
	${APPLICATION_DIR}/mark/BillboardStyleWidget.cpp
	${APPLICATION_DIR}/mark/OverLayStyleWidget.h
	${APPLICATION_DIR}/mark/OverLayStyleWidget.cpp
	${APPLICATION_DIR}/mark/MarkPluginInterface.h
	${APPLICATION_DIR}/mark/MarkPluginInterface.cpp
	#${APPLICATION_DIR}/mark/ModelPreViewWidget.h
	#${APPLICATION_DIR}/mark/ModelPreViewWidget.cpp
	#${APPLICATION_DIR}/mark/ModelLibraryWidget.h
	#${APPLICATION_DIR}/mark/ModelLibraryWidget.cpp
	#${APPLICATION_DIR}/mark/ModelLibraryPropertyWidget.h
	#${APPLICATION_DIR}/mark/ModelLibraryPropertyWidget.cpp
	${APPLICATION_DIR}/mark/ArcStyleWidget.h
	${APPLICATION_DIR}/mark/ArcStyleWidget.cpp
	${APPLICATION_DIR}/mark/EllipseStyleWidget.h
	${APPLICATION_DIR}/mark/EllipseStyleWidget.cpp
	${APPLICATION_DIR}/mark/RectStyleWidget.h
	${APPLICATION_DIR}/mark/RectStyleWidget.cpp
	${APPLICATION_DIR}/mark/SectorStyleWidget.h
	${APPLICATION_DIR}/mark/SectorStyleWidget.cpp
	${APPLICATION_DIR}/mark/CircleStyleWidget.h
	${APPLICATION_DIR}/mark/CircleStyleWidget.cpp
	${APPLICATION_DIR}/mark/TextStyleWidget.h
	${APPLICATION_DIR}/mark/TextStyleWidget.cpp
	${APPLICATION_DIR}/mark/TiltModelStyleWidget.h
	${APPLICATION_DIR}/mark/TiltModelStyleWidget.cpp
	${APPLICATION_DIR}/mark/LabelStyleWidget.h
	${APPLICATION_DIR}/mark/LabelStyleWidget.cpp
	${APPLICATION_DIR}/mark/MarkEventHandler.h
	${APPLICATION_DIR}/mark/MarkEventHandler.cpp
	${APPLICATION_DIR}/mark/MarkVisitProcessor.h
	${APPLICATION_DIR}/mark/MarkVisitProcessor.cpp
	${APPLICATION_DIR}/mark/MarkMenuMgr.h
	${APPLICATION_DIR}/mark/MarkMenuMgr.cpp
	${APPLICATION_DIR}/mark/ParticleStyleWidget.h
	${APPLICATION_DIR}/mark/ParticleStyleWidget.cpp
	${APPLICATION_DIR}/mark/MarkSdkEventCapture.h
	
	
	)
SOURCE_GROUP(Mark FILES ${MARK_H_CPP})



SET(MARK_UI
	#${APPLICATION_DIR}/mark/MarkMgtWidget.ui
	#${APPLICATION_DIR}/mark/ModelLibraryWidget.ui
	#${APPLICATION_DIR}/mark/ModelPreViewWidget.ui
	${APPLICATION_DIR}/mark/PropertyWidget.ui
	${APPLICATION_DIR}/mark/ViewPointStyleWidget.ui
	${APPLICATION_DIR}/mark/DescribeStyleWidget.ui
	${APPLICATION_DIR}/mark/PositionStyleWidget.ui
	${APPLICATION_DIR}/mark/ModelStyleWidget.ui
	${APPLICATION_DIR}/mark/LineStyleWidget.ui
	${APPLICATION_DIR}/mark/BillboardStyleWidget.ui
	${APPLICATION_DIR}/mark/OverLayStyleWidget.ui
	#${APPLICATION_DIR}/mark/VectorMarkWidget.ui
	${APPLICATION_DIR}/mark/ArcStyleWidget.ui
	${APPLICATION_DIR}/mark/EllipseStyleWidget.ui
	${APPLICATION_DIR}/mark/RectStyleWidget.ui
	${APPLICATION_DIR}/mark/SectorStyleWidget.ui
	${APPLICATION_DIR}/mark/CircleStyleWidget.ui
	${APPLICATION_DIR}/mark/TextStyleWidget.ui
	${APPLICATION_DIR}/mark/TiltModelStyleWidget.ui
	${APPLICATION_DIR}/mark/LabelStyleWidget.ui
	${APPLICATION_DIR}/mark/ParticleStyleWidget.ui
	)

if (Qt5Widgets_FOUND)
	qt5_wrap_ui(UICS ${MARK_UI})
else()
	qt4_wrap_ui(UICS ${MARK_UI})
endif()


SOURCE_GROUP(Mark FILES ${MARK_UI})

###########################################################################
#                                                                         #
#                            PLOT									      #
#                                                                         #
###########################################################################
SET(PLOT_H_CPP
	${APPLICATION_DIR}/plot/PlotPluginInterface.h
	${APPLICATION_DIR}/plot/PlotPluginInterface.cpp
	${APPLICATION_DIR}/plot/PlotMenuManager.h
	${APPLICATION_DIR}/plot/PlotMenuManager.cpp
	${APPLICATION_DIR}/plot/PlotVisitProcessor.h
	${APPLICATION_DIR}/plot/PlotVisitProcessor.cpp
	${APPLICATION_DIR}/plot/PlotEventListener.h
	${APPLICATION_DIR}/plot/PlotEventHandler.h
	${APPLICATION_DIR}/plot/PlotEventHandler.cpp
	${APPLICATION_DIR}/plot/PlotPropertyBuilder.h
	${APPLICATION_DIR}/plot/PlotPropertyBuilder.cpp

	)
SOURCE_GROUP(Plot FILES ${PLOT_H_CPP})

SET(PLOT_UI

	)

if (Qt5Widgets_FOUND)
	qt5_wrap_ui(UICS ${PLOT_UI})
else()
	qt4_wrap_ui(UICS ${PLOT_UI})
endif()
SOURCE_GROUP(Plot FILES ${PLOT_UI})

###########################################################################
#                                                                         #
#                            HELP									      #
#                                                                         #
###########################################################################
SET(HELP_H_CPP
	${APPLICATION_DIR}/help/HelpMenu.h
	${APPLICATION_DIR}/help/HelpMenu.cpp
	${APPLICATION_DIR}/help/AboutWidget.h
	${APPLICATION_DIR}/help/AboutWidget.cpp
	)
SOURCE_GROUP(Help FILES ${HELP_H_CPP})

SET(HELP_UI
	${APPLICATION_DIR}/help/AboutWidget.ui
	)

if (Qt5Widgets_FOUND)
	qt5_wrap_ui(UICS ${HELP_UI})
else()
	qt4_wrap_ui(UICS ${HELP_UI})
endif()
SOURCE_GROUP(Help FILES ${HELP_UI})

###########################################################################
#                                                                         #
#                            MEASURE									  #
#                                                                         #
###########################################################################
SET(MEASURE_H_CPP
	${APPLICATION_DIR}/measure/MeasurePluginInterface.h
	${APPLICATION_DIR}/measure/MeasurePluginInterface.cpp

	${APPLICATION_DIR}/measure/DistanceWidget.h
	${APPLICATION_DIR}/measure/DistanceWidget.cpp

	${APPLICATION_DIR}/measure/BuildingsViewWidget.h
	${APPLICATION_DIR}/measure/BuildingsViewWidget.cpp

	${APPLICATION_DIR}/measure/FillCutAnalysisWidget.h
	${APPLICATION_DIR}/measure/FillCutAnalysisWidget.cpp

	${APPLICATION_DIR}/measure/ContourWidget.h
	${APPLICATION_DIR}/measure/ContourWidget.cpp

	${APPLICATION_DIR}/measure/ProfileAnalysisWidget.h
	${APPLICATION_DIR}/measure/ProfileAnalysisWidget.cpp

	${APPLICATION_DIR}/measure/FloodAnalysisWidget.h
	${APPLICATION_DIR}/measure/FloodAnalysisWidget.cpp

	${APPLICATION_DIR}/measure/MeasureDisplayWidget.h
	${APPLICATION_DIR}/measure/MeasureDisplayWidget.cpp
	
	)
SOURCE_GROUP(Measure FILES ${MEASURE_H_CPP})

SET(MEASURE_UI
	${APPLICATION_DIR}/measure/DistanceWidget.ui
	${APPLICATION_DIR}/measure/BuildingsViewWidget.ui
	${APPLICATION_DIR}/measure/FillCutAnalysisWidget.ui	
	${APPLICATION_DIR}/measure/ContourWidget.ui
	${APPLICATION_DIR}/measure/ProfileAnalysisWidget.ui
	${APPLICATION_DIR}/measure/FloodAnalysisWidget.ui
	)

if (Qt5Widgets_FOUND)
	qt5_wrap_ui(UICS ${MEASURE_UI})
else()
	qt4_wrap_ui(UICS ${MEASURE_UI})
endif()
SOURCE_GROUP(Measure FILES ${MEASURE_UI})

###########################################################################
#                                                                         #
#                            VIEWPOINT									  #
#                                                                         #
###########################################################################
SET(VIEW_POINT_H_CPP
	${APPLICATION_DIR}/viewPoint/ViewPointPluginInterface.h
	${APPLICATION_DIR}/viewPoint/ViewPointPluginInterface.cpp
	${APPLICATION_DIR}/viewPoint/LocateWidget.h
	${APPLICATION_DIR}/viewPoint/LocateWidget.cpp
	${APPLICATION_DIR}/viewPoint/ViewPointTree.h
	${APPLICATION_DIR}/viewPoint/ViewPointTree.cpp
	${APPLICATION_DIR}/viewPoint/ViewPointDialog.h
	${APPLICATION_DIR}/viewPoint/ViewPointDialog.cpp
	${APPLICATION_DIR}/viewPoint/ViewPointSerializer.h
	${APPLICATION_DIR}/viewPoint/ViewPointSerializer.cpp

	)
SOURCE_GROUP(ViewPoint FILES ${VIEW_POINT_H_CPP})

SET(VIEW_POINT_UI
	${APPLICATION_DIR}/viewPoint/LocateWidget.ui
	${APPLICATION_DIR}/viewPoint/ViewPointDialog.ui
	)

if (Qt5Widgets_FOUND)
	qt5_wrap_ui(UICS ${VIEW_POINT_UI})
else()
	qt4_wrap_ui(UICS ${VIEW_POINT_UI})
endif()
SOURCE_GROUP(ViewPoint FILES ${VIEW_POINT_UI})

###########################################################################
#                                                                         #
#                            LIB_MOC 									  #
#                                                                         #
###########################################################################
set(LIB_MOC_HDRS
	${APPLICATION_DIR}/mainWindow/BootScreenFrame.h
	${APPLICATION_DIR}/mainWindow/BootScreenRoundBar.h
	${APPLICATION_DIR}/mainWindow/BootScreenBallPath.h

	${APPLICATION_DIR}/mainWindow/FreeMainWindow.h
	
	${APPLICATION_DIR}/mainWindow/FolderWidget.h

	${APPLICATION_DIR}/mainWindow/FreeUtil.h
	${APPLICATION_DIR}/mainWindow/FreeSceneWidget.h
	${APPLICATION_DIR}/mainWindow/FreeMenuWidget.h
	${APPLICATION_DIR}/mainWindow/FreeToolBar.h
	${APPLICATION_DIR}/mainWindow/FreeFramelessWidget.h
	${APPLICATION_DIR}/mainWindow/FreeToolButton.h
	${APPLICATION_DIR}/mainWindow/FreeStatusBar.h
	${APPLICATION_DIR}/mainWindow/FreeServiceDockTree.h
	${APPLICATION_DIR}/mainWindow/FreeServiceButton.h
	${APPLICATION_DIR}/mainWindow/FreeDockFrame.h
	${APPLICATION_DIR}/mainWindow/FreeDockWidget.h
	${APPLICATION_DIR}/mainWindow/FreeTreeWidget.h
	${APPLICATION_DIR}/mainWindow/CustomPushButton.h
	${APPLICATION_DIR}/mainWindow/Free3DDockTreeWidget.h
	${APPLICATION_DIR}/mainWindow/FreeLineEdit.h

	${APPLICATION_DIR}/mainWindow/FreeDialog.h
	${APPLICATION_DIR}/mainWindow/SimicircleMenu.h

	${APPLICATION_DIR}/layer/SystemLayerTree.h
	${APPLICATION_DIR}/layer/ImageLayerAttributeDialog.h
	${APPLICATION_DIR}/layer/CustomLayerTree.h
	${APPLICATION_DIR}/layer/ImageLayerDialog.h
	${APPLICATION_DIR}/layer/ElevationLayerDialog.h
	${APPLICATION_DIR}/layer/VectorLayerDialog.h
	${APPLICATION_DIR}/layer/RGBColorFilterDialog.h
	
	${APPLICATION_DIR}/layer/PropertyImageLayerDialog.h
	
	


	${APPLICATION_DIR}/view/ViewPluginInterface.h

	${APPLICATION_DIR}/service/ServicePluginInterface.h
	${APPLICATION_DIR}/service/OGCServiceWidget.h
	${APPLICATION_DIR}/service/TileServiceWidget.h
	${APPLICATION_DIR}/service/DataServiceWidget.h
	${APPLICATION_DIR}/service/OGCServicePropertyWidget.h
	${APPLICATION_DIR}/service/OGCLayerPropertyWidget.h
	${APPLICATION_DIR}/service/ServiceLayerPropertyWidget.h
	${APPLICATION_DIR}/service/LocalDiskTree.h

	${APPLICATION_DIR}/tool/ToolPluginInterface.h
	${APPLICATION_DIR}/tool/LanguageWidget.h
	${APPLICATION_DIR}/tool/BlackBoard.h
	${APPLICATION_DIR}/tool/BlackBoardControlBtns.h
	${APPLICATION_DIR}/tool/DropScreen.h

	${APPLICATION_DIR}/measure/MeasurePluginInterface.h
	${APPLICATION_DIR}/measure/DistanceWidget.h
	${APPLICATION_DIR}/measure/BuildingsViewWidget.h
	${APPLICATION_DIR}/measure/FillCutAnalysisWidget.h
	${APPLICATION_DIR}/measure/ContourWidget.h	
	${APPLICATION_DIR}/measure/ProfileAnalysisWidget.h
	${APPLICATION_DIR}/measure/FloodAnalysisWidget.h

	${APPLICATION_DIR}/mark/PropertyWidget.h
	${APPLICATION_DIR}/mark/BaseStyleWidget.h
	${APPLICATION_DIR}/mark/ViewPointStyleWidget.h
	${APPLICATION_DIR}/mark/DescribeStyleWidget.h
	${APPLICATION_DIR}/mark/PositionStyleWidget.h
	${APPLICATION_DIR}/mark/ModelStyleWidget.h
	${APPLICATION_DIR}/mark/LineStyleWidget.h
	${APPLICATION_DIR}/mark/BillboardStyleWidget.h
	${APPLICATION_DIR}/mark/OverLayStyleWidget.h
	${APPLICATION_DIR}/mark/MarkPluginInterface.h
	${APPLICATION_DIR}/mark/ArcStyleWidget.h
	${APPLICATION_DIR}/mark/EllipseStyleWidget.h
	${APPLICATION_DIR}/mark/RectStyleWidget.h
	${APPLICATION_DIR}/mark/SectorStyleWidget.h
	${APPLICATION_DIR}/mark/CircleStyleWidget.h
	${APPLICATION_DIR}/mark/TextStyleWidget.h
	${APPLICATION_DIR}/mark/TiltModelStyleWidget.h
	${APPLICATION_DIR}/mark/LabelStyleWidget.h
	${APPLICATION_DIR}/mark/MarkMenuMgr.h
	${APPLICATION_DIR}/mark/MarkSdkEventCapture.h
	${APPLICATION_DIR}/mark/ParticleStyleWidget.h
	
	${APPLICATION_DIR}/plot/PlotPluginInterface.h
	${APPLICATION_DIR}/plot/PlotMenuManager.h
	${APPLICATION_DIR}/plot/PlotEventListener.h
		
	${APPLICATION_DIR}/environment/EnvironmentPluginInterface.h
	${APPLICATION_DIR}/environment/RainSetWidget.h
	${APPLICATION_DIR}/environment/SnowSetWidget.h
	${APPLICATION_DIR}/environment/OceanSetWidget.h
	${APPLICATION_DIR}/environment/SettingWidget.h
	${APPLICATION_DIR}/environment/LightSetWidget.h
	${APPLICATION_DIR}/environment/TimeSetWidget.h
	${APPLICATION_DIR}/environment/SilverLiningSetWidget.h
	${APPLICATION_DIR}/environment/FogSetWidget.h

	${APPLICATION_DIR}/viewPoint/ViewPointPluginInterface.h
	${APPLICATION_DIR}/viewPoint/LocateWidget.h
	${APPLICATION_DIR}/viewPoint/ViewPointTree.h
	${APPLICATION_DIR}/viewPoint/ViewPointDialog.h

	${APPLICATION_DIR}/help/HelpMenu.h
	${APPLICATION_DIR}/help/AboutWidget.h
	)

IF(Qt5Widgets_FOUND)
    SET(LIB_MOC_SRCS)
    FOREACH( LIB_MOC_HDR ${LIB_MOC_HDRS} )
        GET_FILENAME_COMPONENT( LIB_MOC_HDR_ABS ${LIB_MOC_HDR} ABSOLUTE )
        QT5_WRAP_CPP( LIB_MOC_SRC ${LIB_MOC_HDR} OPTIONS "-f${LIB_MOC_HDR_ABS}" )
        LIST( APPEND LIB_MOC_SRCS ${LIB_MOC_SRC} )
    ENDFOREACH()
ELSE()
    INCLUDE( ${QT_USE_FILE} )
    QT4_WRAP_CPP( LIB_MOC_SRCS ${LIB_MOC_HDRS} OPTIONS "-f" )
ENDIF()
set(CMAKEQT_SRCS "")

if(WIN32)
set(RES ${APPLICATION_RESOURCE_DIR}/icon.res)
endif()

add_executable(
	${OUT_NAME}
	${LIB_MOC_HDRS}
    ${LIB_MOC_SRCS}

	${CMAKEQT_SRCS}

	${MAIN_WINDOW_H_CPP}
	${MAIN_WINDOW_MOCS_CPP}
	${MAIN_WINDOW_UI}
	${MAIN_WINDOW_RESOURCE}
	${MAIN_WINDOW_MOCS_RESOURCE}

	#${SYSTEM_H_CPP}
	#${SYSTEM_MOCS_CPP}

	${LAYER_H_CPP}
	${LAYER_UI}
	${LAYER_MOCS_CPP}

	${VIEW_H_CPP}
	${VIEW_UI}

	${VIEW_POINT_H_CPP}
	${VIEW_POINT_MOCS_CPP}
	${VIEW_POINT_UI}
	
	${ENVIRONMENT_H_CPP}
	${ENVIRONMENT_MOCS_CPP}
	${ENVIRONMENT_UI}
	
	${TOOL_H_CPP}
	${TOOL_MOCS_CPP}
	${TOOL_UI}
	
	${MARK_H_CPP}
	${MARK_MOCS_CPP}
	${MARK_UI}

	${PLOT_H_CPP}
	${PLOT_MOCS_CPP}
	${PLOT_UI}

	${SERVICE_H_CPP}
	${SERVICE_UI}

	${MEASURE_H_CPP}
	${MEASURE_MOCS_CPP}
	${MEASURE_UI}

	${HELP_H_CPP}
	${HELP_UI}
	)

if (Qt5Widgets_FOUND)
	target_link_libraries(
		${OUT_NAME}
		${RES}
        Qt5::OpenGL
        Qt5::Widgets
		Qt5::Network
		Qt5::Test
		Qt5::Xml
		debug ${QWT_DEBUG_LIBRARIES} optimized ${QWT_RELEASE_LIBRARIES} 
		
		${OPENSCENEGRAPH_LIBRARIES}

        debug ${osgEarth_DEBUG_LIBRARIES}     optimized ${osgEarth_RELEASE_LIBRARIES}
        debug ${osgEarthUtil_DEBUG_LIBRARIES} optimized ${osgEarthUtil_RELEASE_LIBRARIES}
		
		debug ${FeUtils_DEBUG_LIBRARIES} optimized ${FeUtils_RELEASE_LIBRARIES}
		debug ${FeShell_DEBUG_LIBRARIES} optimized ${FeShell_RELEASE_LIBRARIES}
		debug ${FeEarth_DEBUG_LIBRARIES} optimized ${FeEarth_RELEASE_LIBRARIES}
		debug ${FeMeasure_DEBUG_LIBRARIES} optimized ${FeMeasure_RELEASE_LIBRARIES}
		debug ${FeKits_DEBUG_LIBRARIES} optimized ${FeKits_RELEASE_LIBRARIES}
		debug ${FeExtNode_DEBUG_LIBRARIES} optimized ${FeExtNode_RELEASE_LIBRARIES}
		debug ${FePlots_DEBUG_LIBRARIES} optimized ${FePlots_RELEASE_LIBRARIES}
		debug ${FeManager_DEBUG_LIBRARIES} optimized ${FeManager_RELEASE_LIBRARIES}
		debug ${FeLayers_DEBUG_LIBRARIES} optimized ${FeLayers_RELEASE_LIBRARIES}

		debug ${FeOcean_DEBUG_LIBRARIES} optimized ${FeOcean_RELEASE_LIBRARIES}
		debug ${FeSilverlining_DEBUG_LIBRARIES} optimized ${FeSilverlining_RELEASE_LIBRARIES} 

		debug ${FeServiceProvider_DEBUG_LIBRARIES} optimized ${FeServiceProvider_RELEASE_LIBRARIES} 
	) 
else()
    target_link_libraries(
        ${OUT_NAME}
		${RES}
        ${QT_LIBRARIES}
        debug ${QWT_DEBUG_LIBRARIES}		optimized ${QWT_RELEASE_LIBRARIES}
			     
        debug ${OPENSCENEGRAPH_LIBRARIES}	optimized	  ${OPENSCENEGRAPH_LIBRARIES}

        debug ${osgEarth_DEBUG_LIBRARIES}     optimized ${osgEarth_RELEASE_LIBRARIES}
        debug ${osgEarthUtil_DEBUG_LIBRARIES} optimized ${osgEarthUtil_RELEASE_LIBRARIES}
		
		debug ${FeUtils_DEBUG_LIBRARIES} optimized ${FeUtils_RELEASE_LIBRARIES}
		debug ${FeShell_DEBUG_LIBRARIES} optimized ${FeShell_RELEASE_LIBRARIES}
		debug ${FeEarth_DEBUG_LIBRARIES} optimized ${FeEarth_RELEASE_LIBRARIES}
		debug ${FeMeasure_DEBUG_LIBRARIES} optimized ${FeMeasure_RELEASE_LIBRARIES}
		debug ${FeKits_DEBUG_LIBRARIES} optimized ${FeKits_RELEASE_LIBRARIES}
		debug ${FeExtNode_DEBUG_LIBRARIES} optimized ${FeExtNode_RELEASE_LIBRARIES}
		debug ${FePlots_DEBUG_LIBRARIES} optimized ${FePlots_RELEASE_LIBRARIES}
		debug ${FeManager_DEBUG_LIBRARIES} optimized ${FeManager_RELEASE_LIBRARIES}
		debug ${FeLayers_DEBUG_LIBRARIES} optimized ${FeLayers_RELEASE_LIBRARIES}

		debug ${FeOcean_DEBUG_LIBRARIES} optimized ${FeOcean_RELEASE_LIBRARIES}
		debug ${FeSilverlining_DEBUG_LIBRARIES} optimized ${FeSilverlining_RELEASE_LIBRARIES} 
		debug ${FeServiceProvider_DEBUG_LIBRARIES} optimized ${FeServiceProvider_RELEASE_LIBRARIES} 
        )
endif()
