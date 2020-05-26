/**************************************************************************************************
* @file FeLayerConfig.h
* @note 图层配置信息 ( 需要注意： 所有属性字段名都需要保持小写 )
* @author g00034
* @data 2018-7-16
**************************************************************************************************/
#ifndef FE_LAYER_CONFIG_H
#define FE_LAYER_CONFIG_H

#include <osgEarth/Config>
#include <osgEarth/optional>
#include <osgEarthSymbology/Color>
#include <osgEarthSymbology/AltitudeSymbol>

#include <osgEarth/TerrainEffect>
#include <osgEarth/TerrainEngineNode>

#include "Export.h"



namespace FeLayers
{
	typedef osgEarth::Config		CFeConfig;
	typedef osgEarth::ConfigSet     CFeConfigSet;

	typedef osgEarth::Symbology::AltitudeSymbol::Clamping   AltitudeClamp;
	typedef osgEarth::Symbology::AltitudeSymbol::Technique  AltitudeTechnique;

	/**
	  * @class CFeConfigOption
	  * @brief 属性配置基类
	  * @author g00034
	*/
	class FELAYERS_EXPORT CFeConfigOption
	{
	public:
		CFeConfigOption( const CFeConfig& conf =CFeConfig() )
			: _conf( conf ) { }
		CFeConfigOption( const CFeConfigOption& rhs )
			: _conf( rhs.getConfig() ) { }
		virtual ~CFeConfigOption(){}

		/**  
		  * @brief 合并配置
		*/
		void Merge( const CFeConfigOption& rhs ) 
		{
			_conf.merge( rhs._conf );
			FromConfig( rhs.getConfig() );
		}
		void Merge( const CFeConfig& rhs ) 
		{
			CFeConfigOption opt(rhs);
			Merge(opt);
		}

		/**  
		  * @brief 获取配置信息，名字为getConfig为了适配osgEarth::Config的AddObj函数
		*/
		virtual CFeConfig getConfig() const { return _conf; }
		bool Empty() const { return _conf.empty(); }

	protected:
		/**  
		  * @brief 解析conf中的信息
		*/
		virtual void FromConfig( const CFeConfig& conf ) { }

		// 配置信息
		CFeConfig _conf;
	}; 
}



namespace FeLayers
{
	// 支持的数据源驱动类型
	namespace DriverType
	{
		const std::string DRIVER_GDAL = "gdal";
		const std::string DRIVER_WMS  = "wms";
		const std::string DRIVER_WMTS = "wmts";
		const std::string DRIVER_WCS  = "wcs";
		const std::string DRIVER_WFS  = "wfs";
		const std::string DRIVER_TMS  = "tms";
		const std::string DRIVER_XYZ  = "xyz";
		const std::string DRIVER_OGR  = "ogr";
		const std::string DRIVER_FLATGIS  = "flatgis";
		const std::string DRIVER_MBTILES  = "mbtiles";
	};


	/**
	  * @class CFeDataSource
	  * @brief 数据源配置属性
	  * @note 图册数据源相关配置，管理了常用的数据源配置信息
	  * @author g00034
	*/
	class FELAYERS_EXPORT CFeDataSource : public CFeConfigOption
	{
		typedef CFeConfigOption SuperClass;
	public:
		CFeDataSource( const std::string& driverName, const std::string& url );
		CFeDataSource( const CFeConfig& conf = CFeConfig() );
		CFeDataSource( const CFeDataSource& rhs );
		CFeDataSource& operator = ( const CFeDataSource& rhs );

		/**  
		  * @brief 获取数据源配置信息
		*/
		virtual CFeConfig getConfig() const;

	public:
		osgEarth::optional<std::string>& driver() { return m_driver; }
		const osgEarth::optional<std::string>& driver() const { return m_driver; }

		osgEarth::optional<std::string>& url() { return m_url; }
		const osgEarth::optional<std::string>& url() const { return m_url; }

