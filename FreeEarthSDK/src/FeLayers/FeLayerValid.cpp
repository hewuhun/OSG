#include <osgDB/FileUtils>
#include <osgEarth/Profile>
#include <osgEarth/FileUtils>

#include <FeUtils/StrUtil.h>
#include <FeUtils/ShpAndKMLTypeParser.h>
#include <FeUtils/logger/LoggerDef.h>
#include <FeUtils/PathRegistry.h>

#include <FeLayerValid.h>


namespace FeLayers
{
	bool CLayerConfigValidator::CheckAndValid()
	{
		// 校验数据源配置和图层配置
		CLayerDataSourceValidator dataSourceValid(m_cfg.sourceConfig());
		if( dataSourceValid.CheckAndValid() )
		{
			CFeLayerModelConfig* cfgModelLayer = dynamic_cast<CFeLayerModelConfig*>(&m_cfg);
			if(cfgModelLayer)
			{
				ValidModelLayer(*cfgModelLayer);
				InitModelLayerConfig(*cfgModelLayer);
			}

			CFeLayerImageConfig* cfgImageLayer = dynamic_cast<CFeLayerImageConfig*>(&m_cfg);
			if(cfgImageLayer)
			{
				ValidImageLayer(*cfgImageLayer);
				InitImageLayerConfig(*cfgImageLayer);
			}

			return true;
		}

		return false;
	}

	void CLayerConfigValidator::ValidModelLayer(CFeLayerModelConfig& cfg)
	{
		if( 0 == DriverType::DRIVER_OGR.compare(cfg.sourceConfig().driver().get()) )
		{
			// 本地矢量，如果用户没有设置矢量类型，自动解析
			if( !cfg.featureSymbol().HasPointType() 
				&& !cfg.featureSymbol().HasLineType() 
				&& !cfg.featureSymbol().HasPolygonType())
			{
				// 解析判断具体类型
				std::vector<FeUtil::GeometryType> geomTypes = FeUtil::ShpAndKMLTypeParser::Parse(cfg.sourceConfig().url().get());

				for(int n = 0; n < geomTypes.size(); n++)
				{
					if( FeUtil::POINT == geomTypes.at(n) )
					{
						cfg.featureSymbol().AddFeatureType(CFeFeatureSymbol::FT_POINT);
					}
					else if( FeUtil::POLYLINE == geomTypes.at(n) )
					{
						cfg.featureSymbol().AddFeatureType(CFeFeatureSymbol::FT_LINE);
					}
					else if( FeUtil::POLYGON == geomTypes.at(n) )
					{
						cfg.featureSymbol().AddFeatureType(CFeFeatureSymbol::FT_POLYGON);
						cfg.featureSymbol().AddFeatureType(CFeFeatureSymbol::FT_LINE);
					}
				}

				LOG(LOG_NOTICE) << "矢量图层: " << cfg.name().get() << " 解析包含元素："
					<< (cfg.featureSymbol().HasPointType() ? "点" : ",")
					<< (cfg.featureSymbol().HasLineType() ? "线" : ",")
					<< (cfg.featureSymbol().HasPolygonType() ? "面" : "");
			}
		}

		// 处理相对路径
		if( cfg.featureSymbol().fontName().isSet() )
		{
			cfg.featureSymbol().fontName() = FeUtil::StringReplace(
				FeFileReg->GetFullPath(cfg.featureSymbol().fontName().get()), "\\", "/");
		}
		if( cfg.featureSymbol().pointIconPath().isSet() )
		{
			cfg.featureSymbol().pointIconPath() = FeUtil::StringReplace(
				FeFileReg->GetFullPath(cfg.featureSymbol().pointIconPath().get()), "\\", "/");
		}
	}

	void CLayerConfigValidator::ValidImageLayer(CFeLayerImageConfig& cfg)
	{
		if( 0 == DriverType::DRIVER_XYZ.compare(cfg.sourceConfig().driver().get()) )
		{
			// 百度在线地图特殊处理
			if(cfg.baiduOnline().get())
			{
				CFeConfig cfgBd;
				cfgBd.add("baidu", true);
				cfgBd.add("invert_y", true);
				cfg.sourceConfig().Merge(cfgBd);

				cfg.visibleConfig().maxLevel() = 16;
			}
		}

		// 处理相对路径
		if( cfg.cacheConfig().cachePath().isSet() && osgEarth::isRelativePath(cfg.cacheConfig().cachePath().get()) )
		{
			cfg.cacheConfig().cachePath() = FeUtil::StringReplace(FeFileReg->GetFullPath(cfg.cacheConfig().cachePath().get()), "\\", "/");
		}
	}

	void CLayerConfigValidator::InitModelLayerConfig(CFeLayerModelConfig& cfg)
	{
		// 设置默认值
		if( !cfg.featureSymbol().fontName().isSet() )
		{
			cfg.featureSymbol().fontName() = FeUtil::StringReplace(FeFileReg->GetFullPath("fonts/msyh.ttf"), "\\", "/");
		}
		if( !cfg.featureSymbol().pointIconPath().isSet() )
		{
			cfg.featureSymbol().pointIconPath() = FeUtil::StringReplace(
				FeFileReg->GetFullPath("texture/icon/capitalicon.png"), "\\", "/");
		}
	}

