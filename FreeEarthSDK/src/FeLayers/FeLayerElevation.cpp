#include <FeLayerElevation.h>

using namespace FeLayers;


//////////////////////////////////////////////////////////////////////////
// CFeLayerElevation
CFeLayerElevation::CFeLayerElevation( const CFeLayerElevationConfig& cfg )
	: SuperClass(m_layerConfig)
{
	m_layerConfig = cfg;

	osgEarth::ElevationLayerOptions layerOpt( m_layerConfig.getConfig() );
	layerOpt.driver() = osgEarth::TileSourceOptions(m_layerConfig.sourceConfig().getConfig());

	m_rpLayerEarth = new OsgEarthLayerType(layerOpt);
	m_rpLayerEarth->EnabelCache(m_layerConfig.cacheConfig().cacheEnable().get());
	// 设置缓存以生成正确的路径
	SetCachePath(m_layerConfig.cacheConfig().cachePath().get());
}

CFeLayerElevation::~CFeLayerElevation()
{

}

void FeLayers::CFeLayerElevation::SetCacheEnable( bool bEnable )
{
	if(m_rpLayerEarth.valid())
	{
		m_layerConfig.cacheConfig().cacheEnable() = bEnable;
		m_rpLayerEarth->EnabelCache(bEnable);
	}
}

bool FeLayers::CFeLayerElevation::GetCacheEnable() const
{
	return m_layerConfig.cacheConfig().cacheEnable().get();
}

void FeLayers::CFeLayerElevation::SetCachePath( const std::string& path )
{
	if(m_rpLayerEarth.valid())
	{
		m_layerConfig.cacheConfig().cachePath() = path;

		// 缓存真实路径 = 缓存路径 + 缓存名称
		std::string cachePath = path;
		if(!m_layerConfig.cacheConfig().cacheName()->empty())
		{
			cachePath += "/" + m_layerConfig.cacheConfig().cacheName().get();
		}
		m_rpLayerEarth->SetCachePath(cachePath);
	}
}

std::string FeLayers::CFeLayerElevation::GetCachePath() const
{
	return m_layerConfig.cacheConfig().cachePath().get();
}

void FeLayers::CFeLayerElevation::SetCacheName( const std::string& name )
{
	if(name != m_layerConfig.cacheConfig().cacheName().get())
	{
		m_layerConfig.cacheConfig().cacheName() = name;
		SetCachePath(m_layerConfig.cacheConfig().cachePath().get());
	}
}

std::string FeLayers::CFeLayerElevation::GetCacheName() const
{
	return m_layerConfig.cacheConfig().cacheName().get();
}

void FeLayers::CFeLayerElevation::SetName( const std::string& strName )
{
	SuperClass::SetName(strName);

	if(m_rpLayerEarth.valid())
	{
		m_rpLayerEarth->setName(strName);
	}
}


