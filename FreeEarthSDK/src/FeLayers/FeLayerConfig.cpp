#include <FeLayerConfig.h>

using namespace FeLayers;


//////////////////////////////////////////////////////////////////////////
// CFeDataSourceConfig
CFeDataSource::CFeDataSource( const CFeConfig& conf /*= CFeConfig() */ )
	: SuperClass(conf)
{
	ParseConfig(_conf);
}

CFeDataSource::CFeDataSource( const CFeDataSource& rhs )
	: SuperClass(rhs.getConfig())
{
	ParseConfig(_conf);
}

CFeConfig CFeDataSource::getConfig() const
{
	CFeConfig conf = SuperClass::getConfig();
	conf.updateIfSet( "driver", m_driver);
	conf.updateIfSet( "url", m_url);
	conf.updateIfSet( "srs", m_srs);
	conf.updateIfSet( "layers", m_layerName);
	conf.updateIfSet( "format", m_layerFormat);
	conf.updateIfSet( "wms_version", m_version);
	
	return conf;
}

void CFeDataSource::FromConfig( const CFeConfig& conf )
{
	ParseConfig(conf);
}

void CFeDataSource::ParseConfig( const CFeConfig& conf )
{
	conf.getIfSet( "driver", m_driver);
	conf.getIfSet( "url", m_url);
	conf.getIfSet( "srs", m_srs);
	conf.getIfSet( "layers", m_layerName);	
	conf.getIfSet( "format", m_layerFormat);
	conf.getIfSet( "wms_version", m_version);
}

CFeDataSource& CFeDataSource::operator=( const CFeDataSource& rhs )
{
	if ( this != &rhs ) 
	{
		_conf = rhs.getConfig();
		ParseConfig(_conf);
	}
	return *this;
}




//////////////////////////////////////////////////////////////////////////
// CFeFeatureSymbol
CFeFeatureSymbol::CFeFeatureSymbol( const CFeFeatureSymbol& rhs )
	: SuperClass( rhs.getConfig() )
{
	ParseConfig(_conf);
}

CFeFeatureSymbol& CFeFeatureSymbol::operator=( const CFeFeatureSymbol& rhs )
{
	if ( this != &rhs ) 
	{
		_conf = rhs.getConfig();
		ParseConfig(_conf);
	}
	return *this;
}

CFeFeatureSymbol::CFeFeatureSymbol( const CFeConfig& conf /*= CFeConfig() */ )
	: SuperClass(conf)
{
	m_unFeatureTypeMask = 0;
	m_optFillColor.init(CFeColor(1,0,1,0.5));
	m_optLineColor.init(CFeColor(1,1,0,1));
	m_optFontColor.init(CFeColor(1,1,1,1));
	m_optFontSize.init(12);
	m_optLineWidth.init(1);
	m_optLineStipple.init(LT_SOLID);
	m_optIconVisible.init(false);
	m_optFontHaloColor.init(CFeColor(0.1,0.1,0.1,0.7));
	m_optClamp.init(AltitudeClamp::CLAMP_TO_TERRAIN);
	m_optTechnique.init(AltitudeTechnique::TECHNIQUE_MAP);

	ParseConfig(conf);
}

CFeConfig CFeFeatureSymbol::getConfig() const
{
	CFeConfig conf = SuperClass::getConfig();
	conf.set( "feature_type_mask", m_unFeatureTypeMask);
	conf.updateIfSet( "line_width", m_optLineWidth);
	conf.updateIfSet( "line_color", m_optLineColor );
	conf.updateIfSet( "line_stipple",  m_optLineStipple );
	conf.updateIfSet( "fill_color", m_optFillColor );
	conf.updateIfSet( "font_name", m_optFontName );
	conf.updateIfSet( "font_size", m_optFontSize );
	conf.updateIfSet( "font_color", m_optFontColor );
	conf.updateIfSet( "icon_path", m_optIconPath);
	conf.updateIfSet( "icon_visible", m_optIconVisible);
	conf.updateIfSet( "font_halo_color", m_optFontHaloColor);
	conf.updateIfSet( "altitude-clamping", m_optClamp);
	conf.updateIfSet( "altitude-technique", m_optTechnique);

	return conf;
}

