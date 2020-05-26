/**************************************************************************************************
* @file FogSetWidget.h
* @note 雾效果设置窗口，包括浓度、全局或局部以及高度等参数
* @author c00097
* @data 2017-7-15
**************************************************************************************************/
#ifndef FOG_SET_WIDGET_H
#define FOG_SET_WIDGET_H
#include <FeShell/SystemService.h>
#include <FeShell/EnvironmentSerializer.h>
#include <FeKits/weather/WeatherEffectNode.h>
#include <FeOcean/OceanNode.h>

#include <QDialog>

#include "ui_FogSetWidget.h"
#include "osgEarthUtil/Fog"

namespace FreeViewer
{
	class CSettingWidget;
	class CFogUpdateCallBack;
	/**
	  * @class CFogSetWidget
	  * @brief 雾效果设置对话框
	  * @note 设置雾效果的参数，包括浓度、全局或局部以及高度等参数
	  * @author c00097
	*/
	class CFogSetWidget:public QDialog
	{
		Q_OBJECT
	public:
		/**  
		  * @brief 雾效设置窗口构造函数
		  * @param pSetttingWidget [in] 设置窗口，提供配置数据以及保存等方法
		*/
		CFogSetWidget(CSettingWidget* pSettingWidget);
		
		/**  
		  * @brief 雾效设置窗口析构函数
		*/
		~CFogSetWidget();
	public:
		/**  
		  * @note 根据传入数据重置设置对话框以及对应的效果  
		  * @param data [in] 配置数据
		*/
		void Reset(FeShell::SEnvFogSetData data);
		double GetHight();
		double GetIntensity();

		CSettingWidget* GetSettingWidget() {return m_pSettingWidget;};

	protected:
		/**
		*note 初始化窗口
		*/
		void InitWidget();

		/**
		*note 初始化上下文环境，主要构件界面以及注册一些功能
		*/
		void BuildContext();

	signals:
		/**  
		  * @note 当雾显隐发生变化时，发送的信号
		*/
		void SignalFromSetFog(bool);

	public slots:
		/**  
		  * @note 当外部设置雾显隐发生变化时，响应的槽函数
		*/
		void SlotSetFog(bool);
		
	public slots:
		/**  
		  * @note 控制雾显隐
		*/
		void SlotFogVisible(bool);
		
		/**  
		  * @note 控制雾的浓度，响应QSlider信号
		*/
        void SlotFogIntensity(int);
		
		/**  
		  * @note 控制雾的浓度，响应QSpinBox信号
		*/
		void SlotFogIntensity(double);
			
	protected:
		Ui::FogSet ui;

		///主设置窗口的指针
		CSettingWidget*									m_pSettingWidget;
		osg::ref_ptr<osg::Fog> fog;
		osgEarth::Util::FogEffect * m_pFog;
		osg::ref_ptr<CFogUpdateCallBack> m_rpCallBack;
		double m_dHight;
		double m_dIntensity;
		///系统服务接口
		osg::observer_ptr<FeShell::CSystemService>		m_opSystemService;
	};
	class CFogUpdateCallBack : public osg::NodeCallback
	{
	public:
		CFogUpdateCallBack(osg::Fog * pFog,FeUtil::CRenderContext* pRender,CFogSetWidget* pSettingWidget);

		~CFogUpdateCallBack();

		/**  
		  * @note 初始化大气层节点雾化因素
		*/
		void InitFogUniform(FeKit::CFreeSky*);

		/**  
		  * @note 恢复对海洋和天空节点的修改
		*/
		void ResetOceanAndSky();

	protected:
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

		osg::Fog*															m_pFog;
		FeUtil::CRenderContext*								m_pRender;
		CFogSetWidget*												m_pFogWidget;
		FeKit::CFreeSky*											m_pSkyNode;

		// 天空节点考虑雾化因素
		osg::observer_ptr<osg::Uniform>					m_ipUniformSkyFogEnabled;
		osg::observer_ptr<osg::Uniform>					m_ipUniformSkyFogColor;
		CSettingWidget*									   					m_pSettingWidget;
		osg::observer_ptr<FeOcean::TritonNode>		m_opOceanNode;
	};
}

#endif //FOG_SET_WIDGET_H