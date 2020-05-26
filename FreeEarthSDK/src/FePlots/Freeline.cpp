#include <FePlots/Freeline.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>

namespace FePlots
{
	CFreeLineOption::CFreeLineOption()
	{

	}

	CFreeLineOption::~CFreeLineOption()
	{

	}
}

namespace FePlots
{
	CFreeLine::CFreeLine(FeUtil::CRenderContext *pRender, CFreeLineOption *opt)
		: CLinePlot(pRender, opt)
	{
		CleanCrtlArray();
	}

	CFreeLine::~CFreeLine()
	{

	}

	void CFreeLine::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
	}

	void CFreeLine::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		CLinePlot::DraggerUpdateMatrix(matrix);

		SetVertex(m_rpCtrlArray);
	}

	void CFreeLine::PushBackVertex( const osg::Vec3d& vecCoord )
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

	void CFreeLine::ReplaceBackVertex( const osg::Vec3d& vecCoord )
	{
		if(m_rpCtrlArray.valid())
		{
			m_rpCtrlArray->push_back(vecCoord);
			SetVertex(m_rpCtrlArray);
		}
	}

	osg::Vec3dArray* CFreeLine::CalculateArrow( osg::Vec3dArray* vecControlPoints )
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