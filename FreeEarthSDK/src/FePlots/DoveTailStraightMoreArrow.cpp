#include <FePlots/DoveTailStraightMoreArrow.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>

namespace FePlots
{
	CDoveTailStraightMoreArrowOption::CDoveTailStraightMoreArrowOption()
	{

	}

	CDoveTailStraightMoreArrowOption::~CDoveTailStraightMoreArrowOption()
	{

	}
}

namespace FePlots
{
	CDoveTailStraightMoreArrow::CDoveTailStraightMoreArrow(FeUtil::CRenderContext *pRender, CDoveTailStraightMoreArrowOption *opt)
		: CPolygonPlot(pRender, opt)
	{
		CleanCrtlArray();
	}

	CDoveTailStraightMoreArrow::~CDoveTailStraightMoreArrow()
	{

	}

	void CDoveTailStraightMoreArrow::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
	}

	void CDoveTailStraightMoreArrow::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		CPolygonPlot::DraggerUpdateMatrix(matrix);

		if (m_rpCtrlArray->size() > 2)
		{
			SetVertex(m_rpCtrlArray);
		}
	}

	bool CDoveTailStraightMoreArrow::PushBackVertex( const osg::Vec3d& vecCoord )
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

	void CDoveTailStraightMoreArrow::ReplaceBackVertex( const osg::Vec3d& vecCoord )
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

	osg::Vec3dArray* CDoveTailStraightMoreArrow::CalculateArrow( osg::Vec3dArray* vecControlPoints)
	{
		//燕尾直箭头 
		//Ratio写成了Radio!!!
		double TailRadio=5;
		double Radio = 6;
		//计算箭头总长度
		double dLength = 0;
		//计算燕尾直箭头的宽
		double dWidth = 0;
		//在尾部两个中间插入一个点，是pointS往pointE平移的一个点，为了制作尾巴的效果
		osg::Vec3d point_tail;
		for(int i = 0;i<vecControlPoints->size()-1;i++)
		{
			//取出首尾两个点
			osg::Vec3d pointS = vecControlPoints->at(i);
			osg::Vec3d pointE = vecControlPoints->at(i+1);
			dLength += std::sqrt((pointE.y()-pointS.y())*(pointE.y()-pointS.y())+(pointE.x()-pointS.x())*(pointE.x()-pointS.x()));
			if(i==0)
			{
		
				point_tail = osg::Vec3d((pointE.x()-pointS.x())/TailRadio+pointS.x(),(pointE.y()-pointS.y())/TailRadio+pointS.y(),0.0);
			}
		}
		dWidth = dLength/Radio;
		//定义左右控制点集合
		osg::ref_ptr<osg::Vec3dArray> points_C_l = new osg::Vec3dArray;
		osg::ref_ptr<osg::Vec3dArray> points_C_r = new osg::Vec3dArray;
		//定义尾部左右的起始点
		osg::Vec3d point_t_l;
		osg::Vec3d point_t_r;
		//计算中间的所有交点
		for(int j = 0;j<vecControlPoints->size()-2;j++)
		{
		
			osg::Vec3d pointU_1 = vecControlPoints->at(j);//第一个用户传入的点
			osg::Vec3d pointU_2 = vecControlPoints->at(j+1);//第二个用户传入的点
			osg::Vec3d pointU_3 = vecControlPoints->at(j+2);//第三个用户传入的点
		
			//计算向量
		
			osg::Vec3d v_U_1_2 = osg::Vec3d(pointU_2.x()-pointU_1.x(),pointU_2.y()-pointU_1.y(),0.0);
			osg::Vec3d v_U_2_3 = osg::Vec3d(pointU_3.x()-pointU_2.x(),pointU_3.y()-pointU_2.y(),0.0);
		
		
			osg::ref_ptr<osg::Vec3dArray> v_lr_1_2 = FeUtil::CalculateVector(v_U_1_2,osg::PI_2,dWidth/2);
			osg::Vec3d v_l_1_2 = v_lr_1_2->at(0);
			osg::Vec3d v_r_1_2 = v_lr_1_2->at(1);
			osg::ref_ptr<osg::Vec3dArray> v_lr_2_3 = FeUtil::CalculateVector(v_U_2_3,osg::PI_2,dWidth/2);
			osg::Vec3d v_l_2_3 = v_lr_2_3->at(0);
			osg::Vec3d v_r_2_3 = v_lr_2_3->at(1);
			//获取左右
			osg::Vec3d point_l_1 = osg::Vec3d(pointU_1.x()+v_l_1_2.x(),pointU_1.y()+v_l_1_2.y(),0.0);
			osg::Vec3d point_r_1 = osg::Vec3d(pointU_1.x()+v_r_1_2.x(),pointU_1.y()+v_r_1_2.y(),0.0);
			osg::Vec3d point_l_2 = osg::Vec3d(pointU_2.x()+v_l_2_3.x(),pointU_2.y()+v_l_2_3.y(),0.0);
			osg::Vec3d point_r_2 = osg::Vec3d(pointU_2.x()+v_r_2_3.x(),pointU_2.y()+v_r_2_3.y(),0.0);
			//向量v_U_1_2和向量v-point_l_1和point_r_1是平行的
			//如果向量a=(x1，y1)，b=(x2，y2)，则a//b等价于x1y2－x2y1=0
			//得到(x-point_l_1.x)*v_U_1_2.y=v_U_1_2.x*(y-point_l_1.y)
			//得到(point_l_2.x-x)*v_U_2_3.y=v_U_2_3.x*(point_l_2.y-y)
			//可以求出坐边的交点(x,y)，即控制点
			osg::Vec3d point_C_l = FeUtil::CalculateIntersection(v_U_1_2,v_U_2_3,point_l_1,point_l_2);
			osg::Vec3d point_C_r = FeUtil::CalculateIntersection(v_U_1_2,v_U_2_3,point_r_1,point_r_2);
			//定义中间的控制点
			osg::Vec3d point_C_l_c;
			osg::Vec3d point_C_r_c;
			if(j == 0)
			{
				//记录下箭头尾部的左右两个端点
				point_t_l = point_l_1;
				point_t_r = point_r_1;
				//计算第一个曲线控制点
				point_C_l_c = osg::Vec3d((point_t_l.x()+point_C_l.x())/2,(point_t_l.y()+point_C_l.y())/2,0.0);
				point_C_r_c = osg::Vec3d((point_t_r.x()+point_C_r.x())/2,(point_t_r.y()+point_C_r.y())/2,0.0);
				//添加两个拐角控制点中间的中间控制点
				points_C_l->push_back(point_C_l_c);
				points_C_r->push_back(point_C_r_c);
			}
			else
			{
				//获取前一个拐角控制点
				osg::Vec3d point_C_l_q = points_C_l->at(points_C_l->size()-1);
				osg::Vec3d point_C_r_q = points_C_r->at(points_C_r->size()-1);
				//计算两个拐角之间的中心控制点
				point_C_l_c = osg::Vec3d((point_C_l_q.x()+point_C_l.x())/2,(point_C_l_q.y()+point_C_l.y())/2,0.0);
				point_C_r_c = osg::Vec3d((point_C_r_q.x()+point_C_r.x())/2,(point_C_r_q.y()+point_C_r.y())/2,0.0);
				//添加两个拐角控制点中间的中间控制点
				points_C_l->push_back(point_C_l_c);
				points_C_r->push_back(point_C_r_c);
			}
			//添加后面的拐角控制点
			points_C_l->push_back(point_C_l);
			points_C_r->push_back(point_C_r);
		}
		//计算
		
		
		
		//进入计算头部
		//计算一下头部的长度
		
		osg::Vec3d pointU_E2 = vecControlPoints->at(vecControlPoints->size()-2);//倒数第二个用户点
		osg::Vec3d pointU_E1 = vecControlPoints->at(vecControlPoints->size()-1);//最后一个用户点
		//
		osg::Vec3d v_U_E2_E1 = osg::Vec3d(pointU_E1.x()-pointU_E2.x(),pointU_E1.y()-pointU_E2.y(),0.0);
		double head_d = std::sqrt(v_U_E2_E1.x()*v_U_E2_E1.x() + v_U_E2_E1.y()*v_U_E2_E1.y());
		
		//定义头部的左右两结束点
		osg::Vec3d point_h_l;
		osg::Vec3d point_h_r;
		
		//头部左右两向量数组
		osg::ref_ptr<osg::Vec3dArray> v_lr_h = new osg::Vec3dArray;
		osg::Vec3d v_l_h ;
		osg::Vec3d v_r_h ;
		//定义曲线最后一个控制点，也就是头部结束点和最后一个拐角点的中点
		osg::Vec3d point_C_l_e ;
		osg::Vec3d point_C_r_e ;
		//定义三角形的左右两个点
		osg::Vec3d point_triangle_l ;
		osg::Vec3d point_triangle_r ;
		
		//获取当前的最后的控制点，也就是之前计算的拐角点
		
		osg::Vec3d point_C_l_eq = points_C_l->at(points_C_l->size()-1);
		osg::Vec3d point_C_r_eq = points_C_r->at(points_C_r->size()-1);
		
		//三角的高度都不够
		if(head_d <= dWidth)
		{
			v_lr_h = FeUtil::CalculateVector(v_U_E2_E1,osg::PI_2,dWidth/2);
			v_l_h = v_lr_h->at(0);
			v_r_h = v_lr_h->at(1);
			//获取头部的左右两结束点
			point_h_l = osg::Vec3d(pointU_E2.x()+v_l_h.x(),pointU_E2.y()+v_l_h.y(),0.0);
			point_h_r = osg::Vec3d(pointU_E2.x()+v_r_h.x(),pointU_E2.y()+v_r_h.y(),0.0);
		
		
			//计算最后的控制点
			point_C_l_e = osg::Vec3d((point_C_l_eq.x()+point_h_l.x())/2,(point_C_l_eq.y()+point_h_l.y())/2,0.0);
			point_C_r_e = osg::Vec3d((point_C_r_eq.x()+point_h_r.x())/2,(point_C_r_eq.y()+point_h_r.y())/2,0.0);
		
			//添加最后的控制点（中心点）
			points_C_l->push_back(point_C_l_e);
			points_C_r->push_back(point_C_r_e);
		
		
			//计算三角形的左右两点
			point_triangle_l = osg::Vec3d(2*point_h_l.x()-pointU_E2.x(),2*point_h_l.y()-pointU_E2.y(),0.0);
			point_triangle_r = osg::Vec3d(2*point_h_r.x()-pointU_E2.x(),2*point_h_r.y()-pointU_E2.y(),0.0);
		}
		//足够三角的高度
		else
		{
			//由于够了三角的高度，所以首先去掉三角的高度
		
			//计算向量
			osg::Vec3d v_E2_E1 = osg::Vec3d(pointU_E1.x()-pointU_E2.x(),pointU_E1.y()-pointU_E2.y(),0.0);
			//取模
			double v_E2_E1_d =std::sqrt(v_E2_E1.x()*v_E2_E1.x()+v_E2_E1.y()*v_E2_E1.y());
			//首先需要计算三角形的底部中心点
			osg::Vec3d point_c = osg::Vec3d(pointU_E1.x()-v_E2_E1.x()*dWidth/v_E2_E1_d,pointU_E1.y()-v_E2_E1.y()*dWidth/v_E2_E1_d,0.0);
			//计算出在三角形上底边上头部结束点
		
			v_lr_h = FeUtil::CalculateVector(v_U_E2_E1,osg::PI_2,dWidth/2);
			v_l_h = v_lr_h->at(0);
			v_r_h = v_lr_h->at(1);
			//获取头部的左右两结束点
			point_h_l = osg::Vec3d(point_c.x()+v_l_h.x(),point_c.y()+v_l_h.y(),0.0);
			point_h_r = osg::Vec3d(point_c.x()+v_r_h.x(),point_c.y()+v_r_h.y(),0.0);
		
			//计算最后的控制点
			point_C_l_e = osg::Vec3d((point_C_l_eq.x()+point_h_l.x())/2,(point_C_l_eq.y()+point_h_l.y())/2,0.0);
			point_C_r_e = osg::Vec3d((point_C_r_eq.x()+point_h_r.x())/2,(point_C_r_eq.y()+point_h_r.y())/2,0.0);
		
			//添加最后的控制点（中心点）
			points_C_l->push_back(point_C_l_e);
			points_C_r->push_back(point_C_r_e);
		
			//计算三角形的左右点
			point_triangle_l = osg::Vec3d(2*point_h_l.x()-point_c.x(),2*point_h_l.y()-point_c.y(),0.0);
			point_triangle_r = osg::Vec3d(2*point_h_r.x()-point_c.x(),2*point_h_r.y()-point_c.y(),0.0);
		}
		
		//使用控制点计算差值
		//计算贝塞尔的控制点
		osg::ref_ptr<osg::Vec3dArray> points_BC_l = FeUtil::CreateBezier2(points_C_l,20);
		osg::ref_ptr<osg::Vec3dArray> points_BC_r = FeUtil::CreateBezier2(points_C_r,20);
		//组合左右点集和三角形三个点
		osg::ref_ptr<osg::Vec3dArray> pointsR = new osg::Vec3dArray;
		
		pointsR->push_back(point_t_l);
		//首先连接左边的差值曲线
		
		for (int i=0;i<points_BC_l->size();i++)
		{
			pointsR->push_back(points_BC_l->at(i));
		}
		//添加左边头部结束点
		pointsR->push_back(point_h_l);
		//添加三角形左边点
		pointsR->push_back(point_triangle_l);
		//添加三角形顶点
		pointsR->push_back(pointU_E1);
		//添加三角形右边点
		pointsR->push_back(point_triangle_r);
		//添加右边头部结束点
		pointsR->push_back(point_h_r);
		//合并右边的所有点（先把右边的点倒序）
		for(int  k = points_BC_r->size()-1; k>=0; k--)
		{
			pointsR->push_back(points_BC_r->at(k));
		}
		//添加右边尾部起始点
		pointsR->push_back(point_t_r);
		//添加尾巴点
		pointsR->push_back(point_tail);

		return pointsR.release();
	}

}