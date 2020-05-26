#include <environment/EnvironmentPluginInterface.h>

#include <FeUtils/PathRegistry.h>
#include <FeUtils/StrUtil.h>
#include <FeUtils/logger/LoggerDef.h>

#include <FeKits/weather/WeatherEffectNode.h>
#include <FeKits/weather/WeatherHandler.h>
#include <FeKits/sky/FreeSky.h>
#include <FeShell/EnvironmentSys.h>
#include <FeOcean/OceanSys.h>
#include <FeSilverlining/SilverliningSys.h>

#include <environment/SettingWidget.h>

#include <mainWindow/FreeToolBar.h>
#include <osgEarthUtil/Fog>

namespace FreeViewer
{
	CEnvironmentWidgetPlugin::CEnvironmentWidgetPlugin( CFreeMainWindow* pMainWindow )
		: CUIObserver(pMainWindow)
		, m_pTimeSetWidget(NULL)
		, m_pSetWidget(NULL)
		, m_opFreeSky(NULL)
		, m_rpOceanNode(NULL)
		, m_rpRainNode(NULL)
		, m_rpSnowNode(NULL)
		, m_rpSilverLiningNode(NULL)
		, m_pLightAction(NULL)
		, m_pAtmosphereAction(NULL)
		, m_pRainAction(NULL)
		, m_pSnowAction(NULL)
		, m_pStarsAction(NULL)
		, m_pCloudAction(NULL)
		, m_pOceanAction(NULL)
		,m_pFogAction(NULL)
		,m_rpFog(NULL)
	{
		m_strMenuTitle = tr("Environment");

		LoadData();

		InitWidget();
	}

	CEnvironmentWidgetPlugin::~CEnvironmentWidgetPlugin()
	{
		if (m_pSetWidget)
		{
			delete m_pSetWidget;
			m_pSetWidget = NULL;
		}

		m_opFreeSky = NULL;
		m_rpRainNode = NULL;
		m_rpSnowNode = NULL;
		m_rpOceanNode = NULL;
	}

