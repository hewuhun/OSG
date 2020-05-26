/**************************************************************************************************
* @file IFeLayerFactory.h
* @note 图层工厂接口
* @author g00034
* @data 2018-7-16
**************************************************************************************************/
#ifndef FE_IFELAYER_FACTORY_H
#define FE_IFELAYER_FACTORY_H

#include "IFeLayer.h"

namespace FeLayers
{
	class CFeLayerConfig;

	/**
	  * @class ILayerFactory
	  * @brief 图层工厂接口类
	  * @author g00034
	*/
	class ILayerFactory
	{
	public:
		virtual ~ILayerFactory(){}

		/**  
		  * @brief 创建一个地图图层
		  * @note: 内部根据图层配置属性判断创建的具体图层类型
		  * @param cfg [in] 图层配置信息
		  * @return 返回创建的图层
		*/
		virtual IFeLayer* CreateLayer(const CFeLayerConfig& cfg) = 0;
	};
}

#endif


