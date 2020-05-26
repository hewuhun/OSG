#ifndef FE_DRAW_TEXTURE_H
#define FE_DRAW_TEXTURE_H

#include <FeUtils/Export.h>

#include <osg/Group>
#include <osg/Texture2D>

namespace FeUtil
{
	class FEUTIL_EXPORT CDrawTexture :public osg::Group
	{
	public:

		CDrawTexture(osg::Image *pImage,double dSize);

		CDrawTexture(std::string strPath,double dSize);

		virtual ~CDrawTexture();

	public:
		void SetImage(osg::Image *pImage);

		osg::Image *GetImage();

	protected:
		void Create();
		
	protected:
		osg::Vec3d		m_vecPos;

		osg::ref_ptr<osg::Texture2D>	m_pTex2D;

		std::string						m_strPath;
		double							m_Size;
		osg::Image *					m_pImage;

	};
}

#endif //FE_DRAW_TEXTURE_H