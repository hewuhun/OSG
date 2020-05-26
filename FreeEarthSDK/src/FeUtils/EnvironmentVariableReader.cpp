#include <stdio.h>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <EnvironmentVariableReader.h>

namespace FeUtil
{
	CEnvironmentVariableReader::CEnvironmentVariableReader()
	{

	};

	CEnvironmentVariableReader::~CEnvironmentVariableReader()
	{

	}

	std::string CEnvironmentVariableReader::DoRead(const std::string& strPathName)
	{
		std::string strDataPath = "";
#ifdef RELEASE_ON
		strDataPath = "./Data";
		strDataPath = osgDB::getRealPath(strDataPath);
#else
		const char* pFilePath = NULL;
		if(pFilePath = getenv(strPathName.c_str()))
		{
			strDataPath.assign(pFilePath);
		}

		if(!strDataPath.empty() && (*strDataPath.rbegin()) == ';')
		{
			strDataPath.erase(strDataPath.size() - 1);
		}
#endif

		return strDataPath;
	}

}


