#include <FeExtNode/ExPolygonNode.h>

namespace FeExtNode
{
	CExPolygonNodeOption::CExPolygonNodeOption()
		:CExFeatureNodeOption()
	{

	}

	CExPolygonNodeOption::~CExPolygonNodeOption()
	{

	}

}

namespace FeExtNode
{
	CExPolygonNode::CExPolygonNode( FeUtil::CRenderContext* pRender, CExPolygonNodeOption* opt )
		:CExFeatureNode(pRender, opt)
	{
		InitFeatureNode();
	}

	CExPolygonNode::~CExPolygonNode()
	{

	}

	osgEarth::Features::Feature* CExPolygonNode::GetFeature()
	{
		osgEarth::Features::Feature* feature = new osgEarth::Features::Feature(new osgEarth::Symbology::Polygon, 
			m_opMapNode->getMapSRS());

		if(feature)
		{
			feature->geoInterp() = osgEarth::GEOINTERP_RHUMB_LINE;
		}
		
		return feature;
	}

	osgEarth::Style CExPolygonNode::GetFeatureStyle()
	{
		osgEarth::Style style;
		style.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->clamping() 
			= osgEarth::Symbology::AltitudeSymbol::CLAMP_TO_TERRAIN;
		style.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->technique()
			=osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_DRAPE;
		style.getOrCreate<osgEarth::Symbology::PolygonSymbol>()->fill()->color() =
			osgEarth::Symbology::Color(GetFillColor());
		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->color() = osgEarth::Symbology::Color(GetLineColor());
		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->width() = GetLineWidth();
		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->stipple() = GetLineStipple();

		return style;
	}

	CExPolygonNodeOption* CExPolygonNode::GetOption()
	{
		return dynamic_cast<CExPolygonNodeOption*>(m_rpOptions.get());
	}

	void CExPolygonNode::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}

	bool CExPolygonNode::CheckedVertex()
	{
		if (GetNumVertex() >= 3)
		{
			return true;
		}
		else
		{
			return false;
		}
	}


}