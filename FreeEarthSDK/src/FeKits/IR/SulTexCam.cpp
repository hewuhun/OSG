// SulTexCam.cpp

#include<FeKits/IR/SulTexCam.h>

namespace FeKit
{
CSulTexCam::CSulTexCam( osgViewer::Viewer* viewer, sigma::uint32 w, sigma::uint32 h, ESETUP eSetup ) :
    osg::Camera(),
    m_opViewer(viewer),
m_w( w ),
	m_h( h ),
	m_texW( w ),
	m_texH( h ),
	m_eSetup( eSetup )
{
	initTex();
	initCam();
}

CSulTexCam::CSulTexCam( osg::Texture2D* pTex, ESETUP eSetup ) :
osg::Camera(),
m_w( pTex->getTextureWidth() ),
	m_h( pTex->getTextureHeight() ),
	m_texW( pTex->getTextureWidth() ),
	m_texH( pTex->getTextureHeight() ),
	m_eSetup( eSetup )
{
	m_rpTex2d = pTex;
	initCam();
}

CSulTexCam::~CSulTexCam()
{
    uninstallCam();
    uninstallTex();
}

void CSulTexCam::setTextureSize( sigma::uint32 w, sigma::uint32 h )
{
	m_texW = w;
	m_texH = h;
}

void CSulTexCam::setTexture( osg::Texture2D* pTex )
{
	m_texW = pTex->getTextureWidth();
	m_texH = pTex->getTextureHeight();
    m_rpTex2d = pTex;
}

void CSulTexCam::initTex()
{
	switch ( m_eSetup )
	{
	case STANDARD:
		{
			m_rpTex2d = new osg::Texture2D;
			m_rpTex2d->setTextureSize( m_texW, m_texH );
			m_rpTex2d->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
			m_rpTex2d->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
			m_rpTex2d->setSourceFormat( GL_RGB );
			m_rpTex2d->setInternalFormat( GL_RGB );
			m_rpTex2d->setSourceType( GL_UNSIGNED_BYTE );
			m_rpTex2d->setFilter( osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR );
			m_rpTex2d->setFilter( osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR );
		}
		break;

	case HDR:
		{
			m_rpTex2d = new osg::Texture2D;
			m_rpTex2d->setTextureSize( m_texW, m_texH );
			m_rpTex2d->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
			m_rpTex2d->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
			m_rpTex2d->setSourceFormat( GL_RGB );
			m_rpTex2d->setInternalFormat( GL_RGB16F_ARB );
			m_rpTex2d->setSourceType( GL_FLOAT );
			m_rpTex2d->setFilter( osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR );
			m_rpTex2d->setFilter( osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR );
		}
		break;
	default:
	    break; 
	}
}

void CSulTexCam::initCam()
{
	setViewport( 0, 0, m_texW, m_texH );
	setProjectionMatrix(osg::Matrix::identity());
	setViewMatrix(osg::Matrix::identity());
	setReferenceFrame( osg::Transform::RELATIVE_RF );
#ifndef IR_RENDERBIN
#define IR_RENDERBIN
#define  IR_RENDER_ORDER 5 //设置渲染顺序为5 ，越小越先渲染
#endif
	setRenderOrder(osg::Camera::POST_RENDER, IR_RENDER_ORDER);
   // setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  //  setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	attach( osg::Camera::COLOR_BUFFER, m_rpTex2d );

    if (getRenderingCache())
    {
        getRenderingCache()->releaseGLObjects(0);
    }

}

void CSulTexCam::uninstallTex()
{
   
}

void CSulTexCam::uninstallCam()
{
    if (getRenderingCache())
    {
        getRenderingCache()->releaseGLObjects(0);
    }
    detach(osg::Camera::COLOR_BUFFER);
}

osg::Texture2D* CSulTexCam::getTexture( sigma::uint32 index )
{
	return m_rpTex2d.get();
}


}