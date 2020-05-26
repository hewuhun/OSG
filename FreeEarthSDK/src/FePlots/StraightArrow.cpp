#include <FePlots/StraightArrow.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/MathUtil.h>
#include <FeExtNode/FeNodeEditTool.h>

namespace FePlots
{
	CStraightArrowOption::CStraightArrowOption( void )
	{

	}

	CStraightArrowOption::~CStraightArrowOption( void )
	{

	}
}

namespace FePlots
{
	CStraightArrow::CStraightArrow(FeUtil::CRenderContext *pRender, CStraightArrowOption *opt)
		: CPolygonPlot(pRender, opt)
	{
		m_bMouseMove = true;
		CleanCrtlArray();
	}

	CStraightArrow::~CStraightArrow(void)
	{
	}

	void CStraightArrow::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
	}

	void CStraightArrow::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		CPolygonPlot::DraggerUpdateMatrix(matrix);

		SetVertex(m_rpCtrlArray);
	}

	bool CStraightArrow::PushBackVertex( const osg::Vec3d& vecCoord )
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

	void CStraightArrow::ReplaceBackVertex( const osg::Vec3d& vecCoord )
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

	osg::Vec3dArray* CStraightArrow::CalculateArrow( osg::Vec3dArray* pPoints)
	{
		double dRadio = 6;

		//取出第一和第二两个点
		osg::Vec3d pointS = pPoints->at(0);
		osg::Vec3d pointE = pPoints->at(1);

		//计算箭头总长度，即两个控制点的距离
		double dLength = std::sqrt((pointE.y()-pointS.y())*(pointE.y()-pointS.y())+(pointE.x()-pointS.x())*(pointE.x()-pointS.x()));

		//计算直箭头的宽
		double dWidth = dLength/dRadio;

		//计算三角形的底边中心点坐标
		double x_ = pointS.x() + (pointE.x() - pointS.x())*(dRadio-1)/dRadio;
		double y_ = pointS.y() + (pointE.y() - pointS.y())*(dRadio-1)/dRadio;

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

		osg::ref_ptr<osg::Vec3dArray> rpPoints = new osg::Vec3dArray;
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
