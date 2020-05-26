#include "FeServiceProvider/ServiceManager.h"

#include "FeServiceProvider/ServiceProvider.h"
#include "FeServiceProvider/TMSServiceProvider.h"
#include "FeServiceProvider/WCSServiceProvider.h"
#include "FeServiceProvider/WFSServiceProvider.h"
#include "FeServiceProvider/WMSServiceProvider.h"
#include "FeServiceProvider/WMTSServiceProvider.h"
#include "FeServiceProvider/XYZServiceProvider.h"
#include "FeServiceProvider/DataServiceProvider.h"
#include <iostream>
using namespace FeServiceProvider;
ServiceManager::ServiceManager()
{

}

ServiceManager::~ServiceManager()
{

}


void ServiceManager::AddService(ServiceProvider * serviceProvider)
{
	if (serviceProvider == NULL)
	{
		return;
	}

	for (std::vector<ServiceProvider *>::iterator iter = m_vecServices.begin(); iter != m_vecServices.end(); ++iter)
	{
		if (*iter == serviceProvider)
		{
			return;
		}
	}

	m_vecServices.push_back(serviceProvider);
}


void ServiceManager::RemoveService(ServiceProvider * serviceProvider)
{
	if (serviceProvider == NULL || m_vecServices.size() == 0)
	{
		return;
	}
	std::vector<ServiceProvider *>::iterator it;
	for (std::vector<ServiceProvider *>::iterator iter = m_vecServices.begin(); iter != m_vecServices.end(); ++iter)
	{
		if (*iter == serviceProvider)
		{
			it = iter;
		}
	}

	m_vecServices.erase(it);
}

void ServiceManager::Serialize(const std::string & fileName)
{
	FeUtil::TiXmlDocument doc;
	FeUtil::TiXmlDeclaration declar("1.0", "UTF-8", "");
	doc.InsertEndChild(declar);

	FeUtil::TiXmlElement * servicesEle = new FeUtil::TiXmlElement("Services");
	

	Seria(servicesEle);

	doc.LinkEndChild(servicesEle);
	doc.SaveFile(fileName.c_str());


}


void ServiceManager::Serialize(FeUtil::TiXmlDocument * doc)
{
	if (doc == NULL)
	{
		return;
	}
	FeUtil::TiXmlElement * servicesEle = new FeUtil::TiXmlElement("Services");
	Seria(servicesEle);
	doc->LinkEndChild(servicesEle);
}

void ServiceManager::DeSerialize(const std::string & fileName)
{
	FeUtil::TiXmlDocument doc(fileName.c_str());
	bool loadOkay = doc.LoadFile();
	if (loadOkay)
	{
		FeUtil::TiXmlElement * servicesEle = doc.RootElement();
	
		if (std::string(servicesEle->Value()) != "Services")
		{
			servicesEle = servicesEle->FirstChild("Services")->ToElement();
		}
		DeSeria(servicesEle);
		
	}
}

void ServiceManager::DeSerialize(FeUtil::TiXmlDocument * doc)
{
	if (doc == NULL)
	{
		return;
	}
	FeUtil::TiXmlElement * servicesEle = doc->RootElement();

	if (std::string(servicesEle->Value()) != "Services")
	{
		servicesEle = servicesEle->FirstChild("Services")->ToElement();
	}
	DeSeria(servicesEle);
}

std::vector<ServiceProvider *> ServiceManager::GetServices()
{
	return m_vecServices;
}

void ServiceManager::Seria(FeUtil::TiXmlElement * servicesEle)
{
	for (std::vector<ServiceProvider *>::iterator iter = m_vecServices.begin(); iter != m_vecServices.end(); ++iter)
	{
		if (*iter != NULL)
		{
			std::string serviceType = (*iter)->GetServiceType();
			FeUtil::TiXmlElement * serviceEle = NULL;
			FeUtil::TiXmlElement * name = new FeUtil::TiXmlElement("Name");
			FeUtil::TiXmlText * nameText = new FeUtil::TiXmlText((*iter)->GetName().c_str());

			FeUtil::TiXmlElement * userName = new FeUtil::TiXmlElement("UserName");
			FeUtil::TiXmlText * userNameText = new FeUtil::TiXmlText((*iter)->GetUserName().c_str());

			FeUtil::TiXmlElement * passWord = new FeUtil::TiXmlElement("Password");
			FeUtil::TiXmlText * passWordText = new FeUtil::TiXmlText((*iter)->GetPassword().c_str());

			FeUtil::TiXmlElement * url = new FeUtil::TiXmlElement("Url");
			FeUtil::TiXmlText * urlText = new FeUtil::TiXmlText((*iter)->GetUrl().c_str());

			FeUtil::TiXmlElement * plugin = new FeUtil::TiXmlElement("Plugin");
			FeUtil::TiXmlText * pluginText = new FeUtil::TiXmlText((*iter)->GetPluginType().c_str());

			if (serviceType == "TMS")
			{
				serviceEle = new FeUtil::TiXmlElement("TMS");
			}
			else if (serviceType == "WCS")
			{
				serviceEle = new FeUtil::TiXmlElement("WCS");
			}
			else if (serviceType == "WFS")
			{
				serviceEle = new FeUtil::TiXmlElement("WFS");
			}
			else if (serviceType == "WMS")
			{
				serviceEle = new FeUtil::TiXmlElement("WMS");
			}
			else if (serviceType == "WMTS")
			{
				serviceEle = new FeUtil::TiXmlElement("WMTS");
			}
			else if (serviceType == "XYZ")
			{
				serviceEle = new FeUtil::TiXmlElement("XYZ");
			}
			else if (serviceType == "LocalData")
			{
				serviceEle = new FeUtil::TiXmlElement("LocalData");
			}


			servicesEle->LinkEndChild(serviceEle);

			serviceEle->LinkEndChild(name);
			name->LinkEndChild(nameText);

			serviceEle->LinkEndChild(userName);
			userName->LinkEndChild(userNameText);

			serviceEle->LinkEndChild(passWord);
			passWord->LinkEndChild(passWordText);

			serviceEle->LinkEndChild(url);
			url->LinkEndChild(urlText);

			serviceEle->LinkEndChild(plugin);
			plugin->LinkEndChild(pluginText);

		}
	}
}

void ServiceManager::DeSeria(FeUtil::TiXmlElement * servicesEle)
{
	if (servicesEle->FirstChild() == NULL)
	{
		return;
	}
	FeUtil::TiXmlElement * ele = servicesEle->FirstChild()->ToElement();

	for (ele; ele; ele = ele->NextSiblingElement())
	{
		ServiceProvider * provider = NULL;
		const char * userName = ele->FirstChild("UserName")->ToElement()->GetText();
		const char * passWord = ele->FirstChild("Password")->ToElement()->GetText();
		const char * name = ele->FirstChild("Name")->ToElement()->GetText();
		const char * url = ele->FirstChild("Url")->ToElement()->GetText();

		std::string strUserName;
		std::string strPassword;
		std::string strName;
		std::string strUrl;

		if (userName == NULL)
		{
			strUserName = "";
		}
		else
		{
			strUserName = userName;
		}

		if (passWord == NULL)
		{
			strPassword = "";
		}
		else
		{
			strPassword = passWord;
		}

		if (name == NULL)
		{
			strName = "";
		}
		else
		{
			strName = name;
		}

		if (url == NULL)
		{
			strUrl = "";
		}
		else
		{
			strUrl = url;
		}

		if (std::string(ele->Value()) == "TMS")
		{


			provider = new TMSServiceProvider( 
				strName,
				strUrl,
				strUserName,
				strPassword
				);

		}
		else if (std::string(ele->Value()) == "WCS")
		{
			provider = new WCSServiceProvider( 
				strName,
				strUrl,
				strUserName,
				strPassword
				);
		}
		else if (std::string(ele->Value()) == "WFS")
		{
			provider = new WFSServiceProvider( 
				strName,
				strUrl,
				strUserName,
				strPassword
				);
		}
		else if (std::string(ele->Value()) == "WMS")
		{
			provider = new WMSServiceProvider( 
				strName,
				strUrl,
				strUserName,
				strPassword
				);
		}
		else if (std::string(ele->Value()) == "WMTS")
		{
			provider = new WMTSServiceProvider( 
				strName,
				strUrl,
				strUserName,
				strPassword
				);
		}
		else if (std::string(ele->Value()) == "XYZ")
		{
			provider = new XYZServiceProvider(
				strName,
				strUrl,
				strUserName,
				strPassword
				);
		}
		else if (std::string(ele->Value()) == "LocalData")
		{
			provider = new LocalDataServiceProvider(
				ele->FirstChild("Name")->ToElement()->GetText(),
				ele->FirstChild("Url")->ToElement()->GetText(),
				ele->FirstChild("Plugin")->ToElement()->GetText());
		}

		m_vecServices.push_back(provider);
	}
}

void FeServiceProvider::ServiceManager::CleanService()
{
	for (std::vector<ServiceProvider *>::iterator iter = m_vecServices.begin(); iter != m_vecServices.end(); ++iter)
	{
		if (*iter != NULL)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	m_vecServices.clear();
}