	void CEnvironmentWidgetPlugin::InitWidget()
	{
		if (m_opSystemService.valid())
		{
			osg::observer_ptr<FeShell::CEnvironmentSys> pEnvironmentSys =  m_opSystemService->GetEnvironmentSys();
			if(pEnvironmentSys.valid())
			{
				//星系光照
				m_opFreeSky = pEnvironmentSys->GetSkyNode();
				//初始化光照
				m_opFreeSky->SetSunVisible(m_sEnvSetData.m_sEnvLightSetData.bSun);
				float fValue = (m_sEnvSetData.m_sEnvLightSetData.fDensity) / 255.0;
				m_opFreeSky->SetAmbient(osg::Vec4(fValue, fValue, fValue, 1));
				//初始化大气层
				m_opFreeSky->SetAtmosphereVisible(m_sEnvSetData.m_sEnvLightSetData.bAtmosphere);
				//初始化月球
				m_opFreeSky->SetMoonVisible(m_sEnvSetData.m_sEnvLightSetData.bMoon);
				//初始化星空
				m_opFreeSky->SetStarsVisible(m_sEnvSetData.m_sEnvLightSetData.bStar);
				//初始化星云背景
				m_opFreeSky->SetNebulaVisible(m_sEnvSetData.m_sEnvLightSetData.bNebula);

				//注册天气
				m_rpRainNode = new FeKit::CWeatherEffectNode(m_opSystemService->GetRenderContext());
				std::string strRain = "Rain";
				//初始化雨
				if (m_sEnvSetData.m_sEnvRainSetData.bRain)
				{
					m_rpRainNode->Show();
				}
				else
				{
					m_rpRainNode->Hide();
				}
				m_rpRainNode->SetWorld(m_sEnvSetData.m_sEnvRainSetData.bGlobal);
				m_rpRainNode->SetWeatherPosition(
					osg::Vec3d(m_sEnvSetData.m_sEnvRainSetData.fLongitude,
					m_sEnvSetData.m_sEnvRainSetData.fLatitude,
					m_sEnvSetData.m_sEnvRainSetData.fRadius),
					!m_sEnvSetData.m_sEnvRainSetData.bGlobal);
				m_rpRainNode->SetWeatherHeight(m_sEnvSetData.m_sEnvRainSetData.fheigth); //设置局部雨雪的范围以及云的高度
				m_rpRainNode->rain(m_sEnvSetData.m_sEnvRainSetData.dDensity);
				m_rpRainNode->SetWeatherType(FeKit::HEAVY_RAIN);
				m_rpRainNode->SetKey(strRain);
				m_rpRainNode->SetTitle(strRain);
				pEnvironmentSys->AddSkyTool(m_rpRainNode.get());

				//注册天气
				m_rpSnowNode = new FeKit::CWeatherEffectNode(m_opSystemService->GetRenderContext());
				std::string strSnow = "Snow";
				//初始化雪
				if (m_sEnvSetData.m_sEnvSnowSetData.bSnow)
				{
					m_rpSnowNode->Show();
				}
				else
				{
					m_rpSnowNode->Hide();
				}
				m_rpSnowNode->SetWorld(m_sEnvSetData.m_sEnvSnowSetData.bGlobal);
				m_rpSnowNode->SetWeatherPosition(
					osg::Vec3d(m_sEnvSetData.m_sEnvSnowSetData.fLongitude,
					m_sEnvSetData.m_sEnvSnowSetData.fLatitude,
					m_sEnvSetData.m_sEnvSnowSetData.fRadius),
					!m_sEnvSetData.m_sEnvSnowSetData.bGlobal);
				m_rpSnowNode->SetWeatherHeight(m_sEnvSetData.m_sEnvSnowSetData.fheigth); //设置局部雨雪的范围以及云的高度
				m_rpSnowNode->snow(m_sEnvSetData.m_sEnvSnowSetData.dDensity);
				m_rpSnowNode->SetWeatherType(FeKit::HEAVY_SNOW); 
				m_rpSnowNode->SetKey(strSnow);
				m_rpSnowNode->SetTitle(strSnow);
				pEnvironmentSys->AddSkyTool(m_rpSnowNode.get());
				//初始化雾
				m_rpFog = new osg::Fog;
				m_rpFog->setDensity(m_sEnvSetData.m_sEnvFogSetData.dDensity);
				osg::Vec4 fogColor(0.66f, 0.7f, 0.81f, 1.0f); 
				m_rpFog->setColor(fogColor);
				m_rpFog->setMode(osg::Fog::EXP);
				m_rpFog->setStart(10);
				m_rpFog->setEnd(50);


				//注册海洋
				std::string strTritonRSPath = FeFileReg->GetFullPath("triton/data");
				FeOcean::COceanSys* pOceanSys = new FeOcean::COceanSys(strTritonRSPath);

				if (pOceanSys->Initialize(m_opSystemService->GetRenderContext()))
				{
					if(m_opSystemService->AddAppModuleSys(pOceanSys))
					{
						if (m_opFreeSky.valid())
						{
							pOceanSys->SetAtmosphere(m_opFreeSky->GetAtmosphereNode());
							pOceanSys->SetSunAndMoonNode(m_opFreeSky->GetSunNode(), m_opFreeSky->GetMoonNode());
						}

						m_rpOceanNode = pOceanSys->GetOceanNode();
						if (m_sEnvSetData.m_sEnvOceanSetData.bOcean)
						{
							m_rpOceanNode->Show();
						}
						else
						{
							m_rpOceanNode->Hide();
						}
						m_rpOceanNode->SetSeaLevel(m_sEnvSetData.m_sEnvOceanSetData.fOceanLevel);
						m_rpOceanNode->SetWindSpeed(m_sEnvSetData.m_sEnvOceanSetData.fWindSpeed);
						m_rpOceanNode->SetWindDirection(m_sEnvSetData.m_sEnvOceanSetData.fWindDirection);
						m_rpOceanNode->SetWindLength(m_sEnvSetData.m_sEnvOceanSetData.fWindLength);
						//m_rpOceanNode->SetWave(m_sEnvSetData.m_sEnvOceanSetData.fWindSpeed, m_sEnvSetData.m_sEnvOceanSetData.fWindDirection, m_sEnvSetData.m_sEnvOceanSetData.fWindLength);
					}
				}

					//注册云层
					//std::string strSilverLiningRSPath = FeFileReg->GetFullPath("silverLining/data");
					//osg::ref_ptr<FeSilverLining::CSilverLiningSys> rpSilverLiningSys = new FeSilverLining::CSilverLiningSys(strSilverLiningRSPath);
					////传递光照，为云层设置系统光照 h00021 2016-10-17
					//rpSilverLiningSys->SetLight(m_opFreeSky->GetLight());
					////传递相机，为云层设置系统光照 h00021 2016-10-20
					//rpSilverLiningSys->SetCamera(m_opSystemService->GetRenderContext()->GetCamera());
					//if (rpSilverLiningSys->Initialize(m_opSystemService->GetRenderContext()))
					//{
					//	if (m_opSystemService->AddAppModuleSys(rpSilverLiningSys.get()))
					//	{
					//		m_rpSilverLiningNode = rpSilverLiningSys->GetSilverLiningNode();
					//	}
					//	m_rpSilverLiningNode->SetCloudType(m_sEnvSetData.m_sEnvCloudSetData.nCloudType);
					//	m_rpSilverLiningNode->SetCloudsShow(m_sEnvSetData.m_sEnvCloudSetData.bCloud);
					//	m_rpSilverLiningNode->SetCloudsAlpha(m_sEnvSetData.m_sEnvCloudSetData.dAlpha);
					//	m_rpSilverLiningNode->SetCloudsAltitude(m_sEnvSetData.m_sEnvCloudSetData.nAltitude);
					//	m_rpSilverLiningNode->SetCloudsDensity(m_sEnvSetData.m_sEnvCloudSetData.dDensity);
					//	m_rpSilverLiningNode->SetCloudsThickness(m_sEnvSetData.m_sEnvCloudSetData.nThickness);
					//	m_rpSilverLiningNode->SetWindSpeed(m_sEnvSetData.m_sEnvCloudSetData.nSpeed);
					//	m_rpSilverLiningNode->SetWindDirection(m_sEnvSetData.m_sEnvCloudSetData.nDirection);

					//	osgEarth::DateTime datatime(2016, 8, 11, 4.14);
					//	m_rpSilverLiningNode->SetLocalDateTime(datatime);
					//}
			}

			if(m_pMainWindow)
			{
				if (m_pSetWidget == NULL)
				{
					m_pSetWidget = new CSettingWidget(this, m_pMainWindow);
				}
				BuildContext();
			}
		}
	}

