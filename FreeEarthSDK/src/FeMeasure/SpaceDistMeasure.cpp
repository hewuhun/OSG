#include <FeMeasure/SpaceDistMeasure.h>

#include <osgEarthAnnotation/PlaceNode>
#include <osgEarthSymbology/Geometry>
#include <osgEarthSymbology/AltitudeSymbol>
#include <osgEarthSymbology/RenderSymbol>
#include <osgEarthSymbology/LineSymbol>
#include <osgEarthSymbology/TextSymbol>
#include <FeUtils/PathRegistry.h>
#include <FeUtils/StrUtil.h>

namespace FeMeasure
{
	CSpaceDistResult::CSpaceDistResult( FeUtil::CRenderContext* pContext ) 
		: CDistMeasureResult(pContext)
	{
	}

	bool CSpaceDistResult::DoCalculate()
	{
		if(!GetFeature() || !GetData()) return false;

		if(GetFeature()->getGeometry()->size() <= 1)
		{
			GetData()->Value() = 0.0;
		}
		else
		{
			GetData()->Value() = osgEarth::GeoMath::distance(GetFeature()->getGeometry()->asVector());
		}

		osg::Vec3d vecPos = GetFeature()->getGeometry()->back();

		if(GetMeasureInfoDisplay())
		{
			GetMeasureInfoDisplay()->SetPosition(vecPos);
			GetMeasureInfoDisplay()->SetOrCreateString(0, FeUtil::ToUTF("直线距离：") + ReviseDistanceCalculate(GetData()->Value()));
		}

		return true;
	}

	osgEarth::Features::Feature* CSpaceDistResult::CreateFeature()
	{
		if (!GetRenderContext()) return NULL;
		
		osgEarth::Features::Feature* pFeature = new osgEarth::Features::Feature(
			new osgEarth::Symbology::LineString(),GetRenderContext()->GetMapSRS());

		// offset to mitigate Z fighting
		osgEarth::Symbology::RenderSymbol* pRender = pFeature->style()->getOrCreate<osgEarth::Symbology::RenderSymbol>();
		pRender->depthOffset()->enabled() = true;
		pRender->depthOffset()->minBias() = 1000;

		// define a style for the line
		osgEarth::Symbology::LineSymbol* pLs = pFeature->style()->getOrCreate<osgEarth::Symbology::LineSymbol>();
		pLs->stroke()->color() = osgEarth::Symbology::Color::Yellow;
		pLs->stroke()->width() = 2.0f;
		pLs->stroke()->widthUnits() = osgEarth::Symbology::Units::PIXELS;
		pLs->tessellation() = 150;

		return pFeature;
	}

}

namespace FeMeasure
{
	CSpaceDistMeasure::CSpaceDistMeasure( FeUtil::CRenderContext* pContext )
		: CMeasure(pContext)
	{
		SetKey("lineDist");
		SetTitle("直线距离");
		MeasureType() = EM_SpaceDist;
	}

	CMeasureResult* CSpaceDistMeasure::CreateMeasureResult()
	{
		return new CSpaceDistResult(GetRenderContext());
	}

}

