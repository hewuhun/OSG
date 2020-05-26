#include <FeLayers/FeEffectLayer.h>

#include <osgEarth/Registry>



namespace FeLayers
{
	CFeEffectLayer::CFeEffectLayer( 
		osgEarth::TerrainEffect* pEffectLayer, 
		osgEarth::TerrainEngineNode* pTerrainNode, 
		const std::string& strName, 
		bool bVisible )
		: m_rpEffectLayer(pEffectLayer)
		,m_bVisible(bVisible)
		,m_opTerrainNode(pTerrainNode)
		,m_strName(strName)
	{
		SetVisible(m_bVisible);
	}

	CFeEffectLayer::CFeEffectLayer( const CFeLayerEffectConfig& cfg )
	{
		m_cfg = cfg;

		m_rpEffectLayer = m_cfg.GetTerrainEffect();
		m_opTerrainNode = m_cfg.GetTerrainEngineNode();

		SetVisible(m_cfg.visible().get());
	}

	CFeEffectLayer::~CFeEffectLayer()
	{

	}

	void CFeEffectLayer::SetVisible( bool bVisible )
	{
		if(m_opTerrainNode.valid() && m_rpEffectLayer.valid())
		{
			if(bVisible)
			{
				m_rpEffectLayer->onInstall(m_opTerrainNode.get());
			}
			else
			{
				m_rpEffectLayer->onUninstall(m_opTerrainNode.get());
			}
		}

		m_bVisible = bVisible;
		m_cfg.visible() = bVisible;
	}

	bool CFeEffectLayer::GetVisible() const
	{
		return m_bVisible;
	}

	void CFeEffectLayer::SetEffectLayer( osgEarth::TerrainEffect* pEffectLayer )
	{
		m_rpEffectLayer = pEffectLayer;
		m_cfg.SetTerrainEffect(pEffectLayer);
	}

	void CFeEffectLayer::SetTerrainEngineNode( osgEarth::TerrainEngineNode* pTerrainNode )
	{
		m_opTerrainNode = pTerrainNode;
		m_cfg.SetTerrainEngineNode(pTerrainNode);
	}

	const std::string& CFeEffectLayer::GetName() const
	{
		return m_strName;
	}

	void CFeEffectLayer::SetName( const std::string& strName )
	{
		m_strName = strName;
		m_cfg.name() = strName;
	}

	osgEarth::TerrainEffect* CFeEffectLayer::GetEffectLayer()
	{
		return m_rpEffectLayer.get();
	}

	FeLayerID CFeEffectLayer::GetLayerUID()
	{
		return osgEarth::Registry::instance()->createUID();//getUID();
	}

	CFeLayerConfig CFeEffectLayer::GetLayerConfig()
	{
		return m_cfg;
	}

}


