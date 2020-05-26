#include <FeShell/EnvironmentSerializer.h>

#include <FeUtils/logger/LoggerDef.h>
#include <FeUtils/PathRegistry.h>
#include <FeUtils/StrUtil.h>

#include <sstream>
#include <iostream>

using namespace std;
using namespace FeUtil;
using namespace FeUtil::TinyUtil;

namespace FeShell
{
	bool SEnvLightSetData::ParseConfig(string strName, string strValue)
	{
		if(0 == strcmp("Sun", strName.c_str()))
		{
			bSun = StringToBool(strValue);
			return true;
		}

		if(0 == strcmp("Atmosphere", strName.c_str()))
		{
			bAtmosphere = StringToBool(strValue);
			return true;
		}

		if(0 == strcmp("Moon", strName.c_str()))
		{
			bMoon = StringToBool(strValue);
			return true;
		}

		if(0 == strcmp("Star", strName.c_str()))
		{
			bStar = StringToBool(strValue);
			return true;
		}

		if(0 == strcmp("Nebula", strName.c_str()))
		{
			bNebula = StringToBool(strValue);
			return true;
		}

		if(0 == strcmp("Density", strName.c_str()))
		{
			fDensity = StringToDouble(strValue);
			return true;
		}

		return false;
	}

	void SEnvLightSetData::GetConfig(Configs& configs)
	{
		configs.insert(std::make_pair("Sun", FeUtil::BoolToString(bSun)));
		configs.insert(std::make_pair("Atmosphere", FeUtil::BoolToString(bAtmosphere)));
		configs.insert(std::make_pair("Moon", FeUtil::BoolToString(bMoon)));
		configs.insert(std::make_pair("Star", FeUtil::BoolToString(bStar)));
		configs.insert(std::make_pair("Nebula", FeUtil::BoolToString(bNebula)));
		configs.insert(std::make_pair("Density", FeUtil::DoubleToString(fDensity)));
	}

	bool SEnvRainSetData::ParseConfig(string strName, string strValue)
	{
		if(0 == strcmp("Rain", strName.c_str()))
		{
			bRain = StringToBool(strValue);
			return true;
		}

		if(0 == strcmp("RainGlobal", strName.c_str()))
		{
			bGlobal = StringToBool(strValue);
			return true;
		}

		if(0 == strcmp("dRainDensity", strName.c_str()))
		{
			dDensity = StringToDouble(strValue);
			return true;
		}

		if(0 == strcmp("RainHeight", strName.c_str()))
		{
			fheigth = StringToDouble(strValue);
			return true;
		}

		if(0 == strcmp("RainLongitude", strName.c_str()))
		{
			fLongitude = StringToDouble(strValue);
			return true;
		}

		if(0 == strcmp("RainLatitude", strName.c_str()))
		{
			fLatitude = StringToDouble(strValue);
			return true;
		}

		if(0 == strcmp("RainRadius", strName.c_str()))
		{
			fRadius = StringToDouble(strValue);
			return true;
		}

		return false;
	}

	void SEnvRainSetData::GetConfig(Configs& configs)
	{
		configs.insert(std::make_pair("Rain", FeUtil::BoolToString(bRain)));
		configs.insert(std::make_pair("RainGlobal", FeUtil::BoolToString(bGlobal)));
		configs.insert(std::make_pair("dRainDensity", FeUtil::DoubleToString(dDensity)));
		configs.insert(std::make_pair("RainHeight", FeUtil::DoubleToString(fheigth)));
		configs.insert(std::make_pair("RainLongitude", FeUtil::DoubleToString(fLongitude)));
		configs.insert(std::make_pair("RainLatitude", FeUtil::DoubleToString(fLatitude)));
		configs.insert(std::make_pair("RainRadius", FeUtil::DoubleToString(fRadius)));
	}

