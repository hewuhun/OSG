#include <FePlots/DoveTailStraightArrow.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>
#define Radio 6
#define TailRadio 5

namespace FePlots
{
	CDoveTailStraightArrowOption::CDoveTailStraightArrowOption( void )
	{

	}

	CDoveTailStraightArrowOption::~CDoveTailStraightArrowOption( void )
	{

	}

}

namespace FePlots
{

	CDoveTailStraightArrow::CDoveTailStraightArrow(FeUtil::CRenderContext *pRender, CDoveTailStraightArrowOption *opt)
		: CPolygonPlot(pRender, opt)
	{
		m_bMouseMove = true;
		CleanCrtlArray();
	}

	CDoveTailStraightArrow::~CDoveTailStraightArrow(void)
	{
	}

	void CDoveTailStraightArrow::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
	}

	void CDoveTailStraightArrow::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		CPolygonPlot::DraggerUpdateMatrix(matrix);

		SetVertex(m_rpCtrlArray);
	}

	bool CDoveTailStraightArrow::PushBackVertex( const osg::Vec3d& vecCoord )
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

	void CDoveTailStraightArrow::ReplaceBackVertex( const osg::Vec3d& vecCoord )
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


	osg::Vec3dArray* CDoveTailStraightArrow::CalculateArrow( osg::Vec3dArray* pPoints)
	{
		//取出首尾两个点
		osg::Vec3d pointS = pPoints->at(0);
		osg::Vec3d pointE = pPoints->at(1);
		//计算箭头总长度，即两个控制点的距离
		double dLength = std::sqrt((pointE.y()-pointS.y())*(pointE.y()-pointS.y())+(pointE.x()-pointS.x())*(pointE.x()-pointS.x()));
		//计算燕尾直箭头的宽
		double dWidth = dLength/Radio;

		//计算三角形的底边中心点坐标
		double x_ = pointS.x() + (pointE.x() - pointS.x())*(Radio-1)/Radio;
		double y_ = pointS.y() + (pointE.y() - pointS.y())*(Radio-1)/Radio;

		//计算与基本向量夹角90度的，长度为dWidth/2的向量数组
		osg::ref_ptr<osg::Vec3dArray> v_lr = FeUtil::CalculateVector(osg::Vec3d(pointE.x()-pointS.x(),pointE.y()-pointS.y(),0.0), osg::PI_2, dWidth/2.0);
		//获取左右向量
		osg::Vec3d v_l = v_lr->at(0);
		osg::Vec3d v_r = v_lr->at(1);

		//左1点
		osg::Vec3d point1 = osg::Vec3d(pointS.x()+v_l.x(),pointS.y()+v_l.y(),0.0);
		//左2点
		osg::Vec3d point2 = osg::Vec3d(x_+point1.x()-pointS.x(),y_+point1.y()-pointS.y(),0.0);
		//左3点
		osg::Vec3d point3 = osg::Vec3d(2*point2.x()-x_,2*point2.y()-y_,0.0);
		//顶点
		osg::Vec3d point4 = osg::Vec3d(pointE.x(),pointE.y(),0.0);
		//右3点
		osg::Vec3d point7 = osg::Vec3d(pointS.x()+v_r.x(),pointS.y()+v_r.y(),0.0);
		//右2点
		osg::Vec3d point6 = osg::Vec3d(x_+point7.x()-pointS.x(),y_+point7.y()-pointS.y(),0.0);
		//右1点
		osg::Vec3d point5 = osg::Vec3d(2*point6.x()-x_,2*point6.y()-y_,0.0);
		//在尾部两个中间插入一个点，是pointS往pointE平移的一个点，为了制作尾巴的效果
		osg::Vec3d point_tail = osg::Vec3d((pointE.x()-pointS.x())/TailRadio+pointS.x(), (pointE.y()-pointS.y())/TailRadio+pointS.y(), 0.0);

		osg::ref_ptr<osg::Vec3dArray> rpPoints = new osg::Vec3dArray;
		rpPoints->push_back(point_tail);
		rpPoints->push_back(point1);
		rpPoints->push_back(point2);
		rpPoints->push_back(point3);
		rpPoints->push_back(point4);
		rpPoints->push_back(point5);
		rpPoints->push_back(point6);
		rpPoints->push_back(point7);
		return rpPoints.release();
	}


}


