/**************************************************************************************************
* @file FeLayerSerialize.h
* @note 地图层序列化
* @author g00034
* @data 2018-8-13
**************************************************************************************************/
#ifndef FE_LAYERS_SERIALIZE_H
#define FE_LAYERS_SERIALIZE_H

#include <FeUtils/tinyXml/tinyxml.h>

namespace FeLayers
{

	/**
	  * @class CFeLayerSerialize
	  * @brief 地图层序列化类
	  * @author g00034
	*/
	class CFeLayerSerialize
	{
	public:
		CFeLayerSerialize(CLayerSys* pLayerSys);

		/**  
		  * @brief 保存图层到指定文件
		  * @param filePath [in] 文件路径
		  * @return 无
		*/
		void SaveTo(const std::string& filePath);

		/**  
		  * @brief 保存图层到 xml 节点
		  * @param pRootElement [in] xml 节点
		  * @return 无
		*/
		void SaveTo(FeUtil::TiXmlElement* pRootElement);

		/**
		* @note: 从文件中加载图层信息
		* @param filePath [in] 文件路径
		* @return 成功返回true，否则返回false
		*/
		bool LoadFrom(const std::string& filePath);

		/**
		* @note: 从 xml 节点中加载图层信息
		* @param pRootElement [in]  xml 节点
		* @return 成功返回true，否则返回false
		*/
		bool LoadFrom(FeUtil::TiXmlElement* pRootElement);
		
		/**  
		  * @brief 保存指定图层到指定文件
		  * @param filePath [in] 文件路径
		  * @param layerIDSet [in] 需要序列化的图层ID集合
		*/
		void SaveTo(const CLayerIDSet& layerIDSet, const std::string& filePath);

		/**  
		  * @brief 保存指定图层到指定文件
		  * @param filePath [in] 文件路径
		  * @param layerSet [in] 需要序列化的图层集合
		*/
		void SaveTo(const CLayersSet& layerSet, const std::string& filePath);


	private:
		/**
		*@note: 获取图层系统配置信息
		*/
		CFeConfig GetLayersConfig(const CLayersSet& layerSet);

		/**
		*@note: 添加图层集
		*/
		void AddLayersFromConfig(const CFeConfig& layersConf);
		

	private:
		// 图层系统
		osg::observer_ptr<CLayerSys> m_opLayerSys;
	};


}


#endif // FE_LAYERS_SERIALIZE_H

