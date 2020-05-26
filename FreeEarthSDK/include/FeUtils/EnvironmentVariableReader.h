/**************************************************************************************************
* @file EnvironmentVariableReader.h
* @note 读取环境变量模块
* @author c00005
* @data 2017-4-1
**************************************************************************************************/
#ifndef FE_ENVIRONMENT_VARIABLE_READER_H
#define FE_ENVIRONMENT_VARIABLE_READER_H

#include <string>

#include <FeUtils/Export.h>

namespace FeUtil
{
	/**
      * @class CEnvironmentVariableReader
      * @brief 文件获取模块
      * @note 主要按照指定的环境变量获得其路径
      * @author l00008
    */
	class FEUTIL_EXPORT CEnvironmentVariableReader
	{
	public:
		CEnvironmentVariableReader();

		~CEnvironmentVariableReader();

	public:
		/**
		* @note 执行读取操作，获得指定的环境变量下的路径，如果没有指定默认读取当前的路径,并返回获得的路径
		*/
		std::string DoRead(const std::string& strPathName);
	};

}

#endif //FE_ENVIRONMENT_VARIABLE_READER_H