	bool SEnvSnowSetData::ParseConfig(string strName, string strValue)
	{
		if(0 == strcmp("Snow", strName.c_str()))
		{
			bSnow = StringToBool(strValue);
			return true;
		}

		if(0 == strcmp("SnowGlobal", strName.c_str()))
		{
			bGlobal = StringToBool(strValue);
			return true;
		}

		if(0 == strcmp("dSnowDensity", strName.c_str()))
		{
			dDensity = StringToDouble(strValue);
			return true;
		}

		if(0 == strcmp("SnowHeight", strName.c_str()))
		{
			fheigth = StringToDouble(strValue);
			return true;
		}

		if(0 == strcmp("SnowLongitude", strName.c_str()))
		{
			fLongitude = StringToDouble(strValue);
			return true;
		}

		if(0 == strcmp("SnowLatitude", strName.c_str()))
		{
			fLatitude = StringToDouble(strValue);
			return true;
		}

		if(0 == strcmp("SnowRadius", strName.c_str()))
		{
			fRadius = StringToDouble(strValue);
			return true;
		}

		return false;
	}

	void SEnvSnowSetData::GetConfig(Configs& configs)
	{
		configs.insert(std::make_pair("Snow", FeUtil::BoolToString(bSnow)));
		configs.insert(std::make_pair("SnowGlobal", FeUtil::BoolToString(bGlobal)));
		configs.insert(std::make_pair("dSnowDensity", FeUtil::DoubleToString(dDensity)));
		configs.insert(std::make_pair("SnowHeight", FeUtil::DoubleToString(fheigth)));
		configs.insert(std::make_pair("SnowLongitude", FeUtil::DoubleToString(fLongitude)));
		configs.insert(std::make_pair("SnowLatitude", FeUtil::DoubleToString(fLatitude)));
		configs.insert(std::make_pair("SnowRadius", FeUtil::DoubleToString(fRadius)));
	}

	bool SEnvCloudSetData::ParseConfig(string strName, string strValue)
	{
		if(0 == strcmp("Cloud", strName.c_str()))
		{
			bCloud = StringToBool(strValue);
			return true;
		}
		if(0 == strcmp("CloudAltitude", strName.c_str()))
		{
			nAltitude = StringToInt(strValue);
			return true;
		}
		if(0 == strcmp("CloudThickness", strName.c_str()))
		{
			nThickness = StringToInt(strValue);
			return true;
		}
		if(0 == strcmp("CloudDensity", strName.c_str()))
		{
			nDensity = StringToInt(strValue);
			return true;
		}
		if(0 == strcmp("dCloudDensity", strName.c_str()))
		{
			dDensity = StringToDouble(strValue);
			return true;
		}
		if(0 == strcmp("nCloudAlpha", strName.c_str()))
		{
			nAlpha = StringToInt(strValue);
			return true;
		}
		if(0 == strcmp("dCloudAlpha", strName.c_str()))
		{
			dAlpha = StringToDouble(strValue);
			return true;
		}
		if(0 == strcmp("CloudSpeed", strName.c_str()))
		{
			nSpeed = StringToInt(strValue);
			return true;
		}
		if(0 == strcmp("CloudDirection", strName.c_str()))
		{
			nDirection = StringToInt(strValue);
			return true;
		}
		if(0 == strcmp("CloudType", strName.c_str()))
		{
			nCloudType = StringToInt(strValue);
			return true;
		}

		return false;
	}

	void SEnvCloudSetData::GetConfig(Configs& configs)
	{
		configs.insert(std::make_pair("Cloud", FeUtil::BoolToString(bCloud)));
		configs.insert(std::make_pair("CloudAltitude", FeUtil::IntToString(nAltitude)));
		configs.insert(std::make_pair("CloudThickness", FeUtil::IntToString(nThickness)));
		configs.insert(std::make_pair("CloudDensity", FeUtil::IntToString(nDensity)));
		configs.insert(std::make_pair("dCloudDensity", FeUtil::DoubleToString(dDensity)));
		configs.insert(std::make_pair("nCloudAlpha", FeUtil::IntToString(nAlpha)));
		configs.insert(std::make_pair("dCloudAlpha", FeUtil::DoubleToString(dAlpha)));
		configs.insert(std::make_pair("CloudSpeed", FeUtil::IntToString(nSpeed)));
		configs.insert(std::make_pair("CloudDirection", FeUtil::IntToString(nDirection)));
		configs.insert(std::make_pair("CloudType", FeUtil::IntToString(nCloudType)));
	}

	bool SEnvOceanSetData::ParseConfig(string strName, string strValue)
	{
		if(0 == strcmp("Ocean", strName.c_str()))
		{
			bOcean = StringToBool(strValue);
			return true;
		}
		if(0 == strcmp("OceanLevel", strName.c_str()))
		{
			fOceanLevel = StringToDouble(strValue);
			return true;
		}
		if(0 == strcmp("WindSpeed", strName.c_str()))
		{
			fWindSpeed = StringToDouble(strValue);
			return true;
		}
		if(0 == strcmp("WindDirection", strName.c_str()))
		{
			fWindDirection = StringToDouble(strValue);
			return true;
		}
		if(0 == strcmp("WindLength", strName.c_str()))
		{
			fWindLength = StringToDouble(strValue);
			return true;
		}

		return false;
	}

	void SEnvOceanSetData::GetConfig(Configs& configs)
	{
		configs.insert(std::make_pair("Ocean", FeUtil::BoolToString(bOcean)));
		configs.insert(std::make_pair("OceanLevel", FeUtil::DoubleToString(fOceanLevel)));
		configs.insert(std::make_pair("WindSpeed", FeUtil::DoubleToString(fWindSpeed)));
		configs.insert(std::make_pair("WindDirection", FeUtil::DoubleToString(fWindDirection)));
		configs.insert(std::make_pair("WindLength", FeUtil::DoubleToString(fWindLength)));
	}

	bool SEnvFogSetData::ParseConfig( std::string strName, std::string strValue )
	{
		if(0 == strcmp("Fog", strName.c_str()))
		{
			bFog = StringToBool(strValue);
			return true;
		}

		if(0 == strcmp("FogGlobal", strName.c_str()))
		{
			bGlobal = StringToBool(strValue);
			return true;
		}

		if(0 == strcmp("dFogDensity", strName.c_str()))
		{
			dDensity = StringToDouble(strValue);
			return true;
		}

		if(0 == strcmp("FogHeight", strName.c_str()))
		{
			dHight = StringToDouble(strValue);
			return true;
		}

		if(0 == strcmp("FogLatitude", strName.c_str()))
		{
			fLongitude = StringToDouble(strValue);
			return true;
		}

		if(0 == strcmp("FogLongitude", strName.c_str()))
		{
			fLatitude = StringToDouble(strValue);
			return true;
		}

		if(0 == strcmp("FogRadius", strName.c_str()))
		{
			fRadius = StringToDouble(strValue);
			return true;
		}

		return false;
	}

	void SEnvFogSetData::GetConfig( Configs& configs )
	{
		configs.insert(std::make_pair("Fog", FeUtil::BoolToString(bFog)));
		configs.insert(std::make_pair("dFogDensity", FeUtil::DoubleToString(dDensity)));
	}
}

namespace FeShell
{
	CEnvConfigReader::CEnvConfigReader()
	{

	}

	CEnvConfigReader::~CEnvConfigReader()
	{

	}

	FeShell::CEnvSetData CEnvConfigReader::Execute( const std::string& strFileName )
	{
		FeUtil::TiXmlDocument document(strFileName.c_str());

		if(!document.LoadFile(FeUtil::TIXML_ENCODING_UTF8) || !document.Accept(this))
		{
			LOG(LOG_ERROR)<<"Can't load or parser system config file.";
		}

		return m_sEnvSetDate;
	}

	bool CEnvConfigReader::VisitEnter( const FeUtil::TiXmlElement& ele , const FeUtil::TiXmlAttribute* attr )
	{
		if (AcceptKey("Light", ele.Value())
			|| AcceptKey("Rain", ele.Value())
			|| AcceptKey("Snow", ele.Value())
			|| AcceptKey("Cloud", ele.Value())
			|| AcceptKey("Ocean", ele.Value())
			|| AcceptKey("Fog", ele.Value()))
		{
			return AcceptEnv(ele, attr);
		}

		return true;
	}

	bool CEnvConfigReader::AcceptEnv( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{
		if (strcmp(ele.Value(), "Light") == 0)
		{
			const FeUtil::TiXmlElement* pTinyElement = ele.FirstChildElement();
			while(pTinyElement)
			{
				m_sEnvSetDate.m_sEnvLightSetData.ParseConfig(pTinyElement->Attribute("name"), pTinyElement->Attribute("value"));
				pTinyElement = pTinyElement->NextSiblingElement();
			}
		}

		if (strcmp(ele.Value(), "Rain") == 0)
		{
			const FeUtil::TiXmlElement* pTinyElement = ele.FirstChildElement();
			while(pTinyElement)
			{
				m_sEnvSetDate.m_sEnvRainSetData.ParseConfig(pTinyElement->Attribute("name"), pTinyElement->Attribute("value"));
				pTinyElement = pTinyElement->NextSiblingElement();
			}
		}

		if (strcmp(ele.Value(), "Snow") == 0)
		{
			const FeUtil::TiXmlElement* pTinyElement = ele.FirstChildElement();
			while(pTinyElement)
			{
				m_sEnvSetDate.m_sEnvSnowSetData.ParseConfig(pTinyElement->Attribute("name"), pTinyElement->Attribute("value"));
				pTinyElement = pTinyElement->NextSiblingElement();
			}
		}

		if (strcmp(ele.Value(), "Cloud") == 0)
		{
			const FeUtil::TiXmlElement* pTinyElement = ele.FirstChildElement();
			while(pTinyElement)
			{
				m_sEnvSetDate.m_sEnvCloudSetData.ParseConfig(pTinyElement->Attribute("name"), pTinyElement->Attribute("value"));
				pTinyElement = pTinyElement->NextSiblingElement();
			}
		}

		if (strcmp(ele.Value(), "Ocean") == 0)
		{
			const FeUtil::TiXmlElement* pTinyElement = ele.FirstChildElement();
			while(pTinyElement)
			{
				m_sEnvSetDate.m_sEnvOceanSetData.ParseConfig(pTinyElement->Attribute("name"), pTinyElement->Attribute("value"));
				pTinyElement = pTinyElement->NextSiblingElement();
			}
		}

		if (strcmp(ele.Value(), "Fog") == 0)
		{
			const FeUtil::TiXmlElement* pTinyElement = ele.FirstChildElement();
			while(pTinyElement)
			{
				m_sEnvSetDate.m_sEnvFogSetData.ParseConfig(pTinyElement->Attribute("name"), pTinyElement->Attribute("value"));
				pTinyElement = pTinyElement->NextSiblingElement();
			}
		}

		return true;
	}


	CEnvConfigWriter::CEnvConfigWriter()
	{

	}

	CEnvConfigWriter::~CEnvConfigWriter()
	{

	}

	bool CEnvConfigWriter::Execute(CEnvSetData sEnvSetData)
	{
		return Execute(sEnvSetData, FeFileReg->GetFullPath("config/EnvironmentConifg.xml"));
	}

	bool CEnvConfigWriter::Execute(CEnvSetData sEnvSetData, const std::string &strFileName)
	{
		FeUtil::TiXmlDeclaration* pDeclar = new FeUtil::TiXmlDeclaration("1.0", "UTF-8", "");
		FeUtil::TiXmlDocument* doc = new FeUtil::TiXmlDocument(strFileName.c_str());
		doc->LinkEndChild(pDeclar);

		FeUtil::TiXmlElement * pEnvElement = new FeUtil::TiXmlElement("Environment");
		doc->LinkEndChild(pEnvElement);

		//光照
		FeUtil::TiXmlElement * pLightElement = new FeUtil::TiXmlElement("Light");
		pEnvElement->LinkEndChild(pLightElement);

		Configs configsLight;
		sEnvSetData.m_sEnvLightSetData.GetConfig(configsLight);
		Configs::iterator itrConfigsLight = configsLight.begin();
		while(itrConfigsLight != configsLight.end())
		{
			FeUtil::TiXmlElement* pItemElement = new FeUtil::TiXmlElement("Item");
			pItemElement->SetAttribute("name", itrConfigsLight->first.c_str());
			pItemElement->SetAttribute("value", itrConfigsLight->second.c_str());
			pLightElement->LinkEndChild(pItemElement);

			itrConfigsLight++;
		}

		//雨
		FeUtil::TiXmlElement * pRainElement = new FeUtil::TiXmlElement("Rain");
		pEnvElement->LinkEndChild(pRainElement);

		Configs configsRain;
		sEnvSetData.m_sEnvRainSetData.GetConfig(configsRain);
		Configs::iterator itrConfigsRain = configsRain.begin();
		while(itrConfigsRain != configsRain.end())
		{
			FeUtil::TiXmlElement* pItemElement = new FeUtil::TiXmlElement("Item");
			pItemElement->SetAttribute("name", itrConfigsRain->first.c_str());
			pItemElement->SetAttribute("value", itrConfigsRain->second.c_str());
			pRainElement->LinkEndChild(pItemElement);

			itrConfigsRain++;
		}

		//雪
		FeUtil::TiXmlElement * pSnowElement = new FeUtil::TiXmlElement("Snow");
		pEnvElement->LinkEndChild(pSnowElement);

		Configs configsSnow;
		sEnvSetData.m_sEnvSnowSetData.GetConfig(configsSnow);
		Configs::iterator itrConfigsSnow = configsSnow.begin();
		while(itrConfigsSnow != configsSnow.end())
		{
			FeUtil::TiXmlElement* pItemElement = new FeUtil::TiXmlElement("Item");
			pItemElement->SetAttribute("name", itrConfigsSnow->first.c_str());
			pItemElement->SetAttribute("value", itrConfigsSnow->second.c_str());
			pSnowElement->LinkEndChild(pItemElement);

			itrConfigsSnow++;
		}

		//雾
		FeUtil::TiXmlElement * pFogElement = new FeUtil::TiXmlElement("Fog");
		pEnvElement->LinkEndChild(pFogElement);

		Configs configsFog;
		sEnvSetData.m_sEnvFogSetData.GetConfig(configsFog);
		Configs::iterator itrConfigsFog = configsFog.begin();
		while(itrConfigsFog != configsFog.end())
		{
			FeUtil::TiXmlElement* pItemElement = new FeUtil::TiXmlElement("Item");
			pItemElement->SetAttribute("name", itrConfigsFog->first.c_str());
			pItemElement->SetAttribute("value", itrConfigsFog->second.c_str());
			pFogElement->LinkEndChild(pItemElement);

			itrConfigsFog++;
		}

		//云
		FeUtil::TiXmlElement * pCloudElement = new FeUtil::TiXmlElement("Cloud");
		pEnvElement->LinkEndChild(pCloudElement);

		Configs configsCloud;
		sEnvSetData.m_sEnvCloudSetData.GetConfig(configsCloud);
		Configs::iterator itrConfigsCloud = configsCloud.begin();
		while(itrConfigsCloud != configsCloud.end())
		{
			FeUtil::TiXmlElement* pItemElement = new FeUtil::TiXmlElement("Item");
			pItemElement->SetAttribute("name", itrConfigsCloud->first.c_str());
			pItemElement->SetAttribute("value", itrConfigsCloud->second.c_str());
			pCloudElement->LinkEndChild(pItemElement);

			itrConfigsCloud++;
		}

		//海洋
		FeUtil::TiXmlElement * pOceanElement = new FeUtil::TiXmlElement("Ocean");
		pEnvElement->LinkEndChild(pOceanElement);

		Configs configsOcean;
		sEnvSetData.m_sEnvOceanSetData.GetConfig(configsOcean);
		Configs::iterator itrConfigsOcean = configsOcean.begin();
		while(itrConfigsOcean != configsOcean.end())
		{
			FeUtil::TiXmlElement* pItemElement = new FeUtil::TiXmlElement("Item");
			pItemElement->SetAttribute("name", itrConfigsOcean->first.c_str());
			pItemElement->SetAttribute("value", itrConfigsOcean->second.c_str());
			pOceanElement->LinkEndChild(pItemElement);

			itrConfigsOcean++;
		}

		return doc->SaveFile();
	}
}
