// SulPostFilterBloom.h

#ifndef __SULPOSTFILTERBLOOM_H__
#define __SULPOSTFILTERBLOOM_H__

#include <FeKits/Export.h>
#include <FeKits/IR/SulPostFilterBase.h>
#include <FeKits/IR/SulFilterPass.h>

namespace FeKit
{

	class  CSulPostFilterBloom : public CSulPostFilter
	{
	public:
		CSulPostFilterBloom();
        ~CSulPostFilterBloom();

		void			in( osg::Texture2D* pTex, sigma::int32 w , sigma::int32 h);
		osg::Texture2D* out();

    public:
        osg::ref_ptr<CSulFilterPass> m_opBrightPass;
        osg::ref_ptr<CSulFilterPass> m_opGaussHPass;
        osg::ref_ptr<CSulFilterPass> m_opGaussVPass;
        osg::ref_ptr<CSulFilterPass> m_opCombinePass;
            osg::ref_ptr<CSulFilterPass> m_opIntensityPass;

	private:
		void			filterBrightPass( osg::Texture2D* pTexIn, osg::Texture2D* pTexOut );
		void			filterGaussH( osg::Texture2D* pTexIn, osg::Texture2D* pTexOut );
		void			filterGaussV( osg::Texture2D* pTexIn, osg::Texture2D* pTexOut );
		void			filterCombine( osg::Texture2D* pTexOrg, osg::Texture2D* pTexIn, osg::Texture2D* pTexOut );
        void         filterIntensity( osg::Texture2D* pTexIn, osg::Texture2D* pTexOut );
	private:
		osg::ref_ptr<osg::Texture2D>		m_rTex;
		float								m_fExp;
		float								m_fFactor;
		float								m_fMax;
        float                            m_fIndentity;
	};

}

#endif // __SULPOSTFILTERBLOOM_H__