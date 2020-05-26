#include <FePlots/ParallelSearch.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>

namespace FePlots
{
	CParallelSearchOption::CParallelSearchOption()
	{

	}

	CParallelSearchOption::~CParallelSearchOption()
	{

	}
}

namespace FePlots
{
	CParallelSearch::CParallelSearch(FeUtil::CRenderContext *pRender, CParallelSearchOption *opt)
		: CLinePlot(pRender, opt)
	{
		CleanCrtlArray();
	}

	CParallelSearch::~CParallelSearch()
	{

	}

	void CParallelSearch::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
	}

	void CParallelSearch::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		CLinePlot::DraggerUpdateMatrix(matrix);

		if (m_rpCtrlArray->size() > 2)
		{
			SetVertex(m_rpCtrlArray);
		}
	}

	bool CParallelSearch::PushBackVertex( const osg::Vec3d& vecCoord )
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

	void CParallelSearch::ReplaceBackVertex( const osg::Vec3d& vecCoord )
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

	osg::Vec3dArray* CParallelSearch::CalculateArrow( osg::Vec3dArray* vecControlPoints )
	{

		if(vecControlPoints->size()>2)
		{

			osg::Vec3d firstP=vecControlPoints->at(0);
			osg::Vec3d secondP=vecControlPoints->at(1);
			//第一、二个点的向量为基准向量
			osg::Vec3d vectorBase=FeUtil::ToVector(firstP,secondP);
			//基准向量的法向量
			osg::Vec3d vectorNormal=FeUtil::CalculateVector(vectorBase)->at(0);
			//从第三个点开始，当i为奇数，则第i-1、i个点的向量垂直于基准向量，当i为偶数，则第i-1、i个点的向量平行于垂直基准向量。
			bool isParalel=false;
			osg::ref_ptr<osg::Vec3dArray> rpPoints = new osg::Vec3dArray();
			rpPoints->push_back(firstP);
			osg::ref_ptr<osg::Vec3dArray> multiLines= new osg::Vec3dArray();
			multiLines->push_back(firstP);
			for(int i=1;i<vecControlPoints->size();i++)
			{
				//判断是否平行
				isParalel=(i%2!=0);
				osg::Vec3d pointI=vecControlPoints->at(i);
				//平行
				if(isParalel)
				{
					osg::Vec3d previousP=rpPoints->at(i-1);
					osg::Vec3d point=FeUtil::CalculateIntersection(vectorNormal,vectorBase,pointI,previousP);
					rpPoints->push_back(point);
					osg::ref_ptr<osg::Vec3dArray> arrowLines=FeUtil::CalculateArrowLines(previousP,point,15);
					multiLines->push_back(point);
					for(int i=0;i<arrowLines->size();i++)
					{
						multiLines->push_back(arrowLines->at(i));
					}
				}
				//垂直
				else{

					osg::Vec3d previousP=rpPoints->at(i-1);
					osg::Vec3d point=FeUtil::CalculateIntersection(vectorBase,vectorNormal,pointI,previousP);
					rpPoints->push_back(point);
					osg::ref_ptr<osg::Vec3dArray> arrowLines=FeUtil::CalculateArrowLines(previousP,point,15);
					multiLines->push_back(point);
					for(int i=0;i<arrowLines->size();i++)
					{
						multiLines->push_back(arrowLines->at(i));

					}
				}
			}
			return multiLines.release();
		}
		return nullptr;
	}

}

