#include <FeExtNode/ExLodNode.h>

namespace FeExtNode
{
	using namespace osgEarth;

	CExLodNodeOption::CExLodNodeOption(const double& dMinRange, const double& dMaxRange)
		:CExternNodeOption()
		,m_dMinRange(dMinRange)
		,m_dMaxRange(dMaxRange)
	{
		m_bUserCenter = false;
		m_vecUserCenter = osg::Vec3d(0, 0, 0);
	}

	CExLodNodeOption::~CExLodNodeOption()
	{

	}
}

namespace FeExtNode
{
	CExLodNode::CExLodNode( CExLodNodeOption* opt )
		:CExternNode(opt)
		,m_bLodVisible(true)
	{

	}

	CExLodNode::~CExLodNode()
	{

	}

	void CExLodNode::SetRange(const double& dMinRange, const double& dMaxRange)
	{
		GetOption()->minRange() = dMinRange;
		GetOption()->maxRange() = dMaxRange;
	}

	void CExLodNode::GetRange(double& dMinRange, double& dMaxRange)
	{
		dMinRange = GetOption()->minRange();
		dMaxRange = GetOption()->maxRange();
	}

	void CExLodNode::SetCenter( osg::Vec3d center, bool bUserCenter )
	{
		GetOption()->userCenter() = center;
		GetOption()->isUserCetner() = bUserCenter;
	}

	osg::Vec3d CExLodNode::GetCenter()
	{
		if (GetOption()->isUserCetner())
		{
			return GetOption()->userCenter();
		}

		return getBound().center();
	}

// 	void CExLodNode::SetVisible( const bool& bVisible )
// 	{
// 		if (bVisible == GetOption()->visible())
// 		{
// 			return ;
// 		}
// 
// 		GetOption()->visible() = bVisible;
// 		UpdateVisibleAndEvent();
// 	}

	void CExLodNode::SetLodVisible(bool bVisible)
	{
		if (bVisible == m_bLodVisible)
		{
			return;
		}

		m_bLodVisible = bVisible;
		UpdateVisibleAndEvent();
	}

	bool CExLodNode::GetLodVisible()
	{
		return m_bLodVisible;
	}

	CExLodNodeOption* CExLodNode::GetOption()
	{
		return dynamic_cast<CExLodNodeOption*>(m_rpOptions.get());
	}

	void CExLodNode::traverse(osg::NodeVisitor& nv)
	{
		if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
		{
			/**
			  nv.getEyePoint()获取到的视点位置为局部坐标(使用当前遍历节点的局部坐标系)，而我们设置
			  可视范围时参考的是全局坐标，所以需要转为全局坐标进行计算。（g00034 2017.02.23）
			*/
			//double dis = nv.getDistanceToEyePoint(GetCenter(), true);
			double dis = (GetCenter() - (nv.getEyePoint() * getWorldMatrices().back())).length();

			if (dis >= GetOption()->minRange() && dis <= GetOption()->maxRange())
			{
				SetLodVisible(true);
			}
			else
			{
				SetLodVisible(false);

				return ;
			}
		}

		CExternNode::traverse(nv);
	}

	void CExLodNode::UpdateVisibleAndEvent()
	{
		if (GetOption()->visible() && m_bLodVisible)
		{
			if (GetOption()->acceptEvent())
			{
				setNodeMask(VISIBLE_MASK | ACCEPT_EVENT_MASK);
			}
			else
			{
				setNodeMask(VISIBLE_MASK);
			}

			std::for_each(_children.begin(), _children.end(), 
				[&](osg::ref_ptr<osg::Node>& node){ node->setNodeMask(VISIBLE_MASK);}
			);
		}
		else
		{
			//setNodeMask(UN_VISIBLE_EVENT_MASK);

			/// 为了不影响LOD节点的遍历， 只隐藏子节点，不隐藏LOD节点本身 （g00034 2017.02.23）
			std::for_each(_children.begin(), _children.end(), 
				[&](osg::ref_ptr<osg::Node>& node){ node->setNodeMask(UN_VISIBLE_EVENT_MASK);}
			);
		}
	}

}