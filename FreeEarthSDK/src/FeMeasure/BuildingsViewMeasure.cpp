#include <FeMeasure/BuildingsViewMeasure.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/LineWidth>
#include <osgEarth/DPLineSegmentIntersector>
#include <FeUtils/CoordConverter.h>
#include <osgDB/ConvertUTF>


namespace FeMeasure
{

	CBuildingsViewResult::CBuildingsViewResult( FeUtil::CRenderContext* pContext ) 
		: CMeasureResult(pContext)
	{
		m_rpBuildingsViewNode = new ViewAnalyseNode(GetRenderContext());
		addChild(m_rpBuildingsViewNode);
	}

	bool CBuildingsViewResult::CalculateImplement( const osg::Vec3dArray& vecPointsLLH )
	{
		if(vecPointsLLH.empty()) return false;
		SetLookPosition(vecPointsLLH.back());
		return true;
	}

	osg::Vec3 CBuildingsViewResult::GetLookPosition()
	{
		return m_vecPosLLH;
	}

	void CBuildingsViewResult::SetLookPosition( const osg::Vec3& vecPosLLH )
	{
		if(!GetRenderContext()) return;

		osg::Vec3d ePos = vecPosLLH;
		m_vecPosLLH = vecPosLLH;

		// eyePos
		osg::Vec3d eyePos;
		FeUtil::DegreeLLH2XYZ(GetRenderContext(),ePos,eyePos);

		/// lookPos
		osg::Vec3d vecLookPosLLH;
		vecLookPosLLH = ePos;
		vecLookPosLLH.x() += 0.01;

		osg::Vec3d lookPos;
		FeUtil::DegreeLLH2XYZ(GetRenderContext(),vecLookPosLLH,lookPos);

		if(m_rpBuildingsViewNode.valid())
		{
			m_rpBuildingsViewNode->setEyePos(eyePos);
			m_rpBuildingsViewNode->setLookPos(lookPos);
			m_rpBuildingsViewNode->init();
		}
	}

	osg::Vec4 CBuildingsViewResult::GetFrustumLineColor()
	{
		if(m_rpBuildingsViewNode.valid())
		{
			return m_rpBuildingsViewNode->getFrustumLineColor();
		}
		static osg::Vec4 vecErr;
		return vecErr;
	}

	void CBuildingsViewResult::SetFrustumLineColor( const osg::Vec4& vecColor )
	{
		if(m_rpBuildingsViewNode.valid())
		{
			return m_rpBuildingsViewNode->setFrustumLineColor(vecColor);
		}
	}

	float CBuildingsViewResult::GetNearClip()
	{
		if(m_rpBuildingsViewNode.valid())
		{
			return m_rpBuildingsViewNode->getNearClip();
		}
		return 0;
	}

	void CBuildingsViewResult::SetNearClip( float fVal )
	{
		if(m_rpBuildingsViewNode.valid())
		{
			return m_rpBuildingsViewNode->setNearClip(fVal);
		}
	}

	float CBuildingsViewResult::GetFarClip()
	{
		if(m_rpBuildingsViewNode.valid())
		{
			return m_rpBuildingsViewNode->getFarClip();
		} 
		return 0;
	}

	void CBuildingsViewResult::SetFarClip( float fVal )
	{
		if(m_rpBuildingsViewNode.valid())
		{
			return m_rpBuildingsViewNode->setFarClip(fVal);
		} 
	}

	float CBuildingsViewResult::GetFov()
	{
		if(m_rpBuildingsViewNode.valid())
		{
			return m_rpBuildingsViewNode->getFov();
		} 
		return 0;
	}

	void CBuildingsViewResult::SetFov( float fVal )
	{
		if(m_rpBuildingsViewNode.valid())
		{
			return m_rpBuildingsViewNode->setFov(fVal);
		} 
	}

	float CBuildingsViewResult::GetFovAspect()
	{
		if(m_rpBuildingsViewNode.valid())
		{
			return m_rpBuildingsViewNode->getFovAspect();
		} 
		return 0;
	}

	void CBuildingsViewResult::SetFovAspect( float fVal )
	{
		if(m_rpBuildingsViewNode.valid())
		{
			return m_rpBuildingsViewNode->setFovAspect(fVal);
		} 
	}

	float CBuildingsViewResult::GetPitchAngle()
	{
		return m_fPitchAngle;
	}

	void CBuildingsViewResult::SetPitchAngle( float fVal )
	{
		m_fPitchAngle = fVal;
	}
}

namespace FeMeasure
{
	CBuildingsViewMeasure::CBuildingsViewMeasure( FeUtil::CRenderContext* pContext ) : CMeasure(pContext)
	{
		SetKey("BuildingsViewMeasure");
		SetTitle("建筑群视域分析");
		MeasureType() = EM_BuildingsView;
	}

	CMeasureResult* CBuildingsViewMeasure::CreateMeasureResult()
	{
		m_opResult = new CBuildingsViewResult(GetRenderContext()); 
		return m_opResult.get();
	}

	void CBuildingsViewMeasure::AddCtrlPointImplement( const osg::Vec3d& vecPosLLH )
	{
		if(Vertexes().empty()) Vertexes().push_back(vecPosLLH);
	}

	void CBuildingsViewMeasure::SetCtrlPointsImplement( const osg::Vec3dArray& vecPointsLLH )
	{
		if(!vecPointsLLH.empty())
		{
			Vertexes().clear();
			Vertexes().push_back(vecPointsLLH.front());
		}
	}

}

