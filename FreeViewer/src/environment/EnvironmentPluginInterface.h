/**************************************************************************************************
* @file EnvironmentPluginInterface.h
* @note 环境因素设置窗口
* @author c00005
* @data 2017-2-7
**************************************************************************************************/
#ifndef ENVIRONMENT_PLUGIN_INTERFACE_H
#define ENVIRONMENT_PLUGIN_INTERFACE_H 1

#include <mainWindow/UIObserver.h>

//#include <environment/SkyCtrlBar.h>
#include <environment/RainSetWidget.h>
#include <environment/SnowSetWidget.h>
#include <environment/OceanSetWidget.h>
#include <environment/SettingWidget.h>
#include <environment/SilverLiningSetWidget.h>
#include <environment/TimeSetWidget.h>

#include <FeShell/EnvironmentSerializer.h>
#include "osg/Fog"
#include "osgEarthUtil/Fog"


namespace FreeViewer
{
	/**
	* @class CEnvironmentWidgetPlugin
	* @note 系统环境管理，包括光照、雨雪、大气层、星云等
	* @author c00005
	*/
	class CEnvironmentWidgetPlugin : public CUIObserver
	{
		Q_OBJECT

	public:
		/**  
		  * @brief 构造函数
		*/
		CEnvironmentWidgetPlugin( CFreeMainWindow* pMainWindow );

		/**  
		  * @brief 析构函数
		*/
		~CEnvironmentWidgetPlugin();

    public:
		/**
		* @note 序列化，将环境数据保存到序列化文件中
		*/
		void SaveData();
		
		/**
		* @note 反序列化，读取序列化文件中保存的环境数据
		*/
		void LoadData();

        /**
        *note 获取环境的配置信息
        */
        FeShell::CEnvSetData& GetSetData();

        /**
        *note 加载重置配置数据
        */
		void LoadResetData();

		/**  
		  * @brief 简要说明
		  * @note 获取雾效果 
		  * @return 雾
		*/
		osg::Fog* GetFog();
		

	protected:
		/**
		*note 初始化窗口
		*/
		void InitWidget();

		/**
		*note 初始化上下文环境，主要构件界面以及注册一些功能
		*/
		void BuildContext();

	protected:
		/**
		*note 初始化天空设置
		*/
		void BuildSunSystem();

		/**
		*note 初始化雨雪效果参数
		*/
		void BuildWeatherSystem();

		/**  
		  * @brief 初始化云效果参数
		*/
		void BuildCloudSystem();

		/**
		*note 初始化海洋参数
		*/
		void BuildOceanSystem();
		/**
		*note 初始化雾参数
		*/
		void BuildFogSystem();

		/**
		* @note 设置菜单
		*/
		void BuildSetSystem();

		/**
		* @note 菜单功能与设置同步
		*/
		void BuildMenueConnect();

    ///向设置窗口发送按钮状态改变信号
	signals:
		void SignalToSetLigth(bool);
		void SignalToSetAtmosphere(bool);
		void SignalToSetRain(bool);
		void SignalToSetSnow(bool);
		void SignalToSetCloud(bool);
		void SignalToSetOcean(bool);
		void SignalToSetFog(bool);

	///响应设置窗口中的按钮的状态变化
	public slots:
		void SlotFromSetLight(bool);
		void SlotFromSetAtmosphere(bool);
		void SlotFromSetRain(bool);
		void SlotFromSetSnow(bool);
		void SlotFromSetCloud(bool);
		void SlotFromSetOcean(bool);
		void SlotFormSetFog(bool);

	private slots:
		///显示天空的控制
		void SlotShowLight(bool); 

		///显示和隐藏大气层
		void SlotShowAtmosphere(bool);

		///设置下雨天气效果属性
		void SlotShowRain(bool);

		///设置下雪天气效果属性
		void SlotShowSnow(bool);

		///显示和隐藏海洋
		void SlotShowOcean(bool);

		///显示和隐藏云
		void SlotShowCloud(bool);

		///显示和隐藏雾
		void SlotShowFog(bool);

        ///设置菜单
		void SlotSetting();

	protected:
        FeShell::CEnvSetData     							m_sEnvSetData;

		osg::observer_ptr<FeKit::CFreeSky>					m_opFreeSky;    //光照星系系统
		osg::ref_ptr<FeKit::CWeatherEffectNode>				m_rpRainNode;	//全局雨节点
		osg::ref_ptr<FeKit::CWeatherEffectNode>				m_rpSnowNode;	//全局雪节点
		osg::ref_ptr<FeOcean::TritonNode>					m_rpOceanNode;   //海洋系统
		osg::ref_ptr<FeSilverLining::FeSilverLiningNode>	m_rpSilverLiningNode; //云层节点

		QAction*											m_pLightAction;
		QAction*											m_pAtmosphereAction;
		QAction*											m_pRainAction;
		QAction*											m_pSnowAction;
		QAction*											m_pStarsAction;
		QAction*											m_pOceanAction;
		QAction*											m_pCloudAction;
		QAction*                                         m_pFogAction;

		CTimeSetWidget*										m_pTimeSetWidget;  //天空特效控制器
		CSettingWidget*											m_pSetWidget;	//设置菜单
		osg::ref_ptr<osg::Fog>                             m_rpFog;
		osgEarth::Util::FogEffect * m_pFogEffect;
	};

}
#endif // ENVIRONMENT_PLUGIN_INTERFACE_H
