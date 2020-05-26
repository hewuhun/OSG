#include <FeMeasure/SpaceAreaMeasure.h>

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
	CSpaceAreaResult::CSpaceAreaResult( FeUtil::CRenderContext* pContext ) 
		: CDistMeasureResult(pContext)
	{
	}

	bool CSpaceAreaResult::DoCalculate()
	{
		if(!GetFeature() || !GetData()) return false;

		if(GetFeature()->getGeometry()->size() <= 2)
		{
			GetData()->Value() = 0.0;
			return true;
		}
		else
		{
			//更换了面积计算的算法，使得凹多边形的面积也能准确计算----c00005
			//m_dArea = TriangleArea(m_opFeature->getGeometry()->asVector());
			std::vector<osg::Vec3d> vectorArray = GetFeature()->getGeometry()->asVector();
			osg::ref_ptr<osg::Vec3dArray> pArray = new osg::Vec3dArray;
			for (int i = 0; i < vectorArray.size(); i++)
			{
				osg::Vec3d xyzArray;
				FeUtil::DegreeLLH2XYZ(GetRenderContext(), vectorArray.at(i), xyzArray);
				pArray->push_back(xyzArray);
			}

			GetData()->Value() = FeMath::ComputePolygonSpaceArea(pArray);
		}

		osg::Vec3d vecPos = GetFeature()->getGeometry()->back();

		if(GetMeasureInfoDisplay())
		{
			GetMeasureInfoDisplay()->SetPosition(vecPos);
			GetMeasureInfoDisplay()->SetOrCreateString(0, FeUtil::ToUTF("空间面积：" + ReviseAreaCalculate(GetData()->Value())));
		}
		
		return true;
	}

	osgEarth::Features::Feature* CSpaceAreaResult::CreateFeature()
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
	CSpaceAreaMeasure::CSpaceAreaMeasure( FeUtil::CRenderContext* pContext )
		: CMeasure(pContext)
	{
		SetKey("spaceArea");
		SetTitle("空间面积");
		MeasureType() = EM_SpaceArea;
	}

	CMeasureResult* CSpaceAreaMeasure::CreateMeasureResult()
	{
		return new CSpaceAreaResult(GetRenderContext());
	}

}

