#ifndef FLIGHT_DEMO_H_
#define FLIGHT_DEMO_H_ 1

#include <osg/Group>
#include <QStringList>
#include <QObject>
#include <osg/AnimationPath>
#include <osgEarthAnnotation/PlaceNode>
#include "osg/NodeVisitor"
#include <FeUtils/RenderContext.h>
#include "FeManager/FreeMarkSys.h"
#include "FeUtils/MiniAnimationCtrl.h"
#include "FeEffects/AnimationEffect.h"



class CFlightDemo;

// 飞机位置等信息
class CModelPosReport : public osg::NodeCallback
{
public:
	CModelPosReport(CFlightDemo* pFlightDemo, FeUtil::CRenderContext* pRC);

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

private:
	osg::observer_ptr<CFlightDemo>        m_opFlightDemo;
	osg::observer_ptr<FeUtil::CRenderContext> m_opRenderContext;
	osg::Matrix                               m_mxLast;
};



class CFlightDemo : public QObject, public osg::Group
{
	Q_OBJECT
	
	friend class CModelPosReport;

public:
	CFlightDemo(FeUtil::CRenderContext* rc, FeManager::CFreeMarkSys* pMarkSys);
	~CFlightDemo();

public:
	/**  
	  * @note 初始化
	*/
	bool Init();

	/**  
	  * @note 开始飞行
	*/
	void Start();

	/**  
	  * @note 停止飞行
	*/
	void Stop();

	/**  
	  * @note 重置
	*/
	void Reset();

	/**  
	  * @note 显示航线
	*/
	void ShowAirLine(bool bShow);

	/**  
	  * @note 设置速度
	*/
	void SetSpeed(int nSpeed);
	int GetSpeed() const { return m_speedTimes; }

	/**  
	  * @note 获取飞机模型
	*/
	osg::Node* const GetAirModel() { return m_rpAirModel.get(); }

signals:
	/**  
	  * @note 信号，飞行状态变更
	*/
	void Sig_StateChanged(const QString&);

private:
	osg::observer_ptr<FeUtil::CRenderContext>  m_opRenderContext;
	osg::observer_ptr<FeManager::CFreeMarkSys> m_opMarkSys;

	/// 模型变换矩阵
	osg::ref_ptr<FeEffect::CFeAnimationNode>   m_rpAirModel;
	osg::ref_ptr<FeUtil::CMiniAnimationCtrl>   m_rpAnimationCtrl;
	osg::ref_ptr<osg::Node>                    m_pathNode;

	/// 起落架标识节点
	osg::ref_ptr<osgEarth::Annotation::PlaceNode> m_OnOffLabel[2]; 
	
	double                                     m_speedTimes;
};


#endif //FLIGHT_DEMO_H_


