#include <PluginDependent/GisService.h>

#include <osgEarth/JsonUtils>
#include <osgEarth/Registry>
#include <osg/Notify>
#include <sstream>
#include <limits.h>


namespace FePlugin
{

	CTileInfo::CTileInfo()
		:m_bIsValid(false)
	{

	}

	CTileInfo::CTileInfo( int tile_size, const std::string& format, int min_level, int max_level,
		int num_tiles_wide, int num_tiles_high )
		:m_strFormat(format),
		m_nTileSize(tile_size),
		m_nMinLevel(min_level),
		m_nMaxLevel(max_level),
		m_bIsValid(true),
		m_nNumTilesWide(num_tiles_wide),
		m_nNumTilesHigh(num_tiles_high)
	{

	}

	CTileInfo::CTileInfo( const CTileInfo& rhs )
		:m_strFormat( rhs.GetFormat() ),
		m_nTileSize( rhs.GetTileSize() ),
		m_nMinLevel( rhs.GetMinLevel() ),
		m_nMaxLevel( rhs.GetMaxLevel() ),
		m_bIsValid( rhs.IsValid() ),
		m_nNumTilesWide( rhs.GetNumTilesWide() ),
		m_nNumTilesHigh( rhs.GetNumTilesHigh() )
	{

	}

	CTileInfo & CTileInfo::operator=( const CTileInfo &other )
	{
		if (&other == this)
		{
			return *this;
		}
		else
		{
			this->m_strFormat = other.m_strFormat;
			this->m_nTileSize = other.m_nTileSize;
			this->m_nMinLevel = other.m_nMinLevel;
			this->m_nMaxLevel = other.m_nMaxLevel;
			this->m_bIsValid = other.m_bIsValid;
			this->m_nNumTilesWide = other.m_nNumTilesWide;
			this->m_nNumTilesHigh = other.m_nNumTilesHigh;

			return *this;
		}
	}

	bool CTileInfo::IsValid() const
	{
		return m_bIsValid;
	}

	int CTileInfo::GetTileSize() const
	{
		return m_nTileSize;
	}

	const std::string& CTileInfo::GetFormat() const
	{
		return m_strFormat;
	}

	int CTileInfo::GetMinLevel() const
	{
		return m_nMinLevel;
	}

	int CTileInfo::GetMaxLevel() const
	{
		return m_nMaxLevel;
	}

	int CTileInfo::GetNumTilesWide() const
	{
		return m_nNumTilesWide;
	}

	int CTileInfo::GetNumTilesHigh() const
	{
		return m_nNumTilesHigh;
	}

	//=================================================================================================
	CGisService::CGisService( void )
		:m_bIsValid(false)
	{

	}

	CGisService::CGisService( bool bIsValid, URI uriUri, const Profile * pProfile, 
		std::string strErrorMsg, bool bTiled, CTileInfo tileInfo, std::string strType )
		:m_bIsValid(bIsValid),
		m_uriUri(uriUri),
		m_rpProfile(pProfile),
		m_strErrorMsg(strErrorMsg),
		m_bTiled(bTiled),
		m_tileInfo(tileInfo),
		m_strType(strType)
	{

	}

	CGisService::CGisService( const CGisService& rhs )
	{
		*this = rhs;
	}

	CGisService & CGisService::operator=( const CGisService &other )
	{
		if (&other == this)
		{
			return *this;
		}
		else
		{
			this->m_bIsValid = other.m_bIsValid;
			this->m_uriUri = other.m_uriUri;
			this->m_rpProfile = other.m_rpProfile;
			this->m_strErrorMsg = other.m_strErrorMsg;
			this->m_bTiled = other.m_bTiled;
			this->m_tileInfo = other.m_tileInfo;
			this->m_strType = other.m_strType;

			return *this;
		}
	}