	void CLayerConfigValidator::InitImageLayerConfig(CFeLayerImageConfig& cfg)
	{
		// 设置默认值
		if( !cfg.cacheConfig().cachePath().isSet() )
		{
			cfg.cacheConfig().cachePath() = FeUtil::StringReplace(FeFileReg->GetFullPath("earth/cache_flat"), "\\", "/");
		}
	}
}



namespace FeLayers
{
	bool CLayerDataSourceValidator::CheckAndValid()
	{
		if( !URLValid() || !DriverNameValid() || !LayerFormatValid() || !SRSValid() )
		{
			return false;
		}

		LayerConfigExtraHandle();

		return true;
	}

	bool CLayerDataSourceValidator::DriverNameValid()
	{
		// 如果没有指定驱动名，则根据URL判断
		if(!m_dataSource.driver().isSet() && m_dataSource.url().isSet())
		{
			std::string url = m_dataSource.url().get();
			std::string suffix = FeUtil::StringLower(url.substr(url.find_last_of('.')+1));

			if( 0 == suffix.compare("shp")
				||
				0 == suffix.compare("kml")
				)
			{
				m_dataSource.driver() = DriverType::DRIVER_OGR;
			}
			else if( 0 == suffix.compare("img")
				||
				0 == suffix.compare("tif")
				||
				0 == suffix.compare("tiff")
				)
			{
				m_dataSource.driver() = DriverType::DRIVER_GDAL;
			}
		}

		// 统一小写
		{
			m_dataSource.driver() = FeUtil::StringLower(m_dataSource.driver().get());
		}

		return true;
	}

	bool CLayerDataSourceValidator::LayerFormatValid()
	{
		// 设置默认请求的图层格式
		if(!m_dataSource.layerFormat().isSet() && 
			( 0 == DriverType::DRIVER_WMS.compare(m_dataSource.driver().get())
			||
			0 == DriverType::DRIVER_WCS.compare(m_dataSource.driver().get())
			||
			0 == DriverType::DRIVER_GDAL.compare(m_dataSource.driver().get())
			||
			0 == DriverType::DRIVER_FLATGIS.compare(m_dataSource.driver().get())
			) )
		{
			m_dataSource.layerFormat() = "png";
		}

		if(!m_dataSource.layerFormat().isSet() && 0 == DriverType::DRIVER_WMTS.compare(m_dataSource.driver().get()))
		{
			// 为了适应服务器数据，优化加载速度
			m_dataSource.layerFormat() = "jpeg";
		}

		return true;
	}

	bool CLayerDataSourceValidator::SRSValid()
	{
		if(!m_dataSource.srs().isSet() && 
			( 0 == DriverType::DRIVER_WMS.compare(m_dataSource.driver().get())
			||
			0 == DriverType::DRIVER_WMTS.compare(m_dataSource.driver().get())
			||
			0 == DriverType::DRIVER_WCS.compare(m_dataSource.driver().get())
			) )
		{
			m_dataSource.srs() = "EPSG:4326";
		}

// 		if( 0 == DriverType::DRIVER_XYZ.compare(m_dataSource.driver().get()) )
// 		{
// 			// XYZ 驱动设置数据源默认投影 (去除，已在插件中适配)
// 			CFeConfig profileCfg;
// 			profileCfg.addObj("profile", osgEarth::ProfileOptions("global-mercator"));
// 			//m_dataSource.Merge(profileCfg);
// 		}

		return true;
	}

	bool CLayerDataSourceValidator::URLValid()
	{
		// 校验本地数据相对路径
		if( m_dataSource.url().isSet() && 
			( 0 == DriverType::DRIVER_OGR.compare(m_dataSource.driver().get())
			||
			0 == DriverType::DRIVER_MBTILES.compare(m_dataSource.driver().get())
			||
			0 == DriverType::DRIVER_GDAL.compare(m_dataSource.driver().get())
			||
			0 == DriverType::DRIVER_FLATGIS.compare(m_dataSource.driver().get())
			) )
		{
			if( osgEarth::isRelativePath(m_dataSource.url().get()) )
			{
				std::string url = FeUtil::StringReplace(FeFileReg->GetFullPath(m_dataSource.url().get()), "\\", "/");
				if( osgDB::fileExists(url) )
				{
					m_dataSource.url() = url;
				}
			}
		}
	
		return true;
	}

	void CLayerDataSourceValidator::LayerConfigExtraHandle()
	{
		// 各个驱动需要特殊处理的字段
		if( 0 == DriverType::DRIVER_WFS.compare(m_dataSource.driver().get()) )
		{
			// WFS 驱动图层名
			CFeConfig wfsCfg;
			wfsCfg.add("typename", m_dataSource.layerName().get());
			wfsCfg.add("outputformat", "json");
			m_dataSource.Merge(wfsCfg);
		}
		else if( 0 == DriverType::DRIVER_WCS.compare(m_dataSource.driver().get()) )
		{
			// WCS 驱动图层名
			CFeConfig wcsCfg;
			wcsCfg.add("identifier", m_dataSource.layerName().get());
			m_dataSource.Merge(wcsCfg);
		}
		else if( 0 == DriverType::DRIVER_MBTILES.compare(m_dataSource.driver().get()) )
		{
			// URL 转换为UTF编码，以支持中文路径
			m_dataSource.url() = FeUtil::ToUTF(m_dataSource.url().get());

			// mbtiles 驱动文件路径
			CFeConfig mbtCfg;
			mbtCfg.add("filename", m_dataSource.url().get());
			m_dataSource.Merge(mbtCfg);
		}
	}
}



