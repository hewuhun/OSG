#include <FeMeasure/MeasureFactory.h>
#include <FeMeasure/SpaceDistMeasure.h>
#include <FeMeasure/SurfDistMeasure.h>
#include <FeMeasure/HeiDistMeasure.h>
#include <FeMeasure/SpaceAreaMeasure.h>
#include <FeMeasure/ShadowAreaMeasure.h>
#include <FeMeasure/BuildingsLinesMeasure.h>
#include <FeMeasure/RadialLineVisibleMeasure.h>
#include <FeMeasure/BuildingsViewMeasure.h>
#include <FeMeasure/GradientAnalysisMeasure.h>
#include <FeMeasure/FloodAnalysisMeasure.h>
#include <FeMeasure/ProfileAnalysisMeasure.h>
#include <FeMeasure/FillCutAnalysisMeasure.h>
#include <FeMeasure/ContourMeasure.h>


namespace FeMeasure
{
	
	CMeasure* CSpaceDistFactory::Create()
	{
		return new CSpaceDistMeasure(RenderContex());
	}

	CMeasure* CSurfaceDistFactory::Create()
	{
		return new CSurfDistMeasure(RenderContex());
	}

	CMeasure* CHeiDistFactory::Create()
	{
		return new CHeiDistMeasure(RenderContex());
	}

	CMeasure* CSpaceAreaFactory::Create()
	{
		return new CSpaceAreaMeasure(RenderContex());
	}

	CMeasure* CShadowAreaFactory::Create()
	{
		return new CShadowAreaMeasure(RenderContex());
	}

	CMeasure* CIntervisibleFactory::Create()
	{
		return new CBuildingsLinesMeasure(RenderContex());
	}

	CMeasure* CRadialLineFactory::Create()
	{
		return new CRadialLineVisibleMeasure(RenderContex());
	}

	CMeasure* CBuildingViewFactory::Create()
	{
		return new CBuildingsViewMeasure(RenderContex());
	}

	CMeasure* CGradientFactory::Create()
	{
		return new CGradientAnalysisMeasure(RenderContex());
	}

	CMeasure* CFloodFactory::Create()
	{
		return new CFloodAnalysisMeasure(RenderContex());
	}

	CMeasure* CProfileFactory::Create()
	{
		return new CProfileAnalysisMeasure(RenderContex());
	}

	CMeasure* CFillCutFactory::Create()
	{
		return new CFillCutAnalysisMeasure(RenderContex());
	}

	CMeasure* CContourFactory::Create()
	{
		return new CContourMeasure(RenderContex());
	}

}
