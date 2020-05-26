// SulPostFilterBloom.cpp

#include <FeKits/IR/SulPostFilterBloom.h>
#include <FeKits/IR/SulShaderBrightPass.h>
#include <FeKits/IR/SulShaderGaussH.h>
#include <FeKits/IR/SulShaderGaussV.h>
#include <FeKits/IR/SulShaderCombine.h>
#include <osg/Geode>
#include "FeKits/IR/SulShaderIntensity.h"

namespace FeKit
{

	CSulPostFilterBloom::CSulPostFilterBloom() :
CSulPostFilter(),
	m_fExp( 2.0f ),
	m_fFactor( 1.0f ),
	m_fMax( 1.0f ),
    m_fIndentity(0.6)
{
}

CSulPostFilterBloom::~CSulPostFilterBloom()
{
    if (m_opBrightPass.valid())
    {
        m_opBrightPass = NULL;
    }
    if (m_opGaussHPass.valid())
    {
        m_opGaussHPass = NULL;
    }
    if (m_opGaussVPass.valid())
    {
        m_opGaussVPass = NULL;
    }
    if (m_opCombinePass.valid())
    {
        m_opCombinePass = NULL;
    }
    if (m_opIntensityPass.valid())
    {
        m_opIntensityPass = NULL;
    }
}

void CSulPostFilterBloom::in( osg::Texture2D* pTex, sigma::int32 w , sigma::int32 h )
{
	//修复内存泄漏
	removeChildren(0, getNumChildren());

	m_rTex = createTexture( w, h );

	osg::ref_ptr<osg::Texture2D> tex0 = createTexture( w, h );
	osg::ref_ptr<osg::Texture2D> tex1 = createTexture( w, h );
    osg::ref_ptr<osg::Texture2D> tex2 = createTexture( w, h );

	filterBrightPass( pTex, tex0 );
	filterGaussH( tex0, tex1 );
	filterGaussV( tex1, tex0 );
	filterCombine( pTex, tex0, tex2 );
    filterIntensity(tex2, m_rTex);
}

osg::Texture2D* CSulPostFilterBloom::out()
{
	return m_rTex;
}

void CSulPostFilterBloom::filterBrightPass( osg::Texture2D* pTexIn, osg::Texture2D* pTexOut )
{
//	osg::ref_ptr<osg::Group> pGroup = createFilterPass( pTexIn, pTexOut, "BrightPass" );
//    m_opBrightPass = dynamic_cast<CSulTexCam*>(this->getChild(0));

    m_opBrightPass =  new CSulFilterPass();
    osg::ref_ptr<osg::Group> pGroup = m_opBrightPass->create( pTexIn, pTexOut, "BrightPass" );

    addChild( m_opBrightPass->getTexCam() );

	osg::ref_ptr<CSulShaderBrightPass> rShader = new CSulShaderBrightPass( pGroup, 0.8f );

}

void CSulPostFilterBloom::filterGaussH( osg::Texture2D* pTexIn, osg::Texture2D* pTexOut )
{
//    osg::ref_ptr<osg::Group> pGroup = createFilterPass( pTexIn, pTexOut, "GaussH" );
//    m_opGaussHPass = dynamic_cast<CSulTexCam*>(this->getChild(1));
    m_opGaussHPass =  new CSulFilterPass();
    osg::ref_ptr<osg::Group> pGroup = m_opGaussHPass->create( pTexIn, pTexOut, "GaussH" );

    addChild( m_opGaussHPass->getTexCam() );

    osg::ref_ptr<CSulShaderGaussH> rShader = new CSulShaderGaussH( pGroup, pTexIn->getTextureHeight() );

}

void CSulPostFilterBloom::filterGaussV( osg::Texture2D* pTexIn, osg::Texture2D* pTexOut )
{
//	osg::ref_ptr<osg::Group> pGroup = createFilterPass( pTexIn, pTexOut, "GaussV" );
//    m_opGaussVPass = dynamic_cast<CSulTexCam*>(this->getChild(2));
    m_opGaussVPass =  new CSulFilterPass();
    osg::ref_ptr<osg::Group> pGroup = m_opGaussVPass->create( pTexIn, pTexOut, "GaussV" );

    addChild( m_opGaussVPass->getTexCam() );

	osg::ref_ptr<CSulShaderGaussV> rShader = new CSulShaderGaussV( pGroup, pTexIn->getTextureWidth() );

}

void CSulPostFilterBloom::filterCombine( osg::Texture2D* pTexOrg, osg::Texture2D* pTexIn, osg::Texture2D* pTexOut )
{
//	osg::ref_ptr<osg::Group> pGroup = createFilterPass( pTexIn, pTexOut, "Compose" );
//    m_opCombinePass = dynamic_cast<CSulTexCam*>(this->getChild(3));
    m_opCombinePass =  new CSulFilterPass();
    osg::ref_ptr<osg::Group> pGroup = m_opCombinePass->create( pTexIn, pTexOut, "Compose" );

    addChild( m_opCombinePass->getTexCam() );

	osg::ref_ptr<CSulShaderCombine> rShaderCombine = new CSulShaderCombine( pGroup, m_fExp, m_fFactor, m_fMax, pTexOrg, pTexIn );

}

void CSulPostFilterBloom::filterIntensity( osg::Texture2D* pTexIn, osg::Texture2D* pTexOut )
{
    m_opIntensityPass =  new CSulFilterPass();
    osg::ref_ptr<osg::Group> pGroup = m_opIntensityPass->create( pTexIn, pTexOut, "Compose" );

    addChild( m_opIntensityPass->getTexCam() );
    osg::ref_ptr<CSulShaderIntensity> m = new CSulShaderIntensity( pGroup, m_fIndentity, 1.0f, 1.0f, 1.0f );

}

}
