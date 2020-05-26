#include <FeMeasure/FillCutAnalysisMeasure.h>

#include <osgEarthSymbology/Geometry>
#include <osgEarthSymbology/AltitudeSymbol>
#include <osgEarthSymbology/RenderSymbol>
#include <osgEarthSymbology/LineSymbol>
#include <osgEarthSymbology/TextSymbol>

#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>


namespace FeMeasure
{
	CFillCutAnalysisResult::CFillCutAnalysisResult( FeUtil::CRenderContext* pContext )
		: CMeasureResult(pContext)
		, m_bUserDefRefHei(false)
		, m_pData(0)
	{
		InitFeatureNode();
	}

	bool CFillCutAnalysisResult::CalculateImplement( const osg::Vec3dArray& vecPointsLLH )
	{
		if(m_rpFeature.valid() && m_rpFeatureNode.valid())
		{
			m_rpFeature->getGeometry()->assign(vecPointsLLH.begin(), vecPointsLLH.end());
			m_rpFeatureNode->init();
		}

		return true;
	}

	bool CFillCutAnalysisResult::FinalCalculateImplement()
	{
		if(m_rpFeature->getGeometry()->size() <= 2)
		{
			GetData()->dCutArea = 0.0;
			GetData()->dFillArea = 0.0;
			GetData()->dFillVolume = 0.0;
			GetData()->dCutVolune = 0.0;
		}
		else 
		{
			std::vector<osg::Vec3d> vectorArray = m_rpFeature->getGeometry()->asVector();
			//将顶点数组转化成vec3dArray经纬度坐标数组
			osg::ref_ptr<osg::Vec3dArray> pLLHArray = new osg::Vec3dArray;
			for (int i = 0; i < vectorArray.size(); i++)
			{
				pLLHArray->push_back(vectorArray.at(i));
			}

			//将顶点数组转化成vec3dArray世界坐标数组
			osg::ref_ptr<osg::Vec3dArray> pXYZArray = new osg::Vec3dArray;
			for (int i = 0; i < vectorArray.size(); i++)
			{
				osg::Vec3d xyzArray;
				FeUtil::DegreeLLH2XYZ(GetRenderContext(), vectorArray.at(i), xyzArray);
				pXYZArray->push_back(xyzArray);
			}


			//计算多边形的边界，根据边界对多边形进行网格划分，然后找出位于多边形内部的顶点，进行平均高程的计算
			double dMaxLon, dMinLon, dMaxLat, dMinLat;
			CalculateScope(pLLHArray, dMinLon, dMaxLon, dMinLat, dMaxLat);

			osg::ref_ptr<osg::Vec3dArray> pContainPoints = new osg::Vec3dArray;

			CalculateContainPoints(pLLHArray, osg::Vec3d(dMinLon, dMinLat, 0.0), osg::Vec3d(dMaxLon, dMaxLat, 0.0),
				pContainPoints, GetData()->vecMaxHeiPoint, GetData()->vecMinHeiPoint, 50);


			//计算一个中间高度作为基准面高度
			if (m_bUserDefRefHei == false)
			{
				GetData()->dReferenHei = (GetData()->vecMaxHeiPoint.z()+GetData()->vecMinHeiPoint.z())/2.0;
			}
			m_bUserDefRefHei = false;

			int nUpPoints = 0, nDownPoints = 0;
			double dAverageHei = 0.0;
			for (int i = 0; i < pContainPoints->size(); i++)
			{
				dAverageHei += pContainPoints->at(i).z();
				if (GetData()->dReferenHei < pContainPoints->at(i).z())
				{
					nUpPoints++;
				}
				else
				{
					nDownPoints++;
				}
			}
			dAverageHei /= pContainPoints->size();

			//计算多边形的投影面积
			double dArea = FeMath::PolygonArea(pXYZArray);
			double dVolume = dArea*dAverageHei;

			GetData()->dFillArea = dArea*nDownPoints/(nDownPoints+nUpPoints);
			GetData()->dCutArea = dArea*nUpPoints/(nDownPoints+nUpPoints);

			GetData()->dFillVolume = dVolume*nDownPoints/(nDownPoints+nUpPoints);
			GetData()->dCutVolune = dVolume*nUpPoints/(nDownPoints+nUpPoints);

			UpdateCalculateFeature();

			return true;
		}

		return false;
	}

	void CFillCutAnalysisResult::CalculateScope( osg::Vec3dArray* pArray, double& dMinLon, double& dMaxLon, double& dMinLat, double& dMaxLat )
	{
		int nSize = pArray->size();
		if (nSize > 0)
		{
			dMinLon = pArray->at(0).x();
			dMaxLon = pArray->at(0).x(); 

			dMinLat = pArray->at(0).y();
			dMaxLat = pArray->at(0).y();
		}

		for (int i = 1; i < pArray->size(); i++)
		{
			osg::Vec3d point = pArray->at(i);
			if (dMinLon > point.x())
			{
				dMinLon = point.x();
			}
			if (dMaxLon < point.x())
			{
				dMaxLon = point.x();
			}
			if (dMinLat > point.y())
			{
				dMinLat = point.y();
			}
			if (dMaxLat < point.y())
			{
				dMaxLat = point.y();
			}
		}
	}