	void CEnvironmentWidgetPlugin::BuildContext()
	{
		BuildSunSystem();			
		BuildWeatherSystem();
		BuildCloudSystem();
		BuildOceanSystem();
		BuildFogSystem();
		BuildSetSystem();

		//添加工具条分割线
		CFreeToolBar* pToolBar = GetToolBar();
		pToolBar->AddSperator();
	}

	void CEnvironmentWidgetPlugin::LoadData()
	{
		FeShell::CEnvConfigReader reader;
		m_sEnvSetData = reader.Execute(FeFileReg->GetFullPath("config/EnvironmentConifg.xml"));
	}

	void CEnvironmentWidgetPlugin::SaveData()
	{
		FeShell::CEnvConfigWriter writer;
		writer.Execute(m_sEnvSetData, FeFileReg->GetFullPath("config/EnvironmentConifg.xml"));
	}

	FeShell::CEnvSetData& CEnvironmentWidgetPlugin::GetSetData()
	{
		return m_sEnvSetData;
	}

	void CEnvironmentWidgetPlugin::LoadResetData()
	{
		//光照
		m_sEnvSetData.m_sEnvLightSetData.bSun        = false;
		m_sEnvSetData.m_sEnvLightSetData.bAtmosphere = true;
		m_sEnvSetData.m_sEnvLightSetData.bMoon       = true;
		m_sEnvSetData.m_sEnvLightSetData.bStar       = true;
		m_sEnvSetData.m_sEnvLightSetData.bNebula     = true;

		m_sEnvSetData.m_sEnvLightSetData.fDensity    = 127;

		//雨
		m_sEnvSetData.m_sEnvRainSetData.bRain = false;
		m_sEnvSetData.m_sEnvRainSetData.bGlobal	= true;

		m_sEnvSetData.m_sEnvRainSetData.dDensity	= 0.5;

		m_sEnvSetData.m_sEnvRainSetData.fheigth	= 20000;
		m_sEnvSetData.m_sEnvRainSetData.fLongitude = 0.0;
		m_sEnvSetData.m_sEnvRainSetData.fLatitude = 0.0;
		m_sEnvSetData.m_sEnvRainSetData.fRadius	= 100000;

		//雪
		m_sEnvSetData.m_sEnvSnowSetData.bSnow = false;
		m_sEnvSetData.m_sEnvSnowSetData.bGlobal		= true;

		m_sEnvSetData.m_sEnvSnowSetData.dDensity		= 0.5;

		m_sEnvSetData.m_sEnvSnowSetData.fheigth		= 20000;
		m_sEnvSetData.m_sEnvSnowSetData.fLongitude	= 0.0;
		m_sEnvSetData.m_sEnvSnowSetData.fLatitude	= 0.0;
		m_sEnvSetData.m_sEnvSnowSetData.fRadius		= 100000;

		//雾
		m_sEnvSetData.m_sEnvFogSetData.bFog = false;
		m_sEnvSetData.m_sEnvFogSetData.dDensity		= 0.5;

		m_sEnvSetData.m_sEnvFogSetData.dDensity		= 0;
		/*		m_sEnvSetData.m_sEnvFogSetData.fheigth		= 20000;*/
		m_sEnvSetData.m_sEnvFogSetData.fLongitude	= 0.0;
		m_sEnvSetData.m_sEnvFogSetData.fLatitude	= 0.0;
		m_sEnvSetData.m_sEnvFogSetData.fRadius		= 100000;

		//云
		m_sEnvSetData.m_sEnvCloudSetData.bCloud		= false;
		m_sEnvSetData.m_sEnvCloudSetData.nAltitude	= 8000;
		m_sEnvSetData.m_sEnvCloudSetData.nThickness	= 200;
		m_sEnvSetData.m_sEnvCloudSetData.nDensity	= 60;
		m_sEnvSetData.m_sEnvCloudSetData.dDensity	= 0.6;
		m_sEnvSetData.m_sEnvCloudSetData.nAlpha		= 80;
		m_sEnvSetData.m_sEnvCloudSetData.dAlpha		= 0.8;
		m_sEnvSetData.m_sEnvCloudSetData.nSpeed		= 10;
		m_sEnvSetData.m_sEnvCloudSetData.nDirection	= 45;
		m_sEnvSetData.m_sEnvCloudSetData.nCloudType	= 3;

		//海洋
		m_sEnvSetData.m_sEnvOceanSetData.bOcean = false;
		m_sEnvSetData.m_sEnvOceanSetData.fOceanLevel = 0;
		m_sEnvSetData.m_sEnvOceanSetData.fWindSpeed = 10;
		m_sEnvSetData.m_sEnvOceanSetData.fWindDirection = 0;
		m_sEnvSetData.m_sEnvOceanSetData.fWindLength = 0.0;
	}

