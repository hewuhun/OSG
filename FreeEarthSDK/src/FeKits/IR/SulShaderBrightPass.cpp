// SulShaderBrightPass.cpp

#include <FeKits/IR/SulShaderBrightPass.h>
#include <FeUtils/PathRegistry.h>
namespace FeKit
{

CSulShaderBrightPass::CSulShaderBrightPass( osg::Node* pNode, float fThreshold, osg::Program* pProg ) :
CSulShaderBase( pNode )
{
	if ( !pProg )
	{
		addShaderFrag( FeFileReg->GetFullPath("shaders/ir/brightpass.frag") );  
		addShaderVert( FeFileReg->GetFullPath("shaders/ir/brightpass.vert") );  
	}

	m_rUniform_threshold = new osg::Uniform( osg::Uniform::FLOAT, "brightPassThreshold" );
	m_rUniform_threshold->set( fThreshold );
	m_rSS->addUniform( m_rUniform_threshold );
	
	m_rUniform_falloff= new osg::Uniform( osg::Uniform::FLOAT, "brightPassFalloff" );
	m_rUniform_falloff->set( 0.5f );
	m_rSS->addUniform( m_rUniform_falloff );
}

void CSulShaderBrightPass::setThreshold( float val )
{
	m_rUniform_threshold->set( val );
}

void CSulShaderBrightPass::setFalloff( float val )
{
	m_rUniform_falloff->set( val );
}

}
