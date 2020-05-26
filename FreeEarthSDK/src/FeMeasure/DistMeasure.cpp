#include <osgEarthSymbology/Geometry>
#include <osgEarthSymbology/AltitudeSymbol>
#include <osgEarthSymbology/RenderSymbol>
#include <osgEarthSymbology/LineSymbol>
#include <osgEarthSymbology/TextSymbol>
#include <FeUtils/PathRegistry.h>
#include <FeMeasure/DistMeasure.h>

namespace FeMeasure
{
	CDistMeasureResult::CDistMeasureResult( FeUtil::CRenderContext* pContext )
		: CMeasureResult(pContext)
		, m_pData(0)
	{
	}

	bool CDistMeasureResult::InitFeatureNode()
	{
		if(!GetRenderContext()) return false;

		if(!m_rpFeature.valid())
		{
			m_rpFeature = CreateFeature();
		}

		if(!m_rpFeatureNode.valid() && m_rpFeature.valid())
		{
			osgEarth::Symbology::Style style;
			osgEarth::Features::GeometryCompilerOptions options;
			options.maxGranularity() = 180.0;
			m_rpFeatureNode = new osgEarth::Annotation::FeatureNode(GetRenderContext()->GetMapNode(), m_rpFeature.get(), style, options);
			m_rpFeatureNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
			addChild(m_rpFeatureNode.get());
		}

		if(!m_rpMeasureInfoDisplay.valid())
		{
			m_rpMeasureInfoDisplay = new CDisplayByBillBoard(GetRenderContext()->GetMapNode());
			addChild(m_rpMeasureInfoDisplay->GetDisplayAsOsgNode());
		}

		return true;
	}

	bool CDistMeasureResult::CalculateImplement( const osg::Vec3dArray& vecPointsLLH )
	{
		UpdateFeature(vecPointsLLH);
		return DoCalculate();
	}
	
	void CDistMeasureResult::UpdateFeature( const osg::Vec3dArray& vecPointsLLH )
	{
		if(!InitFeatureNode()) return;

		if(m_rpFeature.valid() && m_rpFeatureNode.valid())
		{
			m_rpFeature->getGeometry()->assign(vecPointsLLH.begin(), vecPointsLLH.end());
			m_rpFeatureNode->init();
		}
	}

}

