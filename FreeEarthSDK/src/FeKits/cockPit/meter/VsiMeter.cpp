#include <FeKits/cockPit/meter/VsiMeter.h>

#include <osgDB/ReadFile>
#include <cstring>

#include <FeUtils/PathRegistry.h>

CVsiMeter::CVsiMeter(void) :
m_pVsiNeedleMt(NULL) ,
m_dMinClimbRatePerMin(-6000),
m_dMaxClimbRatePerMin(6000),
m_dClimbRate(0)
{
	RebuildVsiMeter();
	InitClimbRateMap();
}


CVsiMeter::~CVsiMeter(void)
{
}

osg::Group* CVsiMeter::RebuildVsiMeter()
{
	osg::Group* pNewVsiMeter = new osg::Group;
	std::string strNodePath = FeFileReg->GetFullPath("meter/vsi/vsi.ac");
	osg::Node* pVsiMeter = osgDB::readNodeFile(strNodePath);
	if (!pVsiMeter)
	{
		osg::notify(osg::INFO)<<" 读取模型出错，模型正确路径：" + strNodePath<<std::endl;
		return NULL;
	}
	//分析拆分垂直攀升下降幅度仪表
	osg::Node* pChildLevelOne = pVsiMeter->asGroup()->getChild(0);
	osg::Group* pVsiFaceGroup = pChildLevelOne->asGroup()->getChild(0)->asGroup();
	osg::Group* pVsiMountGroup = pChildLevelOne->asGroup()->getChild(1)->asGroup();
	osg::Group* pVsiNeedleGroup = pChildLevelOne->asGroup()->getChild(2)->asGroup();
	osg::Group* pVsiOffFlagGroup = pChildLevelOne->asGroup()->getChild(3)->asGroup();

	pNewVsiMeter->addChild(pVsiFaceGroup);
	pNewVsiMeter->addChild(pVsiMountGroup);
	osg::MatrixTransform* pNeedleMt = new osg::MatrixTransform;
	pNeedleMt->addChild(pVsiNeedleGroup);
	m_pVsiNeedleMt = pNeedleMt;
	pNewVsiMeter->addChild(pNeedleMt);
	pNewVsiMeter->addChild(pVsiOffFlagGroup);

	addChild(pNewVsiMeter);
	return NULL;
}

void CVsiMeter::InitClimbRateMap()
{
	std::string strModelXmlPath = FeFileReg->GetFullPath("meter/vsi/vsi.xml");
	FeUtil::TiXmlDocument* pXmlDocument = new FeUtil::TiXmlDocument();
	if (!pXmlDocument->LoadFile(strModelXmlPath.c_str()))
	{
	osg::notify(osg::NOTICE)<<"模型配置文件读取错误！"<<std::endl;
	return;
	}
	FeUtil::TiXmlElement* pRootXmlElement = NULL;
	if (!(pRootXmlElement = pXmlDocument->RootElement()))
	{
	osg::notify(osg::NOTICE)<<"模型配置文件无效！"<<std::endl;
	return;
	}
	std::vector<FeUtil::TiXmlElement*> vecXmlElements;

	std::string strNodeName = "entry";
	FindXmlNode(strNodeName, pRootXmlElement, vecXmlElements);

	for (int i=0; i<vecXmlElements.size(); i++)
	{
		double ind = GetNodeValue(vecXmlElements.at(i), "ind");
		double dep = GetNodeValue(vecXmlElements.at(i), "dep");
		m_mapClimbRate[ind] =  dep;
	}
}

double CVsiMeter::GetNodeValue(FeUtil::TiXmlElement* pTemp, char* cName)
{
	FeUtil::TiXmlNode* pTempNode = pTemp->FirstChild(cName)->FirstChild();
	const char* cValue = pTempNode->Value();
	return std::atof(cValue);
}

void CVsiMeter::FindXmlNode( std::string strNodeName, FeUtil::TiXmlElement* pParent, vecXmlElements& vecXmlElem )
{
	if (!pParent)
	{
		return;
	}
	FeUtil::TiXmlElement* pChild = pParent->FirstChildElement();
	while (pChild)
	{
		if (std::strcmp(pChild->Value() , strNodeName.c_str()) == 0)
		{
			vecXmlElem.push_back(pChild);
		}
		else
		{
			FindXmlNode(strNodeName, pChild, vecXmlElem);
		}
		pChild = pChild->NextSiblingElement();
	}
}

void CVsiMeter::RotateNeedle( double dAngle )
{
//	osg::Vec3 pos = osg::X_AXIS;
	osg::Matrix centerMatrix = osg::Matrix::translate(0, 0, 0.011000);
	osg::Matrix NegativeCenterMatrix = osg::Matrix::translate(0, 0, -0.011000);
	m_pVsiNeedleMt->setMatrix(centerMatrix * 
		osg::Matrix::rotate(dAngle, osg::X_AXIS) * 
		NegativeCenterMatrix);
//	m_pVsiNeedleMt->setMatrix(osg::Matrix::rotate(dAngle, osg::Vec3(1, 0.011, 0)));
}

double CVsiMeter::CalcNeedleRotate( double dClimbRate )
{
	//http://enbdev.com/enbseries/forum/viewtopic.php?f=14&t=17&start=15740
	//限制攀升或下降率不能超过仪表所能表示的最大或者最小值
	if (dClimbRate > 6000 || dClimbRate < -6000)
	{
		if (dClimbRate > 0)
		{
			dClimbRate = 6000;
		}
		else
			dClimbRate = -6000;
	}
	//计算攀升或下降比例所对应的指针旋转角度（单位：度）
	int size = m_mapClimbRate.size();
	mapClimbRate::iterator itr;
	mapClimbRate::iterator peritr;
	for (itr = m_mapClimbRate.begin(); itr != m_mapClimbRate.end(); itr++)
	{
		//除以60的含义：Vsi仪表单位为英尺每分钟，传入的dClimbRate的单位是英尺没秒。
		if (dClimbRate/60.0 < itr->first)
		{
			peritr = --itr ;
			break;
		}
	}
	itr++;
	double angle = peritr->second 
		+ (dClimbRate/60.0 - peritr->first)/(itr->first - peritr->first) * (itr->second - peritr->second); 

	return -1.0 * angle;
}

void CVsiMeter::UpdateClimbRate( double dClimbRate )
{
	
	if (std::abs(dClimbRate - GetClimbRate()) < 0.000001)
	{
		return;
	}
	else
	{
		double dAngleDegree = CalcNeedleRotate(dClimbRate);
		RotateNeedle(osg::DegreesToRadians(dAngleDegree));
		SetClimbRate(dClimbRate);
	}
}

