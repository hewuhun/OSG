#include "WCSServiceProvider.h"
#include "ServiceLayer.h"

using namespace FeServiceProvider;

WCSServiceProvider::WCSServiceProvider(const std::string & strName, const std::string & strUrl, const std::string & strUserName /*= ""*/, const std::string & strPassword /*= ""*/)
	:ServiceProvider(strName, strUrl, strUserName, strPassword)
{
	m_strVersion = "1.0.0";
	m_strFullUrl = m_strUrl + "?service=wcs&version=" + m_strVersion+ "&request=GetCapabilities";
	
	
}

WCSServiceProvider::~WCSServiceProvider()
{

}

void WCSServiceProvider::GetAttr()
{
	std::string strXml = GetXml();

	FeUtil::TiXmlDocument doc;
	doc.Parse(strXml.c_str());


	FeUtil::TiXmlElement * root = doc.RootElement();
	if (root != NULL)
	{
		bool find;
		if (m_strVersion == "1.0.0")
		{
			TiXmlElement * layerRoot = ReadXmlByStack(root, "wcs:ContentMetadata", find);

			if (layerRoot != NULL)
			{
				TiXmlElement * ele = layerRoot->FirstChildElement("wcs:CoverageOfferingBrief");

				for (ele; ele; ele = ele->NextSiblingElement())
				{
					ServiceLayer * serviceLayer = new ServiceLayer;
					m_pLayers.push_back(serviceLayer);
					serviceLayer->SetServiceProvider(this);
					if (ele != NULL)
					{
						FeUtil::TiXmlElement * nameEle = ele->FirstChildElement("wcs:name");
						FeUtil::TiXmlElement * bbEle = ele->FirstChildElement("wcs:lonLatEnvelope");
						if (nameEle != NULL)
						{
							serviceLayer->SetName(nameEle->GetText());
						}
						if (bbEle != NULL)
						{
							FeUtil::TiXmlElement * pos1 = bbEle->FirstChildElement("gml:pos");
							serviceLayer->SetLowerCorner(pos1->GetText());
							if (pos1 != NULL)
							{
								FeUtil::TiXmlElement * pos2 = pos1->NextSiblingElement();
								serviceLayer->SetUpperCorner(pos2->GetText());
								serviceLayer->SetCRSCode("WGS84");
							}
						}

					}
				}
			}
		}
		else if (m_strVersion == "2.0.1")
		{
			TiXmlElement * layerRoot = ReadXmlByStack(root, "wcs:Contents", find);

			if (layerRoot != NULL)
			{
				TiXmlElement * ele = layerRoot->FirstChildElement("wcs:CoverageSummary");

				for (ele; ele; ele = ele->NextSiblingElement())
				{
					ServiceLayer * serviceLayer = new ServiceLayer;
					m_pLayers.push_back(serviceLayer);
					serviceLayer->SetServiceProvider(this);
					if (ele != NULL)
					{
						FeUtil::TiXmlElement * nameEle = ele->FirstChildElement("wcs:CoverageId");
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
}

std::string WCSServiceProvider::GetServiceType()
{
	return "WCS";
}

std::string WCSServiceProvider::GetServiceUrl()
{
	return m_strUrl;
}

void WCSServiceProvider::SetVersion(const std::string & strVersion)
{
	m_strVersion = strVersion;
	m_strFullUrl = m_strUrl + "?service=WCS&version=" + m_strVersion + "&request=GetCapabilities";
}

