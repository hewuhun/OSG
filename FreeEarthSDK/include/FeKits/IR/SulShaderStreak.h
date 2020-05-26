﻿// SulShaderStreak.h

#ifndef __SULSHADERSTREAK_H__
#define __SULSHADERSTREAK_H__

#include <FeKits/IR/SulShaderBase.h>
#include <FeKits/Export.h>

namespace FeKit
{

	class  CSulShaderStreak : public CSulShaderBase
	{
	public:
		CSulShaderStreak( osg::Node* pNode, float offsetFactor, float glowFactor=0.2f, osg::Program* pProg=0 );

		void	setGlowFactor( float glowFactor );

	private:
		osg::ref_ptr<osg::Uniform>	m_rUniform_offsetFactor;
		osg::ref_ptr<osg::Uniform>	m_rUniform_glowFactor;
	};


}

#endif // __SULSHADERSTREAK_H__