		osgEarth::optional<std::string>& srs() { return m_srs; }
		const osgEarth::optional<std::string>& srs() const { return m_srs; }

		osgEarth::optional<std::string>& layerName() { return m_layerName; }
		const osgEarth::optional<std::string>& layerName() const { return m_layerName; }
		
		osgEarth::optional<std::string>& layerFormat() { return m_layerFormat; }
		const osgEarth::optional<std::string>& layerFormat() const { return m_layerFormat; }

		osgEarth::optional<std::string>& version() { return m_version; }
		const osgEarth::optional<std::string>& version() const { return m_version; }

	protected:
		/**  
		  * @brief 解析conf中的信息
		*/
		virtual void FromConfig( const CFeConfig& conf );

		/**  
		  * @brief 解析conf中的信息，
		*/
		void ParseConfig( const CFeConfig& conf );
	private:
		osgEarth::optional<std::string> m_driver;
		osgEarth::optional<std::string> m_url;
		osgEarth::optional<std::string> m_srs;
		osgEarth::optional<std::string> m_layerName; // 数据驱动使用的图层名
		osgEarth::optional<std::string> m_layerFormat; 
		osgEarth::optional<std::string> m_version; 
	};
}


namespace FeLayers
{
	typedef osgEarth::Symbology::Color CFeColor;

	/**
	  * @class CFeFeatureSymbol
	  * @brief 矢量符号配置信息
	  * @note 包括：点，线，面的可选配置
	  * @author g00034
	*/
	class FELAYERS_EXPORT CFeFeatureSymbol : public CFeConfigOption
	{
		typedef CFeConfigOption SuperClass;
	public:
		// 矢量符号类型
		typedef enum 
		{
			FT_POINT = 0x1<<0,
			FT_LINE = 0x1<<1,
			FT_POLYGON = 0x1<<2
		}EFeatureType;

		// 线型枚举
		typedef enum
		{
			LT_SOLID = 0xFFFF,  // 实线
			LT_DASH1 = 0xFF00,  // 虚线1
			LT_DASH2 = 0xF0F0,  // 虚线2
			LT_DASH3 = 0xCCCC,  // 虚线3
		}ELineType;

		CFeFeatureSymbol( const CFeConfig& conf = CFeConfig() );
		CFeFeatureSymbol( const CFeFeatureSymbol& rhs );
		CFeFeatureSymbol& operator = ( const CFeFeatureSymbol& rhs );

		/**  
		  * @brief 添加矢量中元素类型
		*/
		void AddFeatureType(EFeatureType ft) { m_unFeatureTypeMask |= ft; }

		/**  
		  * @brief 判断矢量符号类型
		*/
		bool HasPointType() const { return (m_unFeatureTypeMask&FT_POINT) != 0; }
		bool HasLineType() const { return (m_unFeatureTypeMask&FT_LINE) != 0; }
		bool HasPolygonType() const { return (m_unFeatureTypeMask&FT_POLYGON) != 0; }

		/**  
		  * @brief 获取矢量符号配置信息
		*/
		virtual CFeConfig getConfig() const;

	public:
		/**
		*@note: 设置/获取线条颜色
		*/
		osgEarth::optional<CFeColor>& lineColor() { return m_optLineColor; }
		const osgEarth::optional<CFeColor>& lineColor() const  { return m_optLineColor; }

		/**
		*@note: 设置/获取填充颜色
		*/
		osgEarth::optional<CFeColor>& fillColor() { return m_optFillColor; }
		const osgEarth::optional<CFeColor>& fillColor() const  { return m_optFillColor; }

		/**
		*@note: 设置/获取线条宽度
		*/
		osgEarth::optional<float>& lineWidth() { return m_optLineWidth; }
		const osgEarth::optional<float>& lineWidth() const  { return m_optLineWidth; }

		/**
		*@note: 设置/获取线条宽度 (使用 ELineType 枚举)
		*/
		osgEarth::optional<unsigned short>& lineStipple() { return m_optLineStipple; }
		const osgEarth::optional<unsigned short>& lineStipple() const  { return m_optLineStipple; }