void CFeFeatureSymbol::ParseConfig( const CFeConfig& conf )
{
	conf.getIfSet( "feature_type_mask", m_unFeatureTypeMask);
	conf.getIfSet( "line_width", m_optLineWidth);
	conf.getIfSet( "line_color", m_optLineColor );
	conf.getIfSet( "line_stipple",  m_optLineStipple );
	conf.getIfSet( "fill_color", m_optFillColor );
	conf.getIfSet( "font_name", m_optFontName );
	conf.getIfSet( "font_size", m_optFontSize );
	conf.getIfSet( "font_color", m_optFontColor );
	conf.getIfSet( "icon_path", m_optIconPath);
	conf.getIfSet( "icon_visible", m_optIconVisible);
	conf.getIfSet( "font_halo_color", m_optFontHaloColor);
	conf.getIfSet( "altitude-clamping", m_optClamp);
	conf.getIfSet( "altitude-technique", m_optTechnique);
}






//////////////////////////////////////////////////////////////////////////
// CFeLayerConfig
CFeLayerConfig::CFeLayerConfig( const CFeLayerConfig& rhs ) 
	: SuperClass( rhs.getConfig() )
{
	ParseConfig(_conf);
	m_layerType = rhs.m_layerType;
}

CFeLayerConfig::CFeLayerConfig( const CFeConfig& conf /*= CFeConfig() */ ) 
	: SuperClass( conf )
{
	m_visible = true;
	m_layerType = E_LAYER_IMAGE;
	ParseConfig(_conf);
}

CFeLayerConfig& CFeLayerConfig::operator=( const CFeLayerConfig& rhs )
{
	if ( this != &rhs ) 
	{
		_conf = rhs.getConfig();
		ParseConfig(_conf);
		m_layerType = rhs.m_layerType;
	}
	return *this;
}

CFeConfig CFeLayerConfig::getConfig() const
{
	CFeConfig conf = SuperClass::getConfig();
	conf.updateIfSet( "name", m_name);
	conf.updateIfSet( "visible", m_visible );
	conf.set( "layer-type", (int)m_layerType);
// 	if(sourceConfig().layerFormat().isSet() && (0 == sourceConfig().layerFormat()->compare("png")))
// 	{
// 		conf.set( "hastransparency", true);
// 	}

	conf.updateObj("data_source", m_sourceCfg);

	return conf;
}

void CFeLayerConfig::ParseConfig( const CFeConfig& conf )
{
	conf.getIfSet( "name", m_name);
	conf.getIfSet( "visible", m_visible );
	int layerType = m_layerType;
	conf.getIfSet( "layer-type", layerType);
	m_layerType = (E_Layer_Type)layerType;
	conf.getObjIfSet("data_source", m_sourceCfg);
}

void CFeLayerConfig::FromConfig( const CFeConfig& conf  )
{
	ParseConfig( conf );
}



//////////////////////////////////////////////////////////////////////////
// CCacheConfig
CCacheConfig::CCacheConfig( const CCacheConfig& rhs )
	: SuperClass( rhs.getConfig() )
{
	FromConfig(_conf);
}

FeLayers::CCacheConfig::CCacheConfig( const CFeConfig& conf /*= CFeConfig() */ )
{
	m_enablePath = true;
}

CCacheConfig& CCacheConfig::operator=( const CCacheConfig& rhs )
{
	if ( this != &rhs ) 
	{
		_conf = rhs.getConfig();
		FromConfig(_conf);
	}
	return *this;
}

CFeConfig CCacheConfig::getConfig() const
{
	CFeConfig conf = SuperClass::getConfig();
	conf.updateIfSet( "cachepath", m_cachePath);
	conf.updateIfSet( "cache_enable", m_enablePath);
	conf.updateIfSet( "cache_name", m_cacheName);

	return conf;
}

void CCacheConfig::FromConfig( const CFeConfig& conf )
{
	conf.getIfSet( "cachepath", m_cachePath);
	conf.getIfSet( "cache_enable", m_enablePath);
	conf.getIfSet( "cache_name", m_cacheName);
}