	void CFillCutAnalysisResult::CalculateContainPoints( osg::Vec3dArray* pLLHArray, osg::Vec3d minLLHLonLat, osg::Vec3d maxLLHLonLat, 
		osg::Vec3dArray* pLLHOutArray,  osg::Vec3d& maxHeiPoint, osg::Vec3d& minHeiPoint, int nIntervalNum /*= 50*/ )
	{
		if(!GetRenderContext()) return;

		maxHeiPoint = osg::Vec3d(0.0, 0.0, -100000.0);
		minHeiPoint = osg::Vec3d(0.0, 0.0, 100000.0);

		osg::ref_ptr<osg::Vec3dArray> pNoHeigthArray = new osg::Vec3dArray;
		for(int i = 0; i < pLLHArray->size(); i++)
		{
			pNoHeigthArray->push_back(osg::Vec3d(pLLHArray->at(i).x(), pLLHArray->at(i).y(), 0.0));
		}

		double dDeltaLon = (maxLLHLonLat.x() - minLLHLonLat.x())/nIntervalNum;
		double dDeltaLat = (maxLLHLonLat.y() - minLLHLonLat.y())/nIntervalNum;

		for (int i = 0; i < nIntervalNum; i++)
		{
			for(int j = 0; j < nIntervalNum; j++)
			{
				osg::Vec3d testPoint = osg::Vec3d(minLLHLonLat.x()+dDeltaLon*i, minLLHLonLat.y()+dDeltaLat*j, 0.0);

				if (FeMath::ContainPointInPolygon(testPoint, pNoHeigthArray))
				{
					FeUtil::DegreeLL2LLH(GetRenderContext(), testPoint);

					if (minHeiPoint.z() > testPoint.z())
					{
						minHeiPoint = testPoint;
					}
					if (maxHeiPoint.z() < testPoint.z())
					{
						maxHeiPoint = testPoint;
					}
					
					pLLHOutArray->push_back(testPoint);
				}
			}
		}
	}

	void CFillCutAnalysisResult::UpdateCalculateFeature()
	{
		if(m_rpFeature.valid() && GetData())
		{
			//先将以前绘制的图形进行清除
			m_rpFillCutFeature->getGeometry()->clear();
			osg::ref_ptr<osgEarth::Features::MultiGeometry> pMultiGeom = 
				dynamic_cast<osgEarth::Features::MultiGeometry*>(m_rpBorderLineFeature->getGeometry());
			if (pMultiGeom.valid())
			{
				pMultiGeom->getComponents().clear();
			}

			//重新绘制图形
			std::vector<osg::Vec3d> polygonCoords = m_rpFeature->getGeometry()->asVector();
			for(int i = 0; i < polygonCoords.size(); i++)
			{
				osg::Vec3d coord = osg::Vec3d(polygonCoords.at(i).x(), polygonCoords.at(i).y(), GetData()->dReferenHei);
				m_rpFillCutFeature->getGeometry()->push_back(coord);

				osg::ref_ptr<osgEarth::Features::LineString> pLineGeom = new osgEarth::Features::LineString();
				pLineGeom->push_back(polygonCoords.at(i));
				pLineGeom->push_back(coord);
				osg::ref_ptr<osgEarth::Features::MultiGeometry> pMultiGeom = dynamic_cast<osgEarth::Features::MultiGeometry*>(m_rpBorderLineFeature->getGeometry());
				if (pMultiGeom.valid())
				{
					pMultiGeom->add(pLineGeom.get());
				}
			}
		}

		m_rpFillCutFNode->init();
		m_rpBorderLineFNode->init();
	}

	void CFillCutAnalysisResult::SetReferenceHeight( double dHei )
	{
		if(GetData())
		{
			m_bUserDefRefHei = true;
			GetData()->dReferenHei = dHei;
		}
	}

