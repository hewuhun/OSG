#ifndef FE_FLIGHT_WIDGET_H
#define FE_FLIGHT_WIDGET_H 1

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
#include <FeShell/SystemSerializer.h>

#include <FeEarth/3DSceneWidget.h>

#include <FlightDemo.h>

#include "ui_FlightControlPan.h"

/**
* @class CFlightWidget
* @brief 飞行效果控制面板窗口
*/
class CFlightWidget : public QWidget
{
	Q_OBJECT
public:

	CFlightWidget(FeShell::CSystemManager* pSystem);

	~CFlightWidget();


protected slots:
	/**  
	  * @note 控制速度
	*/
	void SlotSpeedChanged(int);
	
	/**  
	  * @note 相机变更
	*/
	void SlotCameraChanged(QAbstractButton *);

	/**  
	  * @note 操作变更
	*/
	void SlotControlChanged(QAbstractButton *);

	/**  
	  * @note 状态信息更新
	*/
	void SlotFlightStateInfo(const QString&);

	/**  
	  * @note 航线显示
	*/
	void SlotAirwayShow(bool);

private:
	bool InitializeUI();
	bool InitializeContext();

private:
	osg::ref_ptr<CFlightDemo>                  m_rpDemo;
	osg::observer_ptr<FeShell::CSystemManager>     m_opSystemMgr;
	osg::observer_ptr<FeKit::CFreeSky>             m_opFreeSky;
	osg::observer_ptr<FeKit::CManipulatorManager>  m_opManipulatorMgr;
	osg::observer_ptr<osg::Node>                   m_opCurModelNode;
	double m_dOriginFov;
	Ui_CFlightWidget ui;
};


#endif 
