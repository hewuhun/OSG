#include <FePlots/LinePlot.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/CoordConverter.h>

const osg::Vec4 g_editPtColor(0.6, 1.0, 0.5, 1);

namespace FePlots
{
	CLinePlotOption::CLinePlotOption( void )
		: FeExtNode::CExLineNodeOption()
	{
	}

	CLinePlotOption::~CLinePlotOption( void )
	{

	}
}

namespace FePlots
{
	CLinePlot::CLinePlot(FeUtil::CRenderContext *pRender, CLinePlotOption *opt)
		: FeExtNode::CExLineNode(pRender, opt)
		, m_pOpt(opt)
		, m_bMouseMove(true)
	{
		m_rpCtrlArray = new osg::Vec3dArray;
	}

	CLinePlot::~CLinePlot(void)
	{
	}

	void CLinePlot::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
	}

	osg::Group* CLinePlot::CreateEditPoints()
	{
		osg::Group* rootNode = new osg::Group;

		if(m_rpCtrlArray.valid())
		{
			for (unsigned int i = 0; i < m_rpCtrlArray->size(); i++)
			{
				FeNodeEditor::CFeEditPoint* editPoint = new FeNodeEditor::CFeEditPoint(m_opMapNode.get(), this, i);
				if(!editPoint) continue;

				editPoint->SetPosition(GeoPoint(m_rpFeature->getSRS()
					, m_rpCtrlArray->at(i).x()
					, m_rpCtrlArray->at(i).y()
					, m_rpCtrlArray->at(i).z() +5
					, osgEarth::ALTMODE_ABSOLUTE));

				editPoint->SetColor(g_editPtColor);

				rootNode->addChild(editPoint);   
			}
		}

		return rootNode;
	}

	osgEarth::GeoPoint CLinePlot::GetEditPointPosition()
	{
		if(m_rpCtrlArray.valid())
		{
			if(m_rpCtrlArray.valid() && m_nCurEditPoint < m_rpCtrlArray->size())
			{
				return osgEarth::GeoPoint(m_opMapNode->getMapSRS(), m_rpCtrlArray->at(m_nCurEditPoint));
			}
		}
		return osgEarth::GeoPoint();
	}

	void CLinePlot::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		osg::Matrix matrixNew;
		m_pointStartDrag.createLocalToWorld(matrixNew);
		matrixNew = matrixNew * matrix;

		osgEarth::GeoPoint pt;
		pt.fromWorld(m_pointStartDrag.getSRS(), matrixNew.getTrans());
		FeUtil::DegreeLL2LLH(m_opRenderContext.get(), pt.vec3d());

		m_rpCtrlArray->at(m_nCurEditPoint) = pt.vec3d();

		FeNodeEditor::CFeEditPoint* point = GetEditPoint(m_nCurEditPoint);
		if(point)
		{
			pt.z() += 5;
			point->SetPosition(pt);
		}
	}

	void CLinePlot::SetVertex( osg::Vec3dArray* pCoord )
	{
		if (m_rpCtrlArray.valid())
		{
			m_rpCtrlArray = pCoord;
			osg::ref_ptr<osg::Vec3dArray> vertexArray = CalculateArrow(m_rpCtrlArray);
			FeExtNode::CExFeatureNode::SetVertex(vertexArray);
		}
	}

	void CLinePlot::GetVertex( osg::Vec3dArray* pCoord )
	{
		if(m_rpCtrlArray.valid() && pCoord)
		{
			pCoord->insert(pCoord->begin(), m_rpCtrlArray->begin(), m_rpCtrlArray->end());
		}
	}

	bool CLinePlot::DeleteVertex( int nIndex )
	{
		if(!m_rpCtrlArray.valid())
		{
			return false;
		}
		else if(nIndex >= m_rpCtrlArray->size() || nIndex < 0)
		{
			return false;
		}

		m_rpCtrlArray->erase(m_rpCtrlArray->begin()+nIndex);

		RebuildEditPoints();
		return true;
	}

	int CLinePlot::GetNumVertex()
	{
		if(m_rpCtrlArray.valid())
		{
			return m_rpCtrlArray->size();
		}
		return 0;
	} 

	void CLinePlot::CleanCrtlArray()
	{
		if (0 == m_rpCtrlArray->size())
		{
			return;
		}

		m_rpCtrlArray->clear();
	}

	void CLinePlot::SetLineColor( osg::Vec4d lineColor )
	{
		m_pOpt->lineColor() = lineColor;
	}

	void CLinePlot::SetLineWidth( int lineWidth )
	{
		m_pOpt->lineWidth() = lineWidth;
	}

	void CLinePlot::SetLineStipple( unsigned int lineStipple )
	{
		m_pOpt->lineStipple() = lineStipple;
	}

}
