#ifndef OCEAN_WIDGET_H
#define OCEAN_WIDGET_H 1

#include <QAction>
#include <QEvent>
#include <QDockWidget>
#include <QMainWindow>
#include <QMetaObject>
#include <QToolBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QTranslator>
#include <QSplashScreen>
#include <QTextEdit>
#include <QWidget>

#include <FeEarth/3DSceneWidget.h>
#include <FeManager/FreeMarkSys.h>
#include <FeOcean/OceanSys.h>

#include "ui_OceanWidget.h"



namespace FeOceanDemo
{
	/**
	* @class COceanWidget
	* @brief 海洋效果控制面板窗口
	*/
	class COceanWidget : public QWidget
	{
		Q_OBJECT

	public:
		COceanWidget(FeShell::CSystemManager* pSystem);

		~COceanWidget();

	private:
		/**  
		  * @brief 初始化UI部分
		*/
		bool InitializeUI();

		/**  
		  * @brief 初始化三维内容
		*/
		bool InitializeContext();

		/**  
		  * @brief 初始化海洋效果
		*/
		void InitializeOceanEffect();

		/**  
		  * @brief 更新当前时间
		*/
		void UpdateCurrentTime( osgEarth::DateTime time, bool bSlider );

		/**  
		  * @brief 根据指定时间和视点，更新场景
		*/
		void UpdateScene(const osgEarth::Viewpoint& vp, int year, int month, int day, int hours, int min);

	protected slots:
		/**  
		* @brief 当前时间的时间轴事件响应
		*/
		void SlotTimeSlider(int);

		/**  
		* @note 控制太阳光照强度
		*/
		void SlotAmbientChanged(int);

		/**  
		* @note 相机并更
		*/
		void SlotCameraChanged(QAbstractButton *);

	private:
		osg::observer_ptr<FeShell::CSystemManager>		m_opSystemMgr;
		osg::observer_ptr<FeKit::CFreeSky>				m_opFreeSky;
		osg::observer_ptr<FeManager::CFreeMarkSys>		m_opMarkSys;
		osg::observer_ptr<FeOcean::COceanSys>           m_opOceanSys;
		osg::observer_ptr<FeKit::CManipulatorManager>	m_opManipulatorMgr;
		
		/// 演示效果模型对象
		osg::ref_ptr<osg::Node>                         m_rpShipWakeNode;
		osg::ref_ptr<osg::Node>                         m_rpRotorWashNode;

		Ui_COceanWidget ui;
	};
}

#endif 
