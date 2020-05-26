#ifndef HDR_LIGHTING_WIDGET_H
#define HDR_LIGHTING_WIDGET_H 1

#include <QWidget>

#include <FeShell/SystemManager.h>
//#include <HDRLighting.h>
#include <FeEffects/HDRLighting.h>
#include <FeEffects/GlareDef.h>
#include "ui_HdrControlPan.h"


/**
  * @class CHdrLightingWidget
  * @brief 环境灯光效果控制面板窗口
  * @author g00034
*/
class CHdrLightingWidget : public QWidget
{
	Q_OBJECT
public:

	CHdrLightingWidget(FeShell::CSystemManager* pSystem);
	~CHdrLightingWidget();

protected slots:
	/**  
	  * @brief 灯光类型变更
	*/
	void SlotLightTypeChanged(int);

	/**  
	  * @note 灯光强度变更
	*/
	void SlotLightPowChanged(int);
	
	/**  
	  * @note 灯光半径变更
	*/
	void SlotLightSizeChanged(int);


private:
	bool InitializeUI();
	bool InitializeContext();

	/**  
	  * @note 根据指定灯光类型，重新生成灯光
	*/
	void RebuildLighting(FeEffect::EGLARELIBTYPE lightType);

	/**  
	  * @note 通过控件值计算灯光强度
	*/
	double GetLightPow(int nSpliderNum);

	/**  
	  * @note 通过控件值计算灯光球大小
	*/
	double GetLightSize(int nSpliderNum);
	
private:
	osg::observer_ptr<FeShell::CSystemManager>     m_opSystemMgr;       // 系统管理类
	osg::ref_ptr<FeEffect::FeHDRLightingNode> m_rpHdrLightingNode; // 灯光节点
	osg::ref_ptr<osg::Group>                       m_rpLightRootNode;   // 存放灯光节点的根节点
	osg::ref_ptr<osg::Node>                        m_rpLightModelNode;  // 被灯光影响的模型节点
	//OpenThreads::Mutex                             m_mutex;

	Ui_HdrWidget ui;
};


#endif 
