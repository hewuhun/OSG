// SulPostFilterBase.h

#ifndef __SULPOSTFILTERBASE_H__
#define __SULPOSTFILTERBASE_H__


#include <FeKits/Export.h>
// #include <FeKits/IR/SulShaderStreak.h>
// #include <FeKits/IR/SulShaderCombineTex.h>
// #include <FeKits/IR/SulShaderBrightPass.h>
#include <FeKits/IR/SulPass.h>
#include <osg/Group>
#include <osg/Texture2D>

namespace FeKit
{

	class  CSulPostFilter : public osg::Group
	{
	public:
		CSulPostFilter();

		void					setTexFormatInternal( GLint texFormat );
		void					setTexFormatSource( GLint texFormat );
		void					setTexSourceType( GLenum texSourceType );

		virtual void			in( osg::Texture2D* pTex );
		virtual osg::Texture2D* out();

	protected:
		osg::Texture2D*			createTexture( unsigned short w, unsigned short h );
		osg::Group*				createFilterPass( osg::Texture2D* pTexIn, osg::Texture2D* pTexOut, const std::string& sNameRTT );

		void addPass( CSulPass* pPass );

	private:
		GLint					m_texFormatInternal;
		GLint					m_texFormatSource;
		GLenum					m_texSourceType;
	};

}

#endif // __SULPOSTFILTERBASE_H__