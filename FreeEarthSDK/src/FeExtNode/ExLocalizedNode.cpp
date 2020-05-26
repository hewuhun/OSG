
#include <osgEarthAnnotation/AnnotationEditing>
#include <FeUtils/RenderContext.h>
#include <FeUtils/CoordConverter.h>
#include <FeExtNode/ExLocalizedNode.h>
#include <ExternNodeEditing.h>



namespace FeExtNode
{
	CExLocalizedNodeOption::CExLocalizedNodeOption()
		:CExLodNodeOption()
	{
		m_vecLineColor = osg::Vec4d(1.0, 1.0, 0.0, 1.0);
		m_vecFillColor = osg::Vec4d(1.0, 0.0, 1.0, 1.0);
		m_nLineWidth = 1;
		m_unLineStipple = 0xFFFFFFFF;
		m_geoPoint.vec3d() = osg::Vec3d(0.0,0.0,0.0);
	}

	CExLocalizedNodeOption::~CExLocalizedNodeOption()
	{

	}

}

namespace FeExtNode
{
	CExLocalizedNode::CExLocalizedNode( FeUtil::CRenderContext* pRender, CExLocalizedNodeOption* opt )
		:CExLodNode(opt)
		,m_opRenderContext(pRender)
	{
		if(m_opRenderContext.valid())
		{
			m_opMapNode = m_opRenderContext->GetMapNode();
		}
	}

	CExLocalizedNode::~CExLocalizedNode()
	{
	}

	bool CExLocalizedNode::InitLocalizedNode()
	{
		if(m_rpLocalizedNode.valid())
		{
			/// 已存在则先移除，防止多次调用生成多个节点
			this->removeChild(m_rpLocalizedNode);
		}

		m_rpLocalizedNode = GetConcreteNode();

		if(m_rpLocalizedNode)
		{
			m_rpLocalizedNode->setStyle(GetConcreteNodeStyle());
			m_rpLocalizedNode->setPosition(GetOption()->geoPoint());
			m_rpLocalizedNode->setDynamic(true);
			addChild(m_rpLocalizedNode.get());

			return true;
		}
		return false;
	}

	osgEarth::Style CExLocalizedNode::GetConcreteNodeStyle()
	{
		osgEarth::Style style;

		style.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->clamping() 
			= osgEarth::Symbology::AltitudeSymbol::CLAMP_TO_TERRAIN;
		style.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->technique()
			=osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_DRAPE;

		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->color() = 
			osgEarth::Symbology::Color(GetLineColor());
		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->width() = 
			GetLineWidth();
		style.getOrCreate<osgEarth::Symbology::PolygonSymbol>()->fill()->color() = 
			osgEarth::Symbology::Color(GetFillColor());

		return style;
	}

	void CExLocalizedNode::SetLineColor( const osg::Vec4d& vecLineColor )
	{
		if (GetLineColor() != vecLineColor)
		{
			GetOption()->lineColor() = vecLineColor;

			osgEarth::Style style = m_rpLocalizedNode->getStyle();
			style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->color() = 
				osgEarth::Symbology::Color(vecLineColor);

			m_rpLocalizedNode->setStyle(style);
		}
	}

	osg::Vec4d CExLocalizedNode::GetLineColor()
	{
		return GetOption()->lineColor();
	}

	void CExLocalizedNode::SetFillColor( const osg::Vec4d& vecFillColor )
	{
		if (GetFillColor() != vecFillColor)
		{
			GetOption()->fillColor() = vecFillColor;

			osgEarth::Style style = m_rpLocalizedNode->getStyle();
			style.getOrCreate<osgEarth::Symbology::PolygonSymbol>()->fill()->color() = 
				osgEarth::Symbology::Color(vecFillColor);

			m_rpLocalizedNode->setStyle(style);
		}
	}

	osg::Vec4d& CExLocalizedNode::GetFillColor()
	{
		return GetOption()->fillColor();
	}

	void CExLocalizedNode::SetLineWidth( const int& nLineWidth )
	{
		if (GetLineWidth() != nLineWidth)
		{
			GetOption()->lineWidth() = nLineWidth;

			osgEarth::Style style = m_rpLocalizedNode->getStyle();
			style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->width() = nLineWidth;

			m_rpLocalizedNode->setStyle(style);
		}
	}

	int CExLocalizedNode::GetLineWidth()
	{
		return GetOption()->lineWidth();
	}

	void CExLocalizedNode::SetLineStipple( const unsigned int& unLineStipple )
	{
		if (GetLineStipple() != unLineStipple)
		{
			GetOption()->lineStipple() = unLineStipple;

			osgEarth::Style style = m_rpLocalizedNode->getStyle();
			style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->stipple() = unLineStipple;

			m_rpLocalizedNode->setStyle(style);
		}
	}

	unsigned int CExLocalizedNode::GetLineStipple()
	{
		return GetOption()->lineStipple();
	}

	void CExLocalizedNode::SetPosition( const osgEarth::GeoPoint& geoPosition )
	{
		if(m_rpLocalizedNode.valid())
		{
			m_rpLocalizedNode->setPosition(geoPosition);
			GetOption()->geoPoint() = geoPosition;
			UpdateEditor();
		}
	}

	osgEarth::GeoPoint CExLocalizedNode::GetPosition()
	{
		if(m_rpLocalizedNode)
		{
			GetOption()->geoPoint() = m_rpLocalizedNode->getPosition();
		}

		return GetOption()->geoPoint();
	}

	CExLocalizedNodeOption* CExLocalizedNode::GetOption()
	{
		return dynamic_cast<CExLocalizedNodeOption*>(m_rpOptions.get());
	}

	FeNodeEditor::CFeNodeEditTool* CExLocalizedNode::CreateEditorTool()
	{
		return new CExLocalizedNodeEditing(m_opRenderContext.get(), this);
	}

	void CExLocalizedNode::DraggerStart()
	{
		m_pointStartDrag = GetPosition();
	}

	void CExLocalizedNode::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		osg::Matrix matrixNew;
		m_pointStartDrag.createLocalToWorld(matrixNew);
		matrixNew = matrixNew * matrix;
		
		osgEarth::GeoPoint pt;
		pt.fromWorld(m_pointStartDrag.getSRS(), matrixNew.getTrans());
		//pt.z() = m_pointStartDrag.z();
		FeUtil::DegreeLL2LLH(m_opRenderContext.get(), pt.vec3d());
		
		SetPosition(pt);	
	}

	osg::Vec3d CExLocalizedNode::GetEditPointPositionXYZ()
	{
// 		osg::Vec3d vecXYZ;
// 		GetPosition().toWorld(vecXYZ);
// 		return vecXYZ;

		/// 折中解决方案，解决高程加载导致编辑位置到地形底下，进而导致编辑器被裁减
		osg::Vec3d llh, xyz;
		osgEarth::GeoPoint geoPt = GetPosition();
		FeUtil::DegreeLL2LLH(m_opRenderContext.get(), geoPt.vec3d());
		geoPt.toWorld(xyz);
		return xyz;
	}

}