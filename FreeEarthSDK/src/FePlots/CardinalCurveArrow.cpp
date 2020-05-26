#include <FePlots/CardinalCurveArrow.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>

namespace FePlots
{
	CCardinalCurveArrowOption::CCardinalCurveArrowOption()
	{

	}

	CCardinalCurveArrowOption::~CCardinalCurveArrowOption()
	{

	}
}

namespace FePlots
{
	CCardinalCurveArrow::CCardinalCurveArrow(FeUtil::CRenderContext *pRender, CCardinalCurveArrowOption *opt)
		: CLinePlot(pRender, opt)
	{
		CleanCrtlArray();
	}

	CCardinalCurveArrow::~CCardinalCurveArrow()
	{

	}

	void CCardinalCurveArrow::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
	}

	void CCardinalCurveArrow::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		CLinePlot::DraggerUpdateMatrix(matrix);

		if (m_rpCtrlArray->size() > 2)
		{
			SetVertex(m_rpCtrlArray);
		}
	}

	bool CCardinalCurveArrow::PushBackVertex( const osg::Vec3d& vecCoord )
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

	void CCardinalCurveArrow::ReplaceBackVertex( const osg::Vec3d& vecCoord )
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

	osg::Vec3dArray* CCardinalCurveArrow::CalculateArrow( osg::Vec3dArray* vecControlPoints )
	{
		osg::ref_ptr<osg::Vec3dArray>  multiLines= new osg::Vec3dArray;
		if (vecControlPoints->size() >2) 
		{
			//曲线
			osg::ref_ptr<osg::Vec3dArray> cardinalPoints = FeUtil::CreateCardinalCurve(vecControlPoints);
			osg::ref_ptr<osg::Vec3dArray> rpPoints = new osg::Vec3dArray;
			for (int i=0;i<cardinalPoints->size();++i)
			{
				if(cardinalPoints->at(i).x()==0&&cardinalPoints->at(i).y()==0 )
					break;
				else
				{
					rpPoints->push_back(cardinalPoints->at(i));
				}
			}

			osg::ref_ptr<osg::Vec3dArray> cardinalCurveN = FeUtil::CreateBezierN(rpPoints,0);

			osg::Vec3d startP=cardinalPoints->at(rpPoints->size()-2);
			osg::Vec3d endP=cardinalPoints->at(rpPoints->size()-1);
			//箭头
			osg::ref_ptr<osg::Vec3dArray> arrowLines=FeUtil::CalculateArrowLines(startP,endP,2);
			multiLines->insert(multiLines->end(),cardinalCurveN->begin(),cardinalCurveN->end());
			multiLines->insert(multiLines->end(),arrowLines->begin(),arrowLines->end());
			return multiLines.release();
		}
	}

}