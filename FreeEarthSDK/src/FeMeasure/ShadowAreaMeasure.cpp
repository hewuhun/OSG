#include <FeMeasure/ShadowAreaMeasure.h>

#include <osgEarthSymbology/Geometry>
#include <osgEarthSymbology/AltitudeSymbol>
#include <osgEarthSymbology/RenderSymbol>
#include <osgEarthSymbology/LineSymbol>
#include <osgEarthSymbology/TextSymbol>

#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>
#include <FeUtils/PathRegistry.h>


namespace FeMeasure
{
	CShadowAreaResult::CShadowAreaResult( FeUtil::CRenderContext* pContext ) 
		: CDistMeasureResult(pContext)
	{
	}

	bool CShadowAreaResult::DoCalculate()
	{
		if(!GetFeature() || !GetData()) return false;

		if(GetFeature()->getGeometry()->size() <= 2)
		{
			GetData()->Value() = 0.0;
			return true;
		}
		else
		{
			std::vector<osg::Vec3d> vectorArray = GetFeature()->getGeometry()->asVector();
			std::vector<double> vecX;
			std::vector<double> vecY;
			for (int i = 0; i < vectorArray.size(); i++)
			{
				vecX.push_back(vectorArray.at(i).x());
				vecY.push_back(vectorArray.at(i).y());
			}

			GetData()->Value() = FeMath::ComputePolygonShadowArea(&vecX[0],&vecY[0],vecY.size());
		}

		osg::Vec3d vecPos = GetFeature()->getGeometry()->back();

		if(GetMeasureInfoDisplay())
		{
			GetMeasureInfoDisplay()->SetPosition(vecPos);
			GetMeasureInfoDisplay()->SetOrCreateString(0, FeUtil::ToUTF("投影面积：" + ReviseAreaCalculate(GetData()->Value())));
		}

		return true;
	}

	osgEarth::Features::Feature* CShadowAreaResult::CreateFeature()
	{
		if (!GetRenderContext()) return NULL;

		osgEarth::Features::Feature* pFeature = new osgEarth::Features::Feature(
			new osgEarth::Symbology::Polygon(),GetRenderContext()->GetMapSRS());

		pFeature->geoInterp() = osgEarth::GEOINTERP_RHUMB_LINE;

		pFeature->style()->getOrCreate<osgEarth::Symbology::PolygonSymbol>()->fill()->color() =
			osgEarth::Symbology::Color(osg::Vec4f(0.9, 0.9, 0.0, 0.5));

		// clamp to the terrain skin as it pages in
		osgEarth::Symbology::AltitudeSymbol* pAlt = pFeature->style()->getOrCreate<osgEarth::Symbology::AltitudeSymbol>();
		pAlt->clamping() = pAlt->CLAMP_TO_TERRAIN;
		pAlt->technique() = pAlt->TECHNIQUE_DRAPE;

		osgEarth::Symbology::LineSymbol* pLine = pFeature->style()->getOrCreate<osgEarth::Symbology::LineSymbol>();
		pLine->stroke()->color() = osgEarth::Symbology::Color(0.5, 0.9, 0.5, 1.0);
		pLine->stroke()->width() =2.0f;
		pLine->stroke()->widthUnits() = osgEarth::Symbology::Units::PIXELS;
		//pLine->tessellation() = 150;

		return pFeature;
	}

}

namespace FeMeasure
{
	CShadowAreaMeasure::CShadowAreaMeasure( FeUtil::CRenderContext* pContext )
		: CMeasure(pContext)
	{
		SetKey("shadowArea");
		SetTitle("投影面积");
		MeasureType() = EM_ShadowArea;
	}

	CMeasureResult* CShadowAreaMeasure::CreateMeasureResult()
	{
		return new CShadowAreaResult(GetRenderContext());
	}

}

