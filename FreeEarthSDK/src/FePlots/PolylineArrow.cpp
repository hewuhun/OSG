#include <FePlots/PolylineArrow.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>

namespace FePlots
{
	CPolyLineArrowOption::CPolyLineArrowOption()
	{

	}

	CPolyLineArrowOption::~CPolyLineArrowOption()
	{

	}
}

namespace FePlots
{
	CPolyLineArrow::CPolyLineArrow(FeUtil::CRenderContext *pRender, CPolyLineArrowOption *opt)
		: CLinePlot(pRender, opt)
	{
		CleanCrtlArray();
	}

	CPolyLineArrow::~CPolyLineArrow()
	{

	}

	void CPolyLineArrow::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
	}

	void CPolyLineArrow::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		CLinePlot::DraggerUpdateMatrix(matrix);

		if (m_rpCtrlArray->size() > 2)
		{
			SetVertex(m_rpCtrlArray);
		}
	}

	bool CPolyLineArrow::PushBackVertex( const osg::Vec3d& vecCoord )
	{
		if (vecCoord.x() == 0 && vecCoord.y() == 0 && vecCoord.z() == 0)
		{
			return false;
		}

		if(m_rpCtrlArray.valid())
		{
			m_rpCtrlArray->push_back(vecCoord);

			RebuildEditPoints();
		}

		return true;
	}

	void CPolyLineArrow::ReplaceBackVertex( const osg::Vec3d& vecCoord )
	{
		if(m_rpCtrlArray.valid())
		{
			if (m_rpCtrlArray->size() == 2)
			{
				m_rpCtrlArray->push_back(vecCoord);
			}
			if (m_rpCtrlArray->size() > 2)
			{
				int nIndex = m_rpCtrlArray->size() - 1;
				m_rpCtrlArray->at(nIndex) = vecCoord;
				SetVertex(m_rpCtrlArray);
			}
		}
	}

	osg::Vec3dArray* CPolyLineArrow::CalculateArrow( osg::Vec3dArray* vecControlPoints )
	{
		//两个控制点时，绘制直线
		if (vecControlPoints->size() > 1) {
			osg::ref_ptr<osg::Vec3dArray> multiLines= new osg::Vec3dArray();
			osg::Vec3d startP=vecControlPoints->at(vecControlPoints->size()-2);
			osg::Vec3d endP=vecControlPoints->at(vecControlPoints->size()-1);
			//直线
			for (int i=0;i<vecControlPoints->size();i++)
			{
				multiLines->push_back(vecControlPoints->at(i));
			}

			//箭头
			osg::ref_ptr<osg::Vec3dArray> arrowLines=FeUtil::CalculateArrowLines(startP,endP,10);
			int size = arrowLines->size();
			for (int j=0;j<arrowLines->size();j++)
			{
				multiLines->push_back(arrowLines->at(j));
			}

			return multiLines.release();
		}
		return nullptr;
	}

}