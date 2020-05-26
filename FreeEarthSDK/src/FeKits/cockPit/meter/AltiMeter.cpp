#include <FeKits/cockPit/meter/AltiMeter.h>

#include <osgDB/ReadFile>

#include <FeUtils/PathRegistry.h>

CAltiMeter::CAltiMeter(void):
m_pdrum1(NULL),
m_pdrum2(NULL),
m_pdrum3(NULL),
m_pdrum_ingh_1(NULL),
m_pdrum_ingh_2(NULL),
m_pdrum_ingh_3(NULL),
m_pdrum_ingh_4(NULL),
m_pAceelNeedle(NULL),
m_pNagetive_flag(NULL),
m_dAltitude(0.0),
m_dMaxAltiValue(99999.0),
m_dMinAltiValue(0.0)
{
	RebuildAltiMeter();
	initializeGeomID();
}

CAltiMeter::~CAltiMeter(void)
{
}

osg::Group* CAltiMeter::RebuildAltiMeter()
{
	osg::Group* pNewAltMeter = new osg::Group;
	std::string strNodePath = FeFileReg->GetFullPath("meter/altimeter/altimeter.ac");
	osg::Node* pAltMeter = osgDB::readNodeFile(strNodePath);
	if (!pAltMeter)
	{
		osg::notify(osg::INFO)<<" 读取模型出错，模型正确路径：" + strNodePath<<std::endl;
		return NULL;
	}
	//分析拆分海拔仪表，并重组仪表以适用于程序控制
	osg::Node* pChildLevelOne = pAltMeter->asGroup()->getChild(0);
	osg::Group* pBKGGroup = pChildLevelOne->asGroup()->getChild(0)->asGroup();
	osg::Group* pSwith_Knod_Group = pChildLevelOne->asGroup()->getChild(1)->asGroup();
	osg::Group* pDrumsGroup = pChildLevelOne->asGroup()->getChild(2)->asGroup();
	osg::Group* pNeedleGroup = pChildLevelOne->asGroup()->getChild(3)->asGroup();
	osg::Group* pFaceGroup = pChildLevelOne->asGroup()->getChild(4)->asGroup();
	osg::Group* pMountGroup = pChildLevelOne->asGroup()->getChild(5)->asGroup();
	osg::Group * pIngh_knod_left_Group = pChildLevelOne->asGroup()->getChild(6)->asGroup();
	osg::Group* pIngh_knod_right_Group = pChildLevelOne->asGroup()->getChild(7)->asGroup();
	
	//drum
	osg::Node* pnagativ_flag = pDrumsGroup->getChild(0);
	//海拔高度判断标示
	osg::Node* pbelow_1000_flag = pDrumsGroup->getChild(1);
	osg::Node* pbelow_10000_flag = pDrumsGroup->getChild(2);
	//气压标示
	osg::Node* pdrum_ingh1 = pDrumsGroup->getChild(3);
	osg::Node* pdrum_ingh2 = pDrumsGroup->getChild(4);
	osg::Node* pdrum_ingh3 = pDrumsGroup->getChild(5);
	osg::Node* pdrum_ingh4 = pDrumsGroup->getChild(6);
	//分别为10000， 1000， 100 单位标示
	osg::Node* pdrum1 = pDrumsGroup->getChild(7);
	osg::Node* pdrum2 = pDrumsGroup->getChild(8);
	osg::Node* pdrum3 = pDrumsGroup->getChild(9);

	osg::Group* pNewdrum = new osg::Group;

	pNewdrum->addChild(pnagativ_flag);
#ifndef _SHOWHIDEFLAG
#define _SHOWHIDEFLAG
#define HIDE 0x00
#define SHOW ~0x00
#endif
	pnagativ_flag->setNodeMask(HIDE);
	pNewdrum->addChild(pdrum_ingh1);
	pNewdrum->addChild(pdrum_ingh2);
	pNewdrum->addChild(pdrum_ingh3);
	pNewdrum->addChild(pdrum_ingh4);
	pNewdrum->addChild(pdrum1);
	pNewdrum->addChild(pdrum2);
	pNewdrum->addChild(pdrum3);

	osg::MatrixTransform* pNeedleMt = new osg::MatrixTransform;
	pNeedleMt->addChild(pNeedleGroup);
	m_pAceelNeedle = pNeedleMt;

	pNewAltMeter->addChild(pNewdrum);
	pNewAltMeter->addChild(pBKGGroup);
	pNewAltMeter->addChild(pSwith_Knod_Group);
	pNewAltMeter->addChild(pNeedleMt);
	pNewAltMeter->addChild(pFaceGroup);
	pNewAltMeter->addChild(pMountGroup);
	pNewAltMeter->addChild(pIngh_knod_left_Group);
	pNewAltMeter->addChild(pIngh_knod_right_Group);

	//成员变量赋值
	m_pdrum_ingh_1 = pdrum_ingh1->asGroup()->getChild(0)->asGeode()->getDrawable(0)->asGeometry();
	m_pdrum_ingh_2 = pdrum_ingh2->asGroup()->getChild(0)->asGeode()->getDrawable(0)->asGeometry();
	m_pdrum_ingh_3 = pdrum_ingh3->asGroup()->getChild(0)->asGeode()->getDrawable(0)->asGeometry();
	m_pdrum_ingh_4 = pdrum_ingh4->asGroup()->getChild(0)->asGeode()->getDrawable(0)->asGeometry();

	m_pdrum1 = pdrum1->asGroup()->getChild(0)->asGeode()->getDrawable(0)->asGeometry();
	m_pdrum2 = pdrum2->asGroup()->getChild(0)->asGeode()->getDrawable(0)->asGeometry();
	m_pdrum3 = pdrum3->asGroup()->getChild(0)->asGeode()->getDrawable(0)->asGeometry();

	m_pNagetive_flag = pnagativ_flag;

	//使纹理起始的位子显示为0
	osg::Vec2Array *pDrumVecArray1 = static_cast<osg::Vec2Array*>(m_pdrum1->getTexCoordArray(0));
	osg::Vec2Array *pDrumVecArray2 = static_cast<osg::Vec2Array*>(m_pdrum2->getTexCoordArray(0));
	osg::Vec2Array *pDrumVecArray3 = static_cast<osg::Vec2Array*>(m_pdrum3->getTexCoordArray(0));
//	osg::Vec2Array *pDrumVecArray4 = static_cast<osg::Vec2Array*>(m_pdrum_ingh_1->getTexCoordArray(0));

	
	int size = pDrumVecArray1->size();
	for (int i=0; i<size; i++)
	{
		double y = pDrumVecArray1->at(i).y();
		y += 0.1;
		pDrumVecArray1->at(i).y() = y;
	
		double y1 = pDrumVecArray2->at(i).y();
		y1 += 0.1;
		pDrumVecArray2->at(i).y() = y1;

		double y2 = pDrumVecArray3->at(i).y();
		y2 += 0.1;
		pDrumVecArray3->at(i).y() = y2;

	}
	m_pdrum1->setTexCoordArray(0,pDrumVecArray1);
	m_pdrum2->setTexCoordArray(0,pDrumVecArray2);
	m_pdrum3->setTexCoordArray(0,pDrumVecArray3);

	addChild(pNewAltMeter);
	return NULL;
}

