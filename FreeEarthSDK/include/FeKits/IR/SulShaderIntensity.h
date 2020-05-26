// SulShaderIntensity.h

#ifndef __SULSHADERINTENSITY_H__
#define __SULSHADERINTENSITY_H__

#include <FeKits/IR/SulShaderBase.h>
#include <FeKits/Export.h>

namespace FeKit
{

	class  CSulShaderIntensity : public CSulShaderBase
	{
	public:
		CSulShaderIntensity( osg::Node* pNode, float fIntensity = 0.8, float fMultiIntR = 1.0, float fMultiIntG = 1.0, float fMultiIntB = 1.0, osg::Program* pProg=0 );

		void setMultiR( float f );
		void setMultiG( float f );
		void setMultiB( float f );
		void setIntensity( float f );

		void setInvert( bool bInvert );
	private:
		osg::ref_ptr<osg::Uniform>	m_uniformInvert;
		osg::ref_ptr<osg::Uniform>	m_rUniform_multiR;
		osg::ref_ptr<osg::Uniform>	m_rUniform_multiG;
		osg::ref_ptr<osg::Uniform>	m_rUniform_multiB;
		osg::ref_ptr<osg::Uniform>	m_rUniform_intensity;
	};

}
#endif // __SULSHADERINTENSITY_H__

