// SulShaderBase.h

#ifndef __SULSHADERBASE_H__
#define __SULSHADERBASE_H__

#include <FeKits/Export.h>
#include <osg/Referenced>
#include <osg/Program>
#include <osg/State>
#include <osg/Uniform>

namespace FeKit
{

	class  CSulShaderBase : public osg::Referenced
	{
	public:
		CSulShaderBase( osg::Node* pNode, osg::Program* pProg=0, bool bSuppressShader=false );

		void	enable( bool bEnable=true );

		void	addShaderFrag( const std::string& file );
		void	addShaderVert( const std::string& file );

	protected:
		osg::ref_ptr<osg::Program>		m_rProg;
		osg::ref_ptr<osg::Program>		m_rProgDefault;
		osg::ref_ptr<osg::StateSet>		m_rSS;
		bool							m_bEnable;
		bool							m_bSuppressShader;
	};
}
#endif // __SULSHADERBASE_H__