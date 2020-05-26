// SulPostFilterBase.cpp

#include <FeKits/IR/SulPostFilterBase.h>
//#include <FeKits/IR/SulRTT.h>
#include <FeKits/IR/SulTexCam.h>
#include <FeKits/IR/SulGeomQuad.h>
#include <FeKits/IR/SulTransScreenAlign.h>
#include <FeKits/IR/SulFilterPass.h>
#include <osg/Geode>

namespace FeKit
{

	CSulPostFilter::CSulPostFilter() :
m_texFormatInternal( GL_RGBA ),
	m_texFormatSource( GL_RGBA ),
	m_texSourceType( GL_UNSIGNED_BYTE )
{
}

void CSulPostFilter::in( osg::Texture2D* pTex )
{
}

osg::Texture2D* CSulPostFilter::out()
{
	return 0;
}

void CSulPostFilter::setTexFormatInternal( GLint texFormat )
{
	m_texFormatInternal = texFormat;
}

void CSulPostFilter::setTexFormatSource( GLint texFormat )
{
	m_texFormatSource = texFormat;
}

void CSulPostFilter::setTexSourceType( GLenum texSourceType )
{
	m_texSourceType = texSourceType;
}

osg::Texture2D* CSulPostFilter::createTexture( unsigned short w, unsigned short h )
{
	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	texture->setTextureSize( w, h );
	texture->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
	texture->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
	texture->setSourceFormat( m_texFormatSource );
	texture->setInternalFormat( m_texFormatInternal );
	texture->setSourceType( m_texSourceType );
	texture->setFilter( osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR );
	texture->setFilter( osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR );

	return texture.release();
}

osg::Group* CSulPostFilter::createFilterPass( osg::Texture2D* pTexIn, osg::Texture2D* pTexOut, const std::string& sNameRTT )
{
	osg::ref_ptr<CSulFilterPass> pFilter =  new CSulFilterPass();
	osg::ref_ptr<osg::Group> pGroup = pFilter->create( pTexIn, pTexOut, sNameRTT );

	addChild( pFilter->getTexCam() );
 
	return pGroup.get();
}

void CSulPostFilter::addPass( CSulPass* pPass )
{
	addChild( pPass->getTexCam() );
}

}
