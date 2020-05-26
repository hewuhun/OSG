/**************************************************************************************************
* @file SkySetWidget.h
* @note 设置时间参数
* @author l00008
* @data 2015-4-27
**************************************************************************************************/
#ifndef TIME_SET_WIDGET_H
#define TIME_SET_WIDGET_H 1

#include <QDialog>
#include <QTimer>

#include <FeKits/sky/FreeSky.h>
#include <FeShell/SystemService.h>
#include <FeShell/EnvironmentSys.h>
#include <FeSilverlining/SilverliningSys.h>

#include "ui_TimeSetWidget.h"

namespace FreeViewer
{
	class CSettingWidget;

    /**
      * @class CSkySetWidget
      * @note 设置天空、星系等参数
      * @author l00008
    */
    class CTimeSetWidget : public QDialog
    {
        Q_OBJECT

    public:
		/**  
		  * @brief 时间设置窗口构造函数
		  * @param pSetttingWidget [in] 设置窗口，提供配置数据以及保存等方法
		*/
		CTimeSetWidget( CSettingWidget* pSetttingWidget );
		
		/**  
		  * @brief 时间设置窗口析构函数
		*/
        ~CTimeSetWidget();

	public:
		/**  
		  * @note 根据传入数据重置设置对话框以及对应的效果  
		*/
		void Reset();

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
		  * @note 更新时间，包括更新时间轴以及时间显示标签的时间，还包括天空、雨效果的时间  
		  * @param time [in] 时间
		  * @param bSlider [in] 是否为QSlider拖动响应更新操作,如果是则不更新QSlider
		*/
		void UpdateCurrentTime(osgEarth::DateTime time, bool bSlider = false);
		
	protected slots:
		/**  
		  * @brief 当前时间的时间轴事件响应
		*/
		void SlotTimeSlider(int);

		/**  
		  * @brief 控制时间的开始于暂停
		*/
		void SlotTimeControl();

		/**  
		  * @brief 重置时间为当前系统时间
		*/
		void SlotResetTime();

		/**  
		  * @brief 模拟时间动画的步长改变响应
		*/
		void SlotTimeSpeedChanged(int);
		
		/**  
		  * @brief 响应模拟时间的定时器的事件
		*/
		void SlotSimulateTime();

		/**  
		  * @brief 同步时间复选框状态改变
		*/
		void SlotSychTimeChanged(int);
		
		/**  
		  * @brief 同步时间定时器响应的事件
		*/
		void SlotSyncTime();

    private:
		Ui::TimeSetWidget   ui;

		///当前是否正在模拟时间变化
		bool											m_bSimulationTime;

		///模拟时间推进
		QTimer											m_timerSimulation;

		///同步系统时间
		QTimer											m_timerSyncTime;

		///主设置窗口的指针
		CSettingWidget*									m_pSettingWidget;

		///系统服务接口
		osg::observer_ptr<FeShell::CSystemService>		m_opSystemService;

		///天空特效
		osg::observer_ptr<FeKit::CFreeSky>				m_opFreeSky;

		///云层特效
		osg::observer_ptr<FeSilverLining::FeSilverLiningNode> m_opSilverLiningNode;

    };

}
#endif // TIME_SET_WIDGET_H