	void CEnvironmentWidgetPlugin::BuildSunSystem()
	{
		if( m_opFreeSky.valid())
		{
			//光照开关
			m_pLightAction = CreateMenuAndToolAction(
				QString(tr("Light")),
				QString(":/images/icon/sun_light.png"),
				QString(":/images/icon/sun_light.png"), 
				QString(":/images/icon/sun_light_press.png"),
				QString(":/images/icon/sun_light.png"),
				true,
				m_opFreeSky->GetSunVisible()
				);
			connect(m_pLightAction, SIGNAL(triggered(bool)), this, SLOT(SlotShowLight(bool)));

			//大气层开关
			m_pAtmosphereAction = CreateMenuAndToolAction(
				QString(tr("Atmosphere")),
				QString(":/images/icon/atmosphere.png"),
				QString(":/images/icon/atmosphere.png"), 
				QString(":/images/icon/atmosphere_press.png"),
				QString(":/images/icon/atmosphere.png"),
				true,
				m_opFreeSky->GetAtmosphereVisible()
				);
			connect(m_pAtmosphereAction, SIGNAL(triggered(bool)), this, SLOT(SlotShowAtmosphere(bool)));
		}
	}

	void CEnvironmentWidgetPlugin::BuildWeatherSystem()
	{		
		//雨效果
		if(m_rpRainNode.valid())
		{
			m_pRainAction = CreateMenuAndToolAction(
				QString(tr("Rain")),
				QString(":/images/icon/rain.png"),
				QString(":/images/icon/rain.png"), 
				QString(":/images/icon/rain_press.png"),
				QString(":/images/icon/rain.png"),
				true,
				!m_rpRainNode->IsHide()
				);
			connect(m_pRainAction, SIGNAL(triggered(bool)), this, SLOT(SlotShowRain(bool)));
		}

		//雪效果
		if(m_rpSnowNode.valid())
		{
			m_pSnowAction = CreateMenuAndToolAction(
				QString(tr("Snow")),
				QString(":/images/icon/snow.png"),
				QString(":/images/icon/snow.png"), 
				QString(":/images/icon/snow_press.png"),
				QString(":/images/icon/snow.png"),
				true,
				!m_rpSnowNode->IsHide()
				);
			connect(m_pSnowAction, SIGNAL(triggered(bool)), this, SLOT(SlotShowSnow(bool)));
		}
	}

