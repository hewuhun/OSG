#include <FeExtNode/ExLineNode.h>

namespace FeExtNode
{
	CExLineNodeOption::CExLineNodeOption()
		:CExFeatureNodeOption()
	{
		
	}

	CExLineNodeOption::~CExLineNodeOption()
	{

	}

}

namespace FeExtNode
{
	CExLineNode::CExLineNode(FeUtil::CRenderContext* pRender, CExLineNodeOption* opt)
		:CExFeatureNode(pRender, opt)
	{
		InitFeatureNode();
	}

	CExLineNode::~CExLineNode()
	{
	}

	osgEarth::Features::Feature* CExLineNode::GetFeature()
	{
		return new osgEarth::Features::Feature(new osgEarth::Symbology::LineString(), 
			m_opMapNode->getMapSRS());
	}

	osgEarth::Style CExLineNode::GetFeatureStyle()
	{
		osgEarth::Style style;
		style.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->clamping()
			= osgEarth::Symbology::AltitudeSymbol::CLAMP_TO_TERRAIN;
		style.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->technique()
			= osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_GPU;
		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->widthUnits()
			= osgEarth::Symbology::Units::PIXELS;
		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->tessellation() = 100;
		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->color() = osgEarth::Symbology::Color(GetLineColor());
		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->width() = GetLineWidth();
		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->stipple() = GetLineStipple();
		return style;
	}

	CExLineNodeOption* CExLineNode::GetOption()
	{
		return dynamic_cast<CExLineNodeOption*>(m_rpOptions.get());
	}

	void CExLineNode::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}

	bool CExLineNode::CheckedVertex()
	{
		if (GetNumVertex() >= 2)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

}