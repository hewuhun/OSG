#include <FeMeasure/FloodAnalysisMeasure.h>

#include <osg/BlendFunc>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/TexMat>
#include <osg/TexGen>
#include <osg/TextureCubeMap>

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
	CFloodHeightCallback::CFloodHeightCallback( CFloodAnalysisResult* pFloodMeasureResult )
		: m_opFloodMeasureResult(pFloodMeasureResult)
		,m_bPause(false)
		,m_bReset(true)
		,m_dPreTime(0.0)
	{
	}

	CFloodHeightCallback::~CFloodHeightCallback()
	{
	}

	void CFloodHeightCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
	{
		if (m_bPause)
		{
			return ;
		}

		if (nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
		{
			double dSimulationTime = nv->getFrameStamp()->getReferenceTime();

			if (m_bReset)
			{
				m_dPreTime = dSimulationTime;
				m_bReset = false;
			}

			//每0.1秒更新一次水面高度
			if ((dSimulationTime - m_dPreTime) > 0.1)
			{
				m_dPreTime = dSimulationTime;

				if (m_opFloodMeasureResult.valid())
				{
					//抬高水面
					const CFloodAnalysisResultData* data = m_opFloodMeasureResult->GetData();
					if(data) 
					{
						m_opFloodMeasureResult->FloodRaiseStep(data->dDeltaHei*0.1);	
					}
				}
			}
		}
	}

	
}

namespace FeMeasure
{

	CFloodAnalysisResult::CFloodAnalysisResult( FeUtil::CRenderContext* pContext, CFloodAnalysisMeasure* pMeasure )
		: CMeasureResult(pContext)
		, m_opMeasure(pMeasure)
		, m_pData(0)
	{
		m_rpContainPonits = new osg::Vec3dArray;
		m_rpFloodHeiCallback = new CFloodHeightCallback(this);
		InitFeatureNode();
	}

	bool CFloodAnalysisResult::CalculateImplement( const osg::Vec3dArray& vecPointsLLH )
	{
		if(m_rpFeature.valid() && m_rpFeatureNode.valid() && GetRenderContext())
		{
			m_rpFeature->getGeometry()->assign(vecPointsLLH.begin(), vecPointsLLH.end());
			m_rpFeatureNode->init();
		}

		return true;
	}

	bool CFloodAnalysisResult::FinalCalculateImplement()
	{
		if(!GetRenderContext()) return false;

		if(m_rpFeature.valid() && m_rpFeatureNode.valid())
		{
			if(m_rpFeature->getGeometry()->size() <= 0)
			{
				return false;
			}
			else if(m_rpFeature->getGeometry()->size() <= 2)
			{
				GetData()->dDeltaHei = 10.0;
				GetData()->dTotalArea = 0.0;
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

				CalculateContainPoints(pLLHArray, osg::Vec3d(dMinLon, dMinLat, 0.0), osg::Vec3d(dMaxLon, dMaxLat, 0.0),
					m_rpContainPonits, GetData()->maxHeiPoint, GetData()->minHeiPoint, 50);

				//计算一个中间高度作为基准面高度
				//默认使用20s的时间进行淹没模拟
				GetData()->dDeltaHei = (GetData()->maxHeiPoint.z()-GetData()->minHeiPoint.z())/20.0;

				BubbleSort(m_rpContainPonits);

				//计算多边形的投影面积
				GetData()->dTotalArea = FeMath::PolygonArea(pXYZArray);
				GetData()->dCurrentFloodHei = GetData()->minHeiPoint.z();


				if (m_rpFloodSurface.valid())
				{
					removeChild(m_rpFloodSurface);
					m_rpFloodSurface = NULL;
				}

				m_rpFloodSurface = new CFloodSurface(GetRenderContext());
				if (m_rpFeature.valid())
				{
					osg::ref_ptr<osg::Vec3dArray> pLLHCoord = new osg::Vec3dArray;
					std::vector<osg::Vec3d> coords = m_rpFeature->getGeometry()->asVector();
					for (int i = 0; i < coords.size(); i++)
					{
						pLLHCoord->push_back(coords.at(i));
					}
					m_rpFloodSurface->SetHeight(GetData()->maxHeiPoint.z(), GetData()->minHeiPoint.z());
					m_rpFloodSurface->SetFloodCoords(pLLHCoord);
					m_rpFloodSurface->InitFloodSurface();
				}
				addChild(m_rpFloodSurface);
			}
		}

		return true;
	}

