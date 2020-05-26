
#include <FeUtils/PathRegistry.h>
#include <FeUtils/EnvironmentVariableReader.h>
#include <FreeMainWindow.h>

#include <FeLayers/LayerSys.h>
#include "osgEarthUtil/ContourMap"

FeShell::CSystemManager* g_pSysMgr = NULL;

class CMapControlHandler : public osgGA::GUIEventHandler
{
public:
	CMapControlHandler(FeLayers::CLayerSys* pFreeMap) 
		: m_opFreeMap(pFreeMap)
	{
	}

	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) 
	{
		if(ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
		{
			if(!m_opFreeMap.valid()) return false;

			switch (ea.getKey())
			{
			case osgGA::GUIEventAdapter::KEY_0:
				{
					
				}break;

			case osgGA::GUIEventAdapter::KEY_1:
				{
					FeLayers::CLayerSys* pLayerSys = m_opFreeMap.get();

					// 本地图层
					{
						static int s_nLayerIndex = 0;
						std::vector<FeLayers::CFeLayerConfig> s_mapLayers;

						// 影像 flat
						FeLayers::CFeLayerImageConfig cfgImgFlat("影像 flat", 
						FeLayers::DriverType::DRIVER_FLATGIS,
						//"G:/DATA/Image_color6_11/");
						"http://192.168.1.114:8085/data/Satellite/");
						s_mapLayers.push_back(cfgImgFlat);

						// 高程
						FeLayers::CFeLayerElevationConfig cfgElevation("高程图层",
							FeLayers::DriverType::DRIVER_FLATGIS,
							"http://192.168.1.114:8085/data/Dem/");
						s_mapLayers.push_back(cfgElevation);

						// 矢量 shp
						FeLayers::CFeLayerModelConfig cfgModelShp("矢量 shp", 
						FeLayers::DriverType::DRIVER_OGR,
						//"D:/0RaymWork/products/FreeStudio/FreeData/trunk/shpFile/china/point_xiangzhen.shp")
						//"D:/0RaymWork/products/FreeStudio/FreeData/trunk/shpFile/china/point_cun.shp");
						//"G:/DATA/vector/KML_Samples.kml");
						//"G:/DATA/vector/陕西省.kml");
						//"G:/DATA/vector/1/line_sheng.shp");
						"D:/0RaymWork/products/FreeStudio/FreeData/trunk/shpFile/world/line_world.shp");
						//s_mapLayers.push_back(cfgModelShp);

						// 矢量 kml
						FeLayers::CFeLayerModelConfig cfgModelKml("矢量 kml", 
						FeLayers::DriverType::DRIVER_OGR,
						"G:/DATA/vector/陕西省.kml");
						//"G:/DATA/vector/kml/QingHai.kml");
						//"G:/DATA/vector/KML_Samples.kml");
						//s_mapLayers.push_back(cfgModelKml);

						// 特效
						FeLayers::CFeLayerEffectConfig cfgEffect;
						cfgEffect.name() = "眩晕特效";
						osg::ref_ptr<osgEarth::Util::ContourMap> rpContourMap = new osgEarth::Util::ContourMap();
						cfgEffect.SetTerrainEffect(rpContourMap);
						cfgEffect.SetTerrainEngineNode(g_pSysMgr->GetRenderContext()->GetMapNode()->getTerrainEngine());
						s_mapLayers.push_back(cfgEffect);

						// 影像 tif
// 							FeLayers::CFeLayerConfig("imageTif_world", 
// 							//FeLayers::DriverType::DRIVER_GDAL,
// 							FeLayers::DriverType::DRIVER_XYZ,
// 							//"arcgis",
// 							//"G:/DATA/globel.tif" );
// 							//"http://webst0[1234].is.autonavi.com/appmaptile?style=6&x={x}&y={y}&z={z}" );
// 							//"http://webrd0[1234].is.autonavi.com/appmaptile?lang=zh_cn&size=1&scale=1&style=7&x={x}&y={y}&z={z}" );
// 							//"http://map.geoq.cn/ArcGIS/rest/services/ChinaOnlineCommunity/MapServer/" );
// 							//"http://mt[0123].google.cn/vt/lyrs=p&hl=zh-CN&gl=cn&&x={x}&y={y}&z={z}");
// 							//"http://t[01234567].tianditu.com/img_w/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=img&tileMatrixSet=w&TileMatrix={z}&TileRow={y}&TileCol={x}&style=default&format=tiles");
// 							"http://online1.map.bdimg.com/onlinelabel/?qt=tile&styles=pl&udt=20160804&scaler=1&p=1&x={x}&y={y}&z={z}")


						//cfgFlat.cachePath() = "g:/DATA/freemap_cache";
						//pLayerSys->AddLayer(cfgFlat);//->setOpacity(0.9);
						
						//cfgGdal.sourceConfig().layerFormat() = "jpeg";
						//cfgGdal.cachePath() = "g:/DATA/freemap_cache";
						//cfgGdal.cacheEnable() = false;
						//cfgGdal.baiduOnline() = true;
						//pLayerSys->AddLayer(cfgGdal);

						//cfgOgr.featureSymbol().AddFeatureType(FeLayers::CFeFeatureSymbol::FT_POLYGON);
						// 						cfgOgr.maxVisibleRange() = 1000000000;
						// 						cfgOgr.minVisibleRange() = 1000000;
						//FeLayers::IFeLayer* pModelLayer = pLayerSys->AddLayer(cfgOgr);
						//pModelLayer->AsModelLayer()->SetFontName("D:/0RaymWork/products/FreeStudio/FreeData/trunk/fonts/msyh.ttf");// ->SetLineColor(osg::Vec4f(1.0,1.0,1.0,1.0));
						//pModelLayer->AsModelLayer()->SetPointIconPath("D:/0RaymWork/products/FreeStudio/FreeData/trunk/texture/icon/capitalicon.png");

						// 					cfgOgr.featureSymbol().AddFeatureType(FeLayers::CFeFeatureSymbol::FT_POINT);
						// 					cfgOgr.featureSymbol().AddFeatureType(FeLayers::CFeFeatureSymbol::FT_POLYGON);
						//pLayerSys->AddLayer(cfgOgr);
						
							
 						//cfgGdal.sourceConfig().layerFormat() = "png";
 						//cfgGdal.cachePath() = "g:/DATA/freemap_cache";

						//pLayerSys->AddLayer(cfgGdal)->setOpacity(0.6);

						if( s_nLayerIndex == s_mapLayers.size() )
						{
							std::cout << "清空网络图层" << std::endl;
							s_nLayerIndex = 0;
							m_opFreeMap->ClearLayers();
						}
						else
						{
							std::cout << "添加图层：" << s_mapLayers.at(s_nLayerIndex).name().get() << std::endl;
							pLayerSys->AddLayer(s_mapLayers.at(s_nLayerIndex));
							s_nLayerIndex++;
						}
					}

				}break;

			case osgGA::GUIEventAdapter::KEY_2:
				{
					FeLayers::CLayerSys* pLayerSys = m_opFreeMap.get();

					// 网络图层使用
					{
						static int s_nLayerIndex = 0;
						std::vector<FeLayers::CFeLayerConfig> s_mapLayers;

						// WMS
						FeLayers::CFeLayerImageConfig cfgWMS("wmsImage", 
							FeLayers::DriverType::DRIVER_WMS,
							"http://192.168.1.114:8080/freeserver/wms");
						cfgWMS.sourceConfig().layerName() = "test.520:geotiff_coverage";
						// 				cfgWMS.sourceConfig().layerFormat() = "png";
						// 				cfgWMS.sourceConfig().srs() = "EPSG:4326";
						cfgWMS.cacheConfig().cachePath() = "g:/DATA/freemap_cache";
						//s_mapLayers.push_back(cfgWMS);


						// WCS
						FeLayers::CFeLayerImageConfig cfgWCS("wcsImage", 
							FeLayers::DriverType::DRIVER_WCS,
							"http://192.168.1.114:8080/freeserver/wcs");
						cfgWCS.sourceConfig().layerName() = "test.520:geotiff_coverage";
						// 				cfgWCS.sourceConfig().layerFormat() = "jpeg";
						// 				cfgWCS.sourceConfig().srs() = "EPSG:4326";

						s_mapLayers.push_back(cfgWCS);


						// WMTS
						FeLayers::CFeLayerImageConfig cfgWMTS("wmtsImage", 
							FeLayers::DriverType::DRIVER_WMTS,
							"http://192.168.1.114:8080/freeserver/gwc/service/wmts");
						cfgWMTS.sourceConfig().layerName() = "test.520:china_part";
						// 				cfgWMTS.sourceConfig().layerFormat() = "png";
						// 				cfgWMTS.sourceConfig().srs() = "EPSG:4326";
						// 				cfgWMTS.minVisibleRange() = 100;
						// 				cfgWMTS.maxVisibleRange() = 1000000000000;

						s_mapLayers.push_back(cfgWMTS);


						// TMS
						FeLayers::CFeLayerImageConfig cfgTMS("tmsImage", 
							FeLayers::DriverType::DRIVER_TMS,
							"http://192.168.1.114:8080/freeserver/gwc/service/tms/1.0.0/test.520%3Apoint_xian@EPSG%3A4326@png/");
						//cfgTMS.sourceConfig().srs() = "EPSG:4326";
						cfgTMS.sourceConfig().layerFormat() = "png";

						s_mapLayers.push_back(cfgTMS);


						// WFS
						FeLayers::CFeLayerModelConfig cfgWFS("wfsLayer", 
							FeLayers::DriverType::DRIVER_WFS,
							"http://192.168.1.114:8080/freeserver/wfs");
						cfgWFS.sourceConfig().layerName() = "test.520:china";
						// 				cfgWFS.minVisibleRange() = 100000;
						// 				cfgWFS.maxVisibleRange() = 1000000000000;
						//cfgWFS.featureSymbol().AddFeatureType(FeLayers::CFeFeatureSymbol::FT_LINE);
						//pLayerSys->AddLayer(cfgWFS);//->AsModelLayer()->SetFillColor(osg::Vec4f(1.0,0.0,0.0,0.5));
						s_mapLayers.push_back(cfgWFS);


						if( s_nLayerIndex == s_mapLayers.size() )
						{
							std::cout << "清空网络图层" << std::endl;
							s_nLayerIndex = 0;
							m_opFreeMap->ClearLayers();
						}
						else
						{
							std::cout << "添加图层：" << s_mapLayers.at(s_nLayerIndex).name().get() << std::endl;
							pLayerSys->AddLayer(s_mapLayers.at(s_nLayerIndex));
							s_nLayerIndex++;
						}
					}

				}break;

			case osgGA::GUIEventAdapter::KEY_3:
				{
					//m_opFreeMap->Save("d:/freemap.xml");
					//m_opFreeMap->GetMapLayerSys()->ClearLayers();
					//m_opFreeMap->Load("d:/freemap.xml");
				}break;

			case osgGA::GUIEventAdapter::KEY_4:
				{
					FeLayers::CModelLayersSet layers;
					m_opFreeMap->GetModelLayers(layers);

					if( layers.size() > 0 )
					{
						layers.begin()->get()->AsModelLayer()->SetVisibleRange(10000, 100000000);
						layers.begin()->get()->AsModelLayer()->SetLineColor(osg::Vec4f(1.0,1.0,0,1));
						layers.begin()->get()->AsModelLayer()->SetLineWidth(3);
					}
				}break;

			case osgGA::GUIEventAdapter::KEY_5:
				{

				}break;

			case osgGA::GUIEventAdapter::KEY_6:
				{

				}break;
			case osgGA::GUIEventAdapter::KEY_7:
				{

				}break;

			case osgGA::GUIEventAdapter::KEY_8:
				{

				}break;

			case osgGA::GUIEventAdapter::KEY_9:
				{

				}break;
			default:
				break;
			}
		}
		return false; 
	}

private:
	osg::observer_ptr<FeLayers::CLayerSys> m_opFreeMap;
};


