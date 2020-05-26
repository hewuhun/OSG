#include <osgEarth/StringUtils>
#include <FeUtils/logger/LoggerDef.h>
#include <FeUtils/PathRegistry.h>
#include <FeShell/SystemSerializer.h>

using namespace FeUtil::TinyUtil;

namespace FeShell
{
    CSysConfig::CSysConfig()
        :m_strEarthPath("./Data/earth/FreeEarth_flat.earth")
        ,m_strSystemTitle("FreeEarth")
        ,m_strFeatureLayerConfig("./Data/featureConfig.xml")
		,m_strSystemIcon("./Data/texture/systemIcon.png")
		,m_strMarksConfig("./Data/config/marksConfig.xml")
		,m_strPlotsConfig("./Data/config/plotsConfig.xml")
		,m_strCustomLayerConfig("./Data/config/customLayerConfig.xml")
    {
    }

    CSysConfig::~CSysConfig()
    {

    }

    std::string CSysConfig::GetSystemTitle() const
    {
        return m_strSystemTitle;
    }

    void CSysConfig::SetSystemTitle( const std::string& strValue )
    {
        m_strSystemTitle = strValue;
    }

	std::string CSysConfig::GetSystemIcon() const
	{
		return m_strSystemIcon;
	}

	void CSysConfig::SetSystemIcon( const std::string& strValue )
	{
		m_strSystemIcon = strValue;
	}

    std::string CSysConfig::GetEarthFile() const
    {
        return m_strEarthPath;
    }

    void CSysConfig::SetEarthFile( const std::string& strValue )
    {
        m_strEarthPath = strValue;
    }

	std::string CSysConfig::GetFeatureLayerConfig() const
	{
		return m_strFeatureLayerConfig;
	}

	void CSysConfig::SetFeatureLayerConfig( const std::string& strValue )
	{
		m_strFeatureLayerConfig = strValue;
	}

	FeUtil::CFreeViewPoint CSysConfig::GetBeginViewPoint() const
	{
		return m_sViewPointBegin;
	}

	void CSysConfig::SetBeginViewPoint( const FeUtil::CFreeViewPoint& sValue )
	{
		m_sViewPointBegin = sValue;
	}

	FeUtil::CFreeViewPoint CSysConfig::GetEndViewPoint() const
	{
		return m_sViewPointEnd;
	}

	void CSysConfig::SetEndViewPoint( const FeUtil::CFreeViewPoint& sValue )
	{
		m_sViewPointEnd = sValue;
	}

	std::string CSysConfig::GetMarksConfig() const
	{
		return m_strMarksConfig;
	}

	void CSysConfig::SetMarksConfig( const std::string& strValue )
	{
		m_strMarksConfig = strValue;
	}

	std::string CSysConfig::GetPlotsConfig() const
	{
		return m_strPlotsConfig;
	}

	void CSysConfig::SetPlotsConfig( const std::string& strValue )
	{
		m_strPlotsConfig = strValue;
	}

	std::string CSysConfig::GetCustomLayerConfig() const
	{
		return m_strCustomLayerConfig;
	}

	void CSysConfig::SetCustomLayerConfig( const std::string& strValue )
	{
		m_strCustomLayerConfig = strValue;
	}

	std::string CSysConfig::GetContourConfig() const
	{
		return m_strContourConfig;
	}

	void CSysConfig::SetContourConfig( const std::string& strValue )
	{
		m_strContourConfig = strValue;
	}
}

namespace FeShell
{

	CSystemConfigLoader::CSystemConfigLoader()
		:FeUtil::TiXmlVisitor()
	{
	}

	CSystemConfigLoader::~CSystemConfigLoader()
	{
	}

	bool CSystemConfigLoader::Execute(const std::string& strFileName, CSysConfig& systemConfig)
	{
		FeUtil::TiXmlDocument document(strFileName.c_str());

		if(!document.LoadFile(FeUtil::TIXML_ENCODING_UTF8) || !document.Accept(this))
		{
			LOG(LOG_ERROR)<<"Can't load or parser system config file.";
			return false;
		}

		systemConfig = m_configModule;

		return true;
	}

	bool CSystemConfigLoader::ExecuteDefault(CSysConfig& systemConfig)
	{
		return Execute(FeFileReg->GetFullPath("config/systemConfig.xml"), systemConfig);
	}

	bool CSystemConfigLoader::VisitEnter(const FeUtil::TiXmlElement& prefix, const FeUtil::TiXmlAttribute* attr)
	{
		if(AcceptKey("common", prefix.Value()))
		{
			return AcceptCommon(prefix, attr);
		}

		if(AcceptKey("viewPoints", prefix.Value()))
		{
			return AcceptViewPoint(prefix, attr);
		}

		return true;
	}


	bool CSystemConfigLoader::AcceptCommon( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{
		m_configModule.SetSystemTitle(ToStdString(ele.Attribute("title")));
		m_configModule.SetSystemIcon(FeFileReg->GetFullPath(ToStdString(ele.Attribute("icon"))));
		m_configModule.SetEarthFile(FeFileReg->GetFullPath(ToStdString(ele.Attribute("earth"))));
		m_configModule.SetMarksConfig(FeFileReg->GetFullPath(ToStdString(ele.Attribute("marks"))));
		m_configModule.SetPlotsConfig(FeFileReg->GetFullPath(ToStdString(ele.Attribute("plots"))));
		m_configModule.SetFeatureLayerConfig(FeFileReg->GetFullPath(ToStdString(ele.Attribute("featureLayer"))));
		m_configModule.SetCustomLayerConfig(FeFileReg->GetFullPath(ToStdString(ele.Attribute("customLayer"))));
		m_configModule.SetContourConfig(FeFileReg->GetFullPath(ToStdString(ele.Attribute("contourMap"))));

		return true;
	}

	bool CSystemConfigLoader::AcceptViewPoint( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{
		const FeUtil::TiXmlElement* pTinyElementBegin = ele.FirstChildElement("viewPointBegin");
		if(pTinyElementBegin)
		{
			FeUtil::CFreeViewPoint viewPointBegin;
			BuildViewPoint(pTinyElementBegin, viewPointBegin);
			m_configModule.SetBeginViewPoint(viewPointBegin);
		}

		const FeUtil::TiXmlElement* pTinyElementEnd = ele.FirstChildElement("viewPointEnd");
		if(pTinyElementEnd)
		{
			FeUtil::CFreeViewPoint viewPointEnd;
			BuildViewPoint(pTinyElementEnd, viewPointEnd);
			m_configModule.SetEndViewPoint(viewPointEnd);
		}

		return true;
	}

	void CSystemConfigLoader::BuildViewPoint( const FeUtil::TiXmlElement* pElement, FeUtil::CFreeViewPoint& viewPoint )
	{
		if(!pElement) return;

		viewPoint.SetViewPoint(
			osgEarth::as<double>(ToStdString(pElement->Attribute("lon")), 0.0),
			osgEarth::as<double>(ToStdString(pElement->Attribute("lat")), 0.0),
			osgEarth::as<double>(ToStdString(pElement->Attribute("hei")), 0.0),
			osgEarth::as<double>(ToStdString(pElement->Attribute("heading")), 0.0),
			osgEarth::as<double>(ToStdString(pElement->Attribute("pitch")), 0.0),
			osgEarth::as<double>(ToStdString(pElement->Attribute("range")), 0.0),
			osgEarth::as<double>(ToStdString(pElement->Attribute("time")), 0.0));
	}

}

