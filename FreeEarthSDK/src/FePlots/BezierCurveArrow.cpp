#include <FePlots/BezierCurveArrow.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>

namespace FePlots
{
	CBezierCurveArrowOption::CBezierCurveArrowOption()
	{

	}

	CBezierCurveArrowOption::~CBezierCurveArrowOption()
	{

	}
}

namespace FePlots
{
	CBezierCurveArrow::CBezierCurveArrow(FeUtil::CRenderContext *pRender, CBezierCurveArrowOption *opt)
		: CLinePlot(pRender, opt)
	{
		CleanCrtlArray();
	}

	CBezierCurveArrow::~CBezierCurveArrow()
	{

	}

	void CBezierCurveArrow::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
	}

	void CBezierCurveArrow::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		CLinePlot::DraggerUpdateMatrix(matrix);

		if (m_rpCtrlArray->size() > 2)
		{
			SetVertex(m_rpCtrlArray);
		}
	}

	bool CBezierCurveArrow::PushBackVertex( const osg::Vec3d& vecCoord )
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

	void CBezierCurveArrow::ReplaceBackVertex( const osg::Vec3d& vecCoord )
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

	osg::Vec3dArray* CBezierCurveArrow::CalculateArrow( osg::Vec3dArray* vecControlPoints )
	{
		osg::ref_ptr<osg::Vec3dArray> multiLines = new osg::Vec3dArray;
		//三个控制点时，绘制二次贝塞尔曲线
		if (vecControlPoints->size() ==3)
		{
			osg::Vec3d startP=vecControlPoints->at(1);
			osg::Vec3d endP=vecControlPoints->at(2);
			//曲线
			osg::ref_ptr<osg::Vec3dArray> bezierCurve2=FeUtil::CreateBezier2(vecControlPoints);
			for (int i=0;i<bezierCurve2->size();i++)
			{
				multiLines->push_back(bezierCurve2->at(i));
			}
			//箭头
			osg::ref_ptr<osg::Vec3dArray> arrowLines=FeUtil::CalculateArrowLines(startP,endP,10);
			multiLines->insert(multiLines->end(),arrowLines->begin(),arrowLines->end());
			return multiLines.release();
		}
		//四个控制点时，绘制三次贝塞尔曲线
		else if (vecControlPoints->size() ==4)
		{
			osg::Vec3d startP=vecControlPoints->at(2);
			osg::Vec3d endP=vecControlPoints->at(3);
			//曲线
			osg::ref_ptr<osg::Vec3dArray> bezierCurve3=FeUtil::CreateBezier3(vecControlPoints);
			for (int i=0;i<bezierCurve3->size();i++)
			{
				multiLines->push_back(bezierCurve3->at(i));
			}
			//箭头
			osg::ref_ptr<osg::Vec3dArray> arrowLines=FeUtil::CalculateArrowLines(startP,endP,10);
			multiLines->insert(multiLines->end(),arrowLines->begin(),arrowLines->end());
			return multiLines.release();
		}
		//N个控制点时，绘制N-1次贝塞尔曲线
		else if (vecControlPoints->size() >4)
		{
			osg::Vec3d startP=vecControlPoints->at(vecControlPoints->size()-2);
			osg::Vec3d endP=vecControlPoints->at(vecControlPoints->size()-1);
			//曲线
			osg::ref_ptr<osg::Vec3dArray> bezierCurveN=FeUtil::CreateBezierN(vecControlPoints,0);
			//箭头
			osg::ref_ptr<osg::Vec3dArray> arrowLines=FeUtil::CalculateArrowLines(startP,endP,10);
			multiLines->insert(multiLines->end(),bezierCurveN->begin(),bezierCurveN->end());
			multiLines->insert(multiLines->end(),arrowLines->begin(),arrowLines->end());
			return multiLines.release();
		}
	}

}