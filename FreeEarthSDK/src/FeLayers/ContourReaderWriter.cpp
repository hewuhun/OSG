#include <FeLayers/ContourReaderWriter.h>

#include <FeUtils/logger/LoggerDef.h>
#include <FeUtils/PathRegistry.h>

using namespace FeUtil::TinyUtil;

namespace FeLayers
{
    CContourConfig::CContourConfig()
        :m_strDefContour("")
    {
    }

    CContourConfig::~CContourConfig()
    {

    }

	CContourConfig::ContourMap CContourConfig::GetContourList()
	{
		return m_mapContour;
	}

	void CContourConfig::SetContourList( std::string strKey, std::string strValue )
	{
		m_mapContour.insert(std::make_pair(strKey, strValue));
	}

	void CContourConfig::DeleteContourList( std::string strKey )
	{
		m_mapContour.erase(strKey);
	}

	std::string CContourConfig::GetDefContour()
	{
		return m_strDefContour;
	}

	void CContourConfig::SetDefContour( std::string strLan )
	{
		m_strDefContour = strLan;
	}

	bool CContourConfig::Valid()
	{
		return m_bValid;
	}

	void CContourConfig::SetValid( bool bValue )
	{
		m_bValid = bValue;
	}


}

namespace FeLayers
{
	CContourCfgReaderWriter::CContourCfgReaderWriter()
	{

	}

	CContourCfgReaderWriter::~CContourCfgReaderWriter()
	{

	}
	
	CContourConfig CContourCfgReaderWriter::Read()
	{
		return Read(FeFileReg->GetFullPath("contour/contourConfig.xml"));
	}

	CContourConfig CContourCfgReaderWriter::Read( const std::string& strFileName )
	{
		FeUtil::TiXmlDocument document(strFileName.c_str());

		if(!document.LoadFile(FeUtil::TIXML_ENCODING_UTF8) || !document.Accept(this))
		{
			m_configModule.SetValid(false);
			LOG(LOG_ERROR)<<"Can't load or parser system config file.";
		}

		return m_configModule;
	}

	bool CContourCfgReaderWriter::VisitEnter( const FeUtil::TiXmlElement& ele , const FeUtil::TiXmlAttribute* attr )
	{
		if(AcceptKey("contour", ele.Value()))
		{
			return AcceptContour(ele, attr);
		}

		return true;
	}

	bool CContourCfgReaderWriter::AcceptContour( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{
		m_configModule.SetDefContour(ToStdString(ele.Attribute("default")));
		CContourConfig::ContourMap mapLan = m_configModule.GetContourList();

		const FeUtil::TiXmlElement* pTinyElement = ele.FirstChildElement("item");
		while(pTinyElement)
		{
			m_configModule.SetContourList(pTinyElement->Attribute("key"), pTinyElement->Attribute("value"));
			pTinyElement = pTinyElement->NextSiblingElement();
		}

		return true;
	}

	bool CContourCfgReaderWriter::Write( CContourConfig config )
	{
		return Write(config, FeFileReg->GetFullPath("contour/contourConfig.xml"));
	}

	bool CContourCfgReaderWriter::Write( CContourConfig config, const std::string& strFileName )
	{
		FeUtil::TiXmlDeclaration* declar = new FeUtil::TiXmlDeclaration("1.0", "UTF-8", "");
		FeUtil::TiXmlDocument* doc = new FeUtil::TiXmlDocument(strFileName.c_str());
		doc->LinkEndChild(declar);

		FeUtil::TiXmlElement * element = new FeUtil::TiXmlElement("contour");  
		element->SetAttribute("default", config.GetDefContour().c_str());
		doc->LinkEndChild(element);  

		CContourConfig::ContourMap contourList = config.GetContourList();
		CContourConfig::ContourMap::iterator it = contourList.begin();
		while (it != contourList.end())
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

namespace FeLayers
{
	CContourReaderWriter::CContourReaderWriter()
	{
		m_pContourColor = new osg::Vec4dArray();
	}

	CContourReaderWriter::~CContourReaderWriter()
	{

	}

	osg::Vec4dArray* CContourReaderWriter::Read( const std::string& strFileName )
	{
		FeUtil::TiXmlDocument document(strFileName.c_str());

		if(!document.LoadFile(FeUtil::TIXML_ENCODING_UTF8) || !document.Accept(this))
		{
			return NULL;
			LOG(LOG_ERROR)<<"Can't load or parser system config file.";
		}

		return m_pContourColor.release();
	}

	bool CContourReaderWriter::VisitEnter( const FeUtil::TiXmlElement& ele , const FeUtil::TiXmlAttribute* attr )
	{
		if(AcceptKey("contour", ele.Value()))
		{
			return AcceptContour(ele, attr);
		}

		return true;
	}

	bool CContourReaderWriter::AcceptContour( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{
		const FeUtil::TiXmlElement* pTinyElement = ele.FirstChildElement("set");
		while(pTinyElement)
		{
			int nOrder = 0, nRed = 0, nGreen = 0, nBlue = 0;
			pTinyElement->QueryIntAttribute("order", &nOrder);
			pTinyElement->QueryIntAttribute("red", &nRed);
			pTinyElement->QueryIntAttribute("green", &nGreen);
			pTinyElement->QueryIntAttribute("blue", &nBlue);

			m_pContourColor->push_back(osg::Vec4d(nRed, nGreen, nBlue, nOrder));
			
			pTinyElement = pTinyElement->NextSiblingElement();
		}

		return true;
	}

	bool CContourReaderWriter::Write( osg::Vec4dArray* pArray )
	{
		return Write(pArray, FeFileReg->GetFullPath("contour/contourConfig.xml").c_str());
	}

	bool CContourReaderWriter::Write( osg::Vec4dArray* pArray, const std::string& strFileName )
	{
		FeUtil::TiXmlDeclaration* declar = new FeUtil::TiXmlDeclaration("1.0", "UTF-8", "");
		FeUtil::TiXmlDocument* doc = new FeUtil::TiXmlDocument(strFileName.c_str());
		doc->LinkEndChild(declar);

		FeUtil::TiXmlElement * element = new FeUtil::TiXmlElement("contour");  
		doc->LinkEndChild(element);  

		osg::Vec4dArray::iterator it = pArray->begin();
		while(it != pArray->end())
		{
			osg::Vec4d color = *it;
			FeUtil::TiXmlElement* itemElement = new FeUtil::TiXmlElement("set");
			itemElement->SetAttribute("order", color.a());
			itemElement->SetAttribute("red", color.r());
			itemElement->SetAttribute("green", color.g());
			itemElement->SetAttribute("blue", color.b());

			element->LinkEndChild(itemElement);

			it++;
		}

		return doc->SaveFile();
	}
}