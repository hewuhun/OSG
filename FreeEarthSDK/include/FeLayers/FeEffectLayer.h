/**************************************************************************************************
* @file FeEffectLayer.h
* @note 该文件主要封装了特效图层类
* @author c00005
* @data 2016-8-11
**************************************************************************************************/
#ifndef FE_EFFECT_LAYER_H
#define FE_EFFECT_LAYER_H

#include <FeLayers/Export.h>

//#include <osgEarth/Layer>
#include <osgEarth/TerrainEffect>
#include <osgEarth/TerrainEngineNode>

#include <FeLayers/IFeLayer.h>


namespace FeLayers
{
	/**
	* @class CFeEffectLayer
	* @brief 特效图层
	* @note 封装特效图层，提供特效图层的显示和隐藏操作
	* @author l00008
	*/
	class FELAYERS_EXPORT CFeEffectLayer: public IFeLayerEffect //, public osgEarth::Layer
	{
	public:
		/**  
		  * @note 构造函数
		  * @param pEffectLayer [in] 特效图层的指针
		  * @param pTerrainNode [in] 地形构建引擎
		  * @param strName [in] 特效的名称
		  * @param bVisible [in] 特效是否显隐
		*/
		CFeEffectLayer(
			osgEarth::TerrainEffect*		pEffectLayer, 
			osgEarth::TerrainEngineNode*	pTerrainNode, 
			const std::string&				strName,
			bool							bVisible);

		CFeEffectLayer(const CFeLayerEffectConfig& cfg);

		/**  
		  * @note 析构函数
		*/
		virtual ~CFeEffectLayer();

	public:
		/**
		*@note: 设置特效的显隐状态
		*/
		virtual void SetVisible(bool bVisible);

		/**
		*@note: 获取特效的显隐状态
		*/
		virtual bool GetVisible() const;

		/**
		*@note: 设置需要封装的特效
		*/
		void SetEffectLayer(osgEarth::TerrainEffect* pEffectLayer);

		/**
		*@note: 获得封装的特效
		*/
		osgEarth::TerrainEffect* GetEffectLayer();

		/**
		*@note: 设置地形构建引擎
		*/
		void SetTerrainEngineNode(osgEarth::TerrainEngineNode* pTerrainNode);

		/**
		*@note: 获得特效名称
		*/
		virtual const std::string& GetName() const;

		/**
		*@note: 设置特效名称
		*/
		virtual void SetName(const std::string& strName);

		/**
		*@note: 获得图层ID
		*/
		virtual FeLayerID GetLayerUID();

		/**
		*@note: 获得图层属性配置
		*/
		virtual CFeLayerConfig GetLayerConfig();

	protected:
		///地形构建引擎
		osg::observer_ptr<osgEarth::TerrainEngineNode>      m_opTerrainNode;

		///特效图层
		osg::ref_ptr<osgEarth::TerrainEffect>               m_rpEffectLayer;

		///特效的显隐状态
		bool                                                m_bVisible;

		///特效的名称
		std::string                                         m_strName;

		///配置
		CFeLayerEffectConfig                                m_cfg;
	};

	///特效的向量
	//typedef std::vector< osg::ref_ptr<CFeEffectLayer> > EffectLayerVector;
}

#endif//FE_EFFECT_LAYER_H
