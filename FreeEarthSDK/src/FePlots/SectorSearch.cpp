#include <FePlots/SectorSearch.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>

namespace FePlots
{
	CSectorSearchOption::CSectorSearchOption()
	{

	}

	CSectorSearchOption::~CSectorSearchOption()
	{

	}
}

namespace FePlots
{
	CSectorSearch::CSectorSearch(FeUtil::CRenderContext *pRender, CSectorSearchOption *opt)
		: CLinePlot(pRender, opt)
	{
		CleanCrtlArray();
	}

	CSectorSearch::~CSectorSearch()
	{

	}

	void CSectorSearch::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
	}

	void CSectorSearch::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		CLinePlot::DraggerUpdateMatrix(matrix);

		SetVertex(m_rpCtrlArray);
	}

	bool CSectorSearch::PushBackVertex( const osg::Vec3d& vecCoord )
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

	void CSectorSearch::ReplaceBackVertex( const osg::Vec3d& vecCoord )
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

	osg::Vec3dArray* CSectorSearch::CalculateArrow( osg::Vec3dArray* vecControlPoints )
	{
		//两个控制点时，绘制直线
		if(vecControlPoints->size()>1)
		{
			osg::ref_ptr<osg::Vec3dArray> multiLines=new osg::Vec3dArray();
			//第一个点为起点，也是中心点
			osg::Vec3d centerPoint=vecControlPoints->at(0);
			double offsetX=2*centerPoint.x();
			double offsetY=2*centerPoint.y();
			//第二个点确定半径和起始方向，且为第一个扇形(Fisrst)的点
			osg::Vec3d point_FB=vecControlPoints->at(vecControlPoints->size()-1);
			double radius=FeUtil::CalculateDistance(centerPoint,point_FB);
			osg::Vec3d vector_S=FeUtil::ToVector(centerPoint,point_FB);
			//起始方向向右120°为第二个方向，确定第一个扇形的点
			osg::ref_ptr<osg::Vec3dArray> vectors=FeUtil::CalculateVector(vector_S,4*osg::PI/3,radius);
			osg::Vec3d vector_FR=vectors->at(0);
			osg::Vec3d point_FC=osg::Vec3d(vector_FR.x()+centerPoint.x(),vector_FR.y()+centerPoint.y(),0.0);

			//第二个(second)扇形
			osg::Vec3d point_SB=osg::Vec3d(-point_FC.x()+offsetX,-point_FC.y()+offsetY,0.0);
			osg::Vec3d vector_SL=vectors->at(1);
			osg::Vec3d point_SC=osg::Vec3d(vector_SL.x()+centerPoint.x(),vector_SL.y()+centerPoint.y(),0.0);

			//第三个(Third)扇形
			osg::Vec3d point_TB=osg::Vec3d(-point_SC.x()+offsetX,-point_SC.y()+offsetY,0.0);
			osg::Vec3d point_TC=osg::Vec3d(-point_FB.x()+offsetX,-point_FB.y()+offsetY,0.0);

			//连接点成扇形搜寻符号   
			multiLines->push_back(centerPoint);
			multiLines->push_back(point_FB);
			multiLines->push_back(point_FC);
			multiLines->push_back(centerPoint);
			multiLines->push_back(point_SB);
			multiLines->push_back(point_SC);
			multiLines->push_back(centerPoint);
			multiLines->push_back(point_TB);
			multiLines->push_back(point_TC);
			multiLines->push_back(centerPoint);

			return multiLines.release();

		}
		return nullptr;
	}

}