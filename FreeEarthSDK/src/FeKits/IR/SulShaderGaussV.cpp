// SulShaderGaussV.cpp

#include <FeKits/IR/SulShaderGaussV.h>
#include <FeUtils/PathRegistry.h>
namespace FeKit
{

	CSulShaderGaussV::CSulShaderGaussV( osg::Node* pNode, float fTexV, osg::Program* pProg ) :
CSulShaderBase( pNode )
{
	if ( !pProg )
	{
		addShaderFrag( FeFileReg->GetFullPath("shaders/ir/gaussv.frag") ); 
		addShaderVert( FeFileReg->GetFullPath("shaders/ir/gaussv.vert") ); 
   
		osg::Uniform* uniform_ftexv = new osg::Uniform( osg::Uniform::FLOAT, "fTexV" );
		uniform_ftexv->set( fTexV );
		m_rSS->addUniform( uniform_ftexv );
        
	}
}


}
