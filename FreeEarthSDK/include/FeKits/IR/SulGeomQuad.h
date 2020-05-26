// SulGeomQuad.h

#ifndef __SULGEOMQUAD_H__
#define __SULGEOMQUAD_H__

#include <FeKits/Export.h>
#include <FeKits/IR/SulGeom.h>
#include <FeKits/IR/SulTypes.h>
#include <FeKits/IR/SulGeode.h>
#include <osg/ref_ptr>
#include <osg/Vec3>
#include <osg/Geode>
#include <osg/Geometry>

namespace FeKit
{

	class  CSulGeomQuad : public CSulGeom
	{
	public:
		enum EPLANE
		{
			PLANE_XY,
			PLANE_XZ,
			PLANE_YZ
		};

	public:
		CSulGeomQuad( float w=1.0f, float h=1.0f, EPLANE ePlane=PLANE_XY );
		CSulGeomQuad( const osg::Vec3& vCenter, float w=1.0f, float h=1.0f, EPLANE ePlane=PLANE_XY );
									
		void							create();
		void							Create( const osg::Vec3& vCenter, float w=1.0f, float h=1.0f, EPLANE ePlane=PLANE_XY );

		void							createUV();

		void							setColor( const osg::Vec4& c );
		void							setColor( float r, float g, float b, float a );
		const osg::Vec4&				getColor( sigma::uint32 index );

		void							setTexture( osg::Image* pImage, GLint internalFormat=GL_RGBA, sigma::uint32 unit=0 );
		void							setTexture( osg::Texture* pTex, sigma::uint32 unit=0, const std::string& uniformName="" );
		osg::Texture2D*					setTexture( const std::string& file, sigma::uint32 unit=0 );

		osg::Texture*					getTexture( sigma::uint32 unit=0 );

		osg::Image*						getImage();

		void							setUV( float uv );
		void							setUV( float u, float v );
		void							setUV( float u0, float u1, float v0, float v1 );
		float							getU();

		void							setWidth( float w );
		void							setHeight( float h );

		float							getWidth();
		float							getHeight();

		void							setCenter( const osg::Vec3& vCenter );

	private:
		void							calcVertPositions();

	private:
		sigma::MAP_TEXTURE				m_mapTex;
		osg::ref_ptr<osg::Vec4Array>	m_rColors;
		osg::ref_ptr<osg::Vec3Array>	m_rVerts;
		osg::ref_ptr<osg::Vec2Array>	m_rUV;
		osg::Vec3						m_vCenter;
		float							m_w;
		float							m_h;
		EPLANE							m_ePlane;

		osg::ref_ptr<osg::Image>		m_rImage;	// only valid if used
	};

}
#endif // __SULGEOMQUAD_H__