// SulPass.cpp

#include <FeKits/IR/SulPass.h>
#include <FeKits/IR/SulTransScreenAlign.h>

namespace FeKit
{

	CSulPass::CSulPass( osg::Texture2D* pTexIn, osg::Texture2D* pTexOut, const std::string& sName )
	{
		create( pTexIn, pTexOut, sName );
	}

	CSulGeomQuad* CSulPass::create( osg::Texture2D* pTexIn, osg::Texture2D* pTexOut, const std::string& sNameRTT )
	{
		int texW = pTexIn->getTextureWidth();
		int texH = pTexIn->getTextureHeight();

		m_rTexCam = new CSulTexCam( pTexOut, CSulTexCam::HDR );
		m_rTexCam->setName( std::string("CIuPostFilter-Cam-")+sNameRTT );

	
		m_rQuad = new CSulGeode;
		CSulGeomQuad* geomQuad = new CSulGeomQuad( osg::Vec3(texW/2.0f, texH/2.0f, 0), texW, texH );
		m_rQuad->addDrawable(geomQuad);

		m_rQuad->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
		m_rQuad->getOrCreateStateSet()->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
		geomQuad->setTexture( pTexIn );

		osg::ref_ptr<CSulTransScreenAlign> rAlign = new CSulTransScreenAlign( texW, texH );
		rAlign->addChild( m_rQuad );

		m_rTexCam->addChild( rAlign->getProjection() );

		return geomQuad;
	}

	CSulTexCam* CSulPass::getTexCam()
	{
		return m_rTexCam;
	}

	CSulGeode* CSulPass::getQuad()
	{
		return m_rQuad;
	}


}