CFreeMainWindow::CFreeMainWindow(const std::string& strDataPath, QWidget *parent, Qt::WindowFlags flags)
	:QMainWindow(parent, flags)
	,m_p3DScenePanel(NULL)
{
	m_p3DScenePanel = new FeEarth::C3DSceneWidget(strDataPath);
}

CFreeMainWindow::~CFreeMainWindow()
{
	if(m_p3DScenePanel)
	{		
		delete m_p3DScenePanel;
		m_p3DScenePanel = NULL;
	}
}




bool CFreeMainWindow::Initialize()
{			
	if(m_p3DScenePanel && m_p3DScenePanel->Initialize())
	{
		return InitializeUI();
	}
	
	return false;
}

bool CFreeMainWindow::InitializeUI()
{ 
	//设置系统标题
	setWindowTitle(QString("DongKangLe"));

	setWindowIcon(QIcon(":/texture/icon/othericon.png"));

	setDockNestingEnabled(true);

	statusBar()->setObjectName(tr("StatusBar"));

	setContextMenuPolicy(Qt::NoContextMenu);

	setCentralWidget(m_p3DScenePanel);

	m_p3DScenePanel->show();

	FeLayers::CLayerSysOptions layerSysOpt;
	FeLayers::CLayerSys* pLayers = new FeLayers::CLayerSys(layerSysOpt);
	pLayers->Initialize(m_p3DScenePanel->GetSystemManager()->GetSystemService()->GetRenderContext());
	m_p3DScenePanel->GetSystemManager()->GetSystemService()->AddAppModuleSys(pLayers);

	m_p3DScenePanel->GetSystemManager()->GetSystemService()->GetRenderContext()->AddEventHandler(
		new CMapControlHandler(pLayers));
	g_pSysMgr = m_p3DScenePanel->GetSystemManager();

	return true;
}

bool CFreeMainWindow::UnInitialize()
{
	if(m_p3DScenePanel)
	{
		m_p3DScenePanel->UnInitialize();
		return true;
	}
	return false;
}

bool CFreeMainWindow::eventFilter( QObject *obj, QEvent *event )
{
	return QMainWindow::eventFilter(obj, event);
}


