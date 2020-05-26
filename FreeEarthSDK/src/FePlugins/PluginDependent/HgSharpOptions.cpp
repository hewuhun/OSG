#include <PluginDependent/HgSharpOptions.h>


namespace FePlugin
{


	CHgSharpOptions::CHgSharpOptions( const TileSourceOptions& opt )
		: TileSourceOptions( opt )
	{
		setDriver( "hggis" );
		FromConfig( _conf );
	}


	CHgSharpOptions::CHgSharpOptions( const CHgSharpOptions& rhs ) 
		:m_urlPath(rhs.m_urlPath),
		m_strToken(rhs.m_strToken), 
		m_strFormat(rhs.m_strFormat), 
		m_strCachePath(rhs.m_strCachePath)
	{

	}


	CHgSharpOptions::~CHgSharpOptions() 
	{

	}


	Config CHgSharpOptions::GetConfig() const 
	{
		Config conf = TileSourceOptions::getConfig();
		conf.updateIfSet("url", m_urlPath );
		conf.updateIfSet("token", m_strToken );
		conf.updateIfSet("format", m_strFormat );
		conf.updateIfSet( "cachePath", m_strCachePath);
		return conf;
	}

	void CHgSharpOptions::MergeConfig( const Config& conf )
	{
		TileSourceOptions::mergeConfig( conf );
		FromConfig( conf );
	}

	void CHgSharpOptions::FromConfig( const Config& conf ) 
	{
		conf.getIfSet( "url", m_urlPath );
		conf.getIfSet( "token", m_strToken);
		conf.getIfSet( "format", m_strFormat);
		conf.getIfSet( "cachePath", m_strCachePath);
	}
}