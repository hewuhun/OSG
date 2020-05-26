#include <FePlots/PolygonPlot.h>
#include <FePlots/ExternPlotsVisitor.h>
#include <FeUtils/CoordConverter.h>

const osg::Vec4 g_editPtColor(0.6, 1.0, 0.5, 1);

namespace FePlots
{
	CPolygonPlotOption::CPolygonPlotOption( void )
	{
	}

	CPolygonPlotOption::~CPolygonPlotOption( void )
	{

	}
}

namespace FePlots
{
	CPolygonPlot::CPolygonPlot(FeUtil::CRenderContext *pRender, CPolygonPlotOption *opt)
		: FeExtNode::CExPolygonNode(pRender, opt)
		, m_bMouseMove(true)
	{
		m_rpCtrlArray = new osg::Vec3dArray;
	}

	CPolygonPlot::~CPolygonPlot(void)
	{

	}

	void CPolygonPlot::Accept( FeExtNode::CExternNodeVisitor& nv )
	{
		CExternPlotsVisitor& plotsVisitor = dynamic_cast<CExternPlotsVisitor&>(nv);
		plotsVisitor.VisitEnter(*this);
	}

	osg::Group* CPolygonPlot::CreateEditPoints()
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

	osgEarth::GeoPoint CPolygonPlot::GetEditPointPosition()
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

	void CPolygonPlot::DraggerUpdateMatrix( const osg::Matrix& matrix )
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

	void CPolygonPlot::SetVertex( osg::Vec3dArray* pCoord )
	{
		if (m_rpCtrlArray.valid())
		{
			m_rpCtrlArray = pCoord;
			osg::ref_ptr<osg::Vec3dArray> vertexArray = CalculateArrow(m_rpCtrlArray);
			FeExtNode::CExFeatureNode::SetVertex(vertexArray);
		}
	}

	void CPolygonPlot::GetVertex( osg::Vec3dArray* pCoord )
	{
		if(m_rpCtrlArray.valid() && pCoord)
		{
			pCoord->insert(pCoord->begin(), m_rpCtrlArray->begin(), m_rpCtrlArray->end());
		}
	}

	bool CPolygonPlot::DeleteVertex( int nIndex )
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

	int CPolygonPlot::GetNumVertex()
	{
		if(m_rpCtrlArray.valid())
		{
			return m_rpCtrlArray->size();
		}
		return 0;
	} 

	void CPolygonPlot::CleanCrtlArray()
	{
		if (0 == m_rpCtrlArray->size())
		{
			return;
		}

		m_rpCtrlArray->clear();
	}
}