	bool CGisService::Init( const URI& uri, const osgDB::Options* options /*=0L */ )
	{
		//构造配置文件的路径
		m_uriUri = uri;
		std::string sep = uri.full().find( "?" ) == std::string::npos ? "?" : "&";
		std::string json_url = uri.full() + "/config.xml";

		//读取config中的内容，如果读取失败，则返回
		ReadResult r = URI(json_url).readString();
		if ( r.failed() )
			return SetError( "无效的XML配置文件信息" );

		Config conf;
		std::stringstream buf(r.getString());
		conf.fromXML(buf);

		//判断配置信息中是否包含tilemap，如果没有则直接返回
		const Config * hgConf = conf.find("tilemap");
		if (!hgConf)
		{
			OE_WARN << "无效的XML配置文件信息" << std::endl;
			return SetError( "无效的XML配置文件信息" );
		}

		//判断key的类型为行或者列或者其他格式的存储方式
		m_strType = hgConf->value("tiletype");
		m_strCheck = hgConf->value("checkblack");

		//读取配置信息中的空间参考系统
		std::string srs = hgConf->value("srs");

		double xmin, ymin, xmax, ymax;
		//读取profile，即配置项boundingbox中的信息
		const Config * bboxConf = hgConf->find("boundingbox");
		if (bboxConf)
		{
			xmin = bboxConf->value<double>("minx", -20037507.2295943);
			ymin = bboxConf->value<double>("miny", -19971868.8804086);
			xmax = bboxConf->value<double>("maxx", 20037507.2295943);
			ymax = bboxConf->value<double>("maxy", 19971868.8804086);
		}
		//判断地理空间信息是否正确
		if ( ! (xmax > xmin && ymax > ymin && srs != "" ) )
		{
			return SetError( "XML文件中定义的BoundingBox无效" );
		}

		m_bTiled = false;
		std::string format = "";
		int tile_rows = 256;
		int tile_cols = 256;
		int min_level = 0;
		int max_level = 22;
		int num_tiles_wide = 1;
		int num_tiles_high = 1;

		//读取瓦片信息
		const Config * tileFormatConf = hgConf->find("tileformat");
		if (tileFormatConf)
		{
			//取得每片瓦片的分辨率
			tile_rows = tileFormatConf->value<int>("width", -1);
			tile_cols = tileFormatConf->value<int>("height", -1);
			if ( tile_rows <= 0 && tile_cols <= 0 )
				return SetError( "XML配置中文件的分辨率配置错误" );

			//计算第0级需要加载的瓦片的数量
			num_tiles_wide = tileFormatConf->value<int>("num_tiles_width_lod0", -1);
			num_tiles_high = tileFormatConf->value<int>("num_tiles_high_lod0", -1);
			if (num_tiles_high <=0 || num_tiles_wide <=0)
			{
				return SetError("XML配置中第0级需要加载的行列值未设置");
			}

			//取得瓦片的格式
			format = tileFormatConf->value("extension");
			if ( format.empty() )
				return SetError( "XML配置中文件的格式未设置" );

			m_bTiled = true;
		}

		//获得设置的最大以及最小的加载级别
		//const Config * levelSetConf = hgConf->find("level");
		//if (levelSetConf)
		//{
		//    min_level = levelSetConf->value<int>("min_level", 0);
		//    max_level = levelSetConf->value<int>("max_level", 20);
		//}

		//创建空间参照系统
		osg::ref_ptr< SpatialReference > spatialReference = SpatialReference::create( srs );
		if (spatialReference->isGeographic())
		{
			//如果已经存在一个地理空间参照系统，则使用当前的
			
			#if 0 //ZS 以前读取方式
			//m_rpProfile = Registry::instance()->getGlobalGeodeticProfile();
			#else
			m_rpProfile = Profile::create(
				"epsg:4326",
                -180.0, -90.0, 180.0, 90.0,
				"",
				num_tiles_wide,
				num_tiles_high);
			#endif 
		}
		else if (spatialReference->isMercator())
		{
			//如果存在墨卡托空间参照系统，则使用墨卡托
			m_rpProfile = Registry::instance()->getGlobalMercatorProfile();
		}
		else
		{
			//如果非以上两种地理参照，则根据读取到的地理信息创建profile
			m_rpProfile = Profile::create(
				spatialReference.get(),
				xmin, ymin, xmax, ymax,
				num_tiles_wide,
				num_tiles_high);
		}    

		//将瓦片信息整合
		m_tileInfo = CTileInfo( tile_rows, format,
			min_level, max_level, num_tiles_wide, num_tiles_high);
		m_bIsValid = true;

		return m_bIsValid;
	}

	bool CGisService::IsValid() const
	{
		return m_bIsValid;
	}

	bool CGisService::IsTiled() const
	{
		return m_bTiled;
	}

	bool CGisService::SetError( const std::string& msg )
	{
		m_strErrorMsg = msg;
		return false;
	}

	const std::string& CGisService::GetError() const
	{
		return m_strErrorMsg;
	}

	const Profile* CGisService::GetProfile() const
	{
		return m_rpProfile.get();
	}

	const CTileInfo& CGisService::GetTileInfo() const
	{
		return m_tileInfo;
	}


