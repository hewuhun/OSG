// SulShaderGaussH.cpp

#include <FeKits/IR/SulShaderGaussH.h>
#include <FeUtils/PathRegistry.h>
namespace FeKit
{

CSulShaderGaussH::CSulShaderGaussH( osg::Node* pNode, float fTexH, osg::Program* pProg ) :
CSulShaderBase( pNode )
{
	if ( !pProg )
	{
		addShaderFrag( FeFileReg->GetFullPath("shaders/ir/gaussh.frag") ); 
		addShaderVert( FeFileReg->GetFullPath("shaders/ir/gaussh.vert") ); 

		osg::Uniform* uniform_ftexh = new osg::Uniform( osg::Uniform::FLOAT, "fTexH" );
		uniform_ftexh->set( fTexH );
		m_rSS->addUniform( uniform_ftexh );	
        
     }
}


}
