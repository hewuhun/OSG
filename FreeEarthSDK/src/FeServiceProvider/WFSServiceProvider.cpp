#include "WFSServiceProvider.h"
#include "ServiceLayer.h"
using namespace FeServiceProvider;
WFSServiceProvider::WFSServiceProvider(const std::string & strName, const std::string & strUrl, const std::string & strUserName /* = "" */, const std::string & strPassword /* = "" */)
	:ServiceProvider(strName, strUrl, strUserName, strPassword)
{
	m_strVersion = "2.0.0";
	m_strFullUrl = m_strUrl + "?service=wfs&version=" + m_strVersion+ "&request=GetCapabilities";
}

WFSServiceProvider::~WFSServiceProvider()
{

}

void WFSServiceProvider::GetAttr()
{
	ServiceProvider::GetAttr();
	std::string strXml = GetXml();

	FeUtil::TiXmlDocument doc;
	doc.Parse(strXml.c_str());


	FeUtil::TiXmlElement * root = doc.RootElement();
	if (root != NULL)
	{
		bool find;
		TiXmlElement * layerRoot = ReadXmlByStack(root, "FeatureTypeList", find);

		if (layerRoot != NULL)
		{
			TiXmlElement * ele = layerRoot->FirstChildElement("FeatureType");

			for (ele; ele; ele = ele->NextSiblingElement())
			{
				ServiceLayer * serviceLayer = new ServiceLayer;
				m_pLayers.push_back(serviceLayer);
				serviceLayer->SetServiceProvider(this);
				if (ele != NULL)
				{
					FeUtil::TiXmlElement * nameEle = ele->FirstChildElement("Name");
					FeUtil::TiXmlElement * bbEle = ele->FirstChildElement("ows:WGS84BoundingBox");

					if (nameEle != NULL)
					{
						serviceLayer->SetName(nameEle->GetText());
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

std::string WFSServiceProvider::GetServiceType()
{
	return "WFS";
}

std::string WFSServiceProvider::GetServiceUrl()
{
	return m_strUrl;
}

void WFSServiceProvider::SetVersion(const std::string & strVersion)
{
	m_strVersion = strVersion;
	m_strFullUrl = m_strUrl + "?service=wfs&version=" + m_strVersion + "&request=GetCapabilities";
}
