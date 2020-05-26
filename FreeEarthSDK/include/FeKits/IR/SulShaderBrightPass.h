// SulShaderBrightPass.h

#ifndef __SULSHADERBRIGHTPASS_H__
#define __SULSHADERBRIGHTPASS_H__

#include <FeKits/Export.h>
#include <FeKits/IR/SulShaderBase.h>

namespace FeKit
{

	class  CSulShaderBrightPass : public CSulShaderBase
	{
	public:
		CSulShaderBrightPass( osg::Node* pNode, float fThreshold, osg::Program* pProg=0 );

		void						setThreshold( float val );
		void						setFalloff( float val );

	private:
		osg::ref_ptr<osg::Uniform>	m_rUniform_threshold;
		osg::ref_ptr<osg::Uniform>	m_rUniform_falloff;
	};

}

#endif // __SULSHADERBRIGHTPASS_H__