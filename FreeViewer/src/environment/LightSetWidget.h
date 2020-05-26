/**************************************************************************************************
* @file LightSetWidget.h
* @note 设置光照、天空、星系等参数
* @author c00005
* @data 2017-2-8
**************************************************************************************************/
#ifndef LIGHT_SET_WIDGET_H
#define LIGHT_SET_WIDGET_H 1

#include <QDialog>

#include <FeKits/sky/FreeSky.h>
#include <FeShell/SystemService.h>
#include <FeShell/EnvironmentSerializer.h>

#include "ui_LightSetWidget.h"

namespace FreeViewer
{
	class CSettingWidget;

    /**
      * @class CLightSetWidget
      * @note 设置光照、天空、星系等参数
      * @author c00005
    */
    class CLightSetWidget : public QDialog
    {
        Q_OBJECT

    public:
		/**  
		  * @brief 光照设置窗口构造函数
		  * @param pSetttingWidget [in] 设置窗口，提供配置数据以及保存等方法
		*/
        CLightSetWidget( CSettingWidget* pSetttingWidget );
		
		/**  
		  * @brief 光照设置窗口析构函数
		*/
        ~CLightSetWidget();

	public:
		/**  
		  * @note 根据传入数据重置设置对话框以及对应的效果  
		  * @param data [in] 配置数据
		*/
		void Reset(FeShell::SEnvLightSetData data);

	protected:
		/**
		*note 初始化窗口
		*/
		void InitWidget();

		/**
		*note 初始化上下文环境，主要构件界面以及注册一些功能
		*/
		void BuildContext();


		//向主设置窗口发送状态变化事件
	signals:
		/**  
		  * @note 当太阳显隐发生变化时，发送的信号
		*/
		void SignalFromSetSun(bool);

		/**  
		  * @note 当大气层显隐发生变化时，发送的信号
		*/
		void SignalFromSetAtmosphere(bool);

		//响应主设置窗口的的状态变化
	public slots:
		/**  
		  * @note 当外部设置太阳显隐发生变化时，响应的槽函数
		*/
		void SlotSetLight(bool);

		/**  
		  * @note 当外部设置大气层显隐发生变化时，响应的槽函数
		*/
		void SlotSetAtmosphere(bool);
	
	public slots:
		/**  
		  * @note 控制太阳显隐
		*/
		void SlotSunVisible(bool);

		/**  
		  * @note 控制太阳光照强度
		*/
		void SlotAmbientChanged(int);

		/**  
		  * @note 控制大气层显隐
		*/
		void SlotAtmosphereVisible(bool); 

		/**  
		  * @note 控制月亮显隐
		*/
        void SlotMoonVisible(bool);

		/**  
		  * @note 控制星辰显隐
		*/
		void SlotStarVisible(bool);

		/**  
		  * @note 控制星云背景显隐
		*/
		void SlotNebulaVisible(bool);

    protected:
        Ui::LightSetWidget    ui;

		///主设置窗口的指针
		CSettingWidget*									m_pSettingWidget;

		///系统服务接口
        osg::observer_ptr<FeShell::CSystemService>		m_opSystemService;

		///天空特效
        osg::observer_ptr<FeKit::CFreeSky>				m_opFreeSky;
	};
}
#endif //LIGHT_SET_WIDGET_H
