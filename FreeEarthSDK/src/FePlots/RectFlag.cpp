#include <FePlots/RectFlag.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>
namespace FePlots
{
	CRectFlagOption::CRectFlagOption( void )
	{

	}

	CRectFlagOption::~CRectFlagOption( void )
	{

	}
}

namespace FePlots
{
	CRectFlag::CRectFlag(FeUtil::CRenderContext *pRender, CRectFlagOption *opt)
		: CPolygonPlot(pRender, opt)
	{
		m_bMouseMove = true;
		CleanCrtlArray();
	}

	CRectFlag::~CRectFlag(void)
	{
	}

	void CRectFlag::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
		SetLineWidth(3.0);
	}

	void CRectFlag::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		CPolygonPlot::DraggerUpdateMatrix(matrix);

		SetVertex(m_rpCtrlArray);
	}

	void CRectFlag::PushBackVertex( const osg::Vec3d& vecCoord )
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

	void CRectFlag::ReplaceBackVertex( const osg::Vec3d& vecCoord )
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

	osg::Vec3dArray* CRectFlag::CalculateArrow( osg::Vec3dArray* vecControlPoints)
	{
		//至少需要两个控制点
		if(vecControlPoints->size()>1)
		{
			//取第一个
			osg::Vec3d vecPointS = vecControlPoints->at(0);
			//取最后一个
			osg::Vec3d vecPointE = vecControlPoints->at(vecControlPoints->size()-1);
			osg::Vec3d point1 = vecPointS;
			osg::Vec3d point2 = osg::Vec3d(vecPointE.x(),vecPointS.y(),0.0);
			osg::Vec3d point3 = osg::Vec3d(vecPointE.x(),(vecPointS.y()+vecPointE.y())/2,0.0);
			osg::Vec3d point4 = osg::Vec3d(vecPointS.x(),(vecPointS.y()+vecPointE.y())/2,0.0);
			osg::Vec3d point5 = osg::Vec3d(vecPointS.x(),vecPointE.y(),0.0);
		
			osg::ref_ptr<osg::Vec3dArray> rpPoints = new osg::Vec3dArray;
			rpPoints->push_back(point1);
			rpPoints->push_back(point2);
			rpPoints->push_back(point3);
			rpPoints->push_back(point4);
			rpPoints->push_back(point5);
			return rpPoints.release();
		}
		return nullptr;
	}
}
