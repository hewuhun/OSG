#include <FePlots/DiagonalArrow.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/MathUtil.h>
namespace FePlots
{
	CDiagonalArrowOption::CDiagonalArrowOption( void )
	{

	}

	CDiagonalArrowOption::~CDiagonalArrowOption( void )
	{

	}
}

namespace FePlots
{
	CDiagonalArrow::CDiagonalArrow(FeUtil::CRenderContext *pRender, CDiagonalArrowOption *opt)
		: CPolygonPlot(pRender, opt)
	{
		m_bMouseMove = true;
		CleanCrtlArray();
	}

	CDiagonalArrow::~CDiagonalArrow(void)
	{
	}

	void CDiagonalArrow::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
	}

	void CDiagonalArrow::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		CPolygonPlot::DraggerUpdateMatrix(matrix);

		SetVertex(m_rpCtrlArray);
	}

	bool CDiagonalArrow::PushBackVertex( const osg::Vec3d& vecCoord )
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

	void CDiagonalArrow::ReplaceBackVertex( const osg::Vec3d& vecCoord )
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

	osg::Vec3dArray* CDiagonalArrow::CalculateArrow( osg::Vec3dArray* pPoints)
	{
		if (pPoints->size() < 2)
		{
			return NULL;
		}

		double dRadio = 6;

		//取出首尾两个点
		osg::Vec3d pointS = pPoints->at(0);
		osg::Vec3d pointE = pPoints->at(1);
		//计算箭头总长度
		double dLength = std::sqrt((pointE.y()-pointS.y())*(pointE.y()-pointS.y())+(pointE.x()-pointS.x())*(pointE.x()-pointS.x()));
		//计算直箭头的宽
		double dWidth = dLength/dRadio;

		//计算三角形的底边中心点坐标
		double x_ = pointS.x() + (pointE.x() - pointS.x())*(dRadio-1)/dRadio;
		double y_ = pointS.y() + (pointE.y() - pointS.y())*(dRadio-1)/dRadio;
		
		//计算
		osg::ref_ptr<osg::Vec3dArray> v_lr_ = FeUtil::CalculateVector(osg::Vec3d(pointE.x()-pointS.x(),pointE.y()-pointS.y(),0.0), osg::PI_2, dWidth/2.0);
		//获取左边尾部向量
		osg::Vec3d v_l_ = v_lr_->at(0);
		//获取右边尾部向量
		osg::Vec3d v_r_ = v_lr_->at(1);
	
		//获取左边尾部点
		osg::Vec3d point_l = osg::Vec3d(v_l_.x()+pointS.x(),v_l_.y()+pointS.y(),0.0);
		//获取右边尾部点
		osg::Vec3d point_r = osg::Vec3d(v_r_.x()+pointS.x(),v_r_.y()+pointS.y(),0.0);

		osg::Vec3d point_h_l = osg::Vec3d(v_l_.x()/dRadio+x_,v_l_.y()/dRadio+y_,0.0);
		osg::Vec3d point_h_r = osg::Vec3d(v_r_.x()/dRadio+x_,v_r_.y()/dRadio+y_,0.0);

		//计算三角形左边点
		osg::Vec3d point_a_l = osg::Vec3d(point_h_l.x()*2-point_h_r.x(),point_h_l.y()*2-point_h_r.y(),0.0);
		//计算三角形右边点
		osg::Vec3d point_a_r = osg::Vec3d(point_h_r.x()*2-point_h_l.x(),point_h_r.y()*2-point_h_l.y(),0.0);
		osg::ref_ptr<osg::Vec3dArray> rpPoints = new osg::Vec3dArray;
		rpPoints->push_back(point_l);
		rpPoints->push_back(point_h_l);
		rpPoints->push_back(point_a_l);
		rpPoints->push_back(pointE);
		rpPoints->push_back(point_a_r);
		rpPoints->push_back(point_h_r);
		rpPoints->push_back(point_r);
		return rpPoints.release();
	}
		
}