SAltiValue CAltiMeter::SpiteAltitude( double dAltitude )
{
	SAltiValue tempAlti;
	tempAlti.dDelta = dAltitude - int(dAltitude);

	tempAlti.dAltiNeedle = double(int(dAltitude) % 100) + tempAlti.dDelta;

	tempAlti.dAltiBoard_below_10000 = double(int(dAltitude) / 10000);

	tempAlti.dAltiBoard_below_1000 = double(int(dAltitude) / 1000) 
		- 10 * tempAlti.dAltiBoard_below_10000;

	tempAlti.dAltiBoard_below_100 = double(int(dAltitude) / 100) 
		- 100 * tempAlti.dAltiBoard_below_10000 
		- 10 * tempAlti.dAltiBoard_below_1000;

	return tempAlti;
}

void CAltiMeter::RotateNeedle( double dAngle )
{
	m_pAceelNeedle->setMatrix(osg::Matrix::rotate(dAngle, osg::X_AXIS));
}

void CAltiMeter::TranslateDrum(int nID, double dTransformTex)
{
	mapGeometryID::iterator itrTemp = m_mapGeomID.find((EAltiMeterGeomID)nID);
	osg::Geometry* pGeometry = itrTemp->second;
	if (!pGeometry)
	{
		return;
	}
	else
	{
		osg::Vec2Array *pDrumVecArray = static_cast<osg::Vec2Array*>(pGeometry->getTexCoordArray(0));
		int size = pDrumVecArray->size();
		for (int i=0; i<size; i++)
		{
			double dy = pDrumVecArray->at(i).y();
			dy -= dTransformTex;
			pDrumVecArray->at(i).y() = dy;
		}
		pGeometry->setTexCoordArray(0,pDrumVecArray);
	}
}