	void CFillCutAnalysisResult::InitFeatureNode()
	{
		if(!GetRenderContext()) return;
		
		//初始化地表的区域形状
		{
			m_rpPolygon = new osgEarth::Symbology::Polygon();

			m_rpFeature = new osgEarth::Features::Feature(m_rpPolygon.get(),GetRenderContext()->GetMapNode()->getMapSRS());
			m_rpFeature->geoInterp() = osgEarth::GEOINTERP_RHUMB_LINE;

			m_rpFeature->style()->getOrCreate<osgEarth::Symbology::PolygonSymbol>()->fill()->color() =
				osgEarth::Symbology::Color(osg::Vec4f(0.9, 0.9, 0.0, 0.5));

			// clamp to the terrain skin as it pages in
			osgEarth::Symbology::AltitudeSymbol* pAlt = m_rpFeature->style()->getOrCreate<osgEarth::Symbology::AltitudeSymbol>();
			pAlt->clamping() = pAlt->CLAMP_TO_TERRAIN;
			pAlt->technique() = pAlt->TECHNIQUE_DRAPE;

			osgEarth::Symbology::LineSymbol* pLine = m_rpFeature->style()->getOrCreate<osgEarth::Symbology::LineSymbol>();
			pLine->stroke()->color() = osgEarth::Symbology::Color(0.9, 0.9, 0.0, 1.0);
			pLine->stroke()->width() =2.0f;
			pLine->stroke()->widthUnits() = osgEarth::Symbology::Units::PIXELS;
			pLine->tessellation() = 150;

			osgEarth::Symbology::Style style;
			osgEarth::Features::GeometryCompilerOptions options;
			options.maxGranularity() = 180.0;
			m_rpFeatureNode = new osgEarth::Annotation::FeatureNode(GetRenderContext()->GetMapNode(), m_rpFeature.get(), style, options);
			m_rpFeatureNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
			addChild(m_rpFeatureNode.get());
		}

		//初始化裁切区的平面的区域形状
		{
			m_rpFillCutPolygon = new osgEarth::Symbology::Polygon();

			m_rpFillCutFeature = new osgEarth::Features::Feature(m_rpFillCutPolygon.get(),GetRenderContext()->GetMapNode()->getMapSRS());
			m_rpFillCutFeature->geoInterp() = osgEarth::GEOINTERP_RHUMB_LINE;

			m_rpFillCutFeature->style()->getOrCreate<osgEarth::Symbology::PolygonSymbol>()->fill()->color() =
				osgEarth::Symbology::Color(osg::Vec4f(0.0, 0.0, 0.9, 0.5));

			osgEarth::Symbology::LineSymbol* pLine = m_rpFillCutFeature->style()->getOrCreate<osgEarth::Symbology::LineSymbol>();
			pLine->stroke()->color() = osgEarth::Symbology::Color(0.5, 0.9, 0.5, 1.0);
			pLine->stroke()->width() =2.0f;
			pLine->stroke()->widthUnits() = osgEarth::Symbology::Units::PIXELS;
			pLine->tessellation() = 150;

			osgEarth::Symbology::Style style;
			osgEarth::Features::GeometryCompilerOptions options;
			options.maxGranularity() = 180.0;
			m_rpFillCutFNode = new osgEarth::Annotation::FeatureNode(GetRenderContext()->GetMapNode(), m_rpFillCutFeature.get(), style, options);
			m_rpFillCutFNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
			addChild(m_rpFillCutFNode.get());
		}

		//初始化边界线的形状
		{
			osg::ref_ptr<osgEarth::Features::MultiGeometry> pMultiGeom = new osgEarth::Features::MultiGeometry();
			m_rpBorderLineFeature = new osgEarth::Features::Feature(pMultiGeom, GetRenderContext()->GetMapNode()->getMapSRS());

			osgEarth::Symbology::LineSymbol* pLs = m_rpBorderLineFeature->style()->getOrCreate<osgEarth::Symbology::LineSymbol>();
			pLs->stroke()->color() = osgEarth::Symbology::Color(0.5, 0.9, 0.5, 1.0);
			pLs->stroke()->width() =2.0f;
			pLs->stroke()->widthUnits() = osgEarth::Symbology::Units::PIXELS;
			pLs->tessellation() = 150;

			osgEarth::Symbology::Style style;
			osgEarth::Features::GeometryCompilerOptions options;
			options.maxGranularity() = 180.0;
			m_rpBorderLineFNode = new osgEarth::Annotation::FeatureNode(GetRenderContext()->GetMapNode(), m_rpBorderLineFeature.get(), style, options);
			m_rpBorderLineFNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
			addChild(m_rpBorderLineFNode.get());
		}
	}
}

namespace FeMeasure
{

	CFillCutAnalysisMeasure::CFillCutAnalysisMeasure( FeUtil::CRenderContext* pContext )
		: CMeasure(pContext)
	{
		SetKey("fillCutAnalysis");
		SetTitle("填挖方分析");
		MeasureType() = EM_FillCutAnalysis;
	}

	CMeasureResult* CFillCutAnalysisMeasure::CreateMeasureResult()
	{
		m_opResult = new CFillCutAnalysisResult(GetRenderContext());
		return m_opResult.get();
	}

	void CFillCutAnalysisMeasure::SetReferenceHeight( double dHei )
	{
		if(m_opResult.get())
		{
			m_opResult->SetReferenceHeight(dHei);
			m_opResult->FinalCalculateImplement();
			SendMeasureMsg(EM_RESULT_CHANGED);
		}
	}
}

