#include <FePlots/RoundedRect.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>

namespace FePlots
{
	CRoundedRectOption::CRoundedRectOption( void )
	{

	}

	CRoundedRectOption::~CRoundedRectOption( void )
	{

	}

}

namespace FePlots
{
	CRoundedRect::CRoundedRect(FeUtil::CRenderContext *pRender, CRoundedRectOption *opt)
		: CPolygonPlot(pRender, opt)
	{
		m_bMouseMove = true;
		CleanCrtlArray();
	}

	CRoundedRect::~CRoundedRect(void)
	{
	}

	void CRoundedRect::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
	}

	void CRoundedRect::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		CPolygonPlot::DraggerUpdateMatrix(matrix);

		SetVertex(m_rpCtrlArray);
	}

	bool CRoundedRect::PushBackVertex( const osg::Vec3d& vecCoord )
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

	void CRoundedRect::ReplaceBackVertex( const osg::Vec3d& vecCoord )
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
	double CRoundedRect::TheMax(double a ,double b) 
	{
		return (a>b)?a:b;
	}
	double CRoundedRect::TheMin(double a ,double b) 
	{
		return (a<b)?a:b;
	}
	osg::Vec3dArray* CRoundedRect::CalculateArrow( osg::Vec3dArray* vecControlPoints)
	{
		double dRadio = 1.0/10;
		if(vecControlPoints->size()>1)
		{
			//取第一个
			osg::Vec3d startPoint = vecControlPoints->at(0);
			//取最后一个
			osg::Vec3d endPoint = vecControlPoints->at(vecControlPoints->size()-1);
			double maxX = TheMax(startPoint.x(),endPoint.x());
			double minX = TheMin(startPoint.x(),endPoint.x());
			double maxY=  TheMax(startPoint.y(),endPoint.y());
			double minY=  TheMin(startPoint.y(),endPoint.y());

			//圆角半径为矩形宽高中最小值的1/10
			double radius=TheMin(std::abs(startPoint.x()-endPoint.x()),std::abs(startPoint.y()-endPoint.y()))*dRadio;
			//圆角的圆心点依次为矩形的左上点、右上点、右下点、左下点
			osg::Vec3d centerPoint0=osg::Vec3d(minX+radius,maxY-radius,0.0);
			osg::Vec3d centerPoint1=osg::Vec3d(maxX-radius,maxY-radius,0.0);
			osg::Vec3d centerPoint2=osg::Vec3d(maxX-radius,minY+radius,0.0);
			osg::Vec3d centerPoint3=osg::Vec3d(minX+radius,minY+radius,0.0);
			//圆角矩形的圆弧依次为矩形的左上、右上、右下、左下
			osg::ref_ptr<osg::Vec3dArray> arc0=FeUtil::CalculateArc(centerPoint0,radius,osg::PI,osg::PI_2,-1,180);
			osg::ref_ptr<osg::Vec3dArray> arc1=FeUtil::CalculateArc(centerPoint1,radius,osg::PI_2,0,-1,180);
			osg::ref_ptr<osg::Vec3dArray> arc2=FeUtil::CalculateArc(centerPoint2,radius,2*osg::PI,osg::PI*3/2,-1,180);
			osg::ref_ptr<osg::Vec3dArray> arc3=FeUtil::CalculateArc(centerPoint3,radius,osg::PI*3/2,osg::PI,-1,180);

			osg::ref_ptr<osg::Vec3dArray> points = new osg::Vec3dArray;
			points->insert(points->end(),arc0->begin(),arc0->end());
			points->insert(points->end(),arc1->begin(),arc1->end());
			points->insert(points->end(),arc2->begin(),arc2->end());
			points->insert(points->end(),arc3->begin(),arc3->end());
			return points.release();
		}
		return nullptr;
	}

}