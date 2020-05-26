// SulPostFilterNightVision.cpp

#include <FeKits/IR/SulPostFilterNightVision.h>
#include <FeKits/IR/SulPostFilterBloom.h>

namespace FeKit
{

	CSulPostFilterNightVision::CSulPostFilterNightVision() :
m_intensity( 0.6f ),
    m_rpbloom(0)
{

}

CSulPostFilterNightVision::CSulPostFilterNightVision( float intensity ):
m_intensity( intensity ),
    m_rpbloom(0)
{

}

CSulPostFilterNightVision::~CSulPostFilterNightVision()
{
    if (m_rpbloom.valid())
    {
        m_rpbloom->removeChildren(0, m_rpbloom->getNumChildren());
        m_rpbloom = NULL;
    }
}

void CSulPostFilterNightVision::in( osg::Texture2D* pTex, float intensityMultiplier, sigma::uint32 w, sigma::uint32 h )
{
	m_rTex/*osg::Texture2D* tex2d*/  = createTexture( w, h );

    if (m_rpbloom.valid())
    {
        m_rpbloom->removeChildren(0, m_rpbloom->getNumChildren());
        m_rpbloom = NULL;
    }

    if (!m_rpbloom.valid())
    {
        m_rpbloom = new CSulPostFilterBloom;
    }
	
	m_rpbloom->in( pTex, w, h); 
	addChild( m_rpbloom );

//	filterIntensity( m_rpbloom->out(), m_rTex );
}

void CSulPostFilterNightVision::filterIntensity( osg::Texture2D* pTexIn, osg::Texture2D* pTexOut )
{
	osg::Group* pGroup = createFilterPass( pTexIn, pTexOut, "Intensity" );
        
    osg::ref_ptr<CSulShaderIntensity> m = new CSulShaderIntensity( pGroup, m_intensity, 1.0f, 1.0f, 1.0f );

}

osg::Texture2D* CSulPostFilterNightVision::out()
{
	return m_rTex.get();
}


}