	//云效果开关
	void CEnvironmentWidgetPlugin::BuildCloudSystem()
	{
		if (m_rpSilverLiningNode.valid())
		{
			m_pCloudAction = CreateMenuAndToolAction(
				QString(tr("Cloud")),
				QString(":/images/icon/cloud.png"),
				QString(":/images/icon/cloud.png"), 
				QString(":/images/icon/cloud_press.png"),
				QString(":/images/icon/cloud.png"),
				true,
				!m_rpSilverLiningNode->IsCloudsShow()
				);
			connect(m_pCloudAction, SIGNAL(triggered(bool)), this ,SLOT(SlotShowCloud(bool)));
		}
	}

	//海洋开关
	void CEnvironmentWidgetPlugin::BuildOceanSystem()
	{
		if(m_rpOceanNode.valid())
		{
			m_pOceanAction = CreateMenuAndToolAction(
				QString(tr("Ocean")),
				QString(":/images/icon/ocean.png"),
				QString(":/images/icon/ocean.png"), 
				QString(":/images/icon/ocean_press.png"),
				QString(":/images/icon/ocean.png"),
				true,
				!m_rpOceanNode->IsHide()
				);
			connect(m_pOceanAction, SIGNAL(triggered(bool)), this ,SLOT(SlotShowOcean(bool)));
		}
	}
	//雾开关
	void CEnvironmentWidgetPlugin::BuildFogSystem()
	{
		if(m_sEnvSetData.m_sEnvFogSetData.bFog)
		{
			m_pFogAction = CreateMenuAndToolAction(
				QString(tr("Fog")),
				QString(":/images/icon/fog.png"),
				QString(":/images/icon/fog.png"), 
				QString(":/images/icon/fog_press.png"),
				QString(":/images/icon/fog.png"),
				true,
				true
				);
		}
		else
		{
			m_pFogAction = CreateMenuAndToolAction(
				QString(tr("Fog")),
				QString(":/images/icon/fog.png"),
				QString(":/images/icon/fog.png"), 
				QString(":/images/icon/fog_press.png"),
				QString(":/images/icon/fog.png"),
				true,
				false
				);
		}


		connect(m_pFogAction, SIGNAL(triggered(bool)), this ,SLOT(SlotShowFog(bool)));

	}

	//设置菜单
	void CEnvironmentWidgetPlugin::BuildSetSystem()
	{
		QAction *pSetAction = CreateMenuAndToolAction(
			QString(tr("Setting")),
			QString(":/images/icon/setttings.png"),
			QString(":/images/icon/setttings.png"), 
			QString(":/images/icon/setttings_press.png"),
			QString(":/images/icon/setttings.png"),
			false
			);
		connect(pSetAction, SIGNAL(triggered()), this, SLOT(SlotSetting()));
	}

	void CEnvironmentWidgetPlugin::SlotShowLight(bool bShow)
	{
		if(m_opFreeSky.valid())
		{
			m_opFreeSky->SetSunVisible(bShow);
		}

		m_sEnvSetData.m_sEnvLightSetData.bSun = bShow;

		emit SignalToSetLigth(bShow);

		SaveData();
	}

	void CEnvironmentWidgetPlugin::SlotShowAtmosphere( bool bShow)
	{
		if(m_opFreeSky.valid())
		{
			m_opFreeSky->SetAtmosphereVisible(bShow);
		}

		m_sEnvSetData.m_sEnvLightSetData.bAtmosphere = bShow;

		emit SignalToSetAtmosphere(bShow);

		SaveData();
	}