void CAltiMeter::initializeGeomID()
{
	m_mapGeomID[AltiBoard1] = m_pdrum1;
	m_mapGeomID[AltiBoard2] = m_pdrum2;
	m_mapGeomID[AltiBoard3] = m_pdrum3;
	m_mapGeomID[Airpressure1] = m_pdrum_ingh_1;
	m_mapGeomID[Airpressure2] = m_pdrum_ingh_2;
	m_mapGeomID[Airpressure3] = m_pdrum_ingh_3;
	m_mapGeomID[Airpressure4] = m_pdrum_ingh_4;
}

void CAltiMeter::ShowNagetiveFlag( bool bflag )
{
#ifndef _SHOWHIDEFLAG
#define _SHOWHIDEFLAG
#define HIDE 0x00
#define SHOW ~0x00
#endif
	unsigned int unFlag = bflag ? SHOW : HIDE;
	if (m_pNagetive_flag->getNodeMask() == unFlag)
	{
		return;
	}
	m_pNagetive_flag->setNodeMask(unFlag);
}

void CAltiMeter::UpdateAltitude( double dHeight )
{
	if (dHeight >= m_dMaxAltiValue || dHeight <= m_dMinAltiValue)
	{
		if (dHeight > (m_dMaxAltiValue+m_dMinAltiValue)/2.0)
		{
			dHeight = m_dMaxAltiValue;
		}
		else
		{
			dHeight = m_dMinAltiValue;
		}
		ShowNagetiveFlag(true);
	}
	else
	{
		ShowNagetiveFlag(false);
	}
	double oldAlti = GetAltitudeValue();
	if (std::abs(oldAlti - dHeight) < 0.0000001)
	{
		return;
	}
	else
	{
		//		osg::MatrixTransform* pNeedle = m_Altimeter->GetAceelNeedle();
		//原高度值
		SAltiValue oldSpiteAltitude;
		oldSpiteAltitude = SpiteAltitude(oldAlti);
		//新高度值
		SAltiValue newSpiteAltitude;
		newSpiteAltitude = SpiteAltitude(dHeight);

#ifndef DEGREESPERFOOT
#define DEGREESPERFOOT
		////////////////////////////////////////
		// -3.6的含义：
		//每改变一英尺仪表指针转动 3.6°
		//负号是因为OSG转动方向顺时针为负。
		////////////////////////////////////////
#define DEGREES_PER_FOOT 3.6
#endif
		double angle = osg::DegreesToRadians(-1 * DEGREES_PER_FOOT * newSpiteAltitude.dAltiNeedle);
		RotateNeedle(angle);
		//定义纹理移动的单位距离0.1
#ifndef TRANSFORM_TEX_RANGE
#define TRANSFORM_TEX_RANGE
#define TRANSFORM_TEX 0.1
#endif
		//0.1 是仪表上显示数值相差1个单位时纹理需要移动的距离
		double dTransTex = 0.1 * int(newSpiteAltitude.dAltiBoard_below_100 - oldSpiteAltitude.dAltiBoard_below_100) ;
//			+ 0.001*(newSpiteAltitude.dAltiNeedle - oldSpiteAltitude.dAltiNeedle);
		TranslateDrum(AltiBoard1, dTransTex);

		dTransTex = 0.1 * int(newSpiteAltitude.dAltiBoard_below_1000 - oldSpiteAltitude.dAltiBoard_below_1000) ;
//			+ dTransTex * 0.1;
		TranslateDrum(AltiBoard2, dTransTex);

		dTransTex = 0.1 * int(newSpiteAltitude.dAltiBoard_below_10000 - oldSpiteAltitude.dAltiBoard_below_10000);
//			+ dTransTex * 0.1;
		TranslateDrum(AltiBoard3, dTransTex);

		SetAltitudeValue(dHeight);
	}
}
