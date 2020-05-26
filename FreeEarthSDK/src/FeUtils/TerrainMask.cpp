#include "FeUtils/TerrainMask.h"
#include <iostream>
namespace FeUtil
{
	
	CTerrainMask::CTerrainMask()
		: m_pMaskLayer(NULL)
		, m_pMap(NULL)
	{

	}

	CTerrainMask::~CTerrainMask()
	{
		   if (m_pMaskLayer != NULL)
		   {
			   Remove();
		   }
	}

	bool CTerrainMask::Create(std::string name, CRenderContext * pRenderContext, std::string fileName)
	{
		 m_strName = name;
		if (pRenderContext == NULL)
		{
			return false;
		}
		osgEarth::MapNode * mapNode = pRenderContext->GetMapNode();
		if (mapNode == NULL)
		{
			return false;
		}

		m_pMap = mapNode->getMap();

		if (m_pMap == NULL)
		{
			return false;
		}

		std::ifstream file;
        //file.open(fileName, std::ios::_Nocreate);
        file.open(fileName, std::ifstream::in);
		bool isOpen = file.is_open();

		if (isOpen == false)
		{
			return false;
		}

		std::ostringstream ss;

		ss << file.rdbuf();
		file.close();

		file.close();
		std::string beginStr = "POLYGON((";
		std::string endStr = ")) ";
		
		
		osgEarth::Config config("mask");
		config.add("driver", "feature");
		config.add("name", m_strName);
		config.add("profile", "global-geodetic");

		osgEarth::Config featureConfig("features");
		featureConfig.add("driver", "ogr");
		featureConfig.add("geometry", beginStr + ss.str() + endStr);
		config.add(featureConfig);

		osgEarth::MaskLayerOptions opt(config);
		opt.name() = config.value( "name" );
		opt.driver() = osgEarth::MaskSourceOptions(opt);



		m_pMaskLayer = new osgEarth::MaskLayer(opt);
		if (m_pMaskLayer == NULL)
		{
			return false;
		}
		m_pMap->addTerrainMaskLayer(m_pMaskLayer);  

		



		return true;
	}

	void CTerrainMask::Remove()
	{
		if (m_pMap != NULL && m_pMaskLayer != NULL)
		{
			m_pMap->removeTerrainMaskLayer(m_pMaskLayer);
			delete m_pMaskLayer;
			m_pMaskLayer = NULL;
		}
	}

}


