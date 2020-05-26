#include <FeUtils/RenderContext.h>
#include <FeUtils/CoordConverter.h>

#include <FeExtNode/ExGridNode.h>


namespace FeExtNode
{
	CExGridNodeOption::CExGridNodeOption()
	{
		m_nRow = m_nCol = 2;
		m_vecRegion.x() = m_vecRegion.z() = 0;
		m_vecRegion.y() = m_vecRegion.w() = 1;

		m_vecLineColor = osg::Vec4d(1.0, 1.0, 0.0, 1.0);
		m_nLineWidth = 1;
		m_unLineStipple = 0xFFFFFFFF;
	}

	osg::Vec4d& CExGridNodeOption::lineColor()
	{
		return m_vecLineColor;
	}

	int& CExGridNodeOption::lineWidth()
	{
		return m_nLineWidth;
	}

	unsigned int& CExGridNodeOption::lineStipple()
	{
		return m_unLineStipple;
	}

	osg::Vec4d& CExGridNodeOption::GridRegion()
	{
		return m_vecRegion;
	}

	int& CExGridNodeOption::Row()
	{
		return m_nRow;
	}

	int& CExGridNodeOption::Col()
	{
		return m_nCol;
	}

}

namespace FeExtNode
{
	CExGridNode::CExGridNode( FeUtil::CRenderContext* pRender, CExGridNodeOption* opt ) : CExLodNode(opt)
	{
		m_opRenderContext = pRender;

		if(m_opRenderContext.valid())
		{
			m_opMapNode = m_opRenderContext->GetMapNode();
		}

		InitFeatureNode();
		RebuildGrid();
	}


	void CExGridNode::SetLineColor( const osg::Vec4d& vecLineColor )
	{
		if (GetLineColor() != vecLineColor)
		{
			GetOption()->lineColor() = vecLineColor;

			osgEarth::Style style = m_rpFeatureNode->getStyle();
			style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->color() = 
				osgEarth::Symbology::Color(vecLineColor);

			m_rpFeatureNode->setStyle(style);
			m_rpFeatureNode->init();
		}
	}

	osg::Vec4d CExGridNode::GetLineColor()
	{
		return GetOption()->lineColor();
	}

	void CExGridNode::SetLineWidth( const int& nLineWidth )
	{
		if (GetLineWidth() != nLineWidth)
		{
			GetOption()->lineWidth() = nLineWidth;

			osgEarth::Style style = m_rpFeatureNode->getStyle();
			style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->width() = nLineWidth;

			m_rpFeatureNode->setStyle(style);
			m_rpFeatureNode->init();
		}
	}

	int CExGridNode::GetLineWidth()
	{
		return GetOption()->lineWidth();
	}

	void CExGridNode::SetLineStipple( const unsigned int& unLineStipple )
	{
		if (GetLineStipple() != unLineStipple)
		{
			GetOption()->lineStipple() = unLineStipple;

			osgEarth::Style style = m_rpFeatureNode->getStyle();
			style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->stipple() = unLineStipple;

			m_rpFeatureNode->setStyle(style);
			m_rpFeatureNode->init();
		}
	}

	unsigned int CExGridNode::GetLineStipple()
	{
		return GetOption()->lineStipple();
	}

	void CExGridNode::SetGridRegion( const osg::Vec4& vecRegion )
	{
		if (GetGridRegion() != vecRegion)
		{
			GetOption()->GridRegion() = vecRegion;
			RebuildGrid();
		}
	}

	osg::Vec4 CExGridNode::GetGridRegion()
	{
		return GetOption()->GridRegion();
	}

	void CExGridNode::SetGridRowAndCol( int nRow, int Col )
	{
		if (GetGridRow() != nRow || GetGridCol() != Col)
		{
			GetOption()->Row() = nRow;
			GetOption()->Col() = Col;
			RebuildGrid();
		}
	}

	int CExGridNode::GetGridRow()
	{
		return GetOption()->Row();
	}

	int CExGridNode::GetGridCol()
	{
		return GetOption()->Col();
	}

	bool CExGridNode::InitFeatureNode()
	{
		if(m_rpFeatureNode.valid())
		{
			/// 已存在则先移除，防止多次调用生成多个节点
			this->removeChild(m_rpFeatureNode);
		}

		m_rpFeature = GetFeature();
		osgEarth::Symbology::Style style;
		osgEarth::Features::GeometryCompilerOptions options;
		options.maxGranularity() = 180.0;
		m_rpFeatureNode = new osgEarth::Annotation::FeatureNode(m_opMapNode.get(), m_rpFeature, style, options);

		if(m_rpFeatureNode)
		{
			m_rpFeatureNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
			m_rpFeatureNode->setStyle(GetFeatureStyle());
			m_rpFeatureNode->setDynamic(true);
			addChild(m_rpFeatureNode.get());
			return true;
		}

		return false;
	}

	osgEarth::Features::Feature* CExGridNode::GetFeature()
	{
		return new osgEarth::Features::Feature(new osgEarth::Symbology::MultiGeometry(), 
			m_opMapNode->getMapSRS());
	}

	osgEarth::Style CExGridNode::GetFeatureStyle()
	{
		osgEarth::Style style;
		style.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->clamping()
			= osgEarth::Symbology::AltitudeSymbol::CLAMP_TO_TERRAIN;
		style.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->technique()
			= osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_GPU;
		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->tessellation() = 10;
		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->color() = osgEarth::Symbology::Color(GetLineColor());
		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->width() = GetLineWidth();
		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->stipple() = GetLineStipple();

		return style;
	}

	CExGridNodeOption* CExGridNode::GetOption()
	{
		return dynamic_cast<CExGridNodeOption*>(m_rpOptions.get());
	}

	void CExGridNode::RebuildGrid()
	{
		if(m_rpFeature.valid() && m_rpFeatureNode.valid())
		{
			double dBeginLon = GetGridRegion().x(), dBeginLat = GetGridRegion().z(), 
				dEndLon = GetGridRegion().y(), dEndLat = GetGridRegion().w();
			int nLonNum = GetGridCol(), nLatNum = GetGridRow();
			double dDeltaLon = (dEndLon-dBeginLon)/(double)(nLonNum);
			double dDeltaLat = (dEndLat-dBeginLat)/(double)(nLatNum);

			osgEarth::Symbology::MultiGeometry* pMultiGeom =  dynamic_cast<osgEarth::Symbology::MultiGeometry*>(
				m_rpFeature->getGeometry());

			if(!pMultiGeom)
			{
				return;
			}

			pMultiGeom->getComponents().clear();

			//沿经线绘制
			for (int i = 0; i <nLonNum; i++)
			{
				osg::ref_ptr<osgEarth::Symbology::LineString> rpLine = new osgEarth::Symbology::LineString();
				for (int j = 0; j < nLatNum; j++)
				{
					rpLine->push_back(osg::Vec3d(dBeginLon+i*dDeltaLon, dBeginLat+j*dDeltaLat, 0));
				}

				pMultiGeom->getComponents().push_back(rpLine);
			}

			//沿纬线绘制
			for (int i = 0; i <nLatNum; i++)
			{
				osg::ref_ptr<osgEarth::Symbology::LineString> rpLine = new osgEarth::Symbology::LineString();
				for (int j = 0; j < nLonNum; j++)
				{
					rpLine->push_back(osg::Vec3d(dBeginLon+j*dDeltaLon, dBeginLat+i*dDeltaLat, 0));
				}

				pMultiGeom->getComponents().push_back(rpLine);
			}

			m_rpFeatureNode->init();
		}
	}

}

