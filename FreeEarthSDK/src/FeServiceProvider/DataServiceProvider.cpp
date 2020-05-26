#include "DataServiceProvider.h"
using namespace FeServiceProvider;

LocalDataServiceProvider::LocalDataServiceProvider(const std::string & strName, const std::string & strUrl, const std::string & plugin /*= "flatgis"*/)
	:ServiceProvider(strName, strUrl, "", "")
{
	m_strUrl = strUrl;
	m_strPlugin = plugin;
}

LocalDataServiceProvider::~LocalDataServiceProvider()
{

}

std::string LocalDataServiceProvider::GetPluginType()
{
	return m_strPlugin;
}

std::string LocalDataServiceProvider::GetServiceType()
{
	return "LocalData";
}

void LocalDataServiceProvider::SetPluginType(const std::string & strPluginType)
{
	m_strPlugin = strPluginType;
}

