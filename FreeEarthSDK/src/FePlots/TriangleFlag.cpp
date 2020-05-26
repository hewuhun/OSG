#include <FePlots/TriangleFlag.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>

#define Radio 6
#define TailRadio 5

namespace FePlots
{
	CTriangleFlagOption::CTriangleFlagOption( void )
	{

	}

	CTriangleFlagOption::~CTriangleFlagOption( void )
	{

	}

}

namespace FePlots
{
	CTriangleFlag::CTriangleFlag(FeUtil::CRenderContext *pRender, CTriangleFlagOption *opt)
		: CPolygonPlot(pRender, opt)
	{
		m_bMouseMove = true;
		CleanCrtlArray();
	}

	CTriangleFlag::~CTriangleFlag(void)
	{
	}

	void CTriangleFlag::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
		SetLineWidth(3.0);
	}

	void CTriangleFlag::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		CPolygonPlot::DraggerUpdateMatrix(matrix);

		SetVertex(m_rpCtrlArray);
	}

	void CTriangleFlag::PushBackVertex( const osg::Vec3d& vecCoord )
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

	void CTriangleFlag::ReplaceBackVertex( const osg::Vec3d& vecCoord )
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

	osg::Vec3dArray* CTriangleFlag::CalculateArrow( osg::Vec3dArray* vecControlPoints)
	{
		if(vecControlPoints->size()>1)
		{
			osg::Vec3d startPoint = vecControlPoints->at(0);
			osg::Vec3d endPoint = vecControlPoints->at(vecControlPoints->size()-1);

			osg::Vec3d point1 = startPoint;
			osg::Vec3d point2 = osg::Vec3d(endPoint.x(),(startPoint.y()+endPoint.y())/2,0.0);
			osg::Vec3d point3 = osg::Vec3d(startPoint.x(),(startPoint.y()+endPoint.y())/2,0.0);
			osg::Vec3d point4 = osg::Vec3d(startPoint.x(),endPoint.y(),0.0);

			osg::ref_ptr<osg::Vec3dArray> rpPoints = new osg::Vec3dArray;

			rpPoints->push_back(point1);
			rpPoints->push_back(point2);
			rpPoints->push_back(point3);
			rpPoints->push_back(point4);

			return rpPoints.release();
		}
		return nullptr;
	}
}