/**************************************************************************************************
* @file NormalMapEffect.h
* @note 地形的NormalMap法线贴图的效果实现
* @author c00005
* @data 2015-9-29
**************************************************************************************************/
#ifndef FE_NORMAL_MAP_EFFECT_H
#define FE_NORMAL_MAP_EFFECT_H

#include <FeLayers/Export.h>

#include <osgEarthUtil/Common>
#include <osgEarth/TerrainEffect>
#include <osgEarth/ImageLayer>

namespace FeLayers
{
	using namespace osgEarth;
	using namespace osgEarth::Util;

	/**
	* @class CNormalMapEffect
	* @note 地形的NormalMap法线贴图的效果实现,继承自osgEarth::TerrainEffect
	* @author c00005
	*/
	class FELAYERS_EXPORT CNormalMapEffect : public TerrainEffect
	{
	public:
		/**  
		  * @note 构造法线光照特效  
		*/
		CNormalMapEffect();

		/**  
		  * @note 构造法线光照特效  
		  * @param conf [in] 法线光照特效的配置文件，用于保存特效依赖图层的名称
		  * @param map [in] 用于根据名称查找特效依赖的图层
		*/
		CNormalMapEffect(const Config& conf, osgEarth::Map* map);

		/**  
		  * @note 析构函数  
		*/
		virtual ~CNormalMapEffect();

	public: 
		/**  
		  * @note 安装特效，即启动法线光照特效  
		  * @param engine [in] 三维数字地球渲染的地球引擎
		*/
		void onInstall(TerrainEngineNode* engine);

		/**  
		  * @note 卸载特效，即关闭法线光照特效  
		  * @param engine [in] 三维数字地球渲染的地球引擎
		*/
		void onUninstall(TerrainEngineNode* engine);

	public:
		/**  
		  * @note 设置特效依赖的数据图层
		*/
		void setNormalMapLayer(ImageLayer* layer);

		/**  
		  * @note 获取特效依赖的数据图层
		*/
		ImageLayer* getNormalMapLayer();
	
		/**  
		  * @note 获取特效的配置信息
		*/
		virtual Config getConfig() const;

	protected:
		///特效依赖图层的名称
		optional<std::string>							m_strLayerName;

		//特效依赖的图层
		osg::observer_ptr<ImageLayer>		m_opImageLayer;

		//一致性变量，用于保存特效图层
		osg::ref_ptr<osg::Uniform>				m_rpSamplerUniform;
	};

} 

#endif // FE_NORMAL_MAP_EFFECT_H
