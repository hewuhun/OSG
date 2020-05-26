#include <osg/LineWidth>
#include <osgViewer/Viewer>
#include <ExternNodeEditing.h>
#include <FeExtNode/ExPlaceNode.h>
#include <FeUtils/CoordConverter.h>

namespace FeExtNode
{
	CExPlaceNodeOption::CExPlaceNodeOption()
		:CExPointNodeOption()
	{
		m_vecSize = osg::Vec2(20, 20);
		m_dTextSize = 15;
		m_vecTextColor = Color::White;
	}

	CExPlaceNodeOption::~CExPlaceNodeOption()
	{

	}

}

namespace FeExtNode
{
	CExPlaceNode::CExPlaceNode( FeUtil::CRenderContext* pRender, CExPlaceNodeOption* opt )
		:CExPointNode(pRender, opt)
		,m_rpPlaceNode(NULL)
	{
		if (m_opMapNode.valid())
		{
			//初始化文本的风格，使用UTF8，字体大小，颜色等
			osg::ref_ptr<osgEarth::Annotation::TextSymbol> rpTextSymbol = new osgEarth::Annotation::TextSymbol;
			rpTextSymbol->encoding() = TextSymbol::ENCODING_UTF8;
			rpTextSymbol->alignment() = TextSymbol::ALIGN_LEFT_CENTER;
			rpTextSymbol->font() = "simhei.ttf";
			rpTextSymbol->size() = GetOption()->TextSize();
			rpTextSymbol->fill()->color() = osgEarth::Symbology::Color(GetOption()->TextColor());

			osg::ref_ptr<osg::Image> rpImage = osgDB::readImageFile(GetImagePath());
			osg::Vec2 vecSize = osg::Vec2(40, 40);
			SetImageSize(vecSize);
			if (rpImage.valid())
			{
				rpImage->scaleImage(GetOption()->imageSize().x(), GetOption()->imageSize().y(), 1.0);
			}

			osgEarth::Annotation::Style style;
			style.addSymbol(rpTextSymbol);
			m_rpPlaceNode = new osgEarth::Annotation::PlaceNode(m_opMapNode.get(), GetOption()->geoPoint(), rpImage.get(), GetName(), style);
			m_rpPlaceNode->setDynamic(true);
			addChild(m_rpPlaceNode);
		}
	}

	CExPlaceNode::~CExPlaceNode()
	{

	}

	void CExPlaceNode::SetName( const std::string& strName )
	{
		if(GetOption()->name() == strName)
		{
			return ;
		}

		GetOption()->name() = strName;
		if (m_rpPlaceNode.valid())
		{
			m_rpPlaceNode->setText(strName);
		}
	}

	std::string CExPlaceNode::GetImagePath()
	{
		return GetOption()->imagePath();
	}

	void CExPlaceNode::SetImagePath( const std::string& strImage )
	{
		if ( strImage == GetOption()->imagePath())
		{
			return;
		}

		GetOption()->imagePath() = strImage;
		if (m_rpPlaceNode.valid())
		{
			osg::ref_ptr<osg::Image> rpImage = osgDB::readImageFile(strImage);
			if (rpImage.valid())
			{
				rpImage->scaleImage(GetOption()->imageSize().x(), GetOption()->imageSize().y(), 1.0);
			}

			m_rpPlaceNode->setIconImage(rpImage);

			UpdateEditor();
		}
	}

	osg::Vec2 CExPlaceNode::GetImageSize()
	{
		return GetOption()->imageSize();
	}

	void CExPlaceNode::SetImageSize( const osg::Vec2& vecSize )
	{
		if (GetOption()->imageSize() == vecSize)
		{
			return;
		}

		GetOption()->imageSize() = vecSize;
		if (m_rpPlaceNode.valid())
		{
			osg::ref_ptr<osg::Image> rpImage = m_rpPlaceNode->getIconImage();
			if (rpImage.valid())
			{
				rpImage->scaleImage(vecSize.x(), vecSize.y(), 1.0);
			}

			m_rpPlaceNode->setIconImage(rpImage);

			UpdateEditor();
		}
	}

	void CExPlaceNode::SetPosition( const osgEarth::GeoPoint& geoPosition )
	{
		GetOption()->geoPoint() = geoPosition;
		FeUtil::DegreeLL2LLH(m_opRenderContext.get(), GetOption()->geoPoint().vec3d());

		if(m_rpPlaceNode.valid())
		{
			m_rpPlaceNode->setPosition(GetOption()->geoPoint());
			dirtyBound();

			UpdateEditor();
		}
	}

	osgEarth::GeoPoint CExPlaceNode::GetPosition()
	{
		if(m_rpPlaceNode.valid())
		{
			GetOption()->geoPoint() = m_rpPlaceNode->getPosition();
		}
		
		return GetOption()->geoPoint();
	}

	void CExPlaceNode::SetTextSymbol( TextSymbol& symbolText )
	{
		if (m_rpPlaceNode.valid())
		{
			osg::ref_ptr<osgEarth::Annotation::TextSymbol> rpTextSymbol = 
				new osgEarth::Annotation::TextSymbol(symbolText);

			osgEarth::Style style = m_rpPlaceNode->getStyle();
			style.addSymbol(rpTextSymbol);

			m_rpPlaceNode->setStyle(style);

			GetOption()->TextColor() = rpTextSymbol->fill()->color();
			GetOption()->TextSize() = rpTextSymbol->size().value().eval();

			UpdateEditor();
		}
	}

	osgEarth::Symbology::TextSymbol CExPlaceNode::GetTextSymbol()
	{
		if (m_rpPlaceNode.valid())
		{
			osgEarth::Style style = m_rpPlaceNode->getStyle();
			TextSymbol* pTextSymbol = style.getOrCreateSymbol<TextSymbol>();

			return *pTextSymbol;
		}

		return TextSymbol();
	}

	void CExPlaceNode::SetTextColor( const osg::Vec4f& color )
	{
		osgEarth::Symbology::TextSymbol ts = GetTextSymbol();
		ts.fill()->color() = color;
		SetTextSymbol(ts);
	}

	osg::Vec4f CExPlaceNode::GetTextColor()
	{
		return GetTextSymbol().fill()->color();
	}

	void CExPlaceNode::SetTextSize( const double& size )
	{
		osgEarth::Symbology::TextSymbol ts = GetTextSymbol();
		ts.size() = size;
		SetTextSymbol(ts);
	}

	double CExPlaceNode::GetTextSize()
	{
		return GetTextSymbol().size().value().eval();
	}

	CExPlaceNodeOption* CExPlaceNode::GetOption()
	{
		return dynamic_cast<CExPlaceNodeOption*>(m_rpOptions.get());
	}

	void CExPlaceNode::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}

	osg::BoundingSphere CExPlaceNode::computeBound() const
	{
		osg::BoundingSphere bs = CExPointNode::computeBound();
		if(!_boundingSphereComputed)
		{                
			_boundingSphere = _initialBound;           

			osgEarth::GeoPoint geoPoint;
			if (m_rpPlaceNode.valid())
			{
				geoPoint = m_rpPlaceNode->getPosition();
			}

			osg::Vec3d vecPos = osg::Vec3d(0.0, 0.0, 0.0);
			geoPoint.toWorld(vecPos);
			_boundingSphere.center() = vecPos;
			_boundingSphere.radius() = bs.radius();

			_boundingSphereComputed = true;            
		}            
		return _boundingSphere;
	}

}


