#ifndef HDR_LIGHTING_WIDGET_H
#define HDR_LIGHTING_WIDGET_H 1

#include <QWidget>

#include <FeShell/SystemManager.h>
#include <FeSilverlining/SilverliningSys.h>

#include "ui_SilverliningControlPan.h"


/**
  * @class CSilverliningWidget
  * @brief 云效果控制面板窗口
  * @author g00034
*/
class CSilverliningWidget : public QWidget
{
	Q_OBJECT
public:

	CSilverliningWidget(FeShell::CSystemManager* pSystem);
	~CSilverliningWidget();

protected slots:
	void SlotCloudsVisibleChanged(bool); //云层开关
	void SlotCloudsAltitudeChanged(int); //云层高度控制
	void SlotCloudsDensityChanged(int); //云层密度
	void SlotCloudsDensityChanged(double);
	void SlotCloudsAlphaChanged(int); //云层透明度
	void SlotCloudsAlphaChanged(double);
	void SlotCloudTypeType(int nIndex); //云层形状

	void SlotWindSpeedChanged(int); //风力速度
	void SlotWindDirectionChanged(int); //风力方向
	void SlotSetCloud(bool);

	/**  
	  * @brief 当前时间的时间轴事件响应
	*/
	void SlotTimeSlider(int);


private:
	bool InitializeUI();
	bool InitializeContext();

	void CloudEnable(bool);

	void ApplyCurAttri();
	void UpdateCurrentTime( osgEarth::DateTime time, bool bSlider );
	
private:
	osg::observer_ptr<FeShell::CSystemManager>          m_opSystemMgr;       // 系统管理类
	osg::observer_ptr<FeSilverLining::CSilverLiningSys> m_opSilverLiningSys; // 云系统
	osg::observer_ptr<FeKit::CFreeSky>                  m_opFreeSky;         // 天空系统

	Ui_SilverliningWidget ui;
};


#endif 