		/**
		*@note: 设置/获取字体大小
		*/
		osgEarth::optional<float>& fontSize() { return m_optFontSize; }
		const osgEarth::optional<float>& fontSize() const  { return m_optFontSize; }

		/**
		*@note: 设置/获取字体名
		*/
		osgEarth::optional<std::string>& fontName() { return m_optFontName; }
		const osgEarth::optional<std::string>& fontName() const  { return m_optFontName; }

		/**
		*@note: 设置/获取字体颜色
		*/
		osgEarth::optional<CFeColor>& fontColor() { return m_optFontColor; }
		const osgEarth::optional<CFeColor>& fontColor() const  { return m_optFontColor; }

		/**
		*@note: 设置/获取字体边框颜色
		*/
		osgEarth::optional<CFeColor>& fontHaloColor() { return m_optFontHaloColor; }
		const osgEarth::optional<CFeColor>& fontHaloColor() const  { return m_optFontHaloColor; }

		/**
		*@note: 设置/获取点标记图片路径
		*/
		osgEarth::optional<std::string>& pointIconPath() { return m_optIconPath; }
		const osgEarth::optional<std::string>& pointIconPath() const  { return m_optIconPath; }

		/**
		*@note: 设置/获取点标记图片显隐
		*/
		osgEarth::optional<bool>& pointIconVisible() { return m_optIconVisible; }
		const osgEarth::optional<bool>& pointIconVisible() const  { return m_optIconVisible; }

		/**
		*@note: 设置/获取贴地方式(使用 AltitudeClamp 枚举)
		*/
		osgEarth::optional<unsigned short>& Clamp() { return m_optClamp; }
		const osgEarth::optional<unsigned short>& Clamp() const  { return m_optClamp; }

		/**
		*@note: 设置/获取贴地技术(使用 AltitudeTechnique 枚举)
		*/
		osgEarth::optional<unsigned short>& Technique() { return m_optTechnique; }
		const osgEarth::optional<unsigned short>& Technique() const  { return m_optTechnique; }
		
	protected:
		/**  
		  * @brief 解析conf中的信息，
		*/
		void ParseConfig( const CFeConfig& conf );
		
	private:
		// 包含的矢量元素掩码
		unsigned int                         m_unFeatureTypeMask;
		osgEarth::optional<float>            m_optLineWidth;
		osgEarth::optional<float>            m_optFontSize;

		// 线型
		osgEarth::optional<unsigned short>   m_optLineStipple;

		// 线条颜色
		osgEarth::optional<CFeColor>         m_optLineColor;

		// 填充色
		osgEarth::optional<CFeColor>         m_optFillColor;

		// 字体颜色
		osgEarth::optional<CFeColor>         m_optFontColor;

		// 字体外边框颜色
		osgEarth::optional<CFeColor>         m_optFontHaloColor;

		// 字体名称(全路径)
		osgEarth::optional<std::string>      m_optFontName;

		// 图标路径
		osgEarth::optional<std::string>      m_optIconPath;
		osgEarth::optional<bool>             m_optIconVisible;

		// 贴地方式和使用技术
		osgEarth::optional<unsigned short>   m_optClamp;
		osgEarth::optional<unsigned short>   m_optTechnique;
	};
}


namespace FeLayers
{
	/**
	  * @class CCacheConfig
	  * @brief 缓存属性配置
	  * @author g00034
	*/
	class FELAYERS_EXPORT CCacheConfig : public CFeConfigOption
	{
		typedef CFeConfigOption SuperClass;
	public:
		CCacheConfig( const CFeConfig& conf = CFeConfig() );
		CCacheConfig( const CCacheConfig& rhs );
		CCacheConfig& operator = ( const CCacheConfig& rhs );

		/**  
		  * @brief 获取图层配置信息
		*/
		virtual CFeConfig getConfig() const;

		/**  
		  * @brief 解析conf中的信息，
		*/
		virtual void FromConfig( const CFeConfig& conf  );

