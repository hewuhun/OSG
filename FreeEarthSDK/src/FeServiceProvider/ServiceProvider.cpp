#include "FeServiceProvider/ServiceProvider.h"
#include "FeServiceProvider/ServiceLayer.h"
#include "CurlHelper.h"
using namespace FeServiceProvider;

ServiceProvider::ServiceProvider(const std::string & strName, const std::string & strUrl, const std::string & strUserName /*= ""*/, const std::string & strPassword /*= ""*/)
{

	m_strName = strName;
	m_strUrl  = strUrl;
	m_strUserName = strUserName;
	m_strPassword = strPassword;

	int index2 = m_strUrl.find("http://");
	if (index2 != 0 || index2 == std::string::npos)
	{
		m_strUrl = "http://" + m_strUrl;
	}

	//int index = m_strUrl.find("ows");
	//if ( index != std::string::npos)
	//{
	//	m_strUrl = m_strUrl.substr(0, index);
	//}
	//else
	//{
	//	if (m_strUrl.find_last_of("/") != (m_strUrl.length()-1))
	//	{
	//		m_strUrl.append("/");
	//	}
	//}

	int index = m_strUrl.find("?");
	if ( index != std::string::npos)
	{
		m_strUrl = m_strUrl.substr(0, index);
	}

	m_strFullUrl = "";

	m_strPlugin = "";

	m_nLayerCount = 0;

	m_pCurlHelper = new FeServiceProvider::CurlHelper;
}

ServiceProvider::~ServiceProvider()
{
	if (m_pCurlHelper != NULL)
	{
		delete m_pCurlHelper;
		m_pCurlHelper = 0;
	}
}


void ServiceProvider::SetUserName(const std::string & strUserName)
{
	m_strUserName = strUserName;
}

void ServiceProvider::SetPassword(const std::string & strPassword)
{
	m_strPassword = strPassword;
}

void ServiceProvider::SetName(const std::string & strName)
{
	m_strName = strName;
}

void ServiceProvider::SetUrl(const std::string & strUrl)
{
	m_strUrl = strUrl;
}

void ServiceProvider::SetPluginType(const std::string & strPluginType)
{
	m_strPlugin = strPluginType;
}

std::string ServiceProvider::GetUserName()
{
	return m_strUserName;
}

std::string ServiceProvider::GetPassword()
{
	return m_strPassword;
}

std::string ServiceProvider::GetName()
{
	return m_strName;
}

std::string ServiceProvider::GetUrl()
{
	return m_strUrl;
}

std::string ServiceProvider::GetPluginType()
{
	return m_strPlugin;
}


std::string ServiceProvider::GetServiceUrl()
{
	return m_strUrl;
}

bool ServiceProvider::TestNet()
{
	std::string strXml;
	bool ret = false;
	if (m_pCurlHelper != NULL)
	{
		ret = m_pCurlHelper->GetReq(m_strFullUrl, strXml);
	}
	return ret;
}

int ServiceProvider::GetLayerCount()
{
	return m_pLayers.size();
}

std::string ServiceProvider::GetServiceType()
{
	return "";
}

std::vector<ServiceLayer *> ServiceProvider::GetLayers()
{
	return m_pLayers;
}

void ServiceProvider::GetAttr()
{
	m_pLayers.clear();
}

std::string ServiceProvider::GetXml()
{
	std::string strXml;

	if (m_pCurlHelper != NULL)
	{
		m_pCurlHelper->GetReq(m_strFullUrl, strXml);
	}
	return strXml;
}

std::string ServiceProvider::GetCurrentVersion()
{
	return m_strVersion;
}

void ServiceProvider::SetVersion(const std::string & strVersion)
{
	m_strVersion = strVersion;
}

TiXmlElement * ReadXmlByStack(TiXmlElement * pRootElement, char * pName, bool & bFind)
{
	std::stack<TiXmlElement *> ElementStack;

	ElementStack.push(pRootElement);
	TiXmlElement * pTempElement = NULL;
	while (ElementStack.size() > 0)
	{
		pTempElement = ElementStack.top();
		ElementStack.pop();

		if (0 == strcmp(pTempElement->Value(), pName))
		{
			bFind = true;
			break;
		}

		TiXmlElement * pTempSibLing = pTempElement->FirstChildElement();
		do 
		{
			if (pTempSibLing)
			{
				ElementStack.push(pTempSibLing);
				pTempSibLing = pTempSibLing->NextSiblingElement();
			}
		} while (pTempSibLing);
	}

	while (ElementStack.size() > 0)
	{
		ElementStack.pop();
	}

	if (bFind)
	{
		return pTempElement;
	}
	else
	{
		return NULL;
	}
}