	void CEnvironmentWidgetPlugin::SlotShowOcean( bool bShow)
	{
		///显示和隐藏海洋
		if(m_rpOceanNode.valid())
		{
			if(bShow)
			{
				m_rpOceanNode->Show();
			}
			else
			{
				m_rpOceanNode->Hide(); 
			}
		}

		m_sEnvSetData.m_sEnvOceanSetData.bOcean = bShow;

		emit SignalToSetOcean(bShow);

		SaveData();
	}

	void CEnvironmentWidgetPlugin::SlotShowCloud( bool bShow)
	{
		///显示和隐藏云
		if (m_rpSilverLiningNode.valid())
		{
			m_rpSilverLiningNode->SetCloudsShow(bShow);	
			m_rpSilverLiningNode->SetSkyShow(bShow);
		}

		m_sEnvSetData.m_sEnvCloudSetData.bCloud = bShow;

		emit SignalToSetCloud(bShow);

		SaveData();
	}

	void CEnvironmentWidgetPlugin::SlotShowFog( bool bShow)
	{
		// 		osg::StateSet* ss = m_opSystemService->GetRenderContext()->GetMapNode()->getOrCreateStateSet();	
		// 		if (bShow)
		// 		{
		// 
		// 
		// 			m_pFogEffect->attach(ss); 
		// 			m_opSystemService->GetRenderContext()->GetMapNode()->getOrCreateStateSet()->setAttributeAndModes( m_rpFog, osg::StateAttribute::ON );  
		// 		}
		// 		else
		// 		{
		// 			m_pFogEffect->detach(ss); 

		//		}

		m_sEnvSetData.m_sEnvFogSetData.bFog = bShow;
		emit SignalToSetFog(bShow);
		SaveData();
	}

	void CEnvironmentWidgetPlugin::SlotShowRain( bool bShow)
	{
		if(m_rpRainNode.valid())
		{
			if(bShow)
			{
				m_rpRainNode->Show();
			}
			else
			{
				m_rpRainNode->Hide();
			}
		}

		m_sEnvSetData.m_sEnvRainSetData.bRain = bShow;

		emit SignalToSetRain(bShow);

		SaveData();
	}

	void CEnvironmentWidgetPlugin::SlotShowSnow( bool bShow )
	{
		if(m_rpSnowNode.valid())
		{
			if(bShow)
			{
				m_rpSnowNode->Show();
			}
			else
			{
				m_rpSnowNode->Hide();
			}
		}

		m_sEnvSetData.m_sEnvSnowSetData.bSnow = bShow;

		emit SignalToSetSnow(bShow);

		SaveData();
	}

	void CEnvironmentWidgetPlugin::SlotSetting()
	{
		if (m_pSetWidget == NULL)
		{
			m_pSetWidget = new CSettingWidget(this, m_pMainWindow);
		}

		m_pSetWidget->ShowDialogNormal();
	}

	void CEnvironmentWidgetPlugin::SlotFromSetLight( bool bShow)
	{
		m_pLightAction->setChecked(bShow);
	}

	void CEnvironmentWidgetPlugin::SlotFromSetAtmosphere( bool bShow)
	{
		m_pAtmosphereAction->setChecked(bShow);
	}

	void CEnvironmentWidgetPlugin::SlotFromSetRain(bool bShow)
	{
		m_pRainAction->setChecked(bShow);
	}

	void CEnvironmentWidgetPlugin::SlotFromSetSnow(bool bShow)
	{
		m_pSnowAction->setChecked(bShow);
	}

	void CEnvironmentWidgetPlugin::SlotFromSetCloud( bool bShow)
	{
		m_pCloudAction->setChecked(bShow);
	}

	void CEnvironmentWidgetPlugin::SlotFormSetFog( bool bShow )
	{
		m_pFogAction->setChecked(bShow);
	}
	void CEnvironmentWidgetPlugin::SlotFromSetOcean( bool bShow)
	{
		m_pOceanAction->setChecked(bShow);
	}

	osg::Fog* CEnvironmentWidgetPlugin::GetFog()
	{
		if (m_rpFog != NULL)
		{
			return m_rpFog.get();
		}
	}
}