//////////////////////////////////////////////////////////////////////////
// CVisibleConfig
CVisibleConfig::CVisibleConfig( const CVisibleConfig& rhs )
	: SuperClass( rhs.getConfig() )
{
	FromConfig(_conf);
}

CVisibleConfig::CVisibleConfig( const CFeConfig& conf /*= CFeConfig() */ )
{
	m_opacity = 1.0;
	m_minRange = 0;
	m_maxRange = FLT_MAX;
}

CVisibleConfig& CVisibleConfig::operator=( const CVisibleConfig& rhs )
{
	if ( this != &rhs ) 
	{
		_conf = rhs.getConfig();
		FromConfig(_conf);
	}
	return *this;
}

CFeConfig CVisibleConfig::getConfig() const
{
	CFeConfig conf = SuperClass::getConfig();
	conf.updateIfSet( "opacity",  m_opacity );
	conf.updateIfSet( "min_range", m_minRange );
	conf.updateIfSet( "max_range", m_maxRange );
	conf.updateIfSet( "min_level", m_minLevel );
	conf.updateIfSet( "max_level", m_maxLevel );
	conf.updateIfSet( "hastransparency", m_enableOpacity );
	
	return conf;
}

void CVisibleConfig::FromConfig( const CFeConfig& conf )
{
	conf.getIfSet( "opacity", m_opacity );
	conf.getIfSet( "min_range", m_minRange );
	conf.getIfSet( "max_range", m_maxRange );
	conf.getIfSet( "min_level", m_minLevel );
	conf.getIfSet( "max_level", m_maxLevel );
	conf.getIfSet( "hastransparency", m_enableOpacity );
}



//////////////////////////////////////////////////////////////////////////
// CFeLayerImageConfig
CFeLayerImageConfig::CFeLayerImageConfig( const CFeLayerImageConfig& rhs )
	: SuperClass( rhs.getConfig() )
{
	m_layerType = E_LAYER_IMAGE;
	FromConfig(_conf);
}

FeLayers::CFeLayerImageConfig::CFeLayerImageConfig( const CFeConfig& conf /*=CFeConfig() */ )
	: SuperClass(conf)
{
	m_layerType = E_LAYER_IMAGE;
	FromConfig(_conf);
}

FeLayers::CFeLayerImageConfig::CFeLayerImageConfig( const std::string& layerName, const std::string& driverName, const std::string& url ) 
{
	m_layerType = E_LAYER_IMAGE;
	name() = layerName;
	sourceConfig().driver() = driverName;
	sourceConfig().url() = url;
}

CFeLayerImageConfig& CFeLayerImageConfig::operator=( const CFeLayerImageConfig& rhs )
{
	if ( this != &rhs ) 
	{
		_conf = rhs.getConfig();
		FromConfig(_conf);
	}
	return *this;
}

CFeConfig CFeLayerImageConfig::getConfig() const
{
	CFeConfig conf = SuperClass::getConfig();
	conf.updateIfSet( "baidu_online", m_baiduOnline);
	conf.merge(m_cfgVisible.getConfig());
	conf.merge(m_cfgCache.getConfig());

	return conf;
}

void CFeLayerImageConfig::FromConfig( const CFeConfig& conf )
{
	SuperClass::FromConfig(conf);
	conf.getIfSet( "baidu_online", m_baiduOnline);
	m_cfgVisible.FromConfig(conf);
	m_cfgCache.FromConfig(conf);
}



//////////////////////////////////////////////////////////////////////////
// CFeLayerModelConfig
CFeLayerModelConfig::CFeLayerModelConfig( const CFeLayerModelConfig& rhs )
	: SuperClass( rhs.getConfig() )
{
	m_layerType = E_LAYER_MODEL;
	FromConfig(_conf);
}

FeLayers::CFeLayerModelConfig::CFeLayerModelConfig( const CFeConfig& conf /*=CFeConfig() */ )
	: SuperClass(conf)
{
	m_layerType = E_LAYER_MODEL;
	FromConfig(_conf);
}

FeLayers::CFeLayerModelConfig::CFeLayerModelConfig( const std::string& layerName, const std::string& driverName, const std::string& url ) 
{
	m_layerType = E_LAYER_MODEL;
	name() = layerName;
	sourceConfig().driver() = driverName;
	sourceConfig().url() = url;
}

CFeLayerModelConfig& CFeLayerModelConfig::operator=( const CFeLayerModelConfig& rhs )
{
	if ( this != &rhs ) 
	{
		_conf = rhs.getConfig();
		FromConfig(_conf);
	}
	return *this;
}

CFeConfig CFeLayerModelConfig::getConfig() const
{
	CFeConfig conf = SuperClass::getConfig();
	conf.updateObj("feature_symbol", m_featureSymbol);
	conf.merge(m_cfgVisible.getConfig());
	return conf;
}

void CFeLayerModelConfig::FromConfig( const CFeConfig& conf )
{
	SuperClass::FromConfig(conf);
	conf.getObjIfSet("feature_symbol", m_featureSymbol);
	m_cfgVisible.FromConfig(conf);
}



//////////////////////////////////////////////////////////////////////////
// CFeLayerElevationConfig
CFeLayerElevationConfig::CFeLayerElevationConfig( const CFeLayerElevationConfig& rhs )
	: SuperClass( rhs.getConfig() )
{
	m_layerType = E_LAYER_ELEVATION;
	FromConfig(_conf);
}

FeLayers::CFeLayerElevationConfig::CFeLayerElevationConfig( const CFeConfig& conf /*=CFeConfig() */ )
	: SuperClass(conf)
{
	m_layerType = E_LAYER_ELEVATION;
	FromConfig(_conf);
}

FeLayers::CFeLayerElevationConfig::CFeLayerElevationConfig( const std::string& layerName, const std::string& driverName, const std::string& url ) 
{
	m_layerType = E_LAYER_ELEVATION;
	name() = layerName;
	sourceConfig().driver() = driverName;
	sourceConfig().url() = url;
}

CFeLayerElevationConfig& CFeLayerElevationConfig::operator=( const CFeLayerElevationConfig& rhs )
{
	if ( this != &rhs ) 
	{
		_conf = rhs.getConfig();
		FromConfig(_conf);
	}
	return *this;
}

CFeConfig CFeLayerElevationConfig::getConfig() const
{
	CFeConfig conf = SuperClass::getConfig();
	conf.merge(m_cfgCache.getConfig());

	return conf;
}

void CFeLayerElevationConfig::FromConfig( const CFeConfig& conf )
{
	SuperClass::FromConfig(conf);
	m_cfgCache.FromConfig(conf);
}



//////////////////////////////////////////////////////////////////////////
// CFeLayerEffectConfig
CFeLayerEffectConfig::CFeLayerEffectConfig( const CFeConfig& conf /*=CFeConfig() */ )
	: SuperClass(conf)
{
	m_layerType = E_LAYER_EFFECT;
	FromConfig(_conf);
}

CFeLayerEffectConfig::CFeLayerEffectConfig( const CFeLayerEffectConfig& rhs )
	: SuperClass( rhs.getConfig() )
{
	m_layerType = E_LAYER_EFFECT;
	FromConfig(_conf);
}

CFeLayerEffectConfig& CFeLayerEffectConfig::operator=( const CFeLayerEffectConfig& rhs )
{
	if ( this != &rhs ) 
	{
		_conf = rhs.getConfig();
		FromConfig(_conf);
	}
	return *this;
}

FeLayers::CFeConfig CFeLayerEffectConfig::getConfig() const
{
	CFeConfig conf = SuperClass::getConfig();
	conf.addNonSerializable("terrain_engine_node", m_opTerrainNode.get());
	conf.addNonSerializable("terrain_effect", m_rpEffectLayer.get());
	return conf;
}

void CFeLayerEffectConfig::FromConfig( const CFeConfig& conf )
{
	SuperClass::FromConfig(conf);
	m_opTerrainNode = conf.getNonSerializable<osgEarth::TerrainEngineNode>("terrain_engine_node");
	m_rpEffectLayer = conf.getNonSerializable<osgEarth::TerrainEffect>("terrain_effect");
}