	std::string CGisService::GetTileType() const
	{
		return m_strType;
	}

	std::string CGisService::GetStrCheck() const
	{
		return m_strCheck;
	}

	bool CGisService::CheckBlackImage( osg::Image* image )
	{
		//判断是否需要检查，如果不需要，则返回true，表示数据有效
		if (0 != m_strCheck.compare("true"))
		{
			return true;
		}

		//第一，整行为0或整列为0，则返回false
		if(NULL == image)
		{
			return false;
		}

		bool readColor = false;
		for(int j = 0; j<1/*image->s()*/; j++)
		{
			for(int k = 0; k<image->t(); k++)
			{
				osg::Vec4d color = image->getColor(j, k, 0);

				if((color.r() >(5.0/255.0)) || (color.g() >(5.0/255.0)) || (color.b() >(5.0/255.0)))
				{
					readColor = true;
				}
			}
		}

		if(readColor == false)
		{
			return false;
		}

		readColor = false;
		for(int j = 0; j<image->s(); j++)
		{
			for(int k = 0; k<1/*image->t()*/; k++)
			{
				osg::Vec4d color = image->getColor(j, k, 0);

				if((color.r() >(5.0/255.0)) || (color.g() >(5.0/255.0)) || (color.b() >(5.0/255.0)))
				{
					readColor = true;
				}
			}
		}

		if(readColor == false)
		{
			return false;
		}

		//第二步，判断四个角，各向内延五个象素，若为全黑，则返回false
		int chi = 1;
		//s[0-chi], t[0-chi]
		readColor = false;
		for(int j = 0; j<chi; j++)
		{
			for(int k = 0; k<1; k++)
			{
				osg::Vec4d color = image->getColor(j, k, 0);

				if((color.r() >(15.0/255.0)) || (color.g() >(15.0/255.0)) || (color.b() >(15.0/255.0)))
				{
					readColor = true;
				}
			}
		}

		for(int j = 0; j<1; j++)
		{
			for(int k = 0; k<chi; k++)
			{
				osg::Vec4d color = image->getColor(j, k, 0);

				if((color.r() >(15.0/255.0)) || (color.g() >(15.0/255.0)) || (color.b() >(15.0/255.0)))
				{
					readColor = true;
				}
			}
		}

		if(readColor == false)
		{
			return false;
		}


		//s[0-chi], t[(255-chi)-255]
		readColor = false;
		for(int j = 0; j<chi; j++)
		{
			for(int k = 255; k<256; k++)
			{
				osg::Vec4d color = image->getColor(j, k, 0);

				if((color.r() >(15.0/255.0)) || (color.g() >(15.0/255.0)) || (color.b() >(15.0/255.0)))
				{
					readColor = true;
				}
			}
		}

		for(int j = 0; j<1; j++)
		{
			for(int k = 255-chi; k<256; k++)
			{
				osg::Vec4d color = image->getColor(j, k, 0);

				if((color.r() >(15.0/255.0)) || (color.g() >(15.0/255.0)) || (color.b() >(15.0/255.0)))
				{
					readColor = true;
				}
			}
		}

		if(readColor == false)
		{
			return false;
		}

		//s[(255-chi)-255], t[0, chi]
		readColor = false;
		for(int k = 0; k<chi; k++)
		{
			for(int j = 255; j<256; j++)
			{
				osg::Vec4d color = image->getColor(j, k, 0);

				if((color.r() >(15.0/255.0)) || (color.g() >(15.0/255.0)) || (color.b() >(15.0/255.0)))
				{
					readColor = true;
				}
			}
		}

		for(int k = 0; k<1; k++)
		{
			for(int j = 255-chi; j<256; j++)
			{
				osg::Vec4d color = image->getColor(j, k, 0);

				if((color.r() >(15.0/255.0)) || (color.g() >(15.0/255.0)) || (color.b() >(15.0/255.0)))
				{
					readColor = true;
				}
			}
		}

		if(readColor == false)
		{
			return false;
		}


		//s[(255-chi)-255], t[(255-chi)-255]
		readColor = false;
		for(int k = 255-chi; k<256; k++)
		{
			for(int j = 255; j<256; j++)
			{
				osg::Vec4d color = image->getColor(j, k, 0);

				if((color.r() >(15.0/255.0)) || (color.g() >(15.0/255.0)) || (color.b() >(15.0/255.0)))
				{
					readColor = true;
				}
			}
		}

		for(int k = 255; k<256; k++)
		{
			for(int j = 255-chi; j<256; j++)
			{
				osg::Vec4d color = image->getColor(j, k, 0);

				if((color.r() >(15.0/255.0)) || (color.g() >(15.0/255.0)) || (color.b() >(15.0/255.0)))
				{
					readColor = true;
				}
			}
		}

		if(readColor == false)
		{
			return false;
		}

		return true;
	}

