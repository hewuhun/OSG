// SulShaderGaussH.h

#ifndef __SULSHADERGAUSSH_H__
#define __SULSHADERGAUSSH_H__

#include <FeKits/Export.h>
#include<FeKits/IR/SulShaderBase.h>

namespace FeKit
{

class  CSulShaderGaussH : public CSulShaderBase
{
public:
	CSulShaderGaussH( osg::Node* pNode, float fTexH, osg::Program* pProg=0 );
};

}

#endif // __SULSHADERGAUSS_H__