#include <osgEarthAnnotation/ImageOverlay>
#include <osgEarthAnnotation/ImageOverlayEditor>
#include <FeExtNode/ExOverLayNode.h>
#include <osgEarth/GeoMath>

namespace FeExtNode
{
	CExOverLayNodeOption::CExOverLayNodeOption()
		:CExPointNodeOption()
	{
		m_vecSize = osg::Vec2(100, 100);
	}

	CExOverLayNodeOption::~CExOverLayNodeOption()
	{

	}
}

namespace FeExtNode
{
	CExOverLayNode::CExOverLayNode( FeUtil::CRenderContext* pRender, CExOverLayNodeOption* opt )
		:CExPointNode(pRender, opt)
		,m_rpImageOverlay(NULL)
	{
		if (m_opMapNode.valid())
		{
			m_rpImageOverlay = new osgEarth::Annotation::ImageOverlay(m_opMapNode.get());
			m_rpImageOverlay->setDynamic(true);
			addChild(m_rpImageOverlay);

			if(!GetImagePath().empty())
			{
				UpdateImage();
			}

			if(GetPosition().isValid())
			{
				UpdatePosition();
				UpdateSize();
			}
		}
	}

	CExOverLayNode::~CExOverLayNode()
	{

	}

	void CExOverLayNode::SetName( const std::string& strName )
	{
		if(GetOption()->name() != strName)
		{
			GetOption()->name() = strName;

			m_rpImageOverlay->setName(strName);
		}
	}

	std::string CExOverLayNode::GetImagePath()
	{
		return GetOption()->imagePath();
	}

	void CExOverLayNode::SetImagePath( const std::string& strImage )
	{
		if ( strImage != GetOption()->imagePath())
		{
			GetOption()->imagePath() = strImage;

			UpdateImage();
		}
	}

	void CExOverLayNode::UpdatePosition()
	{
		osg::Vec3d vecPosition = GetOption()->geoPoint().vec3d();
		m_rpImageOverlay->setCenter(vecPosition.x(), vecPosition.y());

		UpdateSize();
	}

	void CExOverLayNode::UpdateImage()
	{
		m_rpImageNode = osgDB::readImageFile(GetImagePath());

		if (m_rpImageNode.valid() && m_rpImageOverlay.valid())
		{
			m_rpImageOverlay->setImage(m_rpImageNode.get());
		}
	}

	void CExOverLayNode::UpdateSize()
	{
		// 计算覆盖区域经纬坐标
		osg::Vec3d lowerCoordLLH, uperCoordLLH, vecLLH;
		osg::Vec3d vecPosition = GetOption()->geoPoint().vec3d();

#define L_D2R(a) ((a) = osg::DegreesToRadians(a))
#define L_R2D(a) ((a) = osg::RadiansToDegrees(a))

		L_D2R(vecPosition.x());
		L_D2R(vecPosition.y());
		osgEarth::GeoMath::destination( vecPosition.y(), vecPosition.x(), 
			-osg::PI_2,   
			GetOption()->imageSize().x()/2,     
			vecLLH.y(), vecLLH.x()
			);
		lowerCoordLLH.x() = vecLLH.x();
		uperCoordLLH.x() = vecPosition.x() + (vecPosition.x()-vecLLH.x());

		osgEarth::GeoMath::destination( vecPosition.y(), vecPosition.x(), 
			osg::PI,   
			GetOption()->imageSize().y()/2,     
			vecLLH.y(), vecLLH.x()
			);
		lowerCoordLLH.y() = vecLLH.y();
		uperCoordLLH.y() = vecPosition.y() + (vecPosition.y()-vecLLH.y());

		L_R2D(lowerCoordLLH.x());
		L_R2D(lowerCoordLLH.y());
		L_R2D(uperCoordLLH.x());
		L_R2D(uperCoordLLH.y());

		m_rpImageOverlay->setBounds(osgEarth::Bounds(
			lowerCoordLLH.x(), lowerCoordLLH.y(), uperCoordLLH.x(), uperCoordLLH.y()));
	}

	osg::Vec2 CExOverLayNode::GetImageSize()
	{
		return GetOption()->imageSize();
	}

	void CExOverLayNode::SetImageSize( const osg::Vec2& vecSize )
	{
		if (GetOption()->imageSize() != vecSize)
		{
			GetOption()->imageSize() = vecSize;

			UpdateSize();
		}
	}

	void CExOverLayNode::SetPosition( const osgEarth::GeoPoint& geoPosition )
	{
		if(GetOption()->geoPoint() != geoPosition)
		{
			GetOption()->geoPoint() = geoPosition;

			UpdatePosition();
			UpdateEditor();
		}
	}

	osgEarth::GeoPoint CExOverLayNode::GetPosition()
	{
		if(m_rpImageOverlay.valid())
		{
			osg::Vec2d vecCenter = m_rpImageOverlay->getCenter();
			GetOption()->geoPoint().x() = vecCenter.x();
			GetOption()->geoPoint().y() = vecCenter.y();
		}
		
		return GetOption()->geoPoint();
	}

	CExOverLayNodeOption* CExOverLayNode::GetOption()
	{
		return dynamic_cast<CExOverLayNodeOption*>(m_rpOptions.get());
	}

	void CExOverLayNode::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}
}

