// SulFilterPass.cpp

#include <FeKits/IR/SulFilterPass.h>
#include <FeKits/IR/SulGeomQuad.h>
#include <osg/Geode>
#include "osgEarth/NodeUtils"

namespace FeKit
{
	osg::Group* CSulFilterPass::create( osg::Texture2D* pTexIn, osg::Texture2D* pTexOut, const std::string& sNameRTT )
	{
		int texW = pTexIn->getTextureWidth();
		int texH = pTexIn->getTextureHeight();

        if (!m_rTexCam.valid())
        {
            m_rTexCam = new CSulTexCam( pTexOut, CSulTexCam::HDR );
            m_rTexCam->setName( std::string("CSulFilterPass-Cam-")+sNameRTT );
        }
        else
        {
            m_rTexCam->setTexture(pTexOut);
        }

        m_rTexCam->removeChildren(0, m_rTexCam->getNumChildren());

        if (!m_rGroup.valid())
        {
            m_rGroup	= new osg::Group;
        }
        m_rGroup->removeChild(0, m_rGroup->getNumChildren());
        

		osg::ref_ptr<CSulGeomQuad> rQuad = new CSulGeomQuad( osg::Vec3(texW/2.0f, texH/2.0f, 0), texW, texH );
		rQuad->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
		rQuad->getOrCreateStateSet()->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
		rQuad->setTexture( pTexIn );

        osg::ref_ptr<CSulGeode> rGeode = new CSulGeode(rQuad);

        m_rGroup->addChild( rGeode.get() );

        if (!m_rpAlign.valid())
        {
            m_rpAlign = new CSulTransScreenAlign( texW, texH );
        }
 		m_rpAlign->AddChild( m_rGroup );

		m_rTexCam->addChild( m_rpAlign->getProjection() );

		return m_rGroup.get();
	}

	CSulTexCam* CSulFilterPass::getTexCam()
	{
		return m_rTexCam.get();
	}

	osg::Group* CSulFilterPass::getGroup()
	{
		return m_rGroup.get();
	}

    CSulFilterPass::CSulFilterPass() : osg::Referenced()
    {

    }

    CSulFilterPass::~CSulFilterPass()
    {
        if (m_rGroup.valid())
        {
            osg::ref_ptr<CSulGeode> pGeode = osgEarth::findRelativeNodeOfType<CSulGeode>(m_rGroup);
            if (pGeode.valid())
            {
                pGeode->removeDrawables(0, pGeode->getNumDrawables());
                pGeode = NULL;
            }
             m_rGroup->removeChildren(0, m_rGroup->getNumChildren());
            m_rGroup = NULL;
        }
        if (m_rpAlign.valid())
        {
            m_rpAlign->removeChildren(0, m_rpAlign->getNumChildren());
            m_rpAlign = NULL;
        }
        if (m_rTexCam.valid())
        {
            m_rTexCam->uninstallCam();
            m_rTexCam->uninstallTex();
            m_rTexCam->removeChildren(0, m_rTexCam->getNumChildren());
            m_rTexCam = NULL;
        }
       
    }

}