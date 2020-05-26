#ifndef TILE_SOURCE_OPTIONS_WMTS_H
#define TILE_SOURCE_OPTIONS_WMTS_H
		
#include <osgEarth/URI>
#include <osgEarth/TileSource>

using namespace osgEarth;

class CTileSourceOptionsWMTS : public TileSourceOptions
{
public:
	optional<URI>&		 GetURL()		{ return m_url; }
	const optional<URI>& GetURL() const	{ return m_url; }

	optional<std::string>&		 GetLayer()		{ return m_strLayer; }
	const optional<std::string>& GetLayer() const	{ return m_strLayer; }

	optional<std::string>&		 GetSRS()		{ return m_strSRS; }
	const optional<std::string>& GetSRS() const	{ return m_strSRS; }

	optional<std::string>&		 GetFormat()		{ return m_strFormat; }
	const optional<std::string>& GetFormat() const	{ return m_strFormat; }

	optional<int>&		 GetTileSize()		{ return m_nTileSize; }
	const optional<int>& GetTileSize() const	{ return m_nTileSize; }
	
public:
    CTileSourceOptionsWMTS(const TileSourceOptions& options =TileSourceOptions() ) :
		TileSourceOptions( options )
		{
            setDriver( "wmts" );
            fromConfig(_conf);
		}

virtual ~CTileSourceOptionsWMTS() {}

public:

	Config getConfig() const
	{
		Config conf = TileSourceOptions::getConfig();
		conf.updateIfSet( "url", m_url );
		conf.updateIfSet( "layers", m_strLayer );
		conf.updateIfSet( "srs", m_strSRS );
		conf.updateIfSet( "format", m_strFormat );
		conf.updateIfSet( "tile_size", m_nTileSize );
		
		return conf;
	}

	void mergeConfig( const Config& conf ) 
	{
		TileSourceOptions::mergeConfig( conf );
		fromConfig( conf );
	}

	void fromConfig( const Config& conf ) 
	{
		conf.getIfSet( "url", m_url );
		conf.getIfSet( "layers", m_strLayer );
		conf.getIfSet( "srs", m_strSRS );
		conf.getIfSet( "format", m_strFormat );
		conf.getIfSet( "tile_size", m_nTileSize );
	}

protected:
	optional<URI>			m_url;  // 数据路径
	optional<std::string>	m_strLayer;
	optional<std::string>	m_strSRS;
	optional<std::string>	m_strFormat;
	optional<int>			m_nTileSize;
};


#endif
