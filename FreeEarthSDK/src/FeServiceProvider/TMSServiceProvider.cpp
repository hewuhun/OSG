#include "TMSServiceProvider.h"
#include <iostream>
using namespace FeServiceProvider;
TMSServiceProvider::TMSServiceProvider(const std::string & strName, const std::string & strUrl, const std::string & strUserName /* = "" */, const std::string & strPassword /* = "" */)
	:ServiceProvider(strName, strUrl, strUserName, strPassword)
{
	m_strFullUrl = strUrl + "gwc/service/tms/1.0.0";
}

TMSServiceProvider::~TMSServiceProvider()
{

}

void TMSServiceProvider::GetAttr()
{
	ServiceProvider::GetAttr();
}

std::string TMSServiceProvider::GetServiceType()
{
	return "TMS";
}

