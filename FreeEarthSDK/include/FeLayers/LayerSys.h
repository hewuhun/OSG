/**************************************************************************************************
* @file LayerSys.h
* @note 主要封装了图层系统模块，通过此图层模块可以添加、删除、管理图层
* @author c00005
* @data 2016-8-12
**************************************************************************************************/
#ifndef FE_LAYERS_SYSTEM_H
#define FE_LAYERS_SYSTEM_H

#include <FeLayers/Export.h>
#include <FeLayers/FeEffectLayer.h>

#include <FeUtils/SceneSysCall.h>

#include "IFeLayer.h"
#include "IFeLayerFactory.h"

namespace FeLayers
{
	/**
	* @class CLayerSysOptions
	* @brief 图层系统配置选项
	* @author l00008
	*/
	class FELAYERS_EXPORT CLayerSysOptions
	{
	public:
		CLayerSysOptions();

		~CLayerSysOptions();

	public:
		/**  
		* @brief 矢量的配置路径
		* @note 矢量配置单独保存在一个文件中，此函数用于返回配置路径
		*/
		std::string& featurePath(){return m_strFeaturePath;}

		/**  
		* @brief 用户自定义图层的配置路径
		*/
		std::string& customLayerPath(){return m_strCustomLayerPath;}

	protected:
		///矢量配置文件路径
		std::string			m_strFeaturePath;

		///用户自定义图层的配置路径
		std::string			m_strCustomLayerPath;
	};
}

namespace FeLayers
{
	// 定义图层容器
	typedef std::vector< FeLayerID >                        CLayerIDSet;
	typedef std::vector< osg::ref_ptr<IFeLayer> >		    CLayersSet;
	typedef std::vector< osg::ref_ptr<IFeLayerImage> >		CImageLayersSet;
	typedef std::vector< osg::ref_ptr<IFeLayerModel> >		CModelLayersSet;
	typedef std::vector< osg::ref_ptr<IFeLayerElevation> >	CElevationLayersSet;
	typedef std::vector< osg::ref_ptr<IFeLayerEffect> >	    CEffectLayersSet;
	
	///图层管理模块的标识值
	const std::string LAYER_SYSTEM_CALL_DEFAULT_KEY("LayerSystem");

	/**
	* @class CLayerSys
	* @brief 图层系统
	* @note 图层系统，封装了对图层管理的所有功能，包括增加、删除、显示隐藏等功能
	* @author l00008
	*/
	class FELAYERS_EXPORT CLayerSys : public FeUtil::CSceneSysCall
	{
	public:
		CLayerSys(CLayerSysOptions opt);

		virtual ~CLayerSys();

	public:
		/**
		*@note: 获取图层管理的配置信息
		*/
		CLayerSysOptions& GetOptions();

	protected:
		/**
		*@note: 实现本系统模型需要的初始化函数，此函数在父类中调用
		*/
		virtual bool InitiliazeImplement();

		/**
		*@note: 实现本系统模型需要的反初始化函数，此函数在父类中调用
		*/
		virtual bool UnInitiliazeImplement();

	public:
		/**
		*@note: 获取图层创建工厂接口
		*/
		ILayerFactory* LayerFactory();
		
		/**
		* @note: 添加一个地图图层，并返回是否添加成功
		* @param pLayer [in] 图层
		*/
		bool AddLayer(IFeLayer* pLayer);
		IFeLayer* AddLayer(const CFeLayerConfig& cfg);
	
		/**
		*@note:删除图层
		*/
		bool DeleteLayer(IFeLayer* pLayer);
		bool DeleteLayer(FeLayerID id);

		/**
		*@note:清空图层
		*/
		void ClearLayers();

		/**
		*@note: 获得所有图层
		* @param vecLayer [out] 当前系统中的所有图层
		*/
		void GetLayers(CLayersSet& vecLayer);

		/**
		*@note: 获得所有图层数量
		*/
		unsigned int GetLayersCount();

		/**
		*@note: 获得指定索引的图层
		*/
		IFeLayer* GetLayerAt(unsigned int nIndex);

		/**
        *@note: 清空系统
        */
        virtual bool ClearSys(){ ClearLayers(); return true; }

	public:
		/**
		*@note: 获得所有影像图层
		* @param vecLayer [out] 当前系统中的所有的影像图层
		*/
		void GetImageLayers(CImageLayersSet& vecLayer);

		/**
		*@note: 获得所有矢量图层
		* @param vecLayer [out] 系统中所有的矢量图层
		*/
		void GetModelLayers(CModelLayersSet& vecLayer);
		
		/**
		*@note: 获得所有高程图层
		* @param vecLayer [out] 系统中所有的高程图层
		*/
		void GetElevationLayers(CElevationLayersSet& vecLayer);

