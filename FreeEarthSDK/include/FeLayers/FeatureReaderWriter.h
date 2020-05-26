/**************************************************************************************************
* @file FeatureReaderWriter.h
* @note 地理要素配置文件读取写入工具
* @author c00005
* @data 2016-9-9
**************************************************************************************************/
#ifndef FE_LAYERS_FEATURE_READER_WRITER_H
#define FE_LAYERS_FEATURE_READER_WRITER_H

#include <FeLayers/Export.h>

#include <osgDB/Options>

#include <osgEarth/Registry>
#include <osgEarth/URI>
#include <osgEarth/XmlUtils>

namespace FeLayers
{
	/**
	* @class CFeatureReaderWriter
	* @note 地理要素配置文件读取写入工具类
	* @author c00005
	*/
	class CFeatureReaderWriter
	{
	public:
		CFeatureReaderWriter();

		~CFeatureReaderWriter();

	public:
		/**
		*@note: 加载地理要素文件，通过指定地理要素配置文件路径
		*/
		bool Read(const std::string& strFilePath, osgEarth::ConfigSet& configSets);
	};
}

#endif//FE_LAYERS_FEATURE_READER_WRITER_H
