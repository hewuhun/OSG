/**************************************************************************************************
* @file FeModelLayer.h
* @note 矢量图层实现
* @author g00034
* @data 2018-1-16
**************************************************************************************************/
#ifndef FE_MODEL_LAYER_H
#define FE_MODEL_LAYER_H

#include <osgEarth/ModelLayer>
#include <osgEarthFeatures/FeatureModelGraph>
#include "IFeLayer.h"
#include "FeLayerImpl.h"


namespace FeLayers
{
	/**
	  * @class CFeLayerModel
	  * @brief 矢量图层实现
	  * @author g00034
	*/
	class CFeLayerModel : public IFeLayerImpl<IFeLayerModel, osgEarth::ModelLayer>
	{
		typedef IFeLayerImpl<IFeLayerModel, osgEarth::ModelLayer> SuperClass;
	public:
		/**  
		  * @brief 构造函数
		*/
		CFeLayerModel(const CFeLayerModelConfig& cfg);

		/**  
		  * @brief 析构函数
		*/
		~CFeLayerModel();

		/**
		*@note: 获得osgEarth图层
		*/
		virtual osgEarth::ModelLayer* GetEarthModelLayer() { return GetEarthLayer(); }

	public:
		/**
		*@note: 判断矢量中是否包含线元素
		*/
		virtual bool HasLineFeature();

		/**
		*@note: 判断矢量中是否包含面元素
		*/
		virtual bool HasPolygonFeature();

		/**
		*@note: 判断矢量中是否包含点元素
		*/
		virtual bool HasPointFeature();

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
		*@note: 设置字体大小
		*/
		virtual void SetFontSize( float fHeight );

		/**
		*@note: 获取字体大小
		*/
		virtual  float GetFontSize();

		/**
		*@note: 设置字体名
		*/
		virtual void SetFontName( const std::string& name );

		/**
		*@note: 获取字体名
		*/
		virtual std::string GetFontName();

		/**
		*@note: 设置字体颜色
		*/
		virtual void SetFontColor( const osg::Vec4f& color );

		/**
		*@note: 获取字体颜色
		*/
		virtual osg::Vec4f GetFontColor();

		/**
		*@note: 设置字体边框颜色
		*/
		virtual void SetFontHaloColor( const osg::Vec4f& color );

		/**
		*@note: 获取字体边框颜色
		*/
		virtual osg::Vec4f GetFontHaloColor();

		/**
		*@note: 设置点标记图片路径
		*/
		virtual void SetPointIconPath( const std::string& name );

		/**
		*@note: 获取点标记图片路径
		*/
		virtual std::string GetPointIconPath();

		/**
		*@note: 设置点标记显隐
		*/
		virtual void SetPointIconVisible( bool bVisible );

		/**
		*@note: 获取点标记显隐
		*/
		virtual bool GetPointIconVisible();

		/**
		*@note: 设置图层可视范围
		*/
		virtual void SetVisibleRange(float minVal, float maxVal);

	public:
		/**
		*@note: 设置线条颜色
		*/
		virtual void SetLineColor( const osg::Vec4f& color );

		/**
		*@note: 获得线条颜色
		*/
		virtual osg::Vec4f GetLineColor() const;

		/**
		*@note: 设置填充颜色
		*/
		virtual void SetFillColor( const osg::Vec4f& color );

		/**
		*@note: 获得填充颜色
		*/
		virtual osg::Vec4f GetFillColor() const;

		/**
		*@note: 设置线条宽度
		*/
		virtual void SetLineWidth( float width );

		/**
		*@note: 获得线条宽度
		*/
		virtual float GetLineWidth() const;

		/**
		*@note: 设置线条宽度
		*/
		virtual void SetLineStipple( unsigned short stipple );

		/**
		*@note: 获得线条宽度
		*/
		virtual unsigned short GetLineStipple() const;

		/**
		*@note: 获得图层属性配置
		*/
		virtual CFeLayerConfig GetLayerConfig();
			
		/**
		*@note: 图层添加后的处理操作，由图层系统在添加图层后调用
		*/
		void PostAddLayer(osgEarth::MapNode* pMapNode);

	public:
		/**
		*@note: 设置贴地方式
		*/
		virtual void SetAltitudeClamp( AltitudeClamp clamp );

		/**
		*@note: 获取贴地方式
		*/
		virtual AltitudeClamp GetAltitudeClamp();

		/**
		*@note: 设置贴地技术
		*/
		virtual void SetAltitudeTechnique( AltitudeTechnique technique );

		/**
		*@note: 获取贴地技术
		*/
		virtual AltitudeTechnique GetAltitudeTechnique();

	protected:
		/**
		*@note: 获取矢量图层FeatureModelGraph节点
		*/
		osgEarth::Features::FeatureModelGraph* GetLayerFeatureModel();

		/**
		*@note: 根据用户配置信息创建矢量图层所需的配置项
		*/
		CFeConfig CreateModelConfig();

	private:
		/**
		*@note: 检测并校验矢量图元类型（目前主要为了解决WFS数据无法获取图元类型的问题）
		*/
		void CheckAndValidFeatureType();

	protected:
		CFeLayerModelConfig            m_layerConfig;

	private:
		// 用于获取矢量图层Feature节点
		osg::observer_ptr<osgEarth::Features::FeatureModelGraph> m_opFeatureModelGraph;	
	};
}

#endif