	public:
		/**
		*@note: 设置/获取缓存路径
		*/
		osgEarth::optional<std::string>& cachePath() { return m_cachePath; }
		const osgEarth::optional<std::string>& cachePath() const { return m_cachePath; }

		/**
		*@note: 设置/获取缓存名称
		*/
		osgEarth::optional<std::string>& cacheName() { return m_cacheName; }
		const osgEarth::optional<std::string>& cacheName() const { return m_cacheName; }

		/**
		*@note: 设置/获取缓存开关
		*/
		osgEarth::optional<bool>& cacheEnable() { return m_enablePath; }
		const osgEarth::optional<bool>& cacheEnable() const { return m_enablePath; }

	private:
		osgEarth::optional<std::string> m_cachePath;
		osgEarth::optional<std::string> m_cacheName;
		osgEarth::optional<bool>		m_enablePath;
	};

	/**
	  * @class CVisibleConfig
	  * @brief 显示属性配置
	  * @author g00034
	*/
	class FELAYERS_EXPORT CVisibleConfig : public CFeConfigOption
	{
		typedef CFeConfigOption SuperClass;
	public:
		CVisibleConfig( const CFeConfig& conf = CFeConfig() );
		CVisibleConfig( const CVisibleConfig& rhs );
		CVisibleConfig& operator = ( const CVisibleConfig& rhs );

		/**  
		  * @brief 获取图层配置信息
		*/
		virtual CFeConfig getConfig() const;

		/**  
		  * @brief 解析conf中的信息，
		*/
		virtual void FromConfig( const CFeConfig& conf  );

	public:
		/**
		*@note: 设置/获取透明度
		*/
		osgEarth::optional<float>& opacity() { return m_opacity; }
		const osgEarth::optional<float>& opacity() const { return m_opacity; }

		/**
		*@note: 设置/获取透明度开关
		*/
		osgEarth::optional<bool>& opacityEnable() { return m_enableOpacity; }
		const osgEarth::optional<bool>& opacityEnable() const { return m_enableOpacity; }

		/**
		*@note: 设置/获取可见范围
		*/
		osgEarth::optional<float>& minVisibleRange() { return m_minRange; }
		const osgEarth::optional<float>& minVisibleRange() const { return m_minRange; }
		osgEarth::optional<float>& maxVisibleRange() { return m_maxRange; }
		const osgEarth::optional<float>& maxVisibleRange() const { return m_maxRange; }

		/**
		*@note: 设置/获取级别
		*/
		osgEarth::optional<unsigned>& minLevel() { return m_minLevel; }
		const osgEarth::optional<unsigned>& minLevel() const { return m_minLevel; }
		osgEarth::optional<unsigned>& maxLevel() { return m_maxLevel; }
		const osgEarth::optional<unsigned>& maxLevel() const { return m_maxLevel; }

	private:
		// 透明度
		osgEarth::optional<float>       m_opacity;
		osgEarth::optional<bool>		m_enableOpacity;

		// 显示范围
		osgEarth::optional<float>       m_minRange;
		osgEarth::optional<float>       m_maxRange;

		// 数据显示级别范围
		osgEarth::optional<unsigned>    m_minLevel;
		osgEarth::optional<unsigned>    m_maxLevel;
	};
}


namespace FeLayers
{
	// 图层类型枚举
	typedef enum 
	{
		E_LAYER_IMAGE = 0,  // 影像图层
		E_LAYER_MODEL,      // 矢量图层
		E_LAYER_ELEVATION,  // 高程图层
		E_LAYER_EFFECT,     // 特效图层
	}E_Layer_Type;

	/**
	  * @class CFeLayerConfig
	  * @brief 封装图层配置属性
	  * @note 将地图数据源和图层属性集中在一起，简化使用
	  * @author g00034
	*/
	class FELAYERS_EXPORT CFeLayerConfig : public CFeConfigOption
	{
		typedef CFeConfigOption SuperClass;
	public:
		CFeLayerConfig( const CFeConfig& conf = CFeConfig() );
		CFeLayerConfig( const CFeLayerConfig& rhs );
		CFeLayerConfig& operator = ( const CFeLayerConfig& rhs );
	
