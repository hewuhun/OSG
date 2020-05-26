#include <osgEarthAnnotation/CircleNode>
#include "ExSectorNode.h"

namespace FeExtNode
{
	CExSectorNodeOption::CExSectorNodeOption()
	{
		m_dArcStart = 0;
		m_dArcEnd = 90;
	}

	CExSectorNodeOption::~CExSectorNodeOption()
	{

	}

}

namespace FeExtNode
{
	CExSectorNode::CExSectorNode( FeUtil::CRenderContext* pRender, CExSectorNodeOption* opt )
		: CExCircleNode(pRender, opt)
	{
		GetCircleNode()->setArcStart(GetOption()->ArcStart());
		GetCircleNode()->setArcEnd(GetOption()->ArcEnd());
		GetCircleNode()->setPie(true);
	}

	CExSectorNode::~CExSectorNode(void)
	{
	}

	void CExSectorNode::SetArcStart( const double& angle )
	{
		if(angle != GetArcStart())
		{
			GetOption()->ArcStart() = angle;

			GetCircleNode()->setArcStart(angle);
			UpdateEditor();
		}
	}

	double CExSectorNode::GetArcStart()
	{
		if(GetCircleNode())
		{
			GetOption()->ArcStart() = GetCircleNode()->getArcStart().getValue();
		}
		
		return GetOption()->ArcStart();
	}

	void CExSectorNode::SetArcEnd( const double& dAngle )
	{
		if(dAngle != GetArcEnd())
		{
			GetOption()->ArcEnd() = dAngle;

			GetCircleNode()->setArcEnd(dAngle);
			UpdateEditor();
		}
	}

	double CExSectorNode::GetArcEnd()
	{
		if(GetCircleNode())
		{
			GetOption()->ArcEnd() = GetCircleNode()->getArcEnd().getValue();
		}

		return GetOption()->ArcEnd();
	}

	CExSectorNodeOption* CExSectorNode::GetOption()
	{
		return dynamic_cast<CExSectorNodeOption*>(m_rpOptions.get());
	}

	void CExSectorNode::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}

}


namespace FeExtNode
{
	CExArcNode::CExArcNode( FeUtil::CRenderContext* pRender, CExArcNodeOption* opt )
		: CExSectorNode(pRender, opt)
	{
		GetCircleNode()->setPie(false);
	}

	CExArcNode::~CExArcNode( void )
	{
	}

	CExArcNodeOption* CExArcNode::GetOption()
	{
		return dynamic_cast<CExArcNodeOption*>(m_rpOptions.get());
	}

	void CExArcNode::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}

}