		/**
		*@note: 获得所有特效图层
		* @param vecLayer [out] 系统中所有的特效图层
		*/
		void GetEffectLayers(CEffectLayersSet& vecLayer);

	public:
		// 系统图层： 即通过配置文件（earth或其他）加载的预定义图层
		// 这些图层直接通过osgEarth加载，本系统只提供获取接口 
		/**
		*@note: 获得所有影像图层
		* @param vecLayer [out] 当前系统中的所有的影像图层
		*/
		void GetImageLayers(osgEarth::ImageLayerVector& vecLayer);

		/**
		*@note: 获得所有矢量图层
		* @param vecLayer [out] 系统中所有的矢量图层
		*/
		void GetModelLayers(osgEarth::ModelLayerVector& vecLayer);
		
		/**
		*@note: 获得所有高程图层
		* @param vecLayer [out] 系统中所有的高程图层
		*/
		void GetElevationLayers(osgEarth::ElevationLayerVector& vecLayer);

	public:
		/**
		*@note: 添加一个特效图层，并返回添加的特效图层
		* @param pTerrainEffect [in] 特效图层
		* @param strName [in] 特效图层的名称
		* @param bVisible [in] 特效图层是否可见
		*/
		IFeLayerEffect* AddEffectLayer(osgEarth::TerrainEffect* pTerrainEffect, const std::string& strName, bool bVisible);

		/**
		*@note: 通过特效图层的名字获得特效图层
		*/
		IFeLayerEffect* GetEffectLayer(const std::string& strName);

	public:
		/**
		*@note: 添加一个颜色过滤器
		*/
		void AddImageColorFilter(osgEarth::ColorFilter* pColorFilter);

		/**
		*@note: 移除一个颜色过滤器
		*/
		void RemoveImageColorFilter(osgEarth::ColorFilter* pColorFilter);

		/**
		*@note: 移除所有的颜色过滤器
		*/
		void RemoveImageColorFilters();

		/**
		*@note: 注册并初始化特效图层
		*/
		void RegistryEffectLayer();

		/**
		*@note: 注册并初始化Feature(矢量)图层
		*/
		void RegistryFeatureLayer();

	protected:
		/**
		*@note: 获得Map
		*/
		osgEarth::Map* GetMap();

		/**
		*@note: 获得MapNode
		*/
		osgEarth::MapNode* GetMapNode();
		
		/**
		* @note: 添加一个卫星影像图层，并返回是否添加成功
		* @param pLayer [in] 卫星影像图层
		*/
		bool AddImageLayer(IFeLayerImage* pLayer);
	
		/**
		*@note:删除卫星影像图层
		* @param pLayer [in] 卫星影像图层
		*/
		bool DeleteImageLayer(IFeLayerImage* pLayer);

		/**
		*@note: 添加一个矢量图层，并返回是否添加成功
		* @param pLayer [in] 矢量图层
		*/
		bool AddModelLayer(IFeLayerModel* pLayer);

		/**
		*@note:删除矢量图层
		* @param pLayer [in] 矢量图层
		*/
		bool DeleteModelLayer(IFeLayerModel* pLayer);

		/**
		*@note: 添加一个高程图层，并返回是否添加成功
		* @param pLayer [in] 高程图层
		*/
		bool AddElevationLayer(IFeLayerElevation* pLayer);

		/**
		*@note:删除高程图层
		* @param pLayer [in] 高程图层
		*/
		bool DeleteElevationLayer(IFeLayerElevation* pLayer);

		/**
		*@note: 添加一个特效图层，并返回是否添加成功
		* @param pLayer [in] 特效图层
		*/
		bool AddEffectLayer(IFeLayerEffect* pLayer);

		/**
		*@note:删除特效图层
		* @param pLayer [in] 特效图层
		*/
		bool DeleteEffectLayer(IFeLayerEffect* pLayer);

	protected:
		///保存特效图层不被释放
		//EffectLayerVector           m_mapEffectLayer;
		CEffectLayersSet              m_mapEffectLayer;

		///图层管理的配置信息
		CLayerSysOptions			m_optLayerConfig;
	
		/// 图层容器
		CLayersSet                  m_layerSet;
		CImageLayersSet				m_layersImage;
		CModelLayersSet				m_layersModel;
		CElevationLayersSet         m_layersElevation;

		// 系统图层容器
		osgEarth::ImageLayerVector  m_sysLayerImage;
		osgEarth::ModelLayerVector  m_sysLayerModel;
		osgEarth::ElevationLayerVector  m_sysLayerElevation;

	private:
		OpenThreads::Mutex			m_mapDataMutex;
	};
}

#endif//FE_LAYERS_SYSTEM_H
