#include <FeKits/cockPit/meter/MeterManager.h>
#include <cstring>

CMeterManager::CMeterManager(void) :
FeUtil::TiXmlDocument()
{
}

CMeterManager::CMeterManager( const char* cDocumentName ) :
FeUtil::TiXmlDocument(cDocumentName)
{
	if (LoadFile(cDocumentName))
	{
		InitializeMeters();
	}
}


CMeterManager::~CMeterManager(void)
{
}

void CMeterManager::InitializeMeters()
{
	FeUtil::TiXmlElement* pRoot = this->RootElement();
	if (!pRoot)
	{
		return;
	}
	collectMeterXmlNode(pRoot);
	collectLoadingMeterID(pRoot);
	createMeterAttibuteList(UnValueMeter);
}

void CMeterManager::collectMeterXmlNode( FeUtil::TiXmlElement* pRoot )
{
#ifndef METERNODETYPE
#define METERNODETYPE
#define METER "Meter"
#endif
	if (!pRoot)
	{
		return;
	}
	FeUtil::TiXmlElement* pChild = pRoot->FirstChildElement(METER);
	while(pChild)
	{
		m_vecMeterXmlNode.push_back(pChild);
		pChild = pChild->NextSiblingElement(METER);
	}
}

void CMeterManager::collectLoadingMeterID( FeUtil::TiXmlElement* pRoot )
{
#ifndef METERID
#define METERID
#define LAOD_METER_ID "LoadMeterID"
#define METER_ID "MeterID"
#endif
	if (!pRoot)
	{
		return;
	}
	FeUtil::TiXmlElement* pLoadMeterIDlist = pRoot->FirstChildElement(LAOD_METER_ID);
	if (!pLoadMeterIDlist)
	{
		return;
	}
	FeUtil::TiXmlElement* pLoadMeterID = pLoadMeterIDlist->FirstChildElement(METER_ID);
	while(pLoadMeterID)
	{
		m_setLoadMeterID.insert((MeterType)(atoi(pLoadMeterID->FirstChild()->Value())));
		pLoadMeterID = pLoadMeterID->NextSiblingElement(METER_ID);
	}
}

bool CMeterManager::LoadMeterNodeToScene( bool bflag, MeterType ID, ... )
{
	if (!bflag)
	{
		return false;
	}
	else
	{
		va_list vap;
		va_start(vap, ID);
		while(ID != UnValueMeter)
		{
			createMeterAttibuteList(ID);
			ID = va_arg(vap, MeterType);
		}
		va_end(vap);
	}
	return true;
}

bool CMeterManager::createMeterAttibuteList( MeterType ID )
{
	if (m_setLoadMeterID.size() <= 0)
	{
		return false;
	}
	if (m_vecMeterXmlNode.size() <= 0)
	{
		return false;
	}
	//创建仪表信息列表
	for (int i=0; i< m_vecMeterXmlNode.size(); i++)
	{
		FeUtil::TiXmlElement* pTemp = m_vecMeterXmlNode.at(i);
		RecordMeterAttribute(pTemp);
	}

	return true;
}

void CMeterManager::RecordMeterAttribute( FeUtil::TiXmlElement* pTixmlElement )
{
#ifndef METER_ATTRIBUTE
#define METER_ATTRIBUTE

#define OBJECT_NAME "object-name"
#define POS         "position"
#define SCALE       "scale"

#endif

	SMeterAttribute sMeterAttribute;
	SXYZ sXYZ;
	FeUtil::TiXmlElement* pObjectNameAttri = pTixmlElement->FirstChildElement(OBJECT_NAME);
	FeUtil::TiXmlElement* pPosAttri = pTixmlElement->FirstChildElement(POS);
	FeUtil::TiXmlElement* pScaleAttri = pTixmlElement->FirstChildElement(SCALE);
	if (pObjectNameAttri)
	{
		sMeterAttribute.strMeterName = pObjectNameAttri->FirstChild()->Value();
	}
	if (pPosAttri)
	{
		sXYZ.x = atof(pPosAttri->FirstChild("x")->FirstChild()->Value());
		sXYZ.y = atof(pPosAttri->FirstChild("y")->FirstChild()->Value());
		sXYZ.z = atof(pPosAttri->FirstChild("z")->FirstChild()->Value());
		sMeterAttribute.position = sXYZ;
	}
	if (pScaleAttri)
	{
		sXYZ.x = atof(pScaleAttri->FirstChild("x")->FirstChild()->Value());
		sXYZ.y = atof(pScaleAttri->FirstChild("y")->FirstChild()->Value());
		sXYZ.z = atof(pScaleAttri->FirstChild("z")->FirstChild()->Value());
		sMeterAttribute.scale = sXYZ;
	}

	m_vecMeterAttributeList.push_back(sMeterAttribute);
}

int CMeterManager::ReturnMeterIndex( std::string strMeterName )
{
	for (int i=0; i<m_vecMeterAttributeList.size(); i++)
	{
		if (std::strcmp(strMeterName.c_str(), m_vecMeterAttributeList.at(i).strMeterName.c_str()) == 0)
		{
			return i;
		}
	}
	return 0;
}