		/**  
		  * @brief 获取图层配置信息
		*/
		virtual CFeConfig getConfig() const;

	public:
		/**
		*@note: 设置/获取图层名
		*/
		osgEarth::optional<std::string>& name() { return m_name; }
		const osgEarth::optional<std::string>& name() const { return m_name; }
		
		/**
		*@note: 设置/获取是否可见
		*/
		osgEarth::optional<bool>& visible() { return m_visible; }
		const osgEarth::optional<bool>& visible() const { return m_visible; }

		/**
		*@note: 设置/获取数据源
		*/
		CFeDataSource& sourceConfig() { return m_sourceCfg; }
		const CFeDataSource& sourceConfig() const { return m_sourceCfg; }

	public:
		/**  
		  * @brief 配置的图层类型
		*/
		E_Layer_Type GetLayerType() const { return m_layerType; }

	protected:
		/**  
		  * @brief 解析conf中的信息，
		*/
		void ParseConfig( const CFeConfig& conf );

		/**  
		  * @brief 解析conf中的信息，
		*/
		virtual void FromConfig( const CFeConfig& conf  );

	protected:
		// 业务使用的图层名
		osgEarth::optional<std::string> m_name; 
		osgEarth::optional<bool>        m_visible;
		
		// 数据源配置
		CFeDataSource              	    m_sourceCfg;

		// 图层类型
		E_Layer_Type                    m_layerType;
	};

	

	/**
	  * @class CFeLayerImageConfig
	  * @brief 影像图层配置属性
	  * @author g00034
	*/
	class FELAYERS_EXPORT CFeLayerImageConfig : public CFeLayerConfig
	{
		typedef CFeLayerConfig SuperClass;
	public:
		CFeLayerImageConfig( const std::string& layerName, const std::string& driverName, const std::string& url );
		CFeLayerImageConfig( const CFeConfig& conf =CFeConfig() );
		CFeLayerImageConfig( const CFeLayerImageConfig& rhs );
		CFeLayerImageConfig& operator = ( const CFeLayerImageConfig& rhs );

		/**  
		  * @brief 获取图层配置信息
		*/
		virtual CFeConfig getConfig() const;

	public:
		/**
		*@note: 设置/获取是否为百度在线地图，需要特殊处理
		*/
		osgEarth::optional<bool>& baiduOnline() { return m_baiduOnline; }
		const osgEarth::optional<bool>& baiduOnline() const { return m_baiduOnline; }

		/**
		*@note: 缓存配置对象
		*/
		CCacheConfig& cacheConfig() { return m_cfgCache; }
		const CCacheConfig& cacheConfig() const { return m_cfgCache; }
		
		/**
		*@note: 显示配置对象
		*/
		CVisibleConfig& visibleConfig() { return m_cfgVisible; }
		const CVisibleConfig& visibleConfig() const { return m_cfgVisible; }

	protected:
		/**  
		  * @brief 解析conf中的信息，
		*/
		virtual void FromConfig( const CFeConfig& conf  );

	private:
		// 标识是否为百度在线地图，需要特殊处理
		osgEarth::optional<bool>		m_baiduOnline; 

		// 缓存配置
		CCacheConfig                    m_cfgCache;

		// 显示配置
		CVisibleConfig                  m_cfgVisible;
	};


	/**
	  * @class CFeLayerModelConfig
	  * @brief 矢量图层配置属性
	  * @author g00034
	*/
	class FELAYERS_EXPORT CFeLayerModelConfig : public CFeLayerConfig
	{
		typedef CFeLayerConfig SuperClass;
	public:
		CFeLayerModelConfig( const std::string& layerName, const std::string& driverName, const std::string& url );
		CFeLayerModelConfig( const CFeConfig& conf =CFeConfig() );
		CFeLayerModelConfig( const CFeLayerModelConfig& rhs );
		CFeLayerModelConfig& operator = ( const CFeLayerModelConfig& rhs );

