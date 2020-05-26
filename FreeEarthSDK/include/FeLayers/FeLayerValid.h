/**************************************************************************************************
* @file FeLayerValid.h
* @note 图层配置信息校验
* @author g00034
* @data 2018-9-4
**************************************************************************************************/
#ifndef FE_LAYER_VALID_H
#define FE_LAYER_VALID_H

#include <FeLayerConfig.h>

namespace FeLayers
{
	/**
	  * @class CLayerConfigValidator
	  * @brief 地图图层配置校验
	  * @note 对地图图层配置进行校验操作，默认值设置等
	  * @author g00034
	*/
	class CLayerConfigValidator
	{
	public:
		CLayerConfigValidator(CFeLayerConfig& cfg) : m_cfg(cfg) {}

	public:
		// 校验
		bool CheckAndValid();

		// 判断是否为矢量图层
		//bool IsModelLayerConfig();

		// 判断是否为影像图层
		//bool IsImageLayerConfig();

	protected:
		// 初始化图层配置，设置默认配置
		//void InitLayerConfig();
		void InitModelLayerConfig(CFeLayerModelConfig& cfg);
		void InitImageLayerConfig(CFeLayerImageConfig& cfg);

		// 图层属性校验
		//bool LayerValid();
		void ValidModelLayer(CFeLayerModelConfig& cfg);
		void ValidImageLayer(CFeLayerImageConfig& cfg);

	private:
		CFeLayerConfig& m_cfg;
	};



	/**
	  * @class CLayerDataSourceValidator
	  * @brief 地图数据源校验
	  * @note 对地图数据源进行校验操作
	  * @author g00034
	*/
	class CLayerDataSourceValidator
	{
	public:
		CLayerDataSourceValidator(CFeDataSource& dataSource) : m_dataSource(dataSource) {}

	public:
		// 校验
		bool CheckAndValid();

	protected:
		// 驱动名校验
		bool DriverNameValid();

		// 图层格式名校验
		bool LayerFormatValid();

		// 图层投影校验
		bool SRSValid();

		// URL校验
		bool URLValid();

		// 部分数据源，字段名等需要进行特殊处理
		void LayerConfigExtraHandle();

	private:
		CFeDataSource& m_dataSource;
	};

}


#endif // FE_LAYER_VALID_H

