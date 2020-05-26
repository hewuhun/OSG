#include <FePlots/DoubleArrow.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>

namespace FePlots
{
	CDoubleArrowOption::CDoubleArrowOption()
	{

	}

	CDoubleArrowOption::~CDoubleArrowOption()
	{

	}
}

namespace FePlots
{
	CDoubleArrow::CDoubleArrow(FeUtil::CRenderContext *pRender, CDoubleArrowOption *opt)
		: CPolygonPlot(pRender, opt)
	{
		m_bMouseMove = true;
		CleanCrtlArray();
	}

	CDoubleArrow::~CDoubleArrow()
	{

	}

	void CDoubleArrow::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
	}

	void CDoubleArrow::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		CPolygonPlot::DraggerUpdateMatrix(matrix);

		if (m_rpCtrlArray->size() == 4)
		{
			SetVertex(m_rpCtrlArray);
		}
	}

	bool CDoubleArrow::PushBackVertex( const osg::Vec3d& vecCoord )
	{
		if (vecCoord.x() == 0 && vecCoord.y() == 0 && vecCoord.z() == 0)
		{
			return false;
		}

		if(m_rpCtrlArray.valid())
		{
			if (m_rpCtrlArray->size() < 4)
			{
				m_rpCtrlArray->push_back(vecCoord);
			}
			else
			{
				m_bMouseMove = false;
			}
			RebuildEditPoints();
		}

		return true;
	}

	void CDoubleArrow::ReplaceBackVertex( const osg::Vec3d& vecCoord )
	{
		if(m_rpCtrlArray.valid())
		{
			if (m_rpCtrlArray->size() == 3)
			{
				m_rpCtrlArray->push_back(vecCoord);
			}
			if (m_rpCtrlArray->size() == 4 && m_bMouseMove)
			{
				int nIndex = m_rpCtrlArray->size() - 1;
				m_rpCtrlArray->at(nIndex) = vecCoord;
				SetVertex(m_rpCtrlArray);
			}
		}
	}

	osg::Vec3dArray* CDoubleArrow::CalculateArrow( osg::Vec3dArray* vecControlPoints)
	{
		//判定少于四个点或者为空，则直接返回
		if(vecControlPoints == nullptr || vecControlPoints->size()<4)
		{
			return nullptr;
		}
		//定义四个用户输入点
		osg::Vec3d pointU_1 = vecControlPoints->at(0);
		osg::Vec3d pointU_2 = vecControlPoints->at(1);
		osg::Vec3d pointU_3 = vecControlPoints->at(2);
		osg::Vec3d pointU_4 = vecControlPoints->at(3);

		//计算控制点
		//计算中间用户点F
		osg::Vec3d pointU_C = osg::Vec3d(((pointU_1.x()+pointU_2.x())*5+(pointU_3.x()+pointU_4.x()))/12,
			((pointU_1.y()+pointU_2.y())*5+(pointU_3.y()+pointU_4.y()))/12,0.0);
		//计算左边外弧的控制点
		osg::Vec3d pointC_l_out = FeUtil::CalculateIntersectionFromTwoCorner(pointU_1,pointU_4,osg::PI/8,osg::PI/6)->at(0);
		//计算左边内弧的控制点
		osg::Vec3d pointC_l_inner = FeUtil::CalculateIntersectionFromTwoCorner(pointU_C,pointU_4,osg::PI/8,osg::PI/16)->at(0);
		//计算右边外弧的控制点
		osg::Vec3d pointC_r_out = FeUtil::CalculateIntersectionFromTwoCorner(pointU_2,pointU_3,osg::PI/8,osg::PI/6)->at(1);
		//计算右边内弧的控制点
		osg::Vec3d pointC_r_inner = FeUtil::CalculateIntersectionFromTwoCorner(pointU_C,pointU_3,osg::PI/8,osg::PI/16)->at(1);

		osg::Vec3d v_l_out = osg::Vec3d(pointC_l_out.x()-pointU_4.x(),pointC_l_out.y()-pointU_4.y(),0.0);
		double d_l_out = std::sqrt(v_l_out.x()*v_l_out.x()+v_l_out.y()*v_l_out.y());
		//单位向量
		osg::Vec3d v_l_out_1 = osg::Vec3d(v_l_out.x()/d_l_out,v_l_out.y()/d_l_out,0.0);

		osg::Vec3d v_l_inner = osg::Vec3d(pointC_l_inner.x()-pointU_4.x(),pointC_l_inner.y()-pointU_4.y(),0.0);
		double d_l_inner = std::sqrt(v_l_inner.x()*v_l_inner.x()+v_l_inner.y()*v_l_inner.y());
		//单位向量
		osg::Vec3d v_l_inner_1 = osg::Vec3d(v_l_inner.x()/d_l_inner,v_l_inner.y()/d_l_inner,0.0);

		//定义箭头头部的大小比例
		double ab = 0.25;

		//取最短的，除以5是一个经验值，这样效果比较好
		double d_l_a = (d_l_out<d_l_inner)?(d_l_out*ab):(d_l_inner*ab);
		osg::Vec3d pointC_l_out_2 =   osg::Vec3d(v_l_out_1.x()*d_l_a+pointU_4.x(),v_l_out_1.y()*d_l_a+pointU_4.y(),0.0);
		osg::Vec3d pointC_l_inner_2 = osg::Vec3d(v_l_inner_1.x()*d_l_a+pointU_4.x(),v_l_inner_1.y()*d_l_a+pointU_4.y(),0.0);

		//左箭头左边点
		osg::Vec3d pointC_l_a_l = osg::Vec3d(pointC_l_out_2.x()*1.5-pointC_l_inner_2.x()*0.5,pointC_l_out_2.y()*1.5-pointC_l_inner_2.y()*0.5,0.0);
		//左箭头右边点
		osg::Vec3d pointC_l_a_r = osg::Vec3d(pointC_l_inner_2.x()*1.5-pointC_l_out_2.x()*0.5,pointC_l_inner_2.y()*1.5-pointC_l_out_2.y()*0.5,0.0);

		osg::Vec3d v_r_out = osg::Vec3d(pointC_r_out.x()-pointU_3.x(),pointC_r_out.y()-pointU_3.y(),0.0);
		double d_r_out = std::sqrt(v_r_out.x()*v_r_out.x()+v_r_out.y()*v_r_out.y());
		osg::Vec3d v_r_out_1 = osg::Vec3d(v_r_out.x()/d_r_out,v_r_out.y()/d_r_out,0.0);

		osg::Vec3d v_r_inner = osg::Vec3d(pointC_r_inner.x()-pointU_3.x(),pointC_r_inner.y()-pointU_3.y(),0.0);
		double d_r_inner = std::sqrt(v_r_inner.x()*v_r_inner.x()+v_r_inner.y()*v_r_inner.y());
		osg::Vec3d v_r_inner_1 = osg::Vec3d(v_r_inner.x()/d_r_inner,v_r_inner.y()/d_r_inner,0.0);

		//取最短的，除以5是一个经验值，这样效果比较好
		double d_r_a = (d_r_out<d_r_inner)?(d_r_out*ab):(d_r_inner*ab);
		osg::Vec3d pointC_r_out_2 =   osg::Vec3d(v_r_out_1.x()*d_r_a+pointU_3.x(),v_r_out_1.y()*d_r_a+pointU_3.y(),0.0);
		osg::Vec3d pointC_r_inner_2 = osg::Vec3d(v_r_inner_1.x()*d_r_a+pointU_3.x(),v_r_inner_1.y()*d_r_a+pointU_3.y(),0.0);

		//右箭头箭头右边点
		osg::Vec3d pointC_r_a_r = osg::Vec3d(pointC_r_out_2.x()*1.5-pointC_r_inner_2.x()*0.5,pointC_r_out_2.y()*1.5-pointC_r_inner_2.y()*0.5,0.0);
		//左箭头左边点
		osg::Vec3d pointC_r_a_l = osg::Vec3d(pointC_r_inner_2.x()*1.5-pointC_r_out_2.x()*0.5,pointC_r_inner_2.y()*1.5-pointC_r_out_2.y()*0.5,0.0);

		//计算坐边外弧所有点
		osg::ref_ptr<osg::Vec3dArray> rpPoints_1 = new osg::Vec3dArray;
		rpPoints_1->push_back(pointU_1);
		rpPoints_1->push_back(pointC_l_out);
		rpPoints_1->push_back(pointC_l_out_2);
		osg::ref_ptr<osg::Vec3dArray> points_l = FeUtil::CreateBezier2(rpPoints_1);

		//计算控制点
		//定义向量
		osg::Vec3d v_U_4_3 = osg::Vec3d(pointU_3.x()-pointU_4.x(),pointU_3.y()-pointU_4.y(),0.0);

		//取部分
		//需要优化，不能左右都取一样，需要按照左右的长度取值，这样更合理一些
		//取u4和C的向量模
		//取u3和C的向量模
		//根据模的大小来取左右向量的长度来定位置
		osg::Vec3d v_U_4_C = osg::Vec3d(pointU_C.x()-pointU_4.x(),pointU_C.y()-pointU_4.y(),0.0);
		//求模
		double d_U_4_C = std::sqrt(v_U_4_C.x()*v_U_4_C.x()+v_U_4_C.y()*v_U_4_C.y());
		osg::Vec3d v_U_3_C = osg::Vec3d(pointU_C.x()-pointU_3.x(),pointU_C.y()-pointU_3.y(),0.0);
		//求模
		double d_U_3_C = std::sqrt(v_U_3_C.x()*v_U_3_C.x()+v_U_3_C.y()*v_U_3_C.y());

		double percent = 0.4;
		osg::Vec3d v_U_4_3_ =  osg::Vec3d(v_U_4_3.x()*percent,v_U_4_3.y()*percent,0.0);
		osg::Vec3d v_U_4_3_l = osg::Vec3d(v_U_4_3_.x()*d_U_4_C/(d_U_4_C+d_U_3_C),v_U_4_3_.y()*d_U_4_C/(d_U_4_C+d_U_3_C),0.0);
		osg::Vec3d v_U_4_3_r = osg::Vec3d(v_U_4_3_.x()*d_U_3_C/(d_U_4_C+d_U_3_C),v_U_4_3_.y()*d_U_3_C/(d_U_4_C+d_U_3_C),0.0);
		//中心点的左控制点
		osg::Vec3d pointC_c_l = osg::Vec3d(pointU_C.x()-v_U_4_3_l.x(),pointU_C.y()-v_U_4_3_l.y(),0.0);
		//中心点右边的控制点
		osg::Vec3d pointC_c_r = osg::Vec3d(pointU_C.x()+v_U_4_3_r.x(),pointU_C.y()+v_U_4_3_r.y(),0.0);
		//测试
		osg::ref_ptr<osg::Vec3dArray> arr = new osg::Vec3dArray; 
		arr->push_back(pointC_l_inner_2);
		arr->push_back(pointC_l_inner);
		arr->push_back(pointC_c_l);
		arr->push_back(pointU_C);
		arr->push_back(pointC_c_r);
		arr->push_back(pointC_r_inner);
		arr->push_back(pointC_r_inner_2);
		osg::ref_ptr<osg::Vec3dArray> points_c = FeUtil::CreateBezier3(arr,20);

		osg::ref_ptr<osg::Vec3dArray> rpPoints_2 = new osg::Vec3dArray;
		rpPoints_2->push_back(pointC_r_out_2);
		rpPoints_2->push_back(pointC_r_out);
		rpPoints_2->push_back(pointU_2);
		//计算右边外弧的所有点
		osg::ref_ptr<osg::Vec3dArray> points_r = FeUtil::CreateBezier2(rpPoints_2);

		//定义结果数组
		osg::ref_ptr<osg::Vec3dArray> result = new osg::Vec3dArray;
		result->insert(result->end(),points_l->begin(),points_l->end());
		result->push_back(pointC_l_a_l);
		result->push_back(pointU_4);
		result->push_back(pointC_l_a_r);
		result->insert(result->end(),points_c->begin(),points_c->end());
		result->push_back(pointC_r_a_l);
		result->push_back(pointU_3);
		result->push_back(pointC_r_a_r);
		result->insert(result->end(),points_r->begin(),points_r->end());
		//清空原有的所有点
		return result.release();
	}

}