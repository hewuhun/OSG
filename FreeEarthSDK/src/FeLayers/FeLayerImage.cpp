#include <FeLayerImage.h>

using namespace FeLayers;


//////////////////////////////////////////////////////////////////////////
// CFeLayerImage
CFeLayerImage::CFeLayerImage( const CFeLayerImageConfig& cfg )
	: SuperClass(m_layerConfig)
{
	m_layerConfig = cfg;

	osgEarth::ImageLayerOptions layerOpt( m_layerConfig.getConfig() );
	layerOpt.driver() = osgEarth::TileSourceOptions(m_layerConfig.sourceConfig().getConfig());

	m_rpLayerEarth = new OsgEarthLayerType(layerOpt);
	m_rpLayerEarth->EnabelCache(m_layerConfig.cacheConfig().cacheEnable().get());
	// 设置缓存以生成正确的路径
	SetCachePath(m_layerConfig.cacheConfig().cachePath().get());
}

CFeLayerImage::~CFeLayerImage()
{

}

void FeLayers::CFeLayerImage::setOpacity( float opacity )
{
	m_layerConfig.visibleConfig().opacity() = opacity;
	if(m_rpLayerEarth.valid()) m_rpLayerEarth->setOpacity(opacity);
}

float FeLayers::CFeLayerImage::getOpacity() const
{
	return m_layerConfig.visibleConfig().opacity().get();
}

void FeLayers::CFeLayerImage::GetVisibleRange( float& minVal, float& maxVal )
{
	minVal = m_layerConfig.visibleConfig().minVisibleRange().get();
	maxVal = m_layerConfig.visibleConfig().maxVisibleRange().get();
}

void FeLayers::CFeLayerImage::SetVisibleRange( float minVal, float maxVal )
{
	//SuperClass::SetVisibleRange(minVal, maxVal);
	m_layerConfig.visibleConfig().minVisibleRange() = minVal;
	m_layerConfig.visibleConfig().maxVisibleRange() = maxVal;

	if(m_rpLayerEarth.valid())
	{
		m_rpLayerEarth->setMinVisibleRange(minVal);
		m_rpLayerEarth->setMaxVisibleRange(maxVal);
	}
}

void FeLayers::CFeLayerImage::SetCacheEnable( bool bEnable )
{
	if(m_rpLayerEarth.valid())
	{
		m_layerConfig.cacheConfig().cacheEnable() = bEnable;
		m_rpLayerEarth->EnabelCache(bEnable);
	}
}

bool FeLayers::CFeLayerImage::GetCacheEnable() const
{
	return m_layerConfig.cacheConfig().cacheEnable().get();
}

void FeLayers::CFeLayerImage::SetCachePath( const std::string& path )
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

std::string FeLayers::CFeLayerImage::GetCachePath() const
{
	return m_layerConfig.cacheConfig().cachePath().get();
}

void FeLayers::CFeLayerImage::SetCacheName( const std::string& name )
{
	if(name != m_layerConfig.cacheConfig().cacheName().get())
	{
		m_layerConfig.cacheConfig().cacheName() = name;
		SetCachePath(m_layerConfig.cacheConfig().cachePath().get());
	}
}

std::string FeLayers::CFeLayerImage::GetCacheName() const
{
	return m_layerConfig.cacheConfig().cacheName().get();
}

void FeLayers::CFeLayerImage::SetName( const std::string& strName )
{
	SuperClass::SetName(strName);

	if(m_rpLayerEarth.valid())
	{
		m_rpLayerEarth->setName(strName);
	}
}
