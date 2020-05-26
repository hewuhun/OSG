#include <FeMeasure/ProfileAnalysisMeasure.h>

#include <osgEarthSymbology/Geometry>
#include <osgEarthSymbology/AltitudeSymbol>
#include <osgEarthSymbology/RenderSymbol>
#include <osgEarthSymbology/LineSymbol>
#include <osgEarthSymbology/TextSymbol>

#include <FeUtils/CoordConverter.h>


namespace FeMeasure
{

	CProfileAnalysisResult::CProfileAnalysisResult( FeUtil::CRenderContext* pContext )
		: CDistMeasureResult(pContext)
		, m_pData(0)
	{
	}

	bool CProfileAnalysisResult::DoCalculate()
	{
		return true;
	}

	osgEarth::Features::Feature* CProfileAnalysisResult::CreateFeature()
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

	bool CProfileAnalysisResult::FinalCalculateImplement()
	{
		if(!GetData()) return false;

		//将以前的计算结果清空
		GetData()->SamplePoints()->clear();

		//重写，将计算出来的数据进行整体的保存，用于界面中调用使用
		if(GetFeature())
		{
			if(GetFeature()->getGeometry()->size() <= 0)
			{
				return false;
			}
			else if(GetFeature()->getGeometry()->size() <= 1)
			{
				//GetData()->m_dDistance = 0.0;
			}
			else
			{
				//计算地表距离距离
				GetData()->SurfaceDistance() = osgEarth::GeoMath::rhumbDistance(GetFeature()->getGeometry()->asVector());

				//计算直线距离
				GetData()->LineDistance() = osgEarth::GeoMath::distance(GetFeature()->getGeometry()->asVector());

				//采样点数以及采样的距离，计算出每一个采样点的经纬高度信息
				GetData()->SampleDistance() = GetData()->SurfaceDistance() / (GetData()->SampleNum()-1);

				//将所有的顶点进行采样点的采集
				double dDeltaDis = 0.0;
				std::vector<osg::Vec3d> vectorArray = GetFeature()->getGeometry()->asVector();
				if (vectorArray.size() > 0)
				{
					GetData()->SamplePoints()->push_back(vectorArray.at(0));
					GetData()->LowestPoint() = vectorArray.at(0);
					GetData()->HighestPoint() = vectorArray.at(0);
				}

				//开始进行采样点的计算以及找出最大高度和最小高度的关键点
				for(int i = 1; i < vectorArray.size(); i++)
				{
					dDeltaDis = SubsectionArray(vectorArray.at(i-1), vectorArray.at(i), GetData()->SampleDistance(), GetData()->SamplePoints(), dDeltaDis);
				}

				if (!(dDeltaDis > -0.000001 && dDeltaDis < 0.000001))
				{
					GetData()->SamplePoints()->push_back(vectorArray.at(vectorArray.size()-1));
				}
			}
		}

		return true;
	}

	void CProfileAnalysisResult::SetSamplePoint( unsigned int unNum )
	{
		if(GetData())
		{
			GetData()->SampleNum() = unNum;
		}
	}

	double CProfileAnalysisResult::SubsectionArray( osg::Vec3d start, osg::Vec3d end, double dInternal, osg::Vec3dArray* pOutArray, double dDelta /*= 0.0*/ )
	{
		if(!GetData()) return 0.0;

		//将度数转化为弧度
		osg::Vec3d startRadiusLLH = osg::Vec3d(osg::DegreesToRadians(start.x()), osg::DegreesToRadians(start.y()), start.z());
		osg::Vec3d endRadiusLLH = osg::Vec3d(osg::DegreesToRadians(end.x()), osg::DegreesToRadians(end.y()), end.z());

		double dBearing = osgEarth::GeoMath::rhumbBearing(startRadiusLLH.y(), startRadiusLLH.x(), endRadiusLLH.y(), endRadiusLLH.x());
		double dDistance = osgEarth::GeoMath::rhumbDistance(startRadiusLLH.y(), startRadiusLLH.x(), endRadiusLLH.y(), endRadiusLLH.x());

		if (dDelta > -0.000001 && dDelta < 0.000001)
		{
			dDistance -= dInternal;
		}
		else
		{
			dDistance += dDelta;
		}

		int nCount = 1;
		while (dDistance > -0.000001)
		{
			if((dDistance < 0.000001))
			{
				pOutArray->push_back(end);

				//计算最大高度和最小高度值的关键点
				if (GetData()->LowestPoint().z() > end.z())
				{
					GetData()->LowestPoint() = end;
				}
				if (GetData()->HighestPoint().z() < end.z())
				{
					GetData()->HighestPoint() = end;
				}
				break;
			}
			else if (dDistance > 0.000001)
			{
				double dCurDistance = 0.0;
				if (dDelta < -0.000001 || dDelta > 0.000001)
				{
					dCurDistance = -dDelta;
					dDelta = 0.0;
				}
				else
				{
					dCurDistance = dInternal*nCount;
				}

				osg::Vec3d tempPoint;
				osgEarth::GeoMath::rhumbDestination(startRadiusLLH.y(), startRadiusLLH.x(), dBearing, dCurDistance, tempPoint.y(), tempPoint.x());

				//将结果的弧度值转化为角度值，并获取当前点的高度值
				tempPoint.set(osg::Vec3d(osg::RadiansToDegrees(tempPoint.x()), osg::RadiansToDegrees(tempPoint.y()), tempPoint.z()));
				if (GetRenderContext())
				{
					FeUtil::DegreeLL2LLH(GetRenderContext(), tempPoint);
				}

				pOutArray->push_back(tempPoint);

				//计算最大高度和最小高度值的关键点
				if (GetData()->LowestPoint().z() > tempPoint.z())
				{
					GetData()->LowestPoint() = tempPoint;
				}
				if (GetData()->HighestPoint().z() < tempPoint.z())
				{
					GetData()->HighestPoint() = tempPoint;
				}

				dDistance -= dInternal;
				nCount++;
			}
		}

		if (dDistance < -0.000001)
		{
			return dDistance;
		}

		return 0.0;
	}

}


namespace FeMeasure
{
	
	CProfileAnalysisMeasure::CProfileAnalysisMeasure( FeUtil::CRenderContext* pContext ) : CMeasure(pContext)
	{
		SetKey("profileAnalysis");
		SetTitle("剖面分析");
		MeasureType() = EM_ProfileAnalysis;
	}

	CMeasureResult* CProfileAnalysisMeasure::CreateMeasureResult()
	{
		m_opCurResult = new CProfileAnalysisResult(GetRenderContext()); 
		return m_opCurResult.get();
	}

	void CProfileAnalysisMeasure::SetSamplePoint( unsigned int unNum )
	{
		if(m_opCurResult.valid() && unNum > 0)
		{
			m_opCurResult->SetSamplePoint(unNum); 
			m_opCurResult->FinalCalculateImplement();
			SendMeasureMsg(EM_RESULT_CHANGED);
		}
	}


}


