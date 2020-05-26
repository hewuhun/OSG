#include <FePlots/StraightMoreArrow.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>

namespace FePlots
{
	CStraightMoreArrowOption::CStraightMoreArrowOption()
	{

	}

	CStraightMoreArrowOption::~CStraightMoreArrowOption()
	{

	}
}

namespace FePlots
{
	CStraightMoreArrow::CStraightMoreArrow(FeUtil::CRenderContext *pRender, CStraightMoreArrowOption *opt)
		: CPolygonPlot(pRender, opt)
	{
		CleanCrtlArray();
	}

	CStraightMoreArrow::~CStraightMoreArrow()
	{

	}

	void CStraightMoreArrow::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
	}

	void CStraightMoreArrow::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		CPolygonPlot::DraggerUpdateMatrix(matrix);

		if (m_rpCtrlArray->size() > 2)
		{
			SetVertex(m_rpCtrlArray);
		}
	}

	bool CStraightMoreArrow::PushBackVertex( const osg::Vec3d& vecCoord )
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

	void CStraightMoreArrow::ReplaceBackVertex( const osg::Vec3d& vecCoord )
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

	osg::Vec3dArray* CStraightMoreArrow::CalculateArrow( osg::Vec3dArray* vecControlPoints)
	{
		//直箭头
		double dRadio = 6;
		double dArrowLength=0;
		double dArrowWidth =0;

		for (int i=0;i<vecControlPoints->size()-1;i++)
		{
			//取出首尾两个点
			osg::Vec3d pointS = vecControlPoints->at(i);
			osg::Vec3d pointE = vecControlPoints->at(i+1);
			//计算箭头总长度，即两个控制点的距离
			dArrowLength+=std::sqrt((pointE.y()-pointS.y())*(pointE.y()-pointS.y())+(pointE.x()-pointS.x())*(pointE.x()-pointS.x()));
		}
		dArrowWidth =dArrowLength/dRadio;
		//定义左右控制点集合
		osg::ref_ptr<osg::Vec3dArray> rpPoints_C_L = new osg::Vec3dArray();
		osg::ref_ptr<osg::Vec3dArray> rpPoints_C_R = new osg::Vec3dArray();
		//定义尾部左右的起始点
		osg::Vec3d vecPoint_T_L = osg::Vec3d();
		osg::Vec3d vecPoint_T_R = osg::Vec3d();
		//计算中间的所有交点
		for (int j=0;j<vecControlPoints->size()-2;j++)
		{
			//第一个用户传入的点
			osg::Vec3d vecPointU_1 = vecControlPoints->at(j);
			//第二个用户传入的点
			osg::Vec3d vecPointU_2 = vecControlPoints->at(j+1);
			//第三个用户传入的点
			osg::Vec3d vecPointU_3 = vecControlPoints->at(j+2);

			//计算向量
			osg::Vec3d vecV_U_1_2 = osg::Vec3d(vecPointU_2.x()-vecPointU_1.x(),vecPointU_2.y()-vecPointU_1.y(),0.0);
			osg::Vec3d vecV_U_2_3 = osg::Vec3d(vecPointU_3.x()-vecPointU_2.x(),vecPointU_3.y()-vecPointU_2.y(),0.0);

			osg::ref_ptr<osg::Vec3dArray> vecV_LR_1_2 = FeUtil::CalculateVector(vecV_U_1_2,osg::PI_2,dArrowWidth/2);
			osg::Vec3d vecV_L_1_2 = vecV_LR_1_2->at(0);
			osg::Vec3d vecV_R_1_2 = vecV_LR_1_2->at(1);
			osg::ref_ptr<osg::Vec3dArray> vecV_LR_2_3 = FeUtil::CalculateVector(vecV_U_2_3,osg::PI_2,dArrowWidth/2);
			osg::Vec3d vecV_L_2_3 = vecV_LR_2_3->at(0);
			osg::Vec3d vecV_R_2_3 = vecV_LR_2_3->at(1);

			//获取左右
			osg::Vec3d vecPoint_L_1 = osg::Vec3d(vecPointU_1.x()+vecV_L_1_2.x(),vecPointU_1.y()+vecV_L_1_2.y(),0.0);
			osg::Vec3d vecPoint_R_1 = osg::Vec3d(vecPointU_1.x()+vecV_R_1_2.x(),vecPointU_1.y()+vecV_R_1_2.y(),0.0);
			osg::Vec3d vecPoint_L_2 = osg::Vec3d(vecPointU_2.x()+vecV_L_2_3.x(),vecPointU_2.y()+vecV_L_2_3.y(),0.0);
			osg::Vec3d vecPoint_R_2 = osg::Vec3d(vecPointU_2.x()+vecV_R_2_3.x(),vecPointU_2.y()+vecV_R_2_3.y(),0.0);
			//可以求出坐边的交点(x,y)，即控制点
			osg::Vec3d vecPoint_C_L = FeUtil::CalculateIntersection(vecV_U_1_2,vecV_U_2_3,vecPoint_L_1,vecPoint_L_2);
			osg::Vec3d vecPoint_C_R = FeUtil::CalculateIntersection(vecV_U_1_2,vecV_U_2_3,vecPoint_R_1,vecPoint_R_2);
			//定义中间的控制点 Control Left Centre
			osg::Vec3d vecPoint_C_L_C;
			osg::Vec3d vecPoint_C_R_C;
			if (j==0)
			{
				//记录下箭头尾部的左右两个端点
				vecPoint_T_L = vecPoint_L_1;
				vecPoint_T_R = vecPoint_R_1;
				//计算第一个曲线控制点
				vecPoint_C_L_C = osg::Vec3d((vecPoint_T_L.x()+vecPoint_C_L.x())/2,(vecPoint_T_L.y()+vecPoint_C_L.y())/2,0.0);
				vecPoint_C_R_C = osg::Vec3d((vecPoint_T_R.x()+vecPoint_C_R.x())/2,(vecPoint_T_R.y()+vecPoint_C_R.y())/2,0.0);
				//添加两个拐角控制点中间的中间控制点
				rpPoints_C_L->push_back(vecPoint_C_L_C);
				rpPoints_C_R->push_back(vecPoint_C_R_C);
			}
			else
			{
				//获取前一个拐角控制点
				osg::Vec3d vecPoint_C_L_Q = rpPoints_C_L->at(rpPoints_C_L->size()-1);
				osg::Vec3d vecPoint_C_R_Q = rpPoints_C_R->at(rpPoints_C_R->size()-1);
				//计算两个拐角之间的中心控制点
				vecPoint_C_L_C = osg::Vec3d((vecPoint_C_L_Q.x()+vecPoint_C_L.x())/2,(vecPoint_C_L_Q.y()+vecPoint_C_L.y())/2,0.0);
				vecPoint_C_R_C = osg::Vec3d((vecPoint_C_R_Q.x()+vecPoint_C_R.x())/2,(vecPoint_C_R_Q.y()+vecPoint_C_R.y())/2,0.0);
				//添加两个拐角控制点中间的中间控制点
				rpPoints_C_L->push_back(vecPoint_C_L_C);
				rpPoints_C_R->push_back(vecPoint_C_R_C);

			}
			//添加后面的拐角控制点
			rpPoints_C_L->push_back(vecPoint_C_L);
			rpPoints_C_R->push_back(vecPoint_C_R);
		}
		//计算
		//进入计算头部
		//计算一下头部的长度

		//倒数第二个用户点
		osg::Vec3d vecPointU_E_2 = vecControlPoints->at(vecControlPoints->size()-2);
		//最后一个用户点
		osg::Vec3d vecPointU_E_1 = vecControlPoints->at(vecControlPoints->size()-1);
		//
		osg::Vec3d vecV_U_E2_E1 = osg::Vec3d(vecPointU_E_1.x()-vecPointU_E_2.x(),vecPointU_E_1.y()-vecPointU_E_2.y(),0.0);
		double dHead_D = std::sqrt(vecV_U_E2_E1.x()*vecV_U_E2_E1.x()+vecV_U_E2_E1.y()*vecV_U_E2_E1.y());

		//定义头部的左右两结束点
		osg::Vec3d vecPoint_H_L;
		osg::Vec3d vecPoint_H_R;
		//头部左右两向量数组
		osg::ref_ptr<osg::Vec3dArray> rpV_LR_H = new osg::Vec3dArray();

		osg::Vec3d vecV_L_H = osg::Vec3d();
		osg::Vec3d vecV_R_H = osg::Vec3d();
		//定义曲线最后一个控制点，也就是头部结束点和最后一个拐角点的中点
		osg::Vec3d vecPoint_C_L_E = osg::Vec3d();
		osg::Vec3d vecPoint_C_R_E = osg::Vec3d();
		//定义三角形的左右两个点
		osg::Vec3d vecPoint_Triangle_L = osg::Vec3d();
		osg::Vec3d vecPoint_Triangle_R = osg::Vec3d();
		//获取当前的最后的控制点，也就是之前计算的拐角点
		osg::Vec3d vecPoint_C_L_EQ =rpPoints_C_L->at(rpPoints_C_L->size()-1);
		osg::Vec3d vecPoint_C_R_EQ =rpPoints_C_R->at(rpPoints_C_R->size()-1);
		//三角的高度都不够
		if (dHead_D<=dArrowWidth)
		{
			rpV_LR_H = FeUtil::CalculateVector(vecV_U_E2_E1,osg::PI_2,dArrowWidth/2);
			vecV_L_H = rpV_LR_H->at(0);
			vecV_R_H = rpV_LR_H->at(1);
			//获取头部的左右两结束点

			vecPoint_H_L = osg::Vec3d(vecPointU_E_2.x()+vecV_L_H.x(),vecPointU_E_2.y()+vecV_L_H.y(),0.0);
			vecPoint_H_R = osg::Vec3d(vecPointU_E_2.x()+vecV_R_H.x(),vecPointU_E_2.y()+vecV_R_H.y(),0.0);
			//计算最后的控制点
			vecPoint_C_L_E =osg::Vec3d((vecPoint_C_L_EQ.x()+vecPoint_H_L.x())/2,(vecPoint_C_L_EQ.y()+vecPoint_H_L.y())/2,0.0);
			vecPoint_C_R_E =osg::Vec3d((vecPoint_C_R_EQ.x()+vecPoint_H_R.x())/2,(vecPoint_C_R_EQ.y()+vecPoint_H_R.y())/2,0.0);
			//添加最后的控制点（中心点）
			rpPoints_C_L->push_back(vecPoint_C_L_E);
			rpPoints_C_R->push_back(vecPoint_C_R_E);
			//计算三角形的左右两点
			vecPoint_Triangle_L = osg::Vec3d(2*vecPoint_H_L.x()-vecPointU_E_2.x(),2*vecPoint_H_L.y()-vecPointU_E_2.y(),0.0);

			vecPoint_Triangle_R = osg::Vec3d(2*vecPoint_H_R.x()-vecPointU_E_2.x(),2*vecPoint_H_R.y()-vecPointU_E_2.y(),0.0);

		}
		//足够三角的高度
		else
		{
			//由于够了三角的高度，所以首先去掉三角的高度
			//计算向量
			osg::Vec3d vecV_E2_E1 = osg::Vec3d(vecPointU_E_1.x()-vecPointU_E_2.x(),vecPointU_E_1.y()-vecPointU_E_2.y(),0.0);
			//取模  这里因为vecV_E2_E1向量的Z轴分量为0，所以没加进来，若Z轴分量有变，记得加进来
			double dV_E2_E1_d = std::sqrt(vecV_E2_E1.x()*vecV_E2_E1.x()+vecV_E2_E1.y()*vecV_E2_E1.y());
			//首先需要计算三角形的底部中心点
			osg::Vec3d vecPoint_C = osg::Vec3d(vecPointU_E_1.x()-vecV_E2_E1.x()*dArrowWidth/dV_E2_E1_d,
				vecPointU_E_1.y()-vecV_E2_E1.y()*dArrowWidth/dV_E2_E1_d,0.0);
			//计算出在三角形上底边上头部结束点
			rpV_LR_H = FeUtil::CalculateVector(vecV_U_E2_E1,osg::PI_2,dArrowWidth/2);
			vecV_L_H = rpV_LR_H->at(0);
			vecV_R_H = rpV_LR_H->at(1);
			//获取头部的左右两结束点
			vecPoint_H_L = osg::Vec3d(vecPoint_C.x()+vecV_L_H.x(),vecPoint_C.y()+vecV_L_H.y(),0.0);
			vecPoint_H_R = osg::Vec3d(vecPoint_C.x()+vecV_R_H.x(),vecPoint_C.y()+vecV_R_H.y(),0.0);
			//计算最后的控制点
			vecPoint_C_L_E = osg::Vec3d((vecPoint_C_L_EQ.x()+vecPoint_H_L.x())/2,(vecPoint_C_L_EQ.y()+vecPoint_H_L.y())/2,0.0);
			vecPoint_C_R_E = osg::Vec3d((vecPoint_C_R_EQ.x()+vecPoint_H_R.x())/2,(vecPoint_C_R_EQ.y()+vecPoint_H_R.y())/2,0.0);
			//添加最后的控制点（中心点）
			rpPoints_C_L->push_back(vecPoint_C_L_E);
			rpPoints_C_R->push_back(vecPoint_C_R_E);
			//计算三角形的左右点
			vecPoint_Triangle_L = osg::Vec3d(2*vecPoint_H_L.x()-vecPoint_C.x(),2*vecPoint_H_L.y()-vecPoint_C.y(),0.0);
			vecPoint_Triangle_R = osg::Vec3d(2*vecPoint_H_R.x()-vecPoint_C.x(),2*vecPoint_H_R.y()-vecPoint_C.y(),0.0);
		}
		//使用控制点计算差值
		//计算贝塞尔的控制点
		osg::ref_ptr<osg::Vec3dArray> rpPointsBC_L = FeUtil::CreateBezier2(rpPoints_C_L);

		osg::ref_ptr<osg::Vec3dArray> rpPointsBC_R = FeUtil::CreateBezier2(rpPoints_C_R);

		osg::ref_ptr<osg::Vec3dArray> rpPointsR = new osg::Vec3dArray();
		rpPointsR->push_back(vecPoint_T_L);
		for (int i=0;i<rpPointsBC_L->size();i++)
		{
			rpPointsR->push_back(rpPointsBC_L->at(i));
		}

		//添加左边头部结束点
		rpPointsR->push_back(vecPoint_H_L);
		//添加三角形左边点
		rpPointsR->push_back(vecPoint_Triangle_L);
		//添加三角形顶点
		rpPointsR->push_back(vecPointU_E_1);
		//添加三角形右边点
		rpPointsR->push_back(vecPoint_Triangle_R);
		//添加右边头部结束点
		rpPointsR->push_back(vecPoint_H_R);
		//合并右边的所有点（先把右边的点倒序）
		for (int i=rpPointsBC_R->size()-1;i>=0;i--)
		{
			rpPointsR->push_back(rpPointsBC_R->at(i));
		}

		//添加右边尾部起始点
		rpPointsR->push_back(vecPoint_T_R);

		return rpPointsR.release();
	}

}