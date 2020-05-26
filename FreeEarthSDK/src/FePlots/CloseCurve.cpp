#include <FePlots/CloseCurve.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>


namespace FePlots
{
	CCloseCurveOption::CCloseCurveOption()
	{

	}

	CCloseCurveOption::~CCloseCurveOption()
	{

	}
}

namespace FePlots
{
	CCloseCurve::CCloseCurve(FeUtil::CRenderContext *pRender, CCloseCurveOption *opt)
		: CPolygonPlot(pRender, opt)
	{
		CleanCrtlArray();
	}

	CCloseCurve::~CCloseCurve()
	{

	}

	void CCloseCurve::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
	}

	void CCloseCurve::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		CPolygonPlot::DraggerUpdateMatrix(matrix);

		if (m_rpCtrlArray->size() > 2)
		{
			SetVertex(m_rpCtrlArray);
		}
	}

	bool CCloseCurve::PushBackVertex( const osg::Vec3d& vecCoord )
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

	void CCloseCurve::ReplaceBackVertex( const osg::Vec3d& vecCoord )
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

	osg::Vec3dArray* CCloseCurve::CalculateArrow( osg::Vec3dArray* vecControlPoints)
	{
		//至少需要三个控制点
		if(vecControlPoints->size()>2)
		{

			osg::ref_ptr<osg::Vec3dArray> cardinalPoints = FeUtil::CreateCloseCardinal(vecControlPoints);
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
			osg::ref_ptr<osg::Vec3dArray> cardinalCurve3 = FeUtil::CreateBezier3(rpPoints,100);
			return cardinalCurve3.release();
		}
		return nullptr;
		
	}

	

}