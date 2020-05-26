#include <FePlots/CurveFlag.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/MathUtil.h>

namespace FePlots
{
	

	CCurveFlagOption::CCurveFlagOption( void )
	{

	}

	CCurveFlagOption::~CCurveFlagOption( void )
	{

	}

}

namespace FePlots
{
	CCurveFlag::CCurveFlag(FeUtil::CRenderContext *pRender, CCurveFlagOption *opt)
		: CPolygonPlot(pRender, opt)
	{
		m_bMouseMove = true;
		CleanCrtlArray();
	}

	CCurveFlag::~CCurveFlag(void)
	{
	}

	void CCurveFlag::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
		SetLineWidth(3.0);
	}

	void CCurveFlag::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		CPolygonPlot::DraggerUpdateMatrix(matrix);

		SetVertex(m_rpCtrlArray);
	}

	void CCurveFlag::PushBackVertex( const osg::Vec3d& vecCoord )
	{
		if (vecCoord.x() == 0 && vecCoord.y() == 0 && vecCoord.z() == 0)
		{
			return;
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
	}

	void CCurveFlag::ReplaceBackVertex( const osg::Vec3d& vecCoord )
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

	osg::Vec3dArray* CCurveFlag::CalculateArrow( osg::Vec3dArray* vecControlPoints)
	{
		if(vecControlPoints->size()>1)
		{
			//取第一个
			osg::Vec3d vecPointS =vecControlPoints->at(0);
			//取最后一个
			osg::Vec3d vecPointE = vecControlPoints->at(vecControlPoints->size()-1);
			//上曲线起始点
			osg::Vec3d point1 = vecPointS;
			//上曲线第一控制点
			osg::Vec3d point2 = osg::Vec3d((vecPointE.x()-vecPointS.x())/4+vecPointS.x(),(vecPointE.y()-vecPointS.y())/8+vecPointS.y(),0.0);
			//上曲线第二个点
			osg::Vec3d point3 = osg::Vec3d((vecPointS.x()+vecPointE.x())/2,vecPointS.y(),0.0);
			//上曲线第二控制点
			osg::Vec3d point4 = osg::Vec3d((vecPointE.x()-vecPointS.x())*3/4+vecPointS.x(),-(vecPointE.y()-vecPointS.y())/8+vecPointS.y(),0.0);
			//上曲线结束点
			osg::Vec3d point5 = osg::Vec3d(vecPointE.x(),vecPointS.y(),0.0);
			//上曲线控制点数组
			osg::ref_ptr<osg::Vec3dArray> rpTCvCPoints = new osg::Vec3dArray();
			rpTCvCPoints->push_back(point1);
			rpTCvCPoints->push_back(point2);
			rpTCvCPoints->push_back(point3);
			rpTCvCPoints->push_back(point4);
			rpTCvCPoints->push_back(point5);

			//下曲线结束点
			osg::Vec3d point6 = osg::Vec3d(vecPointE.x(),(vecPointS.y()+vecPointE.y())/2,0.0);
			//下曲线第二控制点
			osg::Vec3d point7 = osg::Vec3d((vecPointE.x()-vecPointS.x())*3/4+vecPointS.x(),(vecPointE.y()-vecPointS.y())*3/8+vecPointS.y(),0.0);
			//下曲线第二个点
			osg::Vec3d point8 = osg::Vec3d((vecPointS.x()+vecPointE.x())/2,(vecPointS.y()+vecPointE.y())/2,0.0);
			//下曲线第一控制点
			osg::Vec3d point9 = osg::Vec3d((vecPointE.x()-vecPointS.x())/4+vecPointS.x(),(vecPointE.y()-vecPointS.y())*5/8+vecPointS.y(),0.0);
			//下曲线起始点
			osg::Vec3d point10 = osg::Vec3d(vecPointS.x(),(vecPointS.y()+vecPointE.y())/2,0.0);
			//旗杆底部点
			osg::Vec3d point11 = osg::Vec3d(vecPointS.x(),vecPointE.y(),0.0);
			//下曲线控制点数组
			osg::ref_ptr<osg::Vec3dArray> rpBCvCPoints = new osg::Vec3dArray();
			rpBCvCPoints->push_back(point6);
			rpBCvCPoints->push_back(point7);
			rpBCvCPoints->push_back(point8);
			rpBCvCPoints->push_back(point9);
			rpBCvCPoints->push_back(point10);
			//计算上曲线
			osg::ref_ptr<osg::Vec3dArray>  rpCurveT = FeUtil::CreateBezier2(rpTCvCPoints);
			//计算下曲线
			osg::ref_ptr<osg::Vec3dArray>  rpCurveB = FeUtil::CreateBezier2(rpBCvCPoints);
			//合并
			for (int i=0;i<rpCurveB->size();i++)
			{
				rpCurveT->push_back(rpCurveB->at(i));
			}
			rpCurveT->push_back(point11);
			return rpCurveT.release();
		}

		return nullptr;
		
	}


	
}