#include <FeExtNode/ExTextNode.h>
#include <FeExtNode/ExPointNodeEditor.h>
#include <FeUtils/CoordConverter.h>

namespace FeExtNode
{
	CExTextNodeOption::CExTextNodeOption()
		:CExPointNodeOption()
	{
		m_dTextSize = 15;
		m_vecTextColor = Color::White;
	}

	CExTextNodeOption::~CExTextNodeOption()
	{

	}
}

namespace FeExtNode
{
	CExTextNode::CExTextNode( FeUtil::CRenderContext* pRender, CExTextNodeOption* opt )
		:CExPointNode(pRender, opt)
		,m_rpLabelNode(NULL)
	{
		if (m_opMapNode.valid())
		{
			//初始化文本的风格，使用UTF8，字体大小，颜色等
			osg::ref_ptr<osgEarth::Annotation::TextSymbol> rpTextSymbol = new osgEarth::Annotation::TextSymbol;
			rpTextSymbol->encoding() = TextSymbol::ENCODING_UTF8;
			rpTextSymbol->alignment() = TextSymbol::ALIGN_CENTER_CENTER;
			rpTextSymbol->font() = "simhei.ttf";
			rpTextSymbol->size() = GetOption()->TextSize();
			rpTextSymbol->fill()->color() = osgEarth::Symbology::Color(GetOption()->TextColor());

			osgEarth::Annotation::Style style;
			style.addSymbol(rpTextSymbol);
			m_rpLabelNode = new osgEarth::Annotation::LabelNode(m_opMapNode.get(), GetOption()->geoPoint(), style);
			m_rpLabelNode->setText(GetName());
			m_rpLabelNode->setDynamic(true);

			addChild(m_rpLabelNode);
		}
	}

	CExTextNode::~CExTextNode()
	{

	}

	void CExTextNode::SetName( const std::string& strName )
	{
		if(GetOption()->name() != strName)
		{
			GetOption()->name() = strName;

			if (m_rpLabelNode.valid())
			{
				m_rpLabelNode->setText(strName);
			}
		}
	}

	void CExTextNode::SetPosition( const osgEarth::GeoPoint& geoPosition )
	{
		if(GetOption()->geoPoint() != geoPosition)
		{
			GetOption()->geoPoint() = geoPosition;
			FeUtil::DegreeLL2LLH(m_opRenderContext.get(), GetOption()->geoPoint().vec3d());

			if(m_rpLabelNode.valid())
			{
				m_rpLabelNode->setPosition(GetOption()->geoPoint());
				UpdateEditor();
			}
		}
	}

	osgEarth::GeoPoint CExTextNode::GetPosition()
	{
		if(m_rpLabelNode.valid())
		{
			GetOption()->geoPoint() = m_rpLabelNode->getPosition();
		}
		
		return GetOption()->geoPoint();
	}

	void CExTextNode::SetTextSymbol( TextSymbol& symbolText )
	{
		if (m_rpLabelNode.valid())
		{
			osg::ref_ptr<osgEarth::Annotation::TextSymbol> rpTextSymbol = 
				new osgEarth::Annotation::TextSymbol(symbolText);

			osgEarth::Style style = m_rpLabelNode->getStyle();
			style.addSymbol(rpTextSymbol);
			m_rpLabelNode->setStyle(style);

			GetOption()->TextColor() = rpTextSymbol->fill()->color();
			GetOption()->TextSize() = rpTextSymbol->size().value().eval();
		}
	}

	osgEarth::Symbology::TextSymbol CExTextNode::GetTextSymbol()
	{
		if (m_rpLabelNode.valid())
		{
			osgEarth::Style style = m_rpLabelNode->getStyle();
			TextSymbol* pTextSymbol = style.getOrCreateSymbol<TextSymbol>();

			return *pTextSymbol;
		}

		TextSymbol textSymbol;
		return textSymbol;
	}

	void CExTextNode::SetTextColor( const osg::Vec4f& color )
	{
		osgEarth::Symbology::TextSymbol ts = GetTextSymbol();
		ts.fill()->color() = color;
		SetTextSymbol(ts);
	}

	osg::Vec4f CExTextNode::GetTextColor()
	{
		return GetTextSymbol().fill()->color();
	}

	void CExTextNode::SetTextSize( const double& size )
	{
		osgEarth::Symbology::TextSymbol ts = GetTextSymbol();
		ts.size() = size;
		SetTextSymbol(ts);
	}

	double CExTextNode::GetTextSize()
	{
		return GetTextSymbol().size().value().eval();
	}

	CExTextNodeOption* CExTextNode::GetOption()
	{
		return dynamic_cast<CExTextNodeOption*>(m_rpOptions.get());
	}

	osg::BoundingSphere CExTextNode::computeBound() const
	{
		osg::BoundingSphere bs = CExPointNode::computeBound();
		if(!_boundingSphereComputed)
		{                
			_boundingSphere = _initialBound;           

			osgEarth::GeoPoint geoPoint;
			if (m_rpLabelNode.valid())
			{
				geoPoint = m_rpLabelNode->getPosition();
			}

			osg::Vec3d vecPos = osg::Vec3d(0.0, 0.0, 0.0);
			geoPoint.toWorld(vecPos);
			_boundingSphere.center() = vecPos;
			_boundingSphere.radius() = bs.radius();

			_boundingSphereComputed = true;            
		}            
		return _boundingSphere;
	}

	void CExTextNode::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}

}