#ifndef TERRAIN_MASK_H__
#define TERRAIN_MASK_H__

#include <FeUtils/Export.h>
#include <FeUtils/RenderContext.h>
namespace FeUtil
{
	class FEUTIL_EXPORT CTerrainMask
	{
	public:
		CTerrainMask();

		virtual ~CTerrainMask();

	public:
		bool Create(std::string name, CRenderContext * pRenderContext, std::string fileName);
		
		void Remove();

	private:
		osgEarth::MaskLayer * m_pMaskLayer;
		osgEarth::Map       * m_pMap;
		std::string           m_strName;
	};
}

#endif //  TERRAIN_MASK_H__