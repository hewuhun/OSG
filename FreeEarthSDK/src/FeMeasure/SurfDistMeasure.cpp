#include <FeMeasure/SurfDistMeasure.h>

#include <osg/LineWidth>
#include <osgEarthSymbology/Geometry>
#include <osgEarthSymbology/AltitudeSymbol>
#include <osgEarthSymbology/RenderSymbol>
#include <osgEarthSymbology/LineSymbol>
#include <osgEarthSymbology/TextSymbol>
#include <FeUtils/PathRegistry.h>


namespace FeMeasure
{
	CSurfDistResult::CSurfDistResult( FeUtil::CRenderContext* pContext ) 
		: CDistMeasureResult(pContext)
	{
	}

	bool CSurfDistResult::DoCalculate()
	{
		if(!GetFeature() || !GetData()) return false;

		if(GetFeature()->getGeometry()->size() <= 1)
		{
			GetData()->Value() = 0.0;
		}
		else
		{
			GetData()->Value() = osgEarth::GeoMath::rhumbDistance(GetFeature()->getGeometry()->asVector());
		}

		osg::Vec3d vecPos = GetFeature()->getGeometry()->back();

		if(GetMeasureInfoDisplay())
		{
			GetMeasureInfoDisplay()->SetPosition(vecPos);
			GetMeasureInfoDisplay()->SetOrCreateString(0, FeUtil::ToUTF("地表距离：") + ReviseDistanceCalculate(GetData()->Value()));
		}

		return true;
	}

	osgEarth::Features::Feature* CSurfDistResult::CreateFeature()
	{
		if (!GetRenderContext()) return NULL;

		osgEarth::Features::Feature* pFeature = new osgEarth::Features::Feature(
			new osgEarth::Symbology::LineString(),GetRenderContext()->GetMapSRS());

		pFeature->geoInterp() = osgEarth::GEOINTERP_RHUMB_LINE;

		osgEarth::Symbology::AltitudeSymbol* pAlt = pFeature->style()->getOrCreate<osgEarth::Symbology::AltitudeSymbol>();
		pAlt->clamping() = pAlt->CLAMP_TO_TERRAIN;
		pAlt->technique() = pAlt->TECHNIQUE_GPU;

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
	CSurfDistMeasure::CSurfDistMeasure( FeUtil::CRenderContext* pContext )
		: CMeasure(pContext)
	{
		SetKey("surfdist");
		SetTitle("地表距离");
		MeasureType() = EM_SurfDist;
	}

	CMeasureResult* CSurfDistMeasure::CreateMeasureResult()
	{
		return new CSurfDistResult(GetRenderContext());
	}

}

