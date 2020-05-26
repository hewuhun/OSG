#include <osgEarthAnnotation/CircleNode>
#include <osgEarthAnnotation/AnnotationEditing>
#include "ExCircleNode.h"


namespace FeExtNode
{
	CExCircleNodeOption::CExCircleNodeOption()
	{
		m_dRadius = 1000;
	}
	CExCircleNodeOption::~CExCircleNodeOption()
	{

	}

}

namespace FeExtNode
{
	CExCircleNode::CExCircleNode( FeUtil::CRenderContext* pRender, CExCircleNodeOption* opt )
		: CExLocalizedNode(pRender, opt)
	{
		InitLocalizedNode();
	}

	CExCircleNode::~CExCircleNode(void)
	{
	}

	osgEarth::Annotation::LocalizedNode* CExCircleNode::GetConcreteNode()
	{
		return new osgEarth::Annotation::CircleNode(
			m_opMapNode.get()
			,GetOption()->geoPoint()
			,GetOption()->Radius()
			,GetConcreteNodeStyle()
			,osgEarth::Angle(0.001)
			,osgEarth::Angle(360.0-0.001)
			,false);
	}

	osgEarth::Style CExCircleNode::GetConcreteNodeStyle()
	{
		return CExLocalizedNode::GetConcreteNodeStyle();
	}

	void CExCircleNode::SetRadius( const double& radius )
	{
		if(radius != GetRadius())
		{
			GetOption()->Radius() = radius;

			GetCircleNode()->setRadius(radius);
			UpdateEditor();
		}
	}

	double CExCircleNode::GetRadius()
	{
		if(GetCircleNode())
		{
			GetOption()->Radius() = GetCircleNode()->getRadius().getValue();
		}
		
		return GetOption()->Radius();
	}
		
	CExCircleNodeOption* CExCircleNode::GetOption()
	{
		return dynamic_cast<CExCircleNodeOption*>(m_rpOptions.get());
	}

	osgEarth::Annotation::CircleNode* CExCircleNode::GetCircleNode()
	{
		return dynamic_cast<osgEarth::Annotation::CircleNode*>(m_rpLocalizedNode.get());
	}

	void CExCircleNode::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}

}

