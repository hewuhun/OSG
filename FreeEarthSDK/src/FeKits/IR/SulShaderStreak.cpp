// SulShaderStreak.cpp

#include <FeKits/IR/SulShaderStreak.h>

namespace FeKit
{

	CSulShaderStreak::CSulShaderStreak( osg::Node* pNode, float offsetFactor, float glowFactor, osg::Program* pProg ) :
CSulShaderBase( pNode,pProg )
{
	if ( !pProg )
	{
		addShaderFrag( "shaders/ir/streak.frag" );
		addShaderVert( "shaders/ir/streak.vert" );

		m_rUniform_offsetFactor = new osg::Uniform( osg::Uniform::FLOAT, "offsetFactor" );
		m_rSS->addUniform( m_rUniform_offsetFactor );
		m_rUniform_offsetFactor->set( offsetFactor );

		m_rUniform_glowFactor = new osg::Uniform( osg::Uniform::FLOAT, "glowFactor" );
		m_rSS->addUniform( m_rUniform_glowFactor );
		m_rUniform_glowFactor->set( glowFactor );
	}
}

void CSulShaderStreak::setGlowFactor( float glowFactor )
{
	m_rUniform_glowFactor->set( glowFactor );
}


}
