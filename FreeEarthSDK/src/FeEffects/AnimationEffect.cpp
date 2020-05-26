#include <osg/MatrixTransform>
#include <osg/Geometry>

#include <FeUtils/CoordConverter.h>
#include <FeUtils/PathRegistry.h>
#include <FeEffects/AnimationEffect.h>


namespace FeEffect
{
	CAnimationVisitor::CAnimationVisitor()
		:osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
		,m_modeAnimation()
	{
		m_vecReverseTimeControlPointMap.clear();
		m_vecTimeControlPointMap.clear();
	}

	CAnimationVisitor::~CAnimationVisitor()
	{

	}

	void CAnimationVisitor::apply( osg::MatrixTransform &pMt )
	{
		//pMt.getName() == "";
		osg::AnimationPathCallback *pCallback = dynamic_cast<osg::AnimationPathCallback *>(pMt.getUpdateCallback());
		if(pCallback)
		{
			osg::AnimationPath *pAnimationPath = pCallback->getAnimationPath();
			osg::AnimationPath::TimeControlPointMap mapTimeCtrlTemp;
			if(pAnimationPath)
			{
				osg::AnimationPath::TimeControlPointMap mapTimeCtrl;
				pAnimationPath->setLoopMode(osg::AnimationPath::NO_LOOPING);
				mapTimeCtrl = pAnimationPath->getTimeControlPointMap();
				mapTimeCtrlTemp = mapTimeCtrl;
				m_vecTimeControlPointMap.push_back(mapTimeCtrlTemp);

				double dTime =0.0;
				osg::AnimationPath::TimeControlPointMap mapTimeCtrlReverse;
				osg::AnimationPath::TimeControlPointMap::iterator itrTemp = mapTimeCtrlTemp.end();
				--itrTemp;
				dTime = itrTemp->first;
				double dTimeTemp = 0.0 ;
				double dTimeTempCurr = 0.0;
				for(;itrTemp != mapTimeCtrlTemp.begin();--itrTemp)
				{
					osg::AnimationPath::ControlPoint controlPoint =  itrTemp->second;
					dTimeTemp  = itrTemp->first;
					dTimeTempCurr = dTime - dTimeTemp;
					mapTimeCtrlReverse[dTimeTempCurr] = controlPoint;
				}

				dTimeTemp  = itrTemp->first;
				dTimeTempCurr = dTime - dTimeTemp;

				mapTimeCtrlReverse[dTimeTempCurr] = itrTemp->second;
				m_vecReverseTimeControlPointMap.push_back(mapTimeCtrlReverse);

				pAnimationPath->setTimeControlPointMap(mapTimeCtrlTemp);

				pCallback->reset();
				pCallback->setPause(true);
				m_modeAnimation.push_back(pCallback);

			}

		}

		traverse(pMt);
	}

	CAnimationEffect::MODELANIMATION CAnimationVisitor::GetModelAnimation()
	{
		return m_modeAnimation;
	}

	std::vector<osg::AnimationPath::TimeControlPointMap> & CAnimationVisitor::GetVectorOfTimeControlPointMap()
	{
		return m_vecTimeControlPointMap;
	}

	std::vector<osg::AnimationPath::TimeControlPointMap> & CAnimationVisitor::GetReverseVectorOfTimeControlPointMap()
	{
		return m_vecReverseTimeControlPointMap;
	}

}

namespace FeEffect
{
	CAnimationEffect::CAnimationEffect(osg::Node *pModelNode)
		:CFreeEffect()
		,m_pModelNode(pModelNode)
	{
		m_vModelAnimation.clear();
	}

	CAnimationEffect::~CAnimationEffect()
	{

	}

	bool CAnimationEffect::CreateEffect()
	{
		CAnimationVisitor av;
		m_vModelAnimation.clear();
		if(m_pModelNode)
		{
			m_pModelNode->accept(av);
		}
		 m_vModelAnimation = av.GetModelAnimation();
		 //StopEffect();
		 m_vecTimeControlPointMap = av.GetVectorOfTimeControlPointMap();
		 m_vecReverseTimeControlPointMap = av.GetReverseVectorOfTimeControlPointMap();

		return true;
	}

	bool CAnimationEffect::StartEffect()
	{
		MODELANIMATION::iterator itr = m_vModelAnimation.begin();
		
		for(int n =0;itr!=m_vModelAnimation.end();++itr,++n)
		{
			osg::AnimationPath *pAnimationPath = (*itr)->getAnimationPath();

			if(pAnimationPath)
			{
				pAnimationPath->setTimeControlPointMap(m_vecTimeControlPointMap.at(n));

				pAnimationPath->setLoopMode(osg::AnimationPath::NO_LOOPING);

			}
			(*itr)->reset();
			(*itr)->setPause(false);
		}

		return true;
	}

	bool CAnimationEffect::UpdataEffect()
	{
		return false;
	}

	bool CAnimationEffect::ClearEffect()
	{
		removeChildren(0,this->getNumChildren());
		return false;
	}

	bool CAnimationEffect::StopEffect()
	{
		MODELANIMATION::iterator itr = m_vModelAnimation.begin();

		for(;itr!=m_vModelAnimation.end();++itr)
		{
			osg::AnimationPath *pAnimationPath = (*itr)->getAnimationPath();
			if(pAnimationPath)
			{
				pAnimationPath->setLoopMode(osg::AnimationPath::NO_LOOPING);
			}
			(*itr)->reset();
			(*itr)->setPause(true);
		}
		return true;
	}

