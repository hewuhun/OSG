/**************************************************************************************************
* @file CustomLayerReaderWriter.h
* @note 用户定义图层的配置文件的读写操作
* @author c00005
* @data 2016-10-11
**************************************************************************************************/
#ifndef FE_CUSTOM_LAYER_READER_WRITER_H
#define FE_CUSTOM_LAYER_READER_WRITER_H

#include <FeLayers/Export.h>

#include <osgEarth/XmlUtils>

namespace FeLayers
{
	/**
	* @class CCustomLayerReaderWriter
	* @brief 用户自定义图层配置的读取
	* @note 用于对用户自定义的图层的配置文件进行读写操作
	* @author c00005
	*/
	class FELAYERS_EXPORT CCustomLayerReaderWriter
	{
	public:
		CCustomLayerReaderWriter();

		~CCustomLayerReaderWriter();

	public:
		/**
		*@note: 向配置中添加子配置项
		*/
		void AppendConfig(osgEarth::Config config);

		/**
		*@note: 从指定的路径中读取用户自定义图层的配置
		*/
		osgEarth::Config Load(std::string strPath);

		/**
		*@note: 将用户自定义的图层保存到指定的配置文件中
		*/
		bool Save(std::string strPath);

	protected:
		///用户定义图层的配置信息
		osgEarth::Config					m_rootConfig;
	};
}

#endif//FE_CUSTOM_LAYER_READER_WRITER_H
