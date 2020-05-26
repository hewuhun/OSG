/**************************************************************************************************
* @file SnowSetWidget.h
* @note 雪效果设置窗口，包括浓度、全局或局部以及高度等参数
* @author c00005
* @data 2017-2-20
**************************************************************************************************/
#ifndef SNOW_SET_WIDGET_H
#define SNOW_SET_WIDGET_H 

#include <FeShell/SystemService.h>
#include <FeShell/EnvironmentSerializer.h>
#include <FeKits/weather/WeatherEffectNode.h>

#include <QDialog>

#include "ui_SnowSetWidget.h"

namespace FreeViewer
{
	class CSettingWidget;

	/**
	  * @class CSnowSetWidget
	  * @brief 雪效果设置对话框
	  * @note 设置雪效果的参数，包括浓度、全局或局部以及高度等参数
	  * @author c00005
	*/
	class CSnowSetWidget : public QDialog
	{
		Q_OBJECT

	public:
		/**  
		  * @brief 雪效设置窗口构造函数
		  * @param pSetttingWidget [in] 设置窗口，提供配置数据以及保存等方法
		*/
        CSnowSetWidget( CSettingWidget* pSetttingWidget );

		/**  
		  * @brief 雪效设置窗口析构函数
		*/
		~CSnowSetWidget();

	public:
		/**  
		  * @note 根据传入数据重置设置对话框以及对应的效果  
		  * @param data [in] 配置数据
		*/
		void Reset(FeShell::SEnvSnowSetData data);

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
		  * @note 当雪显隐发生变化时，发送的信号
		*/
		void SignalFromSetSnow(bool);

		//响应主设置窗口的的状态变化
	public slots:
		/**  
		  * @note 当外部设置雪显隐发生变化时，响应的槽函数
		*/
		void SlotSetSnow(bool);
		
	public slots:
		/**  
		  * @note 控制雪显隐
		*/
		void SlotSnowVisible(bool);
		
		/**  
		  * @note 控制雪的浓度，响应QSlider信号
		*/
        void SlotSnowIntensity(int);
		
		/**  
		  * @note 控制雪的浓度，响应QSpinBox信号
		*/
		void SlotSnowIntensity(double);
		
		/**  
		  * @note 控制雪的高度
		*/
        void SlotSnowHeight(double);
		
		/**  
		  * @note 控制雪的位置
		*/
        void SlotSnowPosition(double);
	
		/**  
		  * @note 设置为全局雪
		*/
        void SlotGlobalSnow(bool);
		
		/**  
		  * @note 设置为局部雪
		*/
        void SlotLocalSnow(bool);
		
		/**  
		  * @note 定位到雪位置点
		*/
		void SlotLocation();

	protected:
		Ui::SnowSetWidget    ui;

		///主设置窗口的指针
		CSettingWidget*									m_pSettingWidget;

		///系统服务接口
		osg::observer_ptr<FeShell::CSystemService>		m_opSystemService;

		///雪效果节点
        osg::observer_ptr<FeKit::CWeatherEffectNode>    m_opSnowNode;
	};

}
#endif // SNOW_SET_WIDGET_H
