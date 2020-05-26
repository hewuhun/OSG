#include <FePlots/FreePolygon.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>


namespace FePlots
{
	CFreePolygonOption::CFreePolygonOption()
	{

	}

	CFreePolygonOption::~CFreePolygonOption()
	{

	}
}

namespace FePlots
{
	CFreePolygon::CFreePolygon(FeUtil::CRenderContext *pRender, CFreePolygonOption *opt)
		: CPolygonPlot(pRender, opt)
	{
		CleanCrtlArray();
	}

	CFreePolygon::~CFreePolygon()
	{

	}

	void CFreePolygon::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
	}

	void CFreePolygon::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		CPolygonPlot::DraggerUpdateMatrix(matrix);

		SetVertex(m_rpCtrlArray);
	}

	void CFreePolygon::PushBackVertex( const osg::Vec3d& vecCoord )
	{
		if (vecCoord.x() == 0 && vecCoord.y() == 0 && vecCoord.z() == 0)
		{
			return;
		}

		if(m_rpCtrlArray.valid())
		{
			m_rpCtrlArray->push_back(vecCoord);

			RebuildEditPoints();
		}
	}

	void CFreePolygon::ReplaceBackVertex( const osg::Vec3d& vecCoord )
	{
		m_rpCtrlArray->push_back(vecCoord);
		SetVertex(m_rpCtrlArray);
	}

	osg::Vec3dArray* CFreePolygon::CalculateArrow( osg::Vec3dArray* vecControlPoints)
	{
		if(vecControlPoints->size()>1)
		{
			return vecControlPoints;
		}
		else
		{

			return nullptr;
		}
	}

}