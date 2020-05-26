#include <osgEarthAnnotation/EllipseNode>
#include <osgEarthAnnotation/AnnotationEditing>
#include "ExEllipseNode.h"


namespace FeExtNode
{
	CExEllipseNodeOption::CExEllipseNodeOption()
	{
		m_dLongRadiu = 1000;
		m_dShortRadiu = 500;
		m_dRotate = 0;
	}

	CExEllipseNodeOption::~CExEllipseNodeOption()
	{
	}

}

namespace FeExtNode
{
	CExEllipseNode::CExEllipseNode( FeUtil::CRenderContext* pRender, CExEllipseNodeOption* opt )
		: CExLocalizedNode(pRender, opt)
	{
		InitLocalizedNode();
	}

	CExEllipseNode::~CExEllipseNode(void)
	{
	}

	osgEarth::Annotation::LocalizedNode* CExEllipseNode::GetConcreteNode()
	{
		return new osgEarth::Annotation::EllipseNode(
			m_opMapNode.get()
			,GetPosition()
			,GetLongRadius()
			,GetShortRadius()
			,GetRotate()
			,GetConcreteNodeStyle()
			,osgEarth::Angle(0.001)
			,osgEarth::Angle(360.0-0.001)
			,false);
	}

	osgEarth::Style CExEllipseNode::GetConcreteNodeStyle()
	{
		return CExLocalizedNode::GetConcreteNodeStyle();
	}

	void CExEllipseNode::SetLongRadius( const double& radius )
	{
		if(radius != GetLongRadius())
		{
			GetOption()->LongRadiu() = radius;

			GetEllipseNode()->setRadiusMajor(radius);
			UpdateEditor();
		}
	}

	double CExEllipseNode::GetLongRadius()
	{
		if(GetEllipseNode())
		{
			GetOption()->LongRadiu() = GetEllipseNode()->getRadiusMajor().getValue();
		}
		
		return GetOption()->LongRadiu();
	}

	void CExEllipseNode::SetShortRadius( const double& radius )
	{
		if(radius != GetShortRadius())
		{
			GetOption()->ShortRadiu() = radius;

			GetEllipseNode()->setRadiusMinor(radius);
			UpdateEditor();
		}
	}

	double CExEllipseNode::GetShortRadius()
	{
		if(GetEllipseNode())
		{
			GetOption()->ShortRadiu() = GetEllipseNode()->getRadiusMinor().getValue();
		}
		
		return GetOption()->ShortRadiu();
	}

	void CExEllipseNode::SetRotate( const double& angle )
	{
		if(angle != GetRotate())
		{
			GetOption()->Ratate() = angle;

			GetEllipseNode()->setRotationAngle(angle);
			UpdateEditor();
		}
	}

	double CExEllipseNode::GetRotate()
	{
		if(GetEllipseNode())
		{
			GetOption()->Ratate() = GetEllipseNode()->getRotationAngle().getValue();
		}
		
		return GetOption()->Ratate();
	}

	CExEllipseNodeOption* CExEllipseNode::GetOption()
	{
		return dynamic_cast<CExEllipseNodeOption*>(m_rpOptions.get());
	}

	osgEarth::Annotation::EllipseNode* CExEllipseNode::GetEllipseNode()
	{
		return dynamic_cast<osgEarth::Annotation::EllipseNode*>(m_rpLocalizedNode.get());
	}

	void CExEllipseNode::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}
	
}
