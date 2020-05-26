#include <osgEarthAnnotation/RectangleNode>
#include <osgEarthAnnotation/AnnotationEditing>
#include "ExRectNode.h"



namespace FeExtNode
{
	CExRectNodeOption::CExRectNodeOption()
	{
		m_dWidth = m_dHeight = 1000;
	}
	CExRectNodeOption::~CExRectNodeOption()
	{

	}

}

namespace FeExtNode
{
	CExRectNode::CExRectNode( FeUtil::CRenderContext* pRender, CExRectNodeOption* opt )
		: CExLocalizedNode(pRender, opt)
	{
		InitLocalizedNode();
	}

	CExRectNode::~CExRectNode(void)
	{
	}

	osgEarth::Annotation::LocalizedNode* CExRectNode::GetConcreteNode()
	{
		return new osgEarth::Annotation::RectangleNode(
			m_opMapNode.get()
			,GetPosition()
			,GetWidth()
			,GetHeight()
			,GetConcreteNodeStyle());
	}

	osgEarth::Style CExRectNode::GetConcreteNodeStyle()
	{
		return CExLocalizedNode::GetConcreteNodeStyle();
	}

	void CExRectNode::SetWidth( const double& dWidth )
	{
		if(dWidth != GetWidth())
		{
			GetOption()->Width() = dWidth;

			GetRectNode()->setWidth(dWidth);
			UpdateEditor();
		}
	}

	double CExRectNode::GetWidth()
	{
		if(GetRectNode())
		{
			GetOption()->Width() = GetRectNode()->getWidth().getValue();
		}
		
		return GetOption()->Width();
	}

	void CExRectNode::SetHeight( const double& dHeight )
	{
		if(dHeight != GetHeight())
		{
			GetOption()->Height() = dHeight;

			GetRectNode()->setHeight(dHeight);
			UpdateEditor();
		}
	}

	double CExRectNode::GetHeight()
	{
		if(GetRectNode())
		{
			GetOption()->Height() = GetRectNode()->getHeight().getValue();
		}
		
		return GetOption()->Height();
	}

	CExRectNodeOption* CExRectNode::GetOption()
	{
		return dynamic_cast<CExRectNodeOption*>(m_rpOptions.get());
	}

	osgEarth::Annotation::RectangleNode* CExRectNode::GetRectNode()
	{
		return dynamic_cast<osgEarth::Annotation::RectangleNode*>(m_rpLocalizedNode.get());
	}

	void CExRectNode::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}

}

