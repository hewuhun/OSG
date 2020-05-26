// SulShaderGaussV.h

#ifndef __SULSHADERGAUSSV_H__
#define __SULSHADERGAUSSV_H__

#include <FeKits/Export.h>
#include <FeKits/IR/SulShaderBase.h>

namespace FeKit
{

	class  CSulShaderGaussV : public CSulShaderBase
	{
	public:
		CSulShaderGaussV( osg::Node* pNode, float fTexV, osg::Program* pProg=0 );
	};

}

#endif // __SULSHADERGAUSSV_H__