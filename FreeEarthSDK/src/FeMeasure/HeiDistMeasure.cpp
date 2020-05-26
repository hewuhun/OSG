#include <FeMeasure/HeiDistMeasure.h>

#include <osgEarthSymbology/Geometry>
#include <osgEarthSymbology/AltitudeSymbol>
#include <osgEarthSymbology/RenderSymbol>
#include <osgEarthSymbology/LineSymbol>
#include <osgEarthSymbology/TextSymbol>
#include <FeUtils/PathRegistry.h>


namespace FeMeasure
{
	CHeiDistResult::CHeiDistResult( FeUtil::CRenderContext* pContext ) 
		: CDistMeasureResult(pContext)
	{
		
	}

	bool CHeiDistResult::DoCalculate()
	{
		if(!GetFeature() || !GetData()) return false;

		if(GetFeature()->getGeometry()->size() <= 1)
		{
			GetData()->Value() = 0.0;
			return true;
		}
		else
		{
			GetData()->Value() = GetFeature()->getGeometry()->back().z() - GetFeature()->getGeometry()->front().z();
		}

		osg::Vec3d vecPos = GetFeature()->getGeometry()->back();

		if(m_rpHeiFeature.valid() && m_rpHeiFeature->getGeometry()->size() >= 3)
		{
			osg::Vec3d vecPos = *(m_rpHeiFeature->getGeometry()->begin() + 1);

			if(GetMeasureInfoDisplay())
			{
				GetMeasureInfoDisplay()->SetPosition(vecPos);
				GetMeasureInfoDisplay()->SetOrCreateString(0, FeUtil::ToUTF("高差分析：") + ReviseDistanceCalculate(GetData()->Value()));
			}
		}

		return true;
	}

	void CHeiDistResult::ResetHeiFeature(const osg::Vec3d& vecPosLLH)
	{
		if(m_rpHeiFeature.valid() && m_rpHeiFeatureNode.valid())
		{
			if(m_rpHeiFeature->getGeometry()->size() > 0)
			{
				osg::Vec3d firstPos = m_rpHeiFeature->getGeometry()->front();
				m_rpHeiFeature->getGeometry()->clear();
				double dHei = vecPosLLH.z() <= firstPos.z() ? firstPos.z() : vecPosLLH.z();
				osg::Vec3d secondPos = vecPosLLH.z() <= firstPos.z() ? vecPosLLH : firstPos;
				secondPos.z() = dHei;
				m_rpHeiFeature->getGeometry()->push_back(firstPos);
				m_rpHeiFeature->getGeometry()->push_back(secondPos);
				m_rpHeiFeature->getGeometry()->push_back(vecPosLLH);
			}
			else
			{
				m_rpHeiFeature->getGeometry()->push_back(vecPosLLH);
			}

			m_rpHeiFeatureNode->init();
		}
	}

	osgEarth::Features::Feature* CHeiDistResult::CreateFeature()
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

	void CHeiDistResult::InitHeiFeature()
	{
		if (!GetRenderContext()) return;

		if(!m_rpHeiFeature.valid())
		{
			m_rpHeiFeature = new osgEarth::Features::Feature(new osgEarth::Symbology::LineString(),GetRenderContext()->GetMapSRS());

			// offset to mitigate Z fighting
			osgEarth::Symbology::RenderSymbol* pHeiRender =
				m_rpHeiFeature->style()->getOrCreate<osgEarth::Symbology::RenderSymbol>();
			pHeiRender->depthOffset()->enabled() = true;
			pHeiRender->depthOffset()->minBias() = 1000;

			// define a style for the line
			osgEarth::Symbology::LineSymbol* pHeiLs =
				m_rpHeiFeature->style()->getOrCreate<osgEarth::Symbology::LineSymbol>();
			pHeiLs->stroke()->color() = osgEarth::Symbology::Color::Red;
			pHeiLs->stroke()->width() = 1.2f;
			pHeiLs->stroke()->widthUnits() = osgEarth::Symbology::Units::PIXELS;
			pHeiLs->tessellation() = 150;
		}
		
		if(!m_rpHeiFeatureNode.valid())
		{
			osgEarth::Symbology::Style style;
			osgEarth::Features::GeometryCompilerOptions options;
			options.maxGranularity() = 180.0;
			m_rpHeiFeatureNode = new osgEarth::Annotation::FeatureNode(GetRenderContext()->GetMapNode(), m_rpHeiFeature.get(), style, options);
			m_rpHeiFeatureNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
			addChild(m_rpHeiFeatureNode.get());
		}
	}

	void CHeiDistResult::UpdateFeature( const osg::Vec3dArray& vecPointsLLH )
	{
		if(!InitFeatureNode()) return;
		InitHeiFeature();

		if(GetFeature() && GetFeatureNode() && !vecPointsLLH.empty())
		{
			/// 只使用首尾两个控制点进行计算
			GetFeature()->getGeometry()->clear();
			GetFeature()->getGeometry()->push_back(vecPointsLLH.front());
			if(vecPointsLLH.size() > 1)
			{
				GetFeature()->getGeometry()->push_back(vecPointsLLH.back());
				GetFeatureNode()->init();

				m_rpHeiFeature->getGeometry()->clear();
				ResetHeiFeature(vecPointsLLH.front());
				ResetHeiFeature(vecPointsLLH.back());
			}
		}
	}

}

namespace FeMeasure
{
	CHeiDistMeasure::CHeiDistMeasure( FeUtil::CRenderContext* pContext )
		: CMeasure(pContext)
	{
		SetKey("heigDist");
		SetTitle("高度分析");
		MeasureType() = EM_HeiDist;
	}

	CMeasureResult* CHeiDistMeasure::CreateMeasureResult()
	{
		return new CHeiDistResult(GetRenderContext());
	}

	void CHeiDistMeasure::AddCtrlPointImplement( const osg::Vec3d& vecPosLLH )
	{
		if(Vertexes().size() == 2) 
			Vertexes().back() = vecPosLLH;
		else
			Vertexes().push_back(vecPosLLH);
	}

	void CHeiDistMeasure::SetCtrlPointsImplement( const osg::Vec3dArray& vecPointsLLH )
	{
		if(!vecPointsLLH.empty())
		{
			Vertexes().clear();
			Vertexes().push_back(vecPointsLLH.front());
			Vertexes().push_back(vecPointsLLH.back());
		}
	}

}






