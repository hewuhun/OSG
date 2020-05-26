// SulPostFilterBrightPass.h

#ifndef __SULPOSTFILTERBRIGHTPASS_H__
#define __SULPOSTFILTERBRIGHTPASS_H__

#include <FeKits/Export.h>
#include <FeKits/IR/SulPostFilterBase.h>
#include <FeKits/IR/SulShaderBrightPass.h>

namespace FeKit
{

	class  CSulPostFilterBrightPass : public CSulPostFilter
	{
	public:
		CSulPostFilterBrightPass();

		void			in( osg::Texture2D* pTex );
		osg::Texture2D* out();

	private:
		void			filterBrightPass( osg::Texture2D* pTexIn, osg::Texture2D* pTexOut );

	private:
		osg::ref_ptr<osg::Texture2D>		m_rTex;
		osg::ref_ptr<CSulShaderBrightPass>	m_rShaderBrightPass;
	};

}

#endif // __SULPOSTFILTERBRIGHTPASS_H__