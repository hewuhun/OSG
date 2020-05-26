// SulGeode.h

#ifndef __SULGEODE_H__
#define __SULGEODE_H__

#include <FeKits/Export.h>
#include <osg/Geode>

namespace FeKit
{
	class  CSulGeode : public osg::Geode
	{
	public:
		CSulGeode();
		CSulGeode( osg::Drawable* geom, bool enableLighting=true );

		void enableBlend();
		void zbufferWrite( bool bEnable );
		void enableDebugTexture();
		void lighting( bool bEnable );
	};
}

#endif // __SULGEODE_H__