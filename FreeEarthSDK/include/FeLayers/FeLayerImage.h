/**************************************************************************************************
* @file FeLayerImage.h
* @note 影像图层实现
* @author g00034
* @data 2018-7-16
**************************************************************************************************/
#ifndef FE_IMAGE_LAYER_H
#define FE_IMAGE_LAYER_H

#include <osgEarth/ImageLayer>
#include "IFeLayer.h"
#include "FeLayerImpl.h"


namespace FeLayers
{
	class CImageLayerAgent;

	/**
	  * @class CFeLayerImage
	  * @brief 影像图层实现
	  * @author g00034
	*/
	class CFeLayerImage : public IFeLayerImpl<IFeLayerImage, CTerrianLayerCache<osgEarth::ImageLayer, osgEarth::ImageLayerOptions> >
	{
	public:
		typedef CTerrianLayerCache<osgEarth::ImageLayer, osgEarth::ImageLayerOptions> OsgEarthLayerType;
		typedef IFeLayerImpl<IFeLayerImage, OsgEarthLayerType > SuperClass;

		/**  
		  * @brief 构造函数
		*/
		CFeLayerImage(const CFeLayerImageConfig& cfg);

		/**  
		  * @brief 析构函数
		*/
		~CFeLayerImage();

		/**
		*@note: 获得osgEarth图层
		*/
		virtual osgEarth::ImageLayer* GetEarthImageLayer() { return GetEarthLayer(); }

	public:
		/**
		*@note: 设置图层透明度
		*/
		virtual void setOpacity( float opacity );

		/**
		*@note: 获得图层透明度
		*/
		virtual float getOpacity() const;

		/**
		*@note: 获得图层可视范围
		*/
		virtual void GetVisibleRange(float& minVal, float& maxVal);

		/**
		*@note: 设置图层可视范围
		*/
		virtual void SetVisibleRange(float minVal, float maxVal);

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
		CFeLayerImageConfig            m_layerConfig;
	};

}



#endif