		/**  
		  * @brief 获取图层配置信息
		*/
		virtual CFeConfig getConfig() const;

	public:
		/**
		*@note: 设置/获取矢量图层属性
		*/
		CFeFeatureSymbol& featureSymbol() { return m_featureSymbol; }
		const CFeFeatureSymbol& featureSymbol() const { return m_featureSymbol; }

		/**
		*@note: 显示配置对象
		*/
		CVisibleConfig& visibleConfig() { return m_cfgVisible; }
		const CVisibleConfig& visibleConfig() const { return m_cfgVisible; }

	protected:
		/**  
		  * @brief 解析conf中的信息，
		*/
		virtual void FromConfig( const CFeConfig& conf  );

	private:
		// 矢量符号配置
		CFeFeatureSymbol                m_featureSymbol; 

		// 显示配置
		CVisibleConfig                  m_cfgVisible;
	};


	/**
	  * @class CFeLayerElevationConfig
	  * @brief 高程图层配置属性
	  * @author g00034
	*/
	class FELAYERS_EXPORT CFeLayerElevationConfig : public CFeLayerConfig
	{
		typedef CFeLayerConfig SuperClass;
	public:
		CFeLayerElevationConfig( const std::string& layerName, const std::string& driverName, const std::string& url );
		CFeLayerElevationConfig( const CFeConfig& conf =CFeConfig() );
		CFeLayerElevationConfig( const CFeLayerElevationConfig& rhs );
		CFeLayerElevationConfig& operator = ( const CFeLayerElevationConfig& rhs );

		/**  
		  * @brief 获取图层配置信息
		*/
		virtual CFeConfig getConfig() const;

	public:
		/**
		*@note: 缓存配置对象
		*/
		CCacheConfig& cacheConfig() { return m_cfgCache; }
		const CCacheConfig& cacheConfig() const { return m_cfgCache; }

	protected:
		/**  
		  * @brief 解析conf中的信息，
		*/
		virtual void FromConfig( const CFeConfig& conf  );

	private:
		// 缓存配置
		CCacheConfig                    m_cfgCache;
	};


	/**
	  * @class CFeLayerEffectConfig
	  * @brief 特效图层配置属性
	  * @author g00034
	*/
	class FELAYERS_EXPORT CFeLayerEffectConfig : public CFeLayerConfig
	{
		typedef CFeLayerConfig SuperClass;
	public:
		CFeLayerEffectConfig( const CFeConfig& conf =CFeConfig() );
		CFeLayerEffectConfig( const CFeLayerEffectConfig& rhs );
		CFeLayerEffectConfig& operator = ( const CFeLayerEffectConfig& rhs );

		/**  
		  * @brief 获取图层配置信息
		*/
		virtual CFeConfig getConfig() const;

	public:
		/**
		*@note: 设置/获取地形引擎节点
		*/
		void SetTerrainEngineNode(osgEarth::TerrainEngineNode* node) { m_opTerrainNode = node; }
		osgEarth::TerrainEngineNode* GetTerrainEngineNode() { return m_opTerrainNode.get(); }

		/**
		*@note: 设置/获取图层特效对象
		*/
		void SetTerrainEffect(osgEarth::TerrainEffect* effect) { m_rpEffectLayer = effect; }
		osgEarth::TerrainEffect* GetTerrainEffect() { return m_rpEffectLayer.get(); }

	protected:
		/**  
		  * @brief 解析conf中的信息，
		*/
		virtual void FromConfig( const CFeConfig& conf  );

	private:
		// 地形引擎节点
		osg::observer_ptr<osgEarth::TerrainEngineNode>      m_opTerrainNode;

		// 图层特效对象
		osg::ref_ptr<osgEarth::TerrainEffect>               m_rpEffectLayer;
	};
}



#endif // FE_LAYER_CONFIG_H

