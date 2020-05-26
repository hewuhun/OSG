/**************************************************************************************************
* @file RainSetWidget.h
* @note 雨效果设置窗口，包括浓度、全局或局部以及高度等参数
* @author c00005
* @data 2017-2-20
**************************************************************************************************/
#ifndef RAIN_SET_WIDGET_H
#define RAIN_SET_WIDGET_H 

#include <FeShell/SystemService.h>
#include <FeShell/EnvironmentSerializer.h>
#include <FeKits/weather/WeatherEffectNode.h>

#include <QDialog>

#include "ui_RainSetWidget.h"

namespace FreeViewer
{
	class CSettingWidget;

	/**
	  * @class CRainSetWidget
	  * @brief 雨效果设置对话框
	  * @note 设置雨效果的参数，包括浓度、全局或局部以及高度等参数
	  * @author c00005
	*/
	class CRainSetWidget : public QDialog
	{
		Q_OBJECT

	public:
		/**  
		  * @brief 雨效设置窗口构造函数
		  * @param pSetttingWidget [in] 设置窗口，提供配置数据以及保存等方法
		*/
        CRainSetWidget( CSettingWidget* pSetttingWidget );

		/**  
		  * @brief 雨效设置窗口析构函数
		*/
		~CRainSetWidget();

	public:
		/**  
		  * @note 根据传入数据重置设置对话框以及对应的效果  
		  * @param data [in] 配置数据
		*/
		void Reset(FeShell::SEnvRainSetData data);

	protected:
		/**
		*note 初始化窗口
		*/
		void InitWidget();

		/**
		*note 初始化上下文环境，主要构件界面以及注册一些功能
		*/
		void BuildContext();

		/**
		*note 更新控件状态
		*/
		void UpdateWidgetState();
		
		//向主设置窗口发送状态变化事件
	signals:
		/**  
		  * @note 当雨显隐发生变化时，发送的信号
		*/
		void SignalFromSetRain(bool);

		//响应主设置窗口的的状态变化
	public slots:
		/**  
		  * @note 当外部设置雨显隐发生变化时，响应的槽函数
		*/
		void SlotSetRain(bool);
		
	protected slots:
		/**  
		  * @note 控制雨显隐
		*/
		void SlotRainVisible(bool);
	
		/**  
		  * @note 控制雨的浓度，响应QSlider信号
		*/
		void SlotRainIntensity(int);
		
		/**  
		  * @note 控制雨的浓度，响应QSpinBox信号
		*/
		void SlotRainIntensity(double);
		
		/**  
		  * @note 控制雨的高度
		*/
		void SlotRainHeight(double);
		
		/**  
		  * @note 控制雨的位置
		*/
		void SlotRainPosition(double);
		
		/**  
		  * @note 设置为全局雨
		*/
		void SlotGlobalRain(bool);
	
		/**  
		  * @note 设置为局部雨
		*/
		void SlotLocalRain(bool);
		
		/**  
		  * @note 定位到雨位置点
		*/
		void SlotLocation();
		
	protected:
		Ui::RainSetWidget    ui;

		///主设置窗口的指针
		CSettingWidget*									m_pSettingWidget;

		///系统服务接口
		osg::observer_ptr<FeShell::CSystemService>		m_opSystemService;

		///雨效果节点
        osg::observer_ptr<FeKit::CWeatherEffectNode>    m_opRainNode;
	};

}
#endif // RAIN_SET_WIDGET_H
