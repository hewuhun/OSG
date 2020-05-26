#include <FeUtils/logger/LoggerDef.h>
#include <FeUtils/PathRegistry.h>
#include <FeShell/LanSerializer.h>

using namespace FeUtil::TinyUtil;

namespace FeShell
{
	CLanConfig::CLanConfig()
		:m_strDefLan("English")
	{
	}

	CLanConfig::~CLanConfig()
	{

	}

	CLanConfig::LanMap CLanConfig::GetLanguageList()
	{
		return m_mapLan;
	}

	void CLanConfig::SetLanguageList( std::string strKey, std::string strValue )
	{
		m_mapLan.insert(std::make_pair(strKey, strValue));
	}

	std::string CLanConfig::GetDefLanguage()
	{
		return m_strDefLan;
	}

	void CLanConfig::SetDefLanguage( std::string strLan )
	{
		m_strDefLan = strLan;
	}

	bool CLanConfig::Valid()
	{
		return m_bValid;
	}

	void CLanConfig::SetValid( bool bValue )
	{
		m_bValid = bValue;
	}

}

namespace FeShell
{

	CLanConfigReader::CLanConfigReader()
	{
	}

	CLanConfigReader::~CLanConfigReader()
	{
	}

	CLanConfig CLanConfigReader::Execute(const std::string& strFileName)
	{
		FeUtil::TiXmlDocument document(strFileName.c_str());

		if(!document.LoadFile(FeUtil::TIXML_ENCODING_UTF8) || !document.Accept(this))
		{
			m_configModule.SetValid(false);
			LOG(LOG_ERROR)<<"Can't load or parser system config file.";
		}

		return m_configModule;
	}

	CLanConfig CLanConfigReader::ExecuteDefault()
	{
		return Execute(FeFileReg->GetFullPath("apps/language/lanConfig.xml"));
	}

	bool CLanConfigReader::VisitEnter(const FeUtil::TiXmlElement& prefix, const FeUtil::TiXmlAttribute* attr)
	{
		if(AcceptKey("language", prefix.Value()))
		{
			return AcceptLan(prefix, attr);
		}

		return true;
	}

	bool CLanConfigReader::AcceptLan( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{
		m_configModule.SetDefLanguage(ToStdString(ele.Attribute("default")));
		CLanConfig::LanMap mapLan = m_configModule.GetLanguageList();

		const FeUtil::TiXmlElement* pTinyElement = ele.FirstChildElement("item");
		while(pTinyElement)
		{
			m_configModule.SetLanguageList(pTinyElement->Attribute("key"), pTinyElement->Attribute("value"));
			pTinyElement = pTinyElement->NextSiblingElement();
		}

		return true;
	}




	CLanConfigWriter::CLanConfigWriter()
	{

	}

	CLanConfigWriter::~CLanConfigWriter()
	{

	}

	bool CLanConfigWriter::Execute( CLanConfig pLanConfig )
	{
		return Execute(pLanConfig, FeFileReg->GetFullPath("apps/language/lanConfig.xml"));
	}

	bool CLanConfigWriter::Execute(CLanConfig pLanConfig, const std::string& strFileName)
	{
		FeUtil::TiXmlDeclaration* declar = new FeUtil::TiXmlDeclaration("1.0", "UTF-8", "");
		FeUtil::TiXmlDocument* doc = new FeUtil::TiXmlDocument(strFileName.c_str());
		doc->LinkEndChild(declar);

		FeUtil::TiXmlElement * element = new FeUtil::TiXmlElement("language");  
		element->SetAttribute("default", pLanConfig.GetDefLanguage().c_str());
		doc->LinkEndChild(element);  

		CLanConfig::LanMap lanMap = pLanConfig.GetLanguageList();
		CLanConfig::LanMap::iterator it = lanMap.begin();
		while(it != lanMap.end())
		{
			FeUtil::TiXmlElement* itemElement = new FeUtil::TiXmlElement("item");
			itemElement->SetAttribute("key", it->first.c_str());
			itemElement->SetAttribute("value", it->second.c_str());

			element->LinkEndChild(itemElement);

			it++;
		}

		return doc->SaveFile();
	}
}

