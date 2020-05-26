#include <FePlots/StraightLineArrow.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/MathUtil.h>
#include <FeExtNode/FeNodeEditTool.h>

namespace FePlots
{
	CStraightLineArrowOption::CStraightLineArrowOption( void )
	{

	}

	CStraightLineArrowOption::~CStraightLineArrowOption( void )
	{

	}
}

namespace FePlots
{
	CStraightLineArrow::CStraightLineArrow(FeUtil::CRenderContext *pRender, CStraightLineArrowOption *opt)
		: CLinePlot(pRender, opt)
	{
		m_bMouseMove = true;
		CleanCrtlArray();
	}

	CStraightLineArrow::~CStraightLineArrow(void)
	{
	}

	void CStraightLineArrow::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
	}

	void CStraightLineArrow::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		CLinePlot::DraggerUpdateMatrix(matrix);

		SetVertex(m_rpCtrlArray);
	}

	bool CStraightLineArrow::PushBackVertex( const osg::Vec3d& vecCoord )
	{
		if (vecCoord.x() == 0 && vecCoord.y() == 0 && vecCoord.z() == 0)
		{
			return false;
		}

		if(m_rpCtrlArray.valid())
		{
			if (m_rpCtrlArray->size() == 0)
			{
				m_rpCtrlArray->push_back(vecCoord);
			}
			if (m_rpCtrlArray->size() == 2)
			{
				m_bMouseMove = false;
			}

			RebuildEditPoints();
		}

		return true;
	}

	void CStraightLineArrow::ReplaceBackVertex( const osg::Vec3d& vecCoord )
	{
		if(m_rpCtrlArray.valid())
		{
			if (m_rpCtrlArray->size() == 1)
			{
				m_rpCtrlArray->push_back(vecCoord);
			}
			if (m_rpCtrlArray->size() == 2 && m_bMouseMove)
			{
				int nIndex = m_rpCtrlArray->size() - 1;
				m_rpCtrlArray->at(nIndex) = vecCoord;

				SetVertex(m_rpCtrlArray);
			}
		}
	}

	osg::Vec3dArray* CStraightLineArrow::CalculateArrow( osg::Vec3dArray* vecControlPoints )
	{
		if (vecControlPoints->size() > 1)
		{
			osg::ref_ptr<osg::Vec3dArray> multiLines= new osg::Vec3dArray();
			osg::Vec3d startP=vecControlPoints->at(0);
			osg::Vec3d endP=vecControlPoints->at(1);
			//直线
			multiLines->push_back(startP);
			multiLines->push_back(endP);

			//箭头
			osg::ref_ptr<osg::Vec3dArray> arrowLines=FeUtil::CalculateArrowLines(startP,endP,10);

			for (int j=0;j<arrowLines->size();j++)
			{
				multiLines->push_back(arrowLines->at(j));
			}
			return multiLines.release();
		}
		return nullptr;
	}

}
