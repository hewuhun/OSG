/**************************************************************************************************
* @file FeLayerElevation.h
* @note 高程图层实现
* @author g00034
* @data 2018-7-16
**************************************************************************************************/
#ifndef FE_ELEVATION_LAYER_H
#define FE_ELEVATION_LAYER_H

#include <osgEarth/ElevationLayer>
#include "IFeLayer.h"
#include "FeLayerImpl.h"


namespace FeLayers
{
	class CElevationLayerAgent;

	/**
	  * @class CFeLayerElevation
	  * @brief 影像图层实现
	  * @author g00034
	*/
	class CFeLayerElevation : public IFeLayerImpl<IFeLayerElevation, CTerrianLayerCache<osgEarth::ElevationLayer, osgEarth::ElevationLayerOptions> >
	{
	public:
		typedef CTerrianLayerCache<osgEarth::ElevationLayer, osgEarth::ElevationLayerOptions> OsgEarthLayerType;
		typedef IFeLayerImpl<IFeLayerElevation, OsgEarthLayerType > SuperClass;

		/**  
		  * @brief 构造函数
		*/
		CFeLayerElevation(const CFeLayerElevationConfig& cfg);

		/**  
		  * @brief 析构函数
		*/
		~CFeLayerElevation();

		/**
		*@note: 获得osgEarth图层
		*/
		virtual osgEarth::ElevationLayer* GetEarthElevationLayer() { return GetEarthLayer(); }

	public:
		/**
		*@note: 设置图层缓存使能状态
		*/
		virtual void SetCacheEnable(bool bEnable);

		/**
		*@note: 获取图层缓存使能状态
		*/
		virtual bool GetCacheEnable() const;

		/**
		*@note: 设置图层缓存路径
		*/
		virtual void SetCachePath(const std::string& path);

		/**
		*@note: 获取图层缓存路径
		*/
		virtual std::string GetCachePath() const;

		/**
		*@note: 设置图层缓存名称
		*/
		virtual void SetCacheName(const std::string& name);

		/**
		*@note: 获取图层缓存名称
		*/
		virtual std::string GetCacheName() const;

		/**
		*@note: 设置图层名称
		*/
		virtual void SetName(const std::string& strName);

	protected:
		CFeLayerElevationConfig            m_layerConfig;
	};

}


#endif