	void CFloodAnalysisResult::InitFeatureNode()
	{
		if(!GetRenderContext()) return;
		
		//初始化地面上的几何体的形状
		{
			m_rpPolygon = new osgEarth::Symbology::Polygon();

			m_rpFeature = new osgEarth::Features::Feature(m_rpPolygon.get(), GetRenderContext()->GetMapSRS());
			m_rpFeature->geoInterp() = osgEarth::GEOINTERP_RHUMB_LINE;

			m_rpFeature->style()->getOrCreate<osgEarth::Symbology::PolygonSymbol>()->fill()->color() =
				osgEarth::Symbology::Color(osg::Vec4f(0.9, 0.9, 0.0, 0.5));

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
	}

	void CFloodAnalysisResult::CalculateScope( osg::Vec3dArray* pArray, double& dMinLon, double& dMaxLon, double& dMinLat, double& dMaxLat )
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

	void CFloodAnalysisResult::CalculateContainPoints( osg::Vec3dArray* pLLHArray, osg::Vec3d minLLHLonLat, osg::Vec3d maxLLHLonLat, 
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
					if (GetRenderContext())
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
					}

					pLLHOutArray->push_back(testPoint);
				}
			}
		}
	}

	void CFloodAnalysisResult::BubbleSort( osg::Vec3dArray* pArray )
	{
		if (NULL == pArray)
		{
			return;
		}

		int nSize = pArray->size();
		int nDone = nSize-1;

		for (int i = 0; i < nSize; i++)
		{
			for (int j = i; j < nSize; j++)
			{
				if (pArray->at(i).z() > pArray->at(j).z())
				{
					osg::Vec3d tempPoint = pArray->at(j);
					pArray->at(j) = pArray->at(i);
					pArray->at(i) = tempPoint;
				}
			}
		}
	}

	int CFloodAnalysisResult::BinarySearch( osg::Vec3dArray* pArray, double dHeight )
	{
		int low, high, mid;
		low = 0;
		high = pArray->size() - 1;
		while (low <= high)
		{
			mid = (low + high) / 2;
			if (pArray->at(mid).z() < dHeight && pArray->at(mid+1).z() > dHeight)
			{
				return mid;
			}
			else if (pArray->at(mid).z() < dHeight)
			{
				low = mid + 1;
			}
			else if (pArray->at(mid).z() > dHeight)
			{
				high = mid - 1;
			}
			else 
			{
				return mid;
			}
		}
		return 0;
	}

	bool CFloodAnalysisResult::IsShowBorder()
	{
		if (m_rpFloodSurface.valid())
		{
			return m_rpFloodSurface->IsShowBorder();
		}

		return false;
	}

	void CFloodAnalysisResult::SetBorderShow( bool bShow )
	{
		if (m_rpFloodSurface.valid())
		{
			m_rpFloodSurface->ShowBorder(bShow);
		}
	}

	void CFloodAnalysisResult::SetDeltaHeight( double dHei )
	{
		GetData()->dDeltaHei = dHei;
	}

	bool CFloodAnalysisResult::FloodRaiseStep(double dHeight)
	{
		if (m_rpFloodSurface->Raise(dHeight))
		{
			PauseDynamicCalculate();
			GetData()->dFloodArea = GetData()->dTotalArea;
			GetData()->dCurrentFloodHei = GetData()->maxHeiPoint.z();
			return false; // 水面停止，返回false
		}
		else
		{
			GetData()->dCurrentFloodHei = m_rpFloodSurface->GetCurrentHeight();
			//可以优化，将开始比较的起始索引记下
			int nIndex = BinarySearch(m_rpContainPonits, GetData()->dCurrentFloodHei);
			GetData()->dFloodArea = 1.0*nIndex/(m_rpContainPonits->size()-1)*GetData()->dTotalArea;
		}

		if(m_opMeasure.valid())
		{
			m_opMeasure->SendMeasureMsg(EM_RESULT_CHANGED);
		}

		return true;
	}

	void CFloodAnalysisResult::StopDynamicCalculate()
	{
		if(m_rpFloodHeiCallback.valid() && m_rpFloodSurface.valid())
		{
			removeUpdateCallback(m_rpFloodHeiCallback.get());
			m_rpFloodSurface->Hide();
		}
	}

	void CFloodAnalysisResult::StartDynamicCalculate()
	{
		if(m_rpFloodHeiCallback.valid() && m_rpFloodSurface.valid())
		{
			//如果当前的事件回调中没有添加回调，则设置回调
			if (getUpdateCallback() != m_rpFloodHeiCallback.get())
			{
				setUpdateCallback(m_rpFloodHeiCallback);
			}

			//设置回调的暂停状态为否，并将水面显示出来
			m_rpFloodHeiCallback->SetPause(false);

			m_rpFloodSurface->Show();
		}
	}

	void CFloodAnalysisResult::PauseDynamicCalculate()
	{
		if(m_rpFloodHeiCallback.valid())
		{
			m_rpFloodHeiCallback->SetPause(true);
		}
	}

	void CFloodAnalysisResult::ResetFloodState()
	{
		//将事件回调重置
		if(m_rpFloodHeiCallback.valid())
		{
			m_rpFloodHeiCallback->SetPause(true);
			m_rpFloodHeiCallback->Reset();
		}

		//将水面模拟重置并隐藏
		if (m_rpFloodSurface.valid())
		{
			m_rpFloodSurface->Hide();
			m_rpFloodSurface->Reset();
		}

		//当前的水面的高度置为最低高度、淹没面积置为0
		if(GetData())
		{
			GetData()->dCurrentFloodHei = GetData()->minHeiPoint.z();
			GetData()->dFloodArea = 0.0;
		}
	}

}

namespace FeMeasure
{
	CFloodAnalysisMeasure::CFloodAnalysisMeasure( FeUtil::CRenderContext* pContext ) 
		: CMeasure(pContext)
	{
		SetKey("FloodAnalysis");
		SetTitle("淹没分析");
		MeasureType() = EM_FlootAnalysis;
	}

	CMeasureResult* CFloodAnalysisMeasure::CreateMeasureResult()
	{
		m_opCurResult = new CFloodAnalysisResult(GetRenderContext(), this); 
		return m_opCurResult.get();
	}

}



