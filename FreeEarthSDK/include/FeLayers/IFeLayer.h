/**************************************************************************************************
* @file FeLayer.h
* @note 图层基类，提供图层通用操作
* @author g00034
* @data 2018-7-16
**************************************************************************************************/
#ifndef FE_LAYER_H
#define FE_LAYER_H

#include "FeLayerConfig.h"


namespace osgEarth
{
	class ImageLayer;
	class ModelLayer;
	class ElevationLayer;
}

class CFeEffectLayer;

namespace FeLayers
{
	typedef unsigned int FeLayerID;

	class IFeLayerImage;
	class IFeLayerModel;
	class IFeLayerElevation;
	class IFeLayerEffect;

	/**
	  * @class IFeLayer
	  * @brief 图层接口
	  * @note 对外提供的图层操作
	  * @author g00034
	*/
	class IFeLayer : public osg::Referenced
	{
	public:
		virtual ~IFeLayer() {}

	public:
		/**
		*@note: 判断图层类型
		*/
		virtual bool IsImageLayer(){ return false; }
		virtual bool IsModelLayer(){ return false; }
		virtual bool IsElevationLayer(){ return false; }
		virtual bool IsEffectLayer(){ return false; }

		/**
		*@note: 方便转换类型
		*/
		virtual IFeLayerImage* AsImageLayer(){ return NULL; }
		virtual IFeLayerModel* AsModelLayer(){ return NULL; }
		virtual IFeLayerElevation* AsElevationLayer(){ return NULL; }
		virtual IFeLayerEffect* AsEffectLayer(){ return NULL; }

	public:
		/**
		*@note: 设置图层名称
		*/
		virtual void SetName(const std::string& strName)=0;

		/**
		*@note: 获得图层名称
		*/
		virtual const std::string& GetName() const=0;

		/**
		*@note: 设置图层显隐状态
		*/
		virtual void SetVisible(bool bVisible)=0;

		/**
		*@note: 获取图层显隐状态
		*/
		virtual bool GetVisible() const =0;
		
		/**
		*@note: 获得图层ID
		*/
		virtual FeLayerID GetLayerUID() = 0;

		/**
		*@note: 获得图层属性配置
		*/
		virtual CFeLayerConfig GetLayerConfig() = 0;
	
	};
	
	/**
	  * @class IFeCache
	  * @brief 缓存功能接口
	  * @note 提供缓存操作
	  * @author g00034
	*/
	class IFeCache
	{
	public:
		/**
		*@note: 设置图层缓存使能状态
		*/
		virtual void SetCacheEnable(bool bEnable)=0;

		/**
		*@note: 获取图层缓存使能状态
		*/
		virtual bool GetCacheEnable() const =0;

		/**
		*@note: 设置图层缓存路径
		*/
		virtual void SetCachePath(const std::string& path)=0;

		/**
		*@note: 获取图层缓存路径
		*/
		virtual std::string GetCachePath() const =0;

		/**
		*@note: 设置图层缓存名称
		*/
		virtual void SetCacheName(const std::string& name)=0;

		/**
		*@note: 获取图层缓存名称
		*/
		virtual std::string GetCacheName() const =0;
	};



	/**
	  * @class IFeLayerImage
	  * @brief 影像图层接口
	  * @note 对外提供的影像图层操作
	  * @author g00034
	*/
	class IFeLayerImage : public IFeLayer, public IFeCache
	{
	public:
		/**
		*@note: 判断图层类型
		*/
		virtual bool IsImageLayer(){ return true; }
		virtual IFeLayerImage* AsImageLayer(){ return this; }
		
		/**
		*@note: 获得osgEarth图层
		*/
		virtual osgEarth::ImageLayer* GetEarthImageLayer() = 0;

	public:
		/**
		*@note: 设置图层透明度
		*/
		virtual void setOpacity( float opacity )=0;

		/**
		*@note: 获得图层透明度
		*/
		virtual float getOpacity() const =0;

		/**
		*@note: 设置图层可视范围
		*/
		virtual void SetVisibleRange(float minVal, float maxVal) = 0;

		/**
		*@note: 获得图层可视范围
		*/
		virtual void GetVisibleRange(float& minVal, float& maxVal) = 0;
		
	};



	/**
	  * @class IFeLayerModel
	  * @brief 矢量图层接口
	  * @note 对外提供的矢量图层操作
	  * @author g00034
	*/
	class IFeLayerModel : public IFeLayer
	{
	public:
		/**
		*@note: 判断图层类型
		*/
		virtual bool IsModelLayer(){ return true; }
		virtual IFeLayerModel* AsModelLayer(){ return this; }

		/**
		*@note: 获得osgEarth图层
		*/
		virtual osgEarth::ModelLayer* GetEarthModelLayer() = 0;

	public:
		/**
		*@note: 设置图层透明度
		*/
		virtual void setOpacity( float opacity )=0;

		/**
		*@note: 获得图层透明度
		*/
		virtual float getOpacity() const =0;