	bool CAnimationEffect::OppositeEffect()
	{

		MODELANIMATION::iterator itr = m_vModelAnimation.begin();

		for(int n =0;itr!=m_vModelAnimation.end();++itr,++n)
		{
			osg::AnimationPath *pAnimationPath = (*itr)->getAnimationPath();
			if(pAnimationPath)
			{
				pAnimationPath->setTimeControlPointMap(m_vecReverseTimeControlPointMap.at(n));

				pAnimationPath->setLoopMode(osg::AnimationPath::NO_LOOPING);
			}
			(*itr)->reset();
			(*itr)->setPause(false);
		}

		return true;
	}


}



namespace FeEffect
{
	CFeAnimationNode::CFeAnimationNode( const std::string& strModelPath, double dPeriodSec ) : m_dPeriodSec(dPeriodSec)
	{
		osg::Node* pModel = osgDB::readNodeFile(strModelPath);
		InitModel(pModel);
	}

	CFeAnimationNode::CFeAnimationNode( osg::Node* pModelNode, double dPeriodSec ) : m_dPeriodSec(dPeriodSec)
	{
		InitModel(pModelNode);
	}

	void CFeAnimationNode::FindAnimPath( osg::Node * rootNode )
	{
		if(rootNode)
		{	
			if (osg::AnimationPathCallback* pAnimationPathCB = dynamic_cast<osg::AnimationPathCallback*>(rootNode->getUpdateCallback()))
			{
				m_vecAnimationPaths[rootNode] = pAnimationPathCB->getAnimationPath();
			}	
		}

		osg::Group* gp = dynamic_cast<osg::Group*>(rootNode);
		if(gp)
		{
			for (unsigned int i = 0; i < gp->getNumChildren(); i++)
			{
				FindAnimPath(gp->getChild(i));
			}
		}
	}

	void CFeAnimationNode::SetAnimPause( bool isPause /*= true */ )
	{
		std::map<osg::Node*, osg::AnimationPath*>::iterator it = m_vecAnimationPaths.begin();
		while (it != m_vecAnimationPaths.end())
		{
			if (osg::AnimationPathCallback* pAnimationPathCB = dynamic_cast<osg::AnimationPathCallback*>(it->first->getUpdateCallback()))
			{
				pAnimationPathCB->setPause(isPause);
			}

			it++;
		}
	}

	void CFeAnimationNode::InitModel( osg::Node* pModelNode )
	{
		if(pModelNode)
		{
			FindAnimPath(pModelNode);
			addChild(pModelNode);

			m_rpTimerCB = new CTimerCallback(this);
			addUpdateCallback(m_rpTimerCB);
		}
	}

	void CFeAnimationNode::PlayForTime( double dTime )
	{
		if(m_rpTimerCB.valid())
		{
			m_rpTimerCB->PlayTime(dTime);
		}
	}

	double CFeAnimationNode::GetPeriodSec()
	{
		return m_dPeriodSec;
	}

	void CFeAnimationNode::StepToTime( double dTime )
	{
		std::map<osg::Node*, osg::AnimationPath*>::iterator it = m_vecAnimationPaths.begin();
		while (it != m_vecAnimationPaths.end())
		{
			//静止状态的打开与关闭
			{
				osg::Node* pNode = it->first;
				osg::MatrixTransform* pMt = dynamic_cast<osg::MatrixTransform*>(pNode);
				osg::AnimationPath* pPath = it->second;
				if (pMt && pPath)
				{
					osg::AnimationPath::ControlPoint controlPoint;
					pPath->getInterpolatedControlPoint(dTime, controlPoint);

					osg::Matrix matrix;
					controlPoint.getMatrix(matrix);
					pMt->setMatrix(matrix); 
				}
			}

			it++;
		}
	}

	void CFeAnimationNode::StepToEnd()
	{
		StepToTime(m_dPeriodSec);
	}

	void CFeAnimationNode::StepToBegin()
	{
		StepToTime(0);
	}

	void CFeAnimationNode::Reset()
	{
		std::map<osg::Node*, osg::AnimationPath*>::iterator it = m_vecAnimationPaths.begin();
		while (it != m_vecAnimationPaths.end())
		{
			if (osg::AnimationPathCallback* pAnimationPathCB = dynamic_cast<osg::AnimationPathCallback*>(it->first->getUpdateCallback()))
			{
				pAnimationPathCB->reset();
				pAnimationPathCB->setPause(true);
			}

			it++;
		}

		StepToBegin();
	}

	void CFeAnimationNode::StopAnimation()
	{
		SetAnimPause(true);
	}

	void CFeAnimationNode::StartAnimation()
	{
		SetAnimPause(false);
	}



	CFeAnimationNode::CTimerCallback::CTimerCallback( CFeAnimationNode* pAnimNode ) :m_opAnimNode(pAnimNode)
		, m_bStartPlay(false)
		, m_dStartTime(0)
		, m_dTimeLong(0)
	{

	}

	void CFeAnimationNode::CTimerCallback::PlayTime( double dTime )
	{
		if(dTime > 0)
		{
			m_dTimeLong = dTime;
		}
	}

	void CFeAnimationNode::CTimerCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
	{
		if(!m_bStartPlay && m_dTimeLong > 0)
		{
			if(m_opAnimNode.valid())
			{
				m_opAnimNode->StartAnimation();
			}

			m_dStartTime = nv->getFrameStamp()->getSimulationTime();
			m_bStartPlay = true;
		}
		else if(m_bStartPlay)
		{
			if(nv->getFrameStamp()->getSimulationTime()-m_dStartTime >= m_dTimeLong)
			{
				m_bStartPlay = false;
				m_dTimeLong = 0;
				m_dStartTime = 0;

				if(m_opAnimNode.valid())
				{
					m_opAnimNode->StopAnimation();
				}
			}
		}

		traverse(node, nv);
	}
	
}