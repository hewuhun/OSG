#include <FeMeasure/RadialLineVisibleMeasure.h>

#include <osgEarthSymbology/Geometry>
#include <osgEarthSymbology/TextSymbol>
#include <FeUtils/PathRegistry.h>


namespace FeMeasure
{

	CRadialLineVisibleResult::CRadialLineVisibleResult( FeUtil::CRenderContext* pContext ) 
		: CMeasureResult(pContext)
	{
		getOrCreateStateSet()->setRenderBinDetails(100, "RenderBin");
		
	}

	bool CRadialLineVisibleResult::CalculateImplement( const osg::Vec3dArray& vecPointsLLH )
	{
		//if(pContext)
		if(!m_rpSightNode.valid())
		{
			m_rpSightNode = new FeMeasure::CRadialLineOfSightNode(GetRenderContext()->GetMapNode());
			addChild(m_rpSightNode.get());
		}

		if(GetRenderContext() && m_rpSightNode.valid() && !vecPointsLLH.empty())
		{
			/// first point
			osg::Vec3d firstPt = vecPointsLLH.front();
			firstPt.z() = 0;

			if(m_rpSightNode->GetCenter().vec3d() != firstPt)
			{
				m_rpSightNode->SetCenter(osgEarth::GeoPoint(
					GetRenderContext()->GetMapSRS(), firstPt.x(), firstPt.y(), 0, osgEarth::ALTMODE_RELATIVE));
			}
			
			if(vecPointsLLH.size() > 1)
			{
				osg::Vec3d lastPt = vecPointsLLH.back();
				lastPt.z() = 0;

				if(lastPt != m_rpSightNode->GetEndLLH())
				{
					m_rpSightNode->SetEndLLH(lastPt);
				}
			}

			/// stop computeGrid
			if (m_rpSightNode.valid())
			{
				m_rpSightNode->StopCompute();
			}

			return true;
		}

		return false;
	}

	bool CRadialLineVisibleResult::FinalCalculateImplement()
	{
		if (m_rpSightNode.valid())
		{
			m_rpSightNode->ComputeGrid();
			return true;
		}

		return false;
	}

}

namespace FeMeasure
{
	CRadialLineVisibleMeasure::CRadialLineVisibleMeasure( FeUtil::CRenderContext* pContext ) : CMeasure(pContext)
	{
		SetKey("RadialLineVisibleMeasure");
		SetTitle("视域分析");
		MeasureType() = EM_RadialLineVisible;
	}

	void CRadialLineVisibleMeasure::AddCtrlPointImplement( const osg::Vec3d& vecPosLLH )
	{
		if(Vertexes().size() == 2) 
			Vertexes().back() = vecPosLLH;
		else
			Vertexes().push_back(vecPosLLH);
	}

	void CRadialLineVisibleMeasure::SetCtrlPointsImplement( const osg::Vec3dArray& vecPointsLLH )
	{
		if(!vecPointsLLH.empty())
		{
			Vertexes().clear();
			Vertexes().push_back(vecPointsLLH.front());
			Vertexes().push_back(vecPointsLLH.back());
		}
	}

}
