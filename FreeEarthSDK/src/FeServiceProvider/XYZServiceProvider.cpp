#include "XYZServiceProvider.h"

using namespace FeServiceProvider;
XYZServiceProvider::XYZServiceProvider(const std::string & strName, const std::string & strUrl, const std::string & strUserName /*= ""*/, const std::string & strPassword /*= ""*/)
	:ServiceProvider(strName, strUrl, strUserName, strPassword)
{
	m_strUrl = strUrl;
}

XYZServiceProvider::~XYZServiceProvider()
{

}

std::string XYZServiceProvider::GetServiceType()
{
	return "XYZ";
}