	std::string CGisService::ConstructUrl( const TileKey& key, const CHgSharpOptions& options ) const
	{
		//从key中获取Lod级数、行号、列号
		int level = key.getLevelOfDetail();
		unsigned int tile_x, tile_y;
		key.getTileXY( tile_x, tile_y );

		std::stringstream buf;
		std::string strUrl;
		//从配置项中获取到该影像文件的结构，从而构建不同的url读取影像文件
		//如果是以列存储
		if (m_bTiled)
		{ 
			if( 0 == m_strType.compare("lie") )
			{
				buf << options.GetUrl()->full()
					<< "/" << level
					<< "/" << tile_x
					<< "/" << tile_y
					<< "." << m_tileInfo.GetFormat();
			}
			//否则，如果是以行存储
			else if ( 0 == m_strType.compare("hang") )
			{
				buf << options.GetUrl()->full()
					<< "/" << level
					<< "/" << tile_y
					<< "/" << tile_x
					<< "." << m_tileInfo.GetFormat();
			}
			//倒置的行，也即北极在下
			else if ( 0 == m_strType.compare("hangx") )
			{
				//行总数
				long numSum = (1<<level)-1;
				buf << options.GetUrl()->full() 
					<< "/" << level
					<< "/" << numSum - tile_y
					<< "/" << tile_x 
					<< "." << m_tileInfo.GetFormat();
			}
			//否则，如果是该类型的存储方式
			else if ( 0 == m_strType.compare("sjz_standard_hang") )
			{
				buf<< options.GetUrl()->full()
					<< "/L" << std::setw(2) << std::setfill('0') << level+1 
					<< "/R" << std::setw(6) << tile_y+1
					<< "/C" << std::setw(6) << tile_x+1
					<< "." << m_tileInfo.GetFormat();
			}
			//否则，如果是该类型的存储方式
			else if ( 0 == m_strType.compare("sjz_hang_plus") )
			{
				buf<< options.GetUrl()->full()
					<< "/L" << std::setw(2) << std::setfill('0') << level+1 
					<< "/R" << std::setw(6) << tile_y
					<< "/C" << std::setw(6) << tile_x
					<<"."<< m_tileInfo.GetFormat();
			}
			//否则，如果是该类型的存储方式
			else if ( 0 == m_strType.compare("hang_sjz_standard_hang") )
			{
				if(level<=13)
				{
					buf << options.GetUrl()->full()
						<< "/" << level
						<< "/" << tile_y
						<< "/" << tile_x << "." << m_tileInfo.GetFormat();
				}
				else
				{
					buf<< options.GetUrl()->full()
						<< "/L" << std::setw(2) << std::setfill('0') << level+1 
						<< "/R" << std::setw(6) << tile_y+1
						<< "/C" << std::setw(6) << tile_x+1
						<< "." << m_tileInfo.GetFormat();
				}
			}
			else if (0 == m_strType.compare("NormalImage"))
			{
				buf << options.GetUrl()->full()
					<< "/" << level
					<< "/" << tile_x
					<< "/" << tile_y
					<< "." << m_tileInfo.GetFormat();
			}
		}

		strUrl.assign(buf.str());

		//如果gis服务没有构造出一个URL，则利用传入的key中参数构建一个存有各种配置项的buf
		if (strUrl.empty())
		{
			const GeoExtent& ex = key.getExtent();

			std::stringstream buf;
			buf << std::setprecision(16)
				<< options.GetUrl()->full() << "/export"
				<< "?bbox=" << ex.xMin() << "," << ex.yMin() 
				<< "," << ex.xMax() << "," << ex.yMax()
				<< "&format=" << m_tileInfo.GetFormat() 
				<< "&size=256,256"
				<< "&transparent=true"
				<< "&f=image";

			strUrl.assign(buf.str());
		}

		//如果配置项中有设置安全标记，则为Url增加安全的标记
		if (options.GetToken().isSet())
		{
			std::string token = options.GetToken().value();
			if (!token.empty())
			{
				std::string sep = strUrl.find( "?" ) == std::string::npos ? "?" : "&";
				strUrl.append(sep).append("token=").append(token);
			}
		}

		return strUrl;
	}

}