#include "WMSServiceProvider.h"
#include "ServiceLayer.h"
#include <iostream>

using namespace FeServiceProvider;
WMSServiceProvider::WMSServiceProvider(const std::string & strName, const std::string & strUrl, const std::string & strUserName /* = "" */, const std::string & strPassword /* = "" */)
	:ServiceProvider(strName, strUrl, strUserName, strPassword)
{
	m_strVersion = "1.3.0";
	//m_strFullUrl = m_strUrl + "ows?service=wms&version=" + m_strVersion+ "&request=GetCapabilities";
	m_strFullUrl = m_strUrl + "?service=wms&version=" + m_strVersion+ "&request=GetCapabilities";
}

WMSServiceProvider::~WMSServiceProvider()
{

}

void WMSServiceProvider::GetAttr()
{
	ServiceProvider::GetAttr();
	std::string strXml = GetXml();

	FeUtil::TiXmlDocument doc;
	doc.Parse(strXml.c_str());

	
	FeUtil::TiXmlElement * root = doc.RootElement();
	if (root != NULL)
	{
		bool find;
		TiXmlElement * layerRoot = ReadXmlByStack(root, "Layer", find);
	
		if (layerRoot != NULL)
		{
			TiXmlElement * ele = layerRoot->FirstChildElement("Layer");

			for (ele; ele; ele = ele->NextSiblingElement())
			{
				ServiceLayer * serviceLayer = new ServiceLayer;
				m_pLayers.push_back(serviceLayer);
				serviceLayer->SetServiceProvider(this);
				if (ele != NULL)
				{
					FeUtil::TiXmlElement * nameEle = ele->FirstChildElement("Name");
					FeUtil::TiXmlElement * epsgEle = ele->FirstChildElement("CRS");
					FeUtil::TiXmlElement * exbbEle = ele->FirstChildElement("EX_GeographicBoundingBox");
					
					FeUtil::TiXmlElement * srsEle = ele->FirstChildElement("SRS"); //1.1.1版本
					FeUtil::TiXmlElement * llbbEle = ele->FirstChildElement("LatLonBoundingBox");
					if (srsEle != NULL)
					{
						serviceLayer->SetEPSGCode(srsEle->GetText());
					}

					if (llbbEle != NULL)
					{
						const char * maxY = llbbEle->Attribute("maxy");
						const char * maxX = llbbEle->Attribute("maxx");
						const char * minY = llbbEle->Attribute("minx");
						const char * minX = llbbEle->Attribute("miny");
						std::string str1 = minY;
						str1.append("\t");
						str1.append(minX);

						std::string str2 = maxY;
						str2.append("\t");
						str2.append(maxX);

						serviceLayer->SetLowerCorner(str1);
						serviceLayer->SetUpperCorner(str2);
					}


					if (nameEle != NULL)
					{
						serviceLayer->SetName(nameEle->GetText());
					}

					if (epsgEle != NULL)
					{
						serviceLayer->SetEPSGCode(epsgEle->GetText());
						FeUtil::TiXmlElement * crsEle = epsgEle->NextSiblingElement();
						if (crsEle != NULL)
						{
							if (std::string(crsEle->Value()) == "CRS")
							{
								serviceLayer->SetCRSCode(crsEle->GetText());
							}
						}
					}

					if (exbbEle != NULL)
					{
						FeUtil::TiXmlElement * westEle = exbbEle->FirstChildElement("westBoundLongitude");
						FeUtil::TiXmlElement * southEle = exbbEle->FirstChildElement("southBoundLatitude");
						FeUtil::TiXmlElement * eastEle = exbbEle->FirstChildElement("eastBoundLongitude");
						FeUtil::TiXmlElement * northEle = exbbEle->FirstChildElement("northBoundLatitude");

						std::string str1;
						std::string str2;
						if (westEle != NULL)
						{
							str1.append(westEle->GetText());
							str1.append("\t");
						}
						if (southEle != NULL)
						{
							str1.append(southEle->GetText());
						}
						if (eastEle != NULL)
						{
							str2.append(eastEle->GetText());
							str2.append("\t");
						}
						if (northEle != NULL)
						{
							str2.append(northEle->GetText());
						}

						serviceLayer->SetLowerCorner(str1);
						serviceLayer->SetUpperCorner(str2);
					}
				}
			}
		}
	}
}

std::string WMSServiceProvider::GetServiceType()
{
	return "WMS";
}

std::string WMSServiceProvider::GetServiceUrl()
{
	return m_strUrl/* + "wms?"*/;
}

void WMSServiceProvider::SetVersion(const std::string & strVersion)
{
	m_strVersion = strVersion;
	m_strFullUrl = m_strUrl + "?service=wms&version=" + m_strVersion+ "&request=GetCapabilities";
}

