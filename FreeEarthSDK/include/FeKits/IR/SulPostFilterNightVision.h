// SulPostFilterNightVision.h

#ifndef __SULPOSTFILTERNIGHTVISION_H__
#define __SULPOSTFILTERNIGHTVISION_H__

#include <FeKits/Export.h>
#include  <FeKits/IR/SulPostFilterBase.h>
#include  <FeKits/IR/SulShaderIntensity.h>
#include <FeKits/IR/SulPostFilterBloom.h>
#include "SulGeomQuad.h"

namespace FeKit
{

	class  CSulPostFilterNightVision : public CSulPostFilter
	{
	public:
		CSulPostFilterNightVision();
		CSulPostFilterNightVision(float intensity);
        ~CSulPostFilterNightVision();
		void	in( osg::Texture2D* pTex, float intensityMultiplier, sigma::uint32 w=256, sigma::uint32 h =256 );
		osg::Texture2D* out();

        void ReSize(int w, int h);
	private:
		void	filterIntensity( osg::Texture2D* pTexIn, osg::Texture2D* pTexOut );

	private:
		osg::ref_ptr<osg::Texture2D>		m_rTex;
		float								m_intensity;
// 		osg::ref_ptr<CSulShaderIntensity>	m_rShaderIntensity;
         osg::ref_ptr<CSulPostFilterBloom> m_rpbloom;
//         osg::observer_ptr<CSulTexCam> m_opTexCam;
//         osg::observer_ptr<CSulGeomQuad> m_quad;
	};

}

#endif // __SULPOSTFILTERNIGHTVISION_H__