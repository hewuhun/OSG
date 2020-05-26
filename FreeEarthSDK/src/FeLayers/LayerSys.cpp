#include <FeLayers/LayerSys.h>

#include <osgEarthUtil/LODBlending>
#include <osgEarthUtil/ContourMap>

#include <FeLayers/NormalMapEffect.h>
#include <FeLayers/FeatureReaderWriter.h>

#include <FeLayerImage.h>
#include <FeLayerModel.h>
#include <FeLayerElevation.h>
#include <FeLayerSerialize.h>


#include <FeUtils/StrUtil.h>
#include <FeUtils/PathRegistry.h>


namespace FeLayers
{
	CLayerSysOptions::CLayerSysOptions()
	{

	}

	CLayerSysOptions::~CLayerSysOptions()
	{

	}
}


namespace FeLayers
{
	extern ILayerFactory* LayerFactorySingle();
		
	/**
	  * @class CTerrainEngineCamPosUpdate
	  * @brief 地形引擎节点更新回调
	  * @note 用于实时更新相机高度，目前用于图层显示范围控制
	  * @author g00034
	*/
	class CTerrainEngineCamPosUpdate : public osg::NodeCallback
	{
	public:
		CTerrainEngineCamPosUpdate(osgViewer::View* pView):m_opView(pView){}
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
		{
			if(node && m_opView.valid())
			{
				osg::Vec3f eye, center, up;
				m_opView->getCamera()->getViewMatrixAsLookAt(eye, center, up);

				node->getOrCreateStateSet()->getOrCreateUniform(
					"oe_terrain_cam_height", osg::Uniform::FLOAT)->set( eye.z() );
			}

			traverse(node, nv);
		}

	private:
		osg::observer_ptr<osgViewer::View> m_opView;
	};
	
	
	// 增加地形回调，用于实时将相机位置传入Shader ( 实现影像的可见范围设置 )
	void AddCamPosCallbackToTerrain(FeUtil::CRenderContext* pRC)
	{
		if(pRC && pRC->GetMapNode())
		{
			pRC->GetMapNode()->getTerrainEngine()->addUpdateCallback( new CTerrainEngineCamPosUpdate(pRC->GetView()));
		}
	}
	

	CLayerSys::CLayerSys(CLayerSysOptions opt)
		:FeUtil::CSceneSysCall()
		,m_optLayerConfig(opt)
	{
		SetSysKey(LAYER_SYSTEM_CALL_DEFAULT_KEY);
	}

	CLayerSys::~CLayerSys()
	{

	}

	bool CLayerSys::InitiliazeImplement()
	{
		//暂时删除掉，后续会恢复，c00005
		//注册特效图层
        //RegistryEffectLayer();

		//初始化矢量图层
		//RegistryFeatureLayer();
		
		// 增加地形回调
		AddCamPosCallbackToTerrain(m_opRenderContext.get());

		// 获取系统图层
		if( GetMap() )
		{
			GetMap()->getImageLayers(m_sysLayerImage);
			GetMap()->getModelLayers(m_sysLayerModel);
			GetMap()->getElevationLayers(m_sysLayerElevation);
		}

		// TODO 从配置文件加载图层


		return true;
	}

	bool CLayerSys::UnInitiliazeImplement()
	{
		// TODO 保存图层到文件


		ClearLayers();
	
		return true;
	}

	bool CLayerSys::AddLayer( IFeLayer* pLayer )
	{
		if(!pLayer) return false;

		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mapDataMutex);

		if( m_layerSet.end() == std::find(m_layerSet.begin(), m_layerSet.end(), pLayer) )
		{
			bool bRet = false;

			if(pLayer->IsImageLayer())
			{
				bRet = AddImageLayer( pLayer->AsImageLayer() );
			}
			else if(pLayer->IsModelLayer())
			{
				bRet = AddModelLayer( pLayer->AsModelLayer() );
			}
			else if(pLayer->IsElevationLayer())
			{
				bRet = AddElevationLayer(pLayer->AsElevationLayer());
			}
			else if(pLayer->IsEffectLayer())
			{
				bRet = AddEffectLayer(pLayer->AsEffectLayer());
			}

			if(bRet)
			{
				m_layerSet.push_back(pLayer);
			}

			return bRet;
		}

		return true;
	}

	IFeLayer* CLayerSys::AddLayer( const CFeLayerConfig& cfg )
	{
		IFeLayer* pLayer = LayerFactory()->CreateLayer(cfg);

		if( !AddLayer(pLayer) )
		{
			delete pLayer;
			pLayer = NULL;
		}

		return pLayer;
	}

	void CLayerSys::GetLayers( CLayersSet& vecLayer )
	{
		vecLayer.assign(m_layerSet.begin(), m_layerSet.end());
	}

	unsigned int CLayerSys::GetLayersCount()
	{
		return m_layerSet.size();
	}

	IFeLayer* CLayerSys::GetLayerAt( unsigned int nIndex )
	{
		return (nIndex < m_layerSet.size()) ? m_layerSet.at(nIndex) : NULL;
	}

	bool CLayerSys::DeleteLayer(IFeLayer* pLayer)
	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mapDataMutex);

		CLayersSet::iterator iter = std::find(m_layerSet.begin(), m_layerSet.end(), pLayer);
		if(iter != m_layerSet.end())
		{
			m_layerSet.erase(iter);

			if(pLayer->IsImageLayer())
			{
				return DeleteImageLayer( pLayer->AsImageLayer() );
			}
			else if(pLayer->IsModelLayer())
			{
				return DeleteModelLayer( pLayer->AsModelLayer() );
			}
		}

		return false;
	}

	bool CLayerSys::DeleteLayer( FeLayerID id )
	{
		for(CLayersSet::iterator iter = m_layerSet.begin(); iter!=m_layerSet.end(); iter++)
		{
			if((*iter)->GetLayerUID() == id) return DeleteLayer(*iter);
		}

		return true;
	}

	void CLayerSys::ClearLayers()
	{
		CLayersSet layerSet;
		GetLayers(layerSet);
		for(CLayersSet::iterator iter = layerSet.begin(); iter!=layerSet.end(); iter++)
		{
			DeleteLayer(*iter);
		}
	}

	// 生成图层缓存ID
	std::string GenLayerCacheID(IFeLayer* pLayer, const std::string& srsName)
	{
		CFeLayerConfig layerCfg = pLayer->GetLayerConfig();

		// 为避免图层缓存ID重复，根据数据源信息生成哈希值
		std::string strLayerInfo = layerCfg.sourceConfig().url().get() + layerCfg.sourceConfig().layerName().get();
		unsigned int nHashVal = osgEarth::hashString(strLayerInfo);

		stringstream ss;
		ss << nHashVal;
		return ss.str();
	}

	bool CLayerSys::AddImageLayer(IFeLayerImage* pLayer)
	{
		osgEarth::Map* pMap = GetMap();
		CFeLayerImage* pImageLayer = static_cast<CFeLayerImage*>(pLayer);
		if (pMap && pImageLayer)
		{
			m_layersImage.push_back(pImageLayer);

			//设置缓存ID
			{
				CFeLayerImage::OsgEarthLayerType *layerAgent = dynamic_cast<CFeLayerImage::OsgEarthLayerType*>(pImageLayer->GetEarthLayer());
				if (NULL != layerAgent)
				{
					//layerAgent->SetCacheID( GenLayerCacheID(pLayer, m_opRenderContext->GetMapSRS()->getName()) );
					layerAgent->SetCacheID( layerAgent->getName() ); // 名字可能为空，所以不能使用
				}
			}

			pMap->addImageLayer(pImageLayer->GetEarthLayer());
			return true;
		}

		return false;
	}

	bool CLayerSys::DeleteImageLayer(IFeLayerImage* pLayer)
	{
		osgEarth::Map* pMap = GetMap();
		CFeLayerImage* pImageLayer = dynamic_cast<CFeLayerImage*>(pLayer);
		if (pMap && pImageLayer)
		{
			pMap->removeImageLayer(pImageLayer->GetEarthLayer());

			CImageLayersSet::iterator iter = std::find(m_layersImage.begin(), m_layersImage.end(), pImageLayer);
			if(iter != m_layersImage.end())
			{
				m_layersImage.erase(iter);
			}
			return true;
		}

		return false;
	}

	void CLayerSys::GetImageLayers(CImageLayersSet& vecLayer)
	{
		vecLayer.assign(m_layersImage.begin(), m_layersImage.end());
	}

	void CLayerSys::GetImageLayers( osgEarth::ImageLayerVector& vecLayer )
	{
		vecLayer.assign(m_sysLayerImage.begin(), m_sysLayerImage.end());
	}

	bool CLayerSys::AddModelLayer(IFeLayerModel* pLayer)
	{
		osgEarth::Map* pMap = GetMap();
		CFeLayerModel* pModelLayer = dynamic_cast<CFeLayerModel*>(pLayer);
		if (pMap && pModelLayer)
		{
			pMap->addModelLayer(pModelLayer->GetEarthLayer());
			m_layersModel.push_back(pModelLayer);

			// 在添加图层后调用
			pModelLayer->PostAddLayer(GetMapNode());

			return true;
		}

		return false;
	}

	bool CLayerSys::DeleteModelLayer(IFeLayerModel* pLayer)
	{
		osgEarth::Map* pMap = GetMap();
		CFeLayerModel* pModelLayer = dynamic_cast<CFeLayerModel*>(pLayer);
		if (pMap && pModelLayer)
		{
			pMap->removeModelLayer(pModelLayer->GetEarthLayer());

			CModelLayersSet::iterator iter = std::find(m_layersModel.begin(), m_layersModel.end(), pModelLayer);
			if(iter != m_layersModel.end())
			{
				m_layersModel.erase(iter);
			}
			return true;
		}

		return false;
	}

	void CLayerSys::GetModelLayers(CModelLayersSet& vecLayer)
	{
		vecLayer.assign(m_layersModel.begin(), m_layersModel.end());
	}

	void CLayerSys::GetModelLayers( osgEarth::ModelLayerVector& vecLayer )
	{
		vecLayer.assign(m_sysLayerModel.begin(), m_sysLayerModel.end());
	}

	bool CLayerSys::AddElevationLayer( IFeLayerElevation* pLayer )
	{
		osgEarth::Map* pMap = GetMap();
		CFeLayerElevation* pElevationLayer = dynamic_cast<CFeLayerElevation*>(pLayer);
		if (pMap && pElevationLayer)
		{
			m_layersElevation.push_back(pElevationLayer);

			//设置缓存ID
			{
				CFeLayerImage::OsgEarthLayerType *layerAgent = dynamic_cast<CFeLayerImage::OsgEarthLayerType*>(pElevationLayer->GetEarthLayer());
				if (NULL != layerAgent)
				{
					layerAgent->SetCacheID( GenLayerCacheID(pLayer, m_opRenderContext->GetMapSRS()->getName()) );
				}
			}

			pMap->addElevationLayer(pElevationLayer->GetEarthLayer());

			return true;
		}

		return false;
	}

	bool CLayerSys::DeleteElevationLayer( IFeLayerElevation* pLayer )
	{
		osgEarth::Map* pMap = GetMap();
		CFeLayerElevation* pElevationLayer = dynamic_cast<CFeLayerElevation*>(pLayer);
		if (pMap && pElevationLayer)
		{
			pMap->removeElevationLayer(pElevationLayer->GetEarthLayer());

			CElevationLayersSet::iterator iter = std::find(m_layersElevation.begin(), m_layersElevation.end(), pElevationLayer);
			if(iter != m_layersElevation.end())
			{
				m_layersElevation.erase(iter);
			}
			return true;
		}

		return false;
	}

	void CLayerSys::GetElevationLayers( CElevationLayersSet& vecLayer )
	{
		vecLayer.assign(m_layersElevation.begin(), m_layersElevation.end());
	}

	void CLayerSys::GetElevationLayers( osgEarth::ElevationLayerVector& vecLayer )
	{
		vecLayer.assign(m_sysLayerElevation.begin(), m_sysLayerElevation.end());
	}

	bool CLayerSys::AddEffectLayer(IFeLayerEffect* pLayer)
	{
		if (pLayer)
		{
			m_mapEffectLayer.push_back(pLayer);

			return true;
		}

		return false;
	}

	IFeLayerEffect* CLayerSys::AddEffectLayer(osgEarth::TerrainEffect* pTerrainEffect, const std::string& strName, bool bVisible)
	{
		osg::ref_ptr<osgEarth::MapNode> rpMapNode = GetMapNode();
		if (false == rpMapNode.valid())
		{
			return NULL;
		}

		osg::ref_ptr<CFeEffectLayer> pEffectLayer = new CFeEffectLayer(pTerrainEffect, rpMapNode->getTerrainEngine(), strName, bVisible);
		if (AddEffectLayer(pEffectLayer))
		{
			return pEffectLayer;
		}
		
		return NULL;
	}

	bool CLayerSys::DeleteEffectLayer(IFeLayerEffect* pLayer)
	{
		if (pLayer)
		{
			pLayer->SetVisible(false);
		}

		CEffectLayersSet::iterator it = m_mapEffectLayer.begin();
		while (it != m_mapEffectLayer.end())
		{
			if (*it == pLayer)
			{
				m_mapEffectLayer.erase(it);
				
				return true;
			}
			it++;
		}

		return false;
	}

	IFeLayerEffect* CLayerSys::GetEffectLayer(const std::string& strName)
	{
		CEffectLayersSet::iterator it = m_mapEffectLayer.begin();
		while (it != m_mapEffectLayer.end())
		{
			IFeLayerEffect* pLayer = *it;
			if (pLayer && 0 == strName.compare(pLayer->GetName()))
			{
				return pLayer;
			}

			it++;
		}

		return NULL;
	}

	void CLayerSys::GetEffectLayers( CEffectLayersSet& vecLayer )
	{
		vecLayer.assign(m_mapEffectLayer.begin(), m_mapEffectLayer.end());
	}

	void CLayerSys::AddImageColorFilter(osgEarth::ColorFilter* pColorFilter)
	{
		if (pColorFilter)
		{
			// 系统图层
			for(osgEarth::ImageLayerVector::iterator iter = m_sysLayerImage.begin(); 
				iter != m_sysLayerImage.end(); iter++)
			{
				(*iter)->addColorFilter(pColorFilter);
			}

			// 用户图层
			CImageLayersSet vecLayer;
			GetImageLayers(vecLayer);

			for (CImageLayersSet::iterator itr = vecLayer.begin(); itr != vecLayer.end(); ++itr)
			{
				osgEarth::ImageLayer* pLayer = (*itr)->GetEarthImageLayer();
				if (pLayer /* && pLayer->getEnabled() && pLayer->getVisible()*/)
				{
					pLayer->addColorFilter(pColorFilter);
				}
			}
		}
	}

	void CLayerSys::RemoveImageColorFilter(osgEarth::ColorFilter* pColorFilter)
	{
		if (pColorFilter)
		{
			// 系统图层
			for(osgEarth::ImageLayerVector::iterator iter = m_sysLayerImage.begin(); 
				iter != m_sysLayerImage.end(); iter++)
			{
				(*iter)->removeColorFilter(pColorFilter);
			}

			// 用户图层
			CImageLayersSet vecLayer;
			GetImageLayers(vecLayer);

			for (CImageLayersSet::iterator itr = vecLayer.begin(); itr != vecLayer.end(); ++itr)
			{
				osgEarth::ImageLayer* pLayer = (*itr)->GetEarthImageLayer();
				if (pLayer /* && pLayer->getEnabled() && pLayer->getVisible()*/)
				{
					pLayer->removeColorFilter(pColorFilter);
				}
			}
		}
	}

	void CLayerSys::RemoveImageColorFilters()
	{
		// 系统图层
		for(osgEarth::ImageLayerVector::iterator iterLayer = m_sysLayerImage.begin(); 
			iterLayer != m_sysLayerImage.end(); iterLayer++)
		{
			const osgEarth::ColorFilterChain  colorFilterChain = (*iterLayer)->getColorFilters();
			for (osgEarth::ColorFilterChain::const_iterator iter = colorFilterChain.begin(); iter != colorFilterChain.end(); ++iter)
			{
				(*iterLayer)->removeColorFilter(iter->get());
			}
		}

		// 用户图层
		CImageLayersSet vecLayer;
		GetImageLayers(vecLayer);

		for (CImageLayersSet::iterator itr = vecLayer.begin(); itr != vecLayer.end(); ++itr)
		{
			osgEarth::ImageLayer* pLayer = (*itr)->GetEarthImageLayer();
			if (pLayer/* && pLayer->getEnabled() && pLayer->getVisible()*/)
			{
				const osgEarth::ColorFilterChain  colorFilterChain = pLayer->getColorFilters();
				for (osgEarth::ColorFilterChain::const_iterator iter = colorFilterChain.begin(); iter != colorFilterChain.end(); ++iter)
				{
					pLayer->removeColorFilter(iter->get());
				}
			}
		}
	}

	
	void CLayerSys::RegistryEffectLayer()
	{
		osgEarth::MapNode* pMapNode = m_opRenderContext->GetMapNode();
		if (!pMapNode)
		{
			return;
		}

		osgEarth::TerrainEngineNode* pEngineNode = pMapNode->getTerrainEngine();
		if (!pEngineNode)
		{
			return;
		}

		//支持normalMap效果
		const osgEarth::Config& normalMapConf = pMapNode->externalConfig().child("normal_map");
		if (!normalMapConf.empty())
		{
			std::string strEffectName = FeUtil::ToUTF("法线光照特效");
			osg::ref_ptr<CNormalMapEffect> rpNormalEffect = new CNormalMapEffect(normalMapConf, pMapNode->getMap());

			AddEffectLayer(new CFeEffectLayer(rpNormalEffect, pMapNode->getTerrainEngine(), strEffectName, false));

		}

		//支持lodBlending效果
		const osgEarth::Config& lodBlendingConf = pMapNode->externalConfig().child("lod_blending");
		if (!lodBlendingConf.empty())
		{
			std::string strEffectName = FeUtil::ToUTF("LOD融合特效");
			osg::ref_ptr<osgEarth::Util::LODBlending> rpLodBlending = new osgEarth::Util::LODBlending(lodBlendingConf);

			AddEffectLayer(new CFeEffectLayer(rpLodBlending, pMapNode->getTerrainEngine(), strEffectName, false));
		}

		//支持contourMap晕渲图效果
		const osgEarth::Config& contourMapConf = pMapNode->externalConfig().child("contour_map");
		if (!contourMapConf.empty())
		{
			std::string strEffectName = FeUtil::ToUTF("眩晕特效");
			osg::ref_ptr<osgEarth::Util::ContourMap> rpContourMap = new osgEarth::Util::ContourMap(contourMapConf);

			AddEffectLayer(new CFeEffectLayer(rpContourMap, pMapNode->getTerrainEngine(), strEffectName, false));
		}
	}

	void CLayerSys::RegistryFeatureLayer()
	{
		//将FreeEarth配置文件路径配置到osgDB的filepath中，用于win10以及linux上读取字体使用
		osgDB::FilePathList list = osgDB::Registry::instance()->getDataFilePathList();
		osgDB::FilePathList listNew;
		for (int i = 0; i < list.size(); i++)
		{
			listNew.push_back(list.at(i));
		}
		listNew.push_back(FeFileReg->GetDataPath());
		osgDB::Registry::instance()->setDataFilePathList(listNew);

		osgEarth::ConfigSet featureSets;

		CFeatureReaderWriter reader;
		reader.Read(m_optLayerConfig.featurePath(), featureSets);

		osgEarth::ConfigSet::iterator it = featureSets.begin();
		while (it != featureSets.end())
		{
			osgEarth::Config config = *it;
			config.setReferrer(m_optLayerConfig.featurePath());

			osgEarth::Config featuresConf = config.child("features");

			osgEarth::Config urlConf = featuresConf.child("url");

			featuresConf.updateIfSet("url", osgEarth::optional<osgEarth::Config>(urlConf, urlConf));
			config.updateIfSet("features", osgEarth::optional<osgEarth::Config>(featuresConf, featuresConf));

			osgEarth::ModelLayerOptions layerOpt( config );
			// 系统图层不再由图层系统管理，图层系统只管理用户图层
			//AddModelLayer(layerOpt);
			osg::ref_ptr<osgEarth::ModelLayer> rpModelLayer = new osgEarth::ModelLayer(layerOpt);
			if( GetMap() )
			{
				GetMap()->addModelLayer(rpModelLayer);
				m_sysLayerModel.push_back(rpModelLayer);
			}

			it++;
		}
	}

	osgEarth::Map* CLayerSys::GetMap()
	{
		if (false == m_opRenderContext.valid())
		{
			return NULL;
		}

		osgEarth::MapNode* pMapNode = m_opRenderContext->GetMapNode();
		if (NULL == pMapNode)
		{
			return NULL;
		}

		return pMapNode->getMap();
	}

	osgEarth::MapNode* CLayerSys::GetMapNode()
	{
		if (false == m_opRenderContext.valid())
		{
			return NULL;
		}

		return m_opRenderContext->GetMapNode();
	}

	CLayerSysOptions& CLayerSys::GetOptions()
	{
		return m_optLayerConfig;
	}

	ILayerFactory* CLayerSys::LayerFactory()
	{
		return LayerFactorySingle();
	}

}

	


