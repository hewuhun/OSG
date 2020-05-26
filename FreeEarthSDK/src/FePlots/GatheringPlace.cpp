#include <FePlots/GatheringPlace.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>


namespace FePlots
{
	CGatheringPlaceOption::CGatheringPlaceOption( void )
	{

	}

	CGatheringPlaceOption::~CGatheringPlaceOption( void )
	{

	}

}

namespace FePlots
{
	CGatheringPlace::CGatheringPlace(FeUtil::CRenderContext *pRender, CGatheringPlaceOption *opt)
		: CPolygonPlot(pRender, opt)
	{
		m_bMouseMove = true;
		CleanCrtlArray();
	}

	CGatheringPlace::~CGatheringPlace(void)
	{
	}

	void CGatheringPlace::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
	}

	void CGatheringPlace::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		CPolygonPlot::DraggerUpdateMatrix(matrix);

		SetVertex(m_rpCtrlArray);
	}

	bool CGatheringPlace::PushBackVertex( const osg::Vec3d& vecCoord )
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

	void CGatheringPlace::ReplaceBackVertex( const osg::Vec3d& vecCoord )
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

	osg::Vec3dArray* CGatheringPlace::CalculateArrow( osg::Vec3dArray* vecControlPoints)
	{
		osg::ref_ptr<osg::Vec3dArray> components = new osg::Vec3dArray;
		//至少需要两个控制点
		if(vecControlPoints->size()>1)
		{
			//取第一个点作为第一控制点
			osg::Vec3d originP = vecControlPoints->at(0);
			//取最后一个作为第二控制点
			osg::Vec3d lastP = vecControlPoints->at(vecControlPoints->size()-1);

			osg::ref_ptr<osg::Vec3dArray> points = new osg::Vec3dArray;
			// 向量originP_lastP
			osg::Vec3d vectorOL = osg::Vec3d(lastP.x()-originP.x(),lastP.y()-originP.y(),0.0);
			// 向量originP_lastP的模
			double dOL = std::sqrt(vectorOL.x()*vectorOL.x() + vectorOL.y()*vectorOL.y());

			//计算第一个插值控制点
			//向量originP_P1以originP为起点，与向量originP_lastP的夹角设为60°，模为√3/12*dOL，
			osg::ref_ptr<osg::Vec3dArray> v_O_P1_lr=FeUtil::CalculateVector(vectorOL,osg::PI/3,std::sqrt(3.0)/12*dOL);
			//取左边的向量作为向量originP_P1
			osg::Vec3d originP_P1=v_O_P1_lr->at(0);
			osg::Vec3d p1=osg::Vec3d(originP_P1.x()+originP.x(),originP_P1.y()+originP.y(),0.0);

			//计算第二个插值控制点，取第一控制点和第二控制点的中点为第二个插值控制点
			osg::Vec3d p2=osg::Vec3d((originP.x()+lastP.x())/2,(originP.y()+lastP.y())/2,0.0);

			//计算第三个插值控制点
			//向量originP_P3以lastP为起点，与向量originP_lastP的夹角设为120°，模为√3/12*dOL，
			osg::ref_ptr<osg::Vec3dArray> v_L_P3_lr=FeUtil::CalculateVector(vectorOL,osg::PI*2/3,std::sqrt(3.0)/12*dOL);
			//取左边的向量作为向量originP_P1
			osg::Vec3d lastP_P3=v_L_P3_lr->at(0);
			osg::Vec3d p3=osg::Vec3d(lastP_P3.x()+lastP.x(),lastP_P3.y()+lastP.y(),0.0);

			//计算第四个插值控制点
			//向量originP_P4以向量originP_lastP中点为起点，与向量originP_lastP的夹角设为90°，模为1/2*dOL，
			osg::ref_ptr<osg::Vec3dArray> v_O_P5_lr=FeUtil::CalculateVector(vectorOL,osg::PI_2,0.5*dOL);
			//取左边的向量作为向量originP_P1
			osg::Vec3d v_O_P5=v_O_P5_lr->at(1);
			osg::Vec3d p5=osg::Vec3d(v_O_P5.x()+p2.x(),v_O_P5.y()+p2.y(),0.0);

			osg::Vec3d p0=originP;
			osg::Vec3d p4=lastP;
			points->push_back(p0);
			points->push_back(p1);
			points->push_back(p2);
			points->push_back(p3);
			points->push_back(p4);
			points->push_back(p5);
			osg::ref_ptr<osg::Vec3dArray> cardinalPoints=FeUtil::CreateCloseCardinal(points);
			osg::ref_ptr<osg::Vec3dArray> rpPoints = new osg::Vec3dArray;
			for (int i=0;i<cardinalPoints->size();++i)
			{
				if(cardinalPoints->at(i).x()==0 && cardinalPoints->at(i).y()==0)
				{
					continue;
				}
				else
				{
					rpPoints->push_back(cardinalPoints->at(i));
				}
			}
			osg::ref_ptr<osg::Vec3dArray> cardinalCurve3 = FeUtil::CreateBezier3(rpPoints,100);
			components->insert(components->end(),cardinalCurve3->begin(),cardinalCurve3->end());
			return components.release();
		}
	}

}