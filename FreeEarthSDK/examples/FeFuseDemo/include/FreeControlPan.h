/**************************************************************************************************
* @file FreeMenuWidget.h
* @note 自定义菜单栏，实现了平台logo、菜单栏和工具栏
* @author c00005
* @data 2016-12-22
**************************************************************************************************/
#ifndef FREE_MENU_WIDGET_H
#define FREE_MENU_WIDGET_H 1

#include <QWidget>
#include <QMenuBar>

#include <FeShell/SystemManager.h>
#include <FeExtNode/ExLodModelNode.h>

#include "ui_FreeControlPan.h"
#include <FeUtils/TerrainMask.h>
/**
* @class CFreeControlPan
* @brief 自定义菜单栏
* @note 自定义菜单栏，实现了平台logo、菜单栏和工具栏
* @author c00005
*/
class CFreeControlPan : public QWidget
{
	Q_OBJECT

public:
	/**  
	* @note 构造函数
	*/
	CFreeControlPan( FeShell::CSystemManager* pManager, QWidget* pParent=0 );

	/**  
	* @note 析构函数
	*/
	~CFreeControlPan();

public:
	void InitWidget();

	void InitContext();

protected:
	void CreateUsaAirport();

	void CreateRandomAirport();

public slots:
	void SlotLocateUsa();

	void SlotLocateCurrent();

	void SlotChangePosition(double);

protected:
	///UI
	Ui_CFreeMenuWidget					ui;

	osg::observer_ptr<FeShell::CSystemManager>	m_opSystemManager;

	osg::ref_ptr<FeExtNode::CExLodModelGroup>	m_rpRandomModel;

	osg::ref_ptr<osgEarth::MaskLayer>			m_rpMaskLayer;

	osg::ref_ptr<osg::Vec3dArray>				m_rpBoundarys;
	FeUtil::CTerrainMask * 						m_pTerrainMask;
};
#endif 







