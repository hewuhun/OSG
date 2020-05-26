// SulTexCam.h

#ifndef __SULTEXCAM_H__
#define __SULTEXCAM_H__

#include <FeKits/Export.h>
#include <FeKits/IR/SulTypes.h>
#include <osg/Camera>
#include <osg/Texture2D>
#include <osgViewer/Viewer>

namespace FeKit
{
    class CSulIR;
    class CSulPostFilterNightVision;
	class  CSulTexCam : public osg::Camera
	{
        friend class CSulIR;
        friend class CSulPostFilterNightVision;
	public:
		enum ESETUP
		{
			STANDARD,
			HDR,
		};

	public:
		CSulTexCam( osgViewer::Viewer* viewer, sigma::uint32 w, sigma::uint32 h, ESETUP eSetup=STANDARD );
		CSulTexCam( osg::Texture2D* pTex, ESETUP eSetup=STANDARD );

        virtual ~CSulTexCam();

		void				setTexture( osg::Texture2D* pTex );

		osg::Texture2D*		getTexture( sigma::uint32 index=0 );

		void				setTextureSize( sigma::uint32 w, sigma::uint32 h );

		void				initTex();
		void				initCam();

		void               uninstallTex();
		void               uninstallCam();

	private:
        osg::ref_ptr<osg::Texture2D> m_rpTex2d;

		sigma::uint32					m_w;
		sigma::uint32					m_h;

		ESETUP							m_eSetup;

		sigma::uint32					m_texW;
		sigma::uint32					m_texH;

        osg::observer_ptr<osgViewer::Viewer> m_opViewer;
	};
}
#endif // __SULTEXCAM_H__