		/**
		*@note: 设置图层可视范围
		*/
		virtual void SetVisibleRange(float minVal, float maxVal) = 0;

		/**
		*@note: 获得图层可视范围
		*/
		virtual void GetVisibleRange(float& minVal, float& maxVal) = 0;

	public:
		/**
		*@note: 判断矢量中是否包含线元素
		*/
		virtual bool HasLineFeature() = 0;

		/**
		*@note: 判断矢量中是否包含面元素
		*/
		virtual bool HasPolygonFeature() = 0;

		/**
		*@note: 判断矢量中是否包含点元素
		*/
		virtual bool HasPointFeature() = 0;

	public:
		//////////////////////////////////////////////////////////////////////////
		// 线面属性接口
		/**
		*@note: 设置线条颜色
		*/
		virtual void SetLineColor( const osg::Vec4f& color )=0;

		/**
		*@note: 获得线条颜色
		*/
		virtual osg::Vec4f GetLineColor() const =0;

		/**
		*@note: 设置填充颜色
		*/
		virtual void SetFillColor( const osg::Vec4f& color )=0;

		/**
		*@note: 获得填充颜色
		*/
		virtual osg::Vec4f GetFillColor() const =0;

		/**
		*@note: 设置线条宽度
		*/
		virtual void SetLineWidth( float width )=0;

		/**
		*@note: 获得线条宽度
		*/
		virtual float GetLineWidth() const =0;

		/**
		*@note: 设置线条类型 (使用 CFeFeatureSymbol::ELineType 枚举)
		*/
		virtual void SetLineStipple( unsigned short stipple )=0;

		/**
		*@note: 获得线条类型 (使用 CFeFeatureSymbol::ELineType 枚举)
		*/
		virtual unsigned short GetLineStipple() const =0;


		//////////////////////////////////////////////////////////////////////////
		// 点属性接口
		/**
		*@note: 设置字体大小
		*/
		virtual void SetFontSize( float fHeight )=0;

		/**
		*@note: 获取字体大小
		*/
		virtual  float GetFontSize()=0;

		/**
		*@note: 设置字体名
		*/
		virtual void SetFontName( const std::string& name )=0;

		/**
		*@note: 获取字体名
		*/
		virtual std::string GetFontName()=0;

		/**
		*@note: 设置字体颜色
		*/
		virtual void SetFontColor( const osg::Vec4f& color )=0;

		/**
		*@note: 获取字体颜色
		*/
		virtual osg::Vec4f GetFontColor()=0;

		/**
		*@note: 设置字体边框颜色
		*/
		virtual void SetFontHaloColor( const osg::Vec4f& color )=0;

		/**
		*@note: 获取字体边框颜色
		*/
		virtual osg::Vec4f GetFontHaloColor()=0;

		/**
		*@note: 设置点标记图片路径
		*/
		virtual void SetPointIconPath( const std::string& name )=0;

		/**
		*@note: 获取点标记图片路径
		*/
		virtual std::string GetPointIconPath()=0;

		/**
		*@note: 设置点标记显隐
		*/
		virtual void SetPointIconVisible( bool bVisible )=0;

		/**
		*@note: 获取点标记显隐
		*/
		virtual bool GetPointIconVisible()=0;

		//////////////////////////////////////////////////////////////////////////
		// 贴地属性接口
		/**
		*@note: 设置贴地方式
		*/
		virtual void SetAltitudeClamp( AltitudeClamp clamp )=0;

		/**
		*@note: 获取贴地方式
		*/
		virtual AltitudeClamp GetAltitudeClamp()=0;

		/**
		*@note: 设置贴地技术
		*/
		virtual void SetAltitudeTechnique( AltitudeTechnique technique )=0;

		/**
		*@note: 获取贴地技术
		*/
		virtual AltitudeTechnique GetAltitudeTechnique()=0;
	};


	/**
	  * @class IFeLayerElevation
	  * @brief 高程图层接口
	  * @note 对外提供的高程图层操作
	  * @author g00034
	*/
	class IFeLayerElevation : public IFeLayer, public IFeCache
	{
	public:
		/**
		*@note: 判断图层类型
		*/
		virtual bool IsElevationLayer(){ return true; }
		virtual IFeLayerElevation* AsElevationLayer(){ return this; }

		/**
		*@note: 获得osgEarth图层
		*/
		virtual osgEarth::ElevationLayer* GetEarthElevationLayer() = 0;
	};


	/**
	  * @class IFeLayerEffect
	  * @brief 特效图层接口
	  * @author g00034
	*/
	class IFeLayerEffect : public IFeLayer
	{
	public:
		/**
		*@note: 判断图层类型
		*/
		virtual bool IsEffectLayer(){ return true; }
		virtual IFeLayerEffect* AsEffectLayer(){ return this; }

		/**
		*@note: 获得osgEarth图层
		*/
		//virtual osgEarth::ElevationLayer* GetEffectLayer() = 0;
	};
}

#endif
