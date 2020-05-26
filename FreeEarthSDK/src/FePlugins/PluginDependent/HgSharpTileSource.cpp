#include <PluginDependent/HgSharpTileSource.h>


namespace FePlugin
{



	CHgSharpTileSource::CHgSharpTileSource( const TileSourceOptions& options ) 
		:TileSource( options ),
		m_hgOptions( options ),
		m_profileConf( ProfileOptions() )
	{
	}


	osgEarth::TileSource::Status CHgSharpTileSource::initialize( const osgDB::Options* dbOptions )
	{
		//如果配置项中有设置安全标记，则为Url增加安全的标记
		URI url = m_hgOptions.GetUrl().value();
		if (m_hgOptions.GetToken().isSet())
		{
			std::string token = m_hgOptions.GetToken().value();
			if (!token.empty())
			{
				std::string sep = url.full().find( "?" ) == std::string::npos ? "?" : "&";
				url = url.append( sep + std::string("token=") + token );
			}
		}

		//从影像服务器的配置文件中读取gis服务的配置项，如果初始化失败则返回
		if ( !m_hgGisService.Init(url, dbOptions) )
		{
			return Status::Error( Stringify()
				<< "[osgearth] [HGGIS] gis service initialization failed: "
				<< m_hgGisService.GetError() );
		}

		//创建一个本地的配置项，由于缓存将由TerrainLayer处理，所以将缓存策略设置为没有缓存
		m_hgDbOptions = Registry::instance()->cloneOrCreateOptions( dbOptions );
		CachePolicy::NO_CACHE.apply( m_hgDbOptions.get() );

		//如果还没有Profile，则创建一个Profile
		if ( !getProfile() )
		{
			const Profile* profile = NULL;

			if ( m_profileConf.isSet() )
			{
				profile = Profile::create( m_profileConf.get() );
			}
			else if ( m_hgGisService.GetProfile() )
			{
				profile = m_hgGisService.GetProfile();
			}
			else
			{
				// finally, fall back on lat/long
				profile = osgEarth::Registry::instance()->getGlobalGeodeticProfile();
			}
			setProfile( profile );
		}

		return STATUS_OK;
	}


	osg::Image* CHgSharpTileSource::createImage(const TileKey& key, ProgressCallback* progress)
	{
		std::string strUrl = m_hgGisService.ConstructUrl(key, m_hgOptions);

		//使用上面构造的存放到buf中的影像的路径等信息读取影像文件，并返回
		osg::ref_ptr<osg::Image> image = URI(strUrl).getImage( m_hgDbOptions.get(), progress );
		if (false == m_hgGisService.CheckBlackImage(image))
		{
			return NULL;
		}

		return image.release();
	}


	osg::HeightField* CHgSharpTileSource::createHeightField( const TileKey& key,
		ProgressCallback* progress)
	{
		std::string strUrl = m_hgGisService.ConstructUrl(key, m_hgOptions);

		//使用上面构造的存放到buf中的影像的路径等信息读取影像文件，并返回
		osg::ref_ptr<osg::HeightField> hf;
		if (!strUrl.empty())
		{
			hf = dynamic_cast<osg::HeightField*>(URI(strUrl).readObject( 
				m_hgDbOptions.get(), progress ).getObject());
		}

		return hf.release();
	}


	std::string CHgSharpTileSource::getExtension() const 
	{
		return m_hgGisService.GetTileInfo().GetFormat();
	}


	int CHgSharpTileSource::getPixelsPerTile() const
	{
		return m_hgGisService.GetTileInfo().GetTileSize();
	}

}