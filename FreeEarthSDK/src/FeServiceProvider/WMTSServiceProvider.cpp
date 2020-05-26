#include "WMTSServiceProvider.h"
#include "ServiceLayer.h"
#include <iostream>
using namespace  FeServiceProvider;
WMTSServiceProvider::WMTSServiceProvider(const std::string & strName, const std::string & strUrl, const std::string & strUserName /*= ""*/, const std::string & strPassword /*= ""*/)
	:ServiceProvider(strName, strUrl, strUserName, strPassword)
{
	m_strFullUrl = m_strUrl + "?service=wmts&version=" + m_strVersion+ "&request=GetCapabilities";
}

WMTSServiceProvider::~WMTSServiceProvider()
{

}

void WMTSServiceProvider::GetAttr()
{
	ServiceProvider::GetAttr();
	std::string strXml = GetXml();

	FeUtil::TiXmlDocument doc;
	doc.Parse(strXml.c_str());


	FeUtil::TiXmlElement * root = doc.RootElement();
	if (root != NULL)
	{
		bool find;
		TiXmlElement * contents = ReadXmlByStack(root, "Contents", find);

		if (contents != NULL)
		{
			TiXmlElement * ele = contents->FirstChildElement("Layer");

			for (ele; ele; ele = ele->NextSiblingElement())
			{
				const char * name = ele->Value();
				if (name == NULL)
				{
					return;
				}
				if (std::string(ele->Value()) != std::string("Layer"))
				{
					return;
				}
				ServiceLayer * serviceLayer = new ServiceLayer;
				serviceLayer->SetServiceProvider(this);
				m_pLayers.push_back(serviceLayer);

				if (ele != NULL)
				{
					FeUtil::TiXmlElement * bbEle = ele->FirstChildElement("ows:WGS84BoundingBox");
					FeUtil::TiXmlElement * identEle = ele->FirstChildElement("ows:Identifier");

					if (identEle != NULL)
					{
						serviceLayer->SetName(identEle->GetText());
					}

					if (bbEle != NULL)
					{
						FeUtil::TiXmlElement * lowerEle = bbEle->FirstChildElement("ows:LowerCorner");
						FeUtil::TiXmlElement * upperEle = bbEle->FirstChildElement("ows:UpperCorner");
						if (lowerEle != NULL)
						{
							serviceLayer->SetLowerCorner(lowerEle->GetText());
						}
						if (upperEle != NULL)
						{
							serviceLayer->SetUpperCorner(upperEle->GetText());
						}
						serviceLayer->SetCRSCode("WGS84");
					}
				}
			}
		}
	}
}

std::string WMTSServiceProvider::GetServiceType()
{
	return "WMTS";
}

std::string WMTSServiceProvider::GetServiceUrl()
{
	return m_strUrl;
}

