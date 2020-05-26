#include <FePlots/DovetailArrow.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/MathUtil.h>

#define Radio 6
#define TailRadio 5

namespace FePlots
{
	CDovetailArrowOption::CDovetailArrowOption( void )
	{

	}

	CDovetailArrowOption::~CDovetailArrowOption( void )
	{

	}

}

namespace FePlots
{
	CDovetailArrow::CDovetailArrow(FeUtil::CRenderContext *pRender, CDovetailArrowOption *opt)
		: CPolygonPlot(pRender, opt)
	{
		m_bMouseMove = true;
		CleanCrtlArray();
	}

	CDovetailArrow::~CDovetailArrow(void)
	{
	}

	void CDovetailArrow::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
	}

	void CDovetailArrow::UpdateEditor()
	{
		// 更新编辑器
		CExLodNode::UpdateEditor();
	}

	void CDovetailArrow::PushBackVertex( const osg::Vec3d& vecCoord )
	{
		if(m_rpCtrlArray.valid())
		{
			m_rpCtrlArray->push_back(vecCoord);

			RebuildEditPoints();
		}
	}

	void CDovetailArrow::ReplaceBackVertex( const osg::Vec3d& vecCoord )
	{
		if(m_rpCtrlArray.valid())
		{
			if (m_rpCtrlArray->size() == 1)
			{
				m_rpCtrlArray->push_back(vecCoord);
			}
			if (m_rpCtrlArray->size() > 1)
			{
				int nIndex = m_rpCtrlArray->size() - 1;
				m_rpCtrlArray->at(nIndex) = vecCoord;

				DrawDovetailArrow();
			}

			UpdateEditor();
		}
	}

	osg::Vec3dArray* CDovetailArrow::CalculateArrow( osg::Vec3dArray* pPoints )
	{
		//取出首尾两个点
		osg::Vec3d pointS = pPoints->at(0);
		osg::Vec3d pointE = pPoints->at(1);
		//计算箭头总长度
		double dLength = std::sqrt((pointE.y()-pointS.y())*(pointE.y()-pointS.y())+(pointE.x()-pointS.x())*(pointE.x()-pointS.x()));
		//计算直箭头的宽
		double dWidth = dLength/Radio;

		//计算三角形的底边中心点坐标
		double x_ = pointS.x() + (pointE.x() - pointS.x())*(Radio-1)/Radio;
		double y_ = pointS.y() + (pointE.y() - pointS.y())*(Radio-1)/Radio;

		//计算
		osg::ref_ptr<osg::Vec3dArray> v_lr = FeUtil::CalculateVector(osg::Vec3d(pointE.x()-pointS.x(),pointE.y()-pointS.y(), 0.0), osg::PI_2, dWidth/2);
		//获取左边尾部向量
		osg::Vec3d v_l = v_lr->at(0);
		//获取右边尾部向量
		osg::Vec3d v_r = v_lr->at(1);
		//获取左边尾部点
		osg::Vec3d point_l = osg::Vec3d(v_l.x()+pointS.x(),v_l.y()+pointS.y(), 0.0);
		//获取右边尾部点
		osg::Vec3d point_r = osg::Vec3d(v_r.x()+pointS.x(),v_r.y()+pointS.y(), 0.0);
		//在尾部两个中间插入一个点，是pointS往pointE平移的一个点，为了制作尾巴的效果
		osg::Vec3d point_tail = osg::Vec3d((pointE.x()-pointS.x())/TailRadio+pointS.x(), (pointE.y()-pointS.y())/TailRadio+pointS.y(), 0.0);

		osg::Vec3d point_h_l = osg::Vec3d(v_l.x()/Radio+x_, v_l.y()/Radio+y_, 0.0);
		osg::Vec3d point_h_r = osg::Vec3d(v_r.x()/Radio+x_, v_r.y()/Radio+y_, 0.0);

		//计算三角形左边点
		osg::Vec3d point_a_l = osg::Vec3d(point_h_l.x()*2-point_h_r.x(), point_h_l.y()*2-point_h_r.y(), 0.0);
		//计算三角形右边点
		osg::Vec3d point_a_r = osg::Vec3d(point_h_r.x()*2-point_h_l.x(), point_h_r.y()*2-point_h_l.y(), 0.0);

		osg::ref_ptr<osg::Vec3dArray> rpPoints = new osg::Vec3dArray;
		rpPoints->push_back(point_tail);
		rpPoints->push_back(point_l);
		rpPoints->push_back(point_h_l);
		rpPoints->push_back(point_a_l);
		rpPoints->push_back(pointE);
		rpPoints->push_back(point_a_r);
		rpPoints->push_back(point_h_r);
		rpPoints->push_back(point_r);

		return rpPoints.release();
	}

	void CDovetailArrow::DrawDovetailArrow()
	{
		osg::ref_ptr<osg::Vec3dArray> rpCoord = CalculateArrow(m_rpCtrlArray);
		SetVertex(rpCoord);
	}

	osg::Vec3dArray* CDovetailArrow::CalculateMorePoints( osg::Vec3dArray* controlPois )
	{
		//计算箭头总长度
		double dLength = 0;
		//计算直箭头的宽
		double dWidth = 0;
		//在尾部两个中间插入一个点，是pointS往pointE平移的一个点，为了制作尾巴的效果
		osg::Vec3d point_tail;
		for(int  i = 0; i < controlPois->size()- 1; i++)
		{
			//取出首尾两个点
			osg::Vec3d pointS = controlPois->at(i);
			osg::Vec3d pointE = controlPois->at(i+1);
			dLength += std::sqrt((pointE.y()-pointS.y())*(pointE.y()-pointS.y())+(pointE.x()-pointS.x())*(pointE.x()-pointS.x()));
			if(i==0)
			{
				point_tail = osg::Vec3d((pointE.x()-pointS.x())/TailRadio+pointS.x(),(pointE.y()-pointS.y())/TailRadio+pointS.y(),0.0);
			}
		}
		dWidth = dLength/Radio;

		double a = std::atan(dWidth/(2*dLength));

		//定义左右控制点集合
		osg::ref_ptr<osg::Vec3dArray> points_C_l = new osg::Vec3dArray ;
		osg::ref_ptr<osg::Vec3dArray> points_C_r = new osg::Vec3dArray;
		//定义尾部左右的起始点
		osg::Vec3d point_t_l = osg::Vec3d();
		osg::Vec3d point_t_r = osg::Vec3d();


		//计算中间的所有交点
		for(int  j = 0; j < controlPois->size()-2; j++)
		{
			osg::Vec3d pointU_1 = controlPois->at(j);//第一个用户传入的点
			osg::Vec3d pointU_2 = controlPois->at(j+1);//第二个用户传入的点
			osg::Vec3d pointU_3 = controlPois->at(j+2);//第三个用户传入的点

			//计算向量
			osg::Vec3d v_U_1_2 = osg::Vec3d(pointU_2.x()-pointU_1.x(),pointU_2.y()-pointU_1.y(),0.0);
			osg::Vec3d v_U_2_3 = osg::Vec3d(pointU_3.x()-pointU_2.x(),pointU_3.y()-pointU_2.y(),0.0);


			//定义左边第一个控制点
			osg::Vec3d point_l_1 = osg::Vec3d();
			//定义右边第一个控制点
			osg::Vec3d point_r_1 = osg::Vec3d();
			//如果j=0时，左右第一个控制点需要计算
			if(j == 0)
			{
				osg::ref_ptr<osg::Vec3dArray> v_lr_ = FeUtil::CalculateVector(v_U_1_2,osg::PI_2,dWidth/2);
				//获取左边尾部点
				osg::Vec3d v_l_ = v_lr_->at(0);
				//获取右边尾部点
				osg::Vec3d v_r_ = v_lr_->at(1);
				//获取左边尾部点
				point_t_l = point_l_1 = osg::Vec3d(v_l_.x()+pointU_1.x(),v_l_.y()+pointU_1.y(),0.0);
				//获取右边尾部点
				point_t_r = point_r_1 = osg::Vec3d(v_r_.x()+pointU_1.x(),v_r_.y()+pointU_1.y(),0.0);
			}
			//否则获取上一次的记录
			else
			{
				point_l_1 = points_C_l->at(points_C_l->size()-1);
				point_r_1 = points_C_r->at(points_C_r->size()-1);
			}
			osg::ref_ptr<osg::Vec3dArray> v_lr = FeUtil::CalculateVector(v_U_1_2,a,1);
			//这里的向量需要反过来
			//获取左边向量
			osg::Vec3d v_l = v_lr->at(1);
			//获取右边向量
			osg::Vec3d v_r = v_lr->at(0);
			//定义角平分线向量
			osg::Vec3d v_angularBisector = FeUtil::CalculateAngularBisector(osg::Vec3d(-v_U_1_2.x(),-v_U_1_2.y(),0.0),v_U_2_3);
			//求交点
			//计算左边第二个控制点
			osg::Vec3d point_l_2 = FeUtil::CalculateIntersection(v_l,v_angularBisector,point_l_1,pointU_2);
			osg::Vec3d point_r_2 = FeUtil::CalculateIntersection(v_r,v_angularBisector,point_r_1,pointU_2);


			//添加后面的拐角控制点
			points_C_l->push_back(osg::Vec3d((point_l_1.x()+point_l_2.x())/2,(point_l_1.y()+point_l_2.y())/2,0.0));
			points_C_l->push_back(point_l_2);
			points_C_r->push_back(osg::Vec3d((point_r_1.x()+point_r_2.x())/2,(point_r_1.y()+point_r_2.y())/2,0.0));
			points_C_r->push_back(point_r_2);
		}

		//进入计算头部
		//计算一下头部的长度
		osg::Vec3d pointU_E2 = controlPois->at(controlPois->size()-2);//倒数第二个用户点
		osg::Vec3d pointU_E1 = controlPois->at(controlPois->size()-1);//最后一个用户点
		double head_d = std::sqrt((pointU_E2.x()-pointU_E1.x())*(pointU_E2.x()-pointU_E1.x()) + (pointU_E2.y()-pointU_E1.y())*(pointU_E2.y()-pointU_E1.y()));
		//定义头部的左右两结束点
		osg::Vec3d point_h_l = osg::Vec3d();
		osg::Vec3d point_h_r = osg::Vec3d();
		//三角形左右两点数组
		osg::ref_ptr<osg::Vec3dArray> point_lr_t = new osg::Vec3dArray;
		//定义曲线最后一个控制点，也就是头部结束点和最后一个拐角点的中点
		osg::Vec3d point_C_l_e = osg::Vec3d();
		osg::Vec3d point_C_r_e = osg::Vec3d();
		//定义三角形的左右两个点
		osg::Vec3d point_triangle_l = osg::Vec3d();
		osg::Vec3d point_triangle_r = osg::Vec3d();

		//获取当前的最后的控制点，也就是之前计算的拐角点
		osg::Vec3d point_C_l_eq = points_C_l->at(points_C_l->size()-1);
		osg::Vec3d point_C_r_eq = points_C_r->at(points_C_r->size()-1);   ///错误
		//申明三角形的两边向量
		osg::Vec3d v_l_t = osg::Vec3d();
		osg::Vec3d v_r_t = osg::Vec3d();
		//三角的高度都不够
		if(head_d <= dWidth)
		{
			point_lr_t = FeUtil::CalculateVector(osg::Vec3d(pointU_E1.x()-pointU_E2.x(),pointU_E1.y()-pointU_E2.y(),0.0),osg::PI_2,dWidth/2);
			//获取三角形左右两个向量
			v_l_t = point_lr_t->at(0);
			v_r_t = point_lr_t->at(1);

			point_h_l =osg::Vec3d(v_l_t.x()/Radio+pointU_E2.x(),v_l_t.y()/Radio+pointU_E2.y(),0.0);
			point_h_r = osg::Vec3d(v_r_t.x()/Radio+pointU_E2.x(),v_r_t.y()/Radio+pointU_E2.y(),0.0);
			//计算三角形的左右两点
			point_triangle_l = osg::Vec3d(point_h_l.x()*2-point_h_r.x(),point_h_l.y()*2-point_h_r.y(),0.0);
			point_triangle_r = osg::Vec3d(point_h_r.x()*2-point_h_l.x(),point_h_r.y()*2-point_h_l.y(),0.0);


			//计算最后的控制点
			point_C_l_e = osg::Vec3d((point_C_l_eq.x()+point_h_l.x())/2,(point_C_l_eq.y()+point_h_l.y())/2,0.0);
			point_C_r_e = osg::Vec3d((point_C_r_eq.x()+point_h_r.x())/2,(point_C_r_eq.y()+point_h_r.y())/2,0.0);

			//添加最后的控制点（中心点）
			points_C_l->push_back(point_C_l_e);
			points_C_r->push_back(point_C_r_e);

		}
		//足够三角的高度
		else
		{
			//由于够了三角的高度，所以首先去掉三角的高度

			//计算向量
			osg::Vec3d v_E2_E1 = osg::Vec3d(pointU_E1.x()-pointU_E2.x(),pointU_E1.y()-pointU_E2.y(),0.0);
			//取模
			double v_E2_E1_d = std::sqrt(v_E2_E1.x()*v_E2_E1.x()+v_E2_E1.y()*v_E2_E1.y());
			//首先需要计算三角形的底部中心点
			osg::Vec3d point_c = osg::Vec3d(pointU_E1.x()-v_E2_E1.x()*dWidth/v_E2_E1_d,pointU_E1.y()-v_E2_E1.y()*dWidth/v_E2_E1_d,0.0);

			//计算出在三角形上底边上头部结束点
			point_lr_t = FeUtil::CalculateVector(osg::Vec3d(pointU_E1.x()-point_c.x(),pointU_E1.y()-point_c.y(),0.0),osg::PI_2,dWidth/2);
			//获取三角形左右两个向量
			v_l_t = point_lr_t->at(0);
			v_r_t = point_lr_t->at(1);

			point_h_l = osg::Vec3d(v_l_t.x()/Radio+point_c.x(),v_l_t.y()/Radio+point_c.y(),0.0);
			point_h_r = osg::Vec3d(v_r_t.x()/Radio+point_c.x(),v_r_t.y()/Radio+point_c.y(),0.0);
			//计算三角形的左右两点
			point_triangle_l = osg::Vec3d(point_h_l.x()*2-point_h_r.x(),point_h_l.y()*2-point_h_r.y(),0.0);
			point_triangle_r = osg::Vec3d(point_h_r.x()*2-point_h_l.x(),point_h_r.y()*2-point_h_l.y(),0.0);

			//计算最后的控制点
			point_C_l_e = osg::Vec3d((point_C_l_eq.x()+point_h_l.x())/2,(point_C_l_eq.y()+point_h_l.y())/2,0.0);
			point_C_r_e = osg::Vec3d((point_C_r_eq.x()+point_h_r.x())/2,(point_C_r_eq.y()+point_h_r.y())/2,0.0);

			//添加最后的控制点（中心点）
			points_C_l->push_back(point_C_l_e);
			points_C_r->push_back(point_C_r_e);
		}
		//使用控制点计算差值
		//计算贝塞尔的控制点
		osg::ref_ptr<osg::Vec3dArray> points_BC_l = FeUtil::CreateBezier2(points_C_l);
		osg::ref_ptr<osg::Vec3dArray> points_BC_r = FeUtil::CreateBezier2(points_C_r);
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
		//合并右边的所有点
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