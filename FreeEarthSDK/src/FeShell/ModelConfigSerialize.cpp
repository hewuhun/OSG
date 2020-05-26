#include <FeUtils/logger/LoggerDef.h>
#include <FeUtils/PathRegistry.h>
#include <FeUtils/tinyXml/tinyutil.h>
#include <osgEarth/StringUtils>
#include <FeUtils/tinyXml/tinyutil.h>

#include <FeShell/ModelConfigSerialize.h>

namespace FeShell
{
	CModelConfig::CModelConfig()
		:m_bValid(true)
	{
		m_vecModelInfo = new std::vector<ModelInfo*>;
	}

	CModelConfig::~CModelConfig()
	{

	}

	std::vector<ModelInfo*>* CModelConfig::GetModelInfo() const
	{
		return m_vecModelInfo;
	}

	void CModelConfig::SetModelInfo( std::vector<ModelInfo*>* vecModelInfo )
	{
		m_vecModelInfo = vecModelInfo;
	}

	bool CModelConfig::Valid()
	{
		return m_bValid;
	}

	void CModelConfig::SetValid( bool bValue )
	{
		m_bValid = bValue;
	}

	void CModelConfig::SetMapOfModelInfo(const std::map<std::string,ModelInfo> &mapStrModelInfo )
	{
		m_mapStrModelInfo = mapStrModelInfo;
	}

	std::map<std::string,ModelInfo>* CModelConfig::GetMapOfModelInfo()
	{
		return &m_mapStrModelInfo;
	}
}

namespace FeShell
{
	CModelConfigDeSerialization::CModelConfigDeSerialization()
	{
		
	}


	CModelConfigDeSerialization::~CModelConfigDeSerialization()
	{
	}

	CModelConfig CModelConfigDeSerialization::Execute(const std::string& strFileName)
	{
		FeUtil::TiXmlDocument document(strFileName.c_str());

		if(!document.LoadFile(FeUtil::TIXML_ENCODING_UTF8) || !document.Accept(this))
		{
			m_configModule.SetValid(false);
			LOG(LOG_ERROR)<<"Can't load or parser system config file.";
		}

		return m_configModule;
	}

	bool CModelConfigDeSerialization::VisitEnter(const FeUtil::TiXmlElement& prefix, const FeUtil::TiXmlAttribute* attr)
	{
		if (FeUtil::TinyUtil::AcceptKey("Catagory", prefix.Value()))
		{
		return AcceptModel(prefix, attr);
		}
		return true;
	}

	bool CModelConfigDeSerialization::AcceptModel( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{		
		if (strcmp(ele.Value(), "Catagory") == 0)
		{
			std::string strCatagory = FeUtil::TinyUtil::ToStdString(ele.Attribute("name"));			
			for (const FeUtil::TiXmlElement* pEle = ele.FirstChildElement(); pEle != NULL; pEle = pEle->NextSiblingElement())
			{
				ModelInfo* SModelInfo = new ModelInfo;
				SModelInfo->strModelCatagory = strCatagory;

				const char *c = pEle->Value();
				if (strcmp(pEle->Value(), "Model") == 0)
				{
					SModelInfo->strModelName = FeUtil::TinyUtil::ToStdString(pEle->Attribute("name"));
					SModelInfo->strModelPath = FeUtil::TinyUtil::ToStdString(pEle->Attribute("path"));
					SModelInfo->strModelIcon = FeUtil::TinyUtil::ToStdString(pEle->Attribute("icon"));
					SModelInfo->strDescribe = FeUtil::TinyUtil::ToStdString(pEle->Attribute("discribe"));
					std::vector<ModelInfo*>* vecModelInfo = m_configModule.GetModelInfo();
					vecModelInfo->push_back(SModelInfo);
				}
			}

		}

		return true;
	}
}

namespace FeShell
{
	CModelConfigSerialization::CModelConfigSerialization()
	{

	}

	CModelConfigSerialization::~CModelConfigSerialization()
	{

	}

	bool CModelConfigSerialization::Execute(CModelConfig modelConfig, const std::string& strFileName )
	{
		if( !strFileName.empty())
		{

			FeUtil::TiXmlDeclaration declar("1.0", "UTF-8", "");
			FeUtil::TiXmlDocument doc(strFileName.c_str());

			doc.InsertEndChild(declar);

			FeUtil::TiXmlElement* pRootElement = new FeUtil::TiXmlElement("ModelList");
			doc.LinkEndChild(pRootElement);

			std::map<std::string,std::vector<ModelInfo*> > mapStr;
			std::vector<ModelInfo*>* vecModelInfo = modelConfig.GetModelInfo();
			std::vector<ModelInfo*>::iterator itr = vecModelInfo->begin();
			for(;itr != vecModelInfo->end();itr++ )
			{
				if(!mapStr.count((*itr)->strModelCatagory))
				{
					std::vector<ModelInfo*> vecModelInfoUnit;
					mapStr[(*itr)->strModelCatagory] = vecModelInfoUnit;
				}

				mapStr[(*itr)->strModelCatagory].push_back(*itr);

			}

			std::map<std::string,std::vector<ModelInfo*> >::iterator itrMap = mapStr.begin();
			for(;itrMap != mapStr.end();itrMap++ )
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("Catagory");
				pElement->SetAttribute("name",itrMap->first.c_str());

				std::vector<ModelInfo*> vecModelInfoUnit = itrMap->second;
				int nSize = vecModelInfoUnit.size();
				for(int n =0;n < nSize;n++)
				{
					FeUtil::TiXmlElement* pModelElement = new FeUtil::TiXmlElement("Model");
					pModelElement->SetAttribute("name", vecModelInfoUnit.at(n)->strModelName.c_str());
					pModelElement->SetAttribute("icon", vecModelInfoUnit.at(n)->strModelIcon.c_str());
					pModelElement->SetAttribute("path", vecModelInfoUnit.at(n)->strModelPath.c_str());
					pModelElement->SetAttribute("describe", vecModelInfoUnit.at(n)->strDescribe.c_str());

					pElement->LinkEndChild(pModelElement);
				}

				pRootElement->LinkEndChild(pElement);
			}

			return doc.SaveFile();
		}
		return false;
	}

}