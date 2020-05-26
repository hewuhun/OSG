#include <FeKits/cockPit/meter/HsiMeter.h>

#include <osgDB/ReadFile>

#include <FeUtils/PathRegistry.h>

CHsiMeter::CHsiMeter(void) :
m_pDrum_course1(NULL),
m_pDrum_course2(NULL),
m_pDrum_course3(NULL),
m_pDrum_mile1(NULL),
m_pDrum_mile2(NULL),
m_pDrum_mile3(NULL),
m_pHsi_compass_cardMT(NULL),
m_pHsi_NeedlesMT(NULL),
m_pHsi_HDG_knodsMT(NULL),
m_pHsi_CRS_knodsMT(NULL),
m_pHsi_cdiMT(NULL),
m_pNegativeflag(NULL),
m_dVelocity(0.0),
m_dMile(0.0),
m_dVeloMaxRange(999),
m_dVeloMinRange(0),
m_dMileMaxRange(999),
m_dMileMinRange(0)
{
	RebuildHsiMeter();
	initializeHsiGeomID();
}


CHsiMeter::~CHsiMeter(void)
{
}

osg::Group* CHsiMeter::RebuildHsiMeter()
{
	osg::Group* pNewHsiMeter = new osg::Group;
	std::string strNodePath = FeFileReg->GetFullPath("meter/hsi/hsi.ac");
	osg::Node* pHsiMeter = osgDB::readNodeFile(strNodePath);
	if (!pHsiMeter)
	{
		osg::notify(osg::INFO)<<" 读取模型出错，模型正确路径：" + strNodePath<<std::endl;
		return NULL;
	}
	//分析拆分海拔仪表，并重组仪表以适用于程序控制
	osg::Node* pChildLevelOne = pHsiMeter->asGroup()->getChild(0);
	//Hsi_cdi
	osg::Group* pHsi_cdi = pChildLevelOne->asGroup()->getChild(0)->asGroup();
	//Hsi_drum
	osg::Group* pHsi_drums = pChildLevelOne->asGroup()->getChild(1)->asGroup();
	//Hsi_knods
	osg::Group* pHsi_knods = pChildLevelOne->asGroup()->getChild(2)->asGroup();
	//Hsi_Needles
	osg::Group* pHsi_Needles = pChildLevelOne->asGroup()->getChild(3)->asGroup();
	//Hsi_aircraft_symbol
	osg::Group* pHsi_aircraft_symbol = pChildLevelOne->asGroup()->getChild(4)->asGroup();
	//Hsi_bkg
	osg::Group* pHsi_bkg = pChildLevelOne->asGroup()->getChild(5)->asGroup();
	//Hsi_compass_card
	osg::Group* pHsi_compass_card = pChildLevelOne->asGroup()->getChild(6)->asGroup();
	//Hsi_course_knob_left/right  Hsi_bdg_knod_left/right  Hsi_mount
	osg::Group* pHsi_course_knod_left = pChildLevelOne->asGroup()->getChild(7)->asGroup();
	osg::Group* pHsi_course_knod_right = pChildLevelOne->asGroup()->getChild(8)->asGroup();
	osg::Group* pHsi_bdg_knod_left = pChildLevelOne->asGroup()->getChild(9)->asGroup();
	osg::Group* pHsi_bdg_knod_right = pChildLevelOne->asGroup()->getChild(10)->asGroup();
	osg::Group* pHsi_mount = pChildLevelOne->asGroup()->getChild(11)->asGroup();
	//off_flag
	osg::Group* pHsi_off_flag = pChildLevelOne->asGroup()->getChild(12)->asGroup();

	///////拆解drumGroup
	osg::Group* pNewDrums = new osg::Group;
	//course drum
	osg::Group* pDrum_course_1 = pHsi_drums->getChild(0)->asGroup();
	osg::Group* pDrum_course_2 = pHsi_drums->getChild(1)->asGroup();
	osg::Group* pDrum_course_3 = pHsi_drums->getChild(2)->asGroup();
	//mile drum
	osg::Group* pDrum_mile_1 = pHsi_drums->getChild(3)->asGroup();
	osg::Group* pDrum_mile_2 = pHsi_drums->getChild(4)->asGroup();
	osg::Group* pDrum_mile_3 = pHsi_drums->getChild(5)->asGroup();
	//nagetive-flag
	osg::Group* pNegativeflag = pHsi_drums->getChild(6)->asGroup();

	
	//开始重组
	m_pHsi_cdiMT = new osg::MatrixTransform;
	m_pHsi_cdiMT->addChild(pHsi_cdi);
	pNewHsiMeter->addChild(m_pHsi_cdiMT);
	
	m_pDrum_course1 = pDrum_course_1->asGroup()->getChild(0)->asGeode()->getDrawable(0)->asGeometry();
	m_pDrum_course2 = pDrum_course_2->asGroup()->getChild(0)->asGeode()->getDrawable(0)->asGeometry();
	m_pDrum_course3 = pDrum_course_3->asGroup()->getChild(0)->asGeode()->getDrawable(0)->asGeometry();
	m_pDrum_mile1 = pDrum_mile_1->asGroup()->getChild(0)->asGeode()->getDrawable(0)->asGeometry();
	m_pDrum_mile2 = pDrum_mile_2->asGroup()->getChild(0)->asGeode()->getDrawable(0)->asGeometry();
	m_pDrum_mile3 = pDrum_mile_3->asGroup()->getChild(0)->asGeode()->getDrawable(0)->asGeometry();
	pNewHsiMeter->addChild(pDrum_course_1);
	pNewHsiMeter->addChild(pDrum_course_2);
	pNewHsiMeter->addChild(pDrum_course_3);
	pNewHsiMeter->addChild(pDrum_mile_1);
	pNewHsiMeter->addChild(pDrum_mile_2);
	pNewHsiMeter->addChild(pDrum_mile_3);

	m_pNegativeflag = pNegativeflag;
#ifndef _SHOWHIDEFLAG
#define _SHOWHIDEFLAG
#define HIDE 0x00
#define SHOW ~0x00
#endif
	m_pNegativeflag->setNodeMask(HIDE);
	pNewHsiMeter->addChild(pNegativeflag);
	
	m_pHsi_HDG_knodsMT = new osg::MatrixTransform;
	m_pHsi_HDG_knodsMT->addChild(pHsi_knods->getChild(1));
	pNewHsiMeter->addChild(m_pHsi_HDG_knodsMT);

	m_pHsi_CRS_knodsMT = new osg::MatrixTransform;
	m_pHsi_CRS_knodsMT->addChild(pHsi_knods->getChild(0));
	pNewHsiMeter->addChild(m_pHsi_CRS_knodsMT);

	m_pHsi_NeedlesMT = new osg::MatrixTransform;
	m_pHsi_NeedlesMT->addChild(pHsi_Needles);
	pNewHsiMeter->addChild(m_pHsi_NeedlesMT);

	pNewHsiMeter->addChild(pHsi_bkg);
	pNewHsiMeter->addChild(pHsi_aircraft_symbol);

	m_pHsi_compass_cardMT = new osg::MatrixTransform;
	m_pHsi_compass_cardMT->addChild(pHsi_compass_card);
	pNewHsiMeter->addChild(m_pHsi_compass_cardMT);

	pNewHsiMeter->addChild(pHsi_bdg_knod_left);
	pNewHsiMeter->addChild(pHsi_bdg_knod_right);
	pNewHsiMeter->addChild(pHsi_course_knod_left);
	pNewHsiMeter->addChild(pHsi_course_knod_right);
	pNewHsiMeter->addChild(pHsi_mount);
	pNewHsiMeter->addChild(pHsi_off_flag);

	osg::Vec2Array *pDrumVecArray1 = static_cast<osg::Vec2Array*>(m_pDrum_course1->getTexCoordArray(0));
	osg::Vec2Array *pDrumVecArray2 = static_cast<osg::Vec2Array*>(m_pDrum_course2->getTexCoordArray(0));
	osg::Vec2Array *pDrumVecArray3 = static_cast<osg::Vec2Array*>(m_pDrum_course3->getTexCoordArray(0));
	osg::Vec2Array *pDrumVecArray4 = static_cast<osg::Vec2Array*>(m_pDrum_mile1->getTexCoordArray(0));
	osg::Vec2Array *pDrumVecArray5 = static_cast<osg::Vec2Array*>(m_pDrum_mile2->getTexCoordArray(0));
	osg::Vec2Array *pDrumVecArray6 = static_cast<osg::Vec2Array*>(m_pDrum_mile3->getTexCoordArray(0));
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
	m_pDrum_course1->setTexCoordArray(0,pDrumVecArray1);
	m_pDrum_course2->setTexCoordArray(0,pDrumVecArray2);
	m_pDrum_course3->setTexCoordArray(0,pDrumVecArray3);

	size = pDrumVecArray4->size();
	for (int i=0; i<size; i++)
	{
		double y = pDrumVecArray4->at(i).y();
		y += 0.1;
		pDrumVecArray4->at(i).y() = y;

		double y1 = pDrumVecArray5->at(i).y();
		y1 += 0.1;
		pDrumVecArray5->at(i).y() = y1;

		double y2 = pDrumVecArray6->at(i).y();
		y2 += 0.1;
		pDrumVecArray6->at(i).y() = y2;
	}
	m_pDrum_mile1->setTexCoordArray(0,pDrumVecArray4);
	m_pDrum_mile2->setTexCoordArray(0,pDrumVecArray5);
	m_pDrum_mile3->setTexCoordArray(0,pDrumVecArray6);

	addChild(pNewHsiMeter);

	return NULL;
}

void CHsiMeter::AircraftYaw( double dAnlge )
{
	double dRadian = osg::DegreesToRadians(dAnlge);
	osg::Matrix centerMatrix = osg::Matrix::translate(0, 0.0043, 0);
	osg::Matrix NegativeCenterMatrix = osg::Matrix::translate(0, -0.0043, 0);
	m_pHsi_compass_cardMT->setMatrix(centerMatrix * 
		osg::Matrix::rotate(dRadian, osg::X_AXIS) * 
		NegativeCenterMatrix);
}

void CHsiMeter::initializeHsiGeomID()
{
	m_mapGeomID[VelocityBoard1] = m_pDrum_course1;
	m_mapGeomID[VelocityBoard2] = m_pDrum_course2;
	m_mapGeomID[VelocityBoard3] = m_pDrum_course3;
	m_mapGeomID[MileBoard1] = m_pDrum_mile1;
	m_mapGeomID[MileBoard2] = m_pDrum_mile2;
	m_mapGeomID[MileBoard3] = m_pDrum_mile3;
}

SVelocityValue CHsiMeter::SpiteVelocity( double dVelocity )
{
	SVelocityValue velocityValue;
//	double dDelta = dVelocity - int(dVelocity);
	velocityValue.dVelo_below1000 = double(int(dVelocity) / 100);
	velocityValue.dVelo_below100 = double(int(dVelocity) / 10) 
		- 10 * velocityValue.dVelo_below1000;
	velocityValue.dVelo_below10 = int(dVelocity)
		- 100*velocityValue.dVelo_below1000 
		- 10*velocityValue.dVelo_below100;
	return velocityValue;	
}

SMileValue CHsiMeter::SpiteMile( double dMile )
{
	SMileValue mileValue;
	mileValue.dMile_below1000 = double(int(dMile) / 100);
	mileValue.dMile_below100 = double(int(dMile) / 10) 
		- 10*mileValue.dMile_below1000;
	mileValue.dMile_below10 = int(dMile) 
		- 100*mileValue.dMile_below1000 
		- 10*mileValue.dMile_below100;
	return mileValue;	
}

void CHsiMeter::TranslateTexDrum( int nID, double dTransformTex )
{
	mapHsiGeometryID::iterator itrTemp = m_mapGeomID.find((EHsiMeterGeomID)nID);
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

void CHsiMeter::ShowNagetiveFlag( bool bflag )
{
#ifndef _SHOWHIDEFLAG
#define _SHOWHIDEFLAG
#define HIDE 0x00
#define SHOW ~0x00
#endif
	unsigned int unFlag = bflag ? SHOW : HIDE;
	if (m_pNegativeflag->getNodeMask() == unFlag)
	{
		return;
	}
	m_pNegativeflag->setNodeMask(unFlag);
}

void CHsiMeter::UpdateVelocity( double dVelocity )
{
	//实时飞行速度
	double dOldVelocity = GetVelocity();
	if (dVelocity >= GetVeloMaxRange() || dVelocity <= GetVeloMinRange())
	{
		if (dVelocity > (GetVeloMaxRange() + GetVeloMinRange())/2.0)
		{
			dVelocity = GetVeloMaxRange();
		}
		else
		{
			dVelocity = GetVeloMinRange();
		}
	}
	if (std::abs(dOldVelocity - dVelocity) < 0.0000001)
	{
		return;
	}
	else
	{
		//原速度
		SVelocityValue SOldVelocity;
		SOldVelocity = SpiteVelocity(dOldVelocity);
		//新速度
		SVelocityValue SNewVelocity;
		SNewVelocity = SpiteVelocity(dVelocity);
		//定义纹理移动的单位距离0.1
#ifndef TRANSFORM_TEX_RANGE
#define TRANSFORM_TEX_RANGE
#define TRANSFORM_TEX 0.1
#endif
		double dTransTex = 0;
		if (int(SNewVelocity.dVelo_below1000) == 9 
			&& int(SNewVelocity.dVelo_below100) == 9 )
		{
			dTransTex = 0.1 * (int(SNewVelocity.dVelo_below10) - int(SOldVelocity.dVelo_below10));
			TranslateTexDrum(VelocityBoard1, dTransTex);
			if (int(SNewVelocity.dVelo_below10) == 9)
			{
				dTransTex = 0.1 * (SNewVelocity.dVelo_below100 - SOldVelocity.dVelo_below100);
//					+ dTransTex * 0.1;
				TranslateTexDrum(VelocityBoard2, dTransTex);
			}
		}
		else
		{
			dTransTex = 0.1 * (int(SNewVelocity.dVelo_below10) - int(SOldVelocity.dVelo_below10));
			TranslateTexDrum(VelocityBoard1, dTransTex);
		}
//		if (int(SNewVelocity.dVelo_below1000) != 9 || int(SNewVelocity.dVelo_below100) != 9)
//		{
			dTransTex = 0.1 * (SNewVelocity.dVelo_below100 - SOldVelocity.dVelo_below100);
//				+ dTransTex * 0.1;
			TranslateTexDrum(VelocityBoard2, dTransTex);
//		}
//		if (int(SNewVelocity.dVelo_below1000) != 9)
//		{
			dTransTex = 0.1 * (SNewVelocity.dVelo_below1000 - SOldVelocity.dVelo_below1000);
//				+ dTransTex * 0.1;
			TranslateTexDrum(VelocityBoard3, dTransTex);
//		}
		RecordVelocity(dVelocity);
	}
}

void CHsiMeter::UpdateMiles( double dMile )
{
	//航程
	double dOldMile = GetMile();
	if (dMile >= GetMileMaxRange() || dMile <= GetMileMinRange())
	{
		if (dMile > (GetMileMaxRange() + GetMileMinRange())/2.0)
		{
			dMile = GetMileMaxRange();
			ShowNagetiveFlag(true);
		}
		else
		{
			dMile = GetMileMinRange();
			ShowNagetiveFlag(false);
		}
	}
	if (std::abs(dOldMile - dMile) < 0.0000001)
	{
		return;
	}
	else
	{
		//原速度
		SMileValue SOldMile;
		SOldMile = SpiteMile(dOldMile);
		//新速度
		SMileValue SNewMile;
		SNewMile = SpiteMile(dMile);
		//定义纹理移动的单位距离0.1
#ifndef TRANSFORM_TEX_RANGE
#define TRANSFORM_TEX_RANGE
#define TRANSFORM_TEX 0.1
#endif
		double dTransTex = 0;
		if (int(SNewMile.dMile_below1000) == 9 
			&& int(SNewMile.dMile_below100) == 9 )
		{
			dTransTex = 0.1 * (int(SNewMile.dMile_below10) - int(SOldMile.dMile_below10));
			TranslateTexDrum(MileBoard1, dTransTex);
			if (int(SNewMile.dMile_below10) == 9)
			{
				dTransTex = 0.1 * (SNewMile.dMile_below100 - SOldMile.dMile_below100) ;
//					+ 0.1 * dTransTex;
				TranslateTexDrum(MileBoard2, dTransTex);
			}
		}
		else
		{
			dTransTex = 0.1 * (int(SNewMile.dMile_below10) - int(SOldMile.dMile_below10));
			TranslateTexDrum(MileBoard1, dTransTex);
		}
//		if (int(SNewMile.dMile_below1000) != 9 || int(SNewMile.dMile_below100) != 9)
//		{
			dTransTex = 0.1 * (SNewMile.dMile_below100 - SOldMile.dMile_below100);
//				+ dTransTex * 0.1;
			TranslateTexDrum(MileBoard2, dTransTex);
//		}
//		if (int(SNewMile.dMile_below1000) != 9)
//		{
			dTransTex = 0.1 * (SNewMile.dMile_below1000 - SOldMile.dMile_below1000);
//				+ dTransTex * 0.1;
			TranslateTexDrum(MileBoard3, dTransTex);
//		}
		RecordMile(dMile);
	}
}

void CHsiMeter::UpdateHeading( double dHeading )
{
	double dHeadingdelta = dHeading - int(dHeading);
	int nHeading = int(dHeading) % 360;
	double dNewHeading = nHeading + dHeadingdelta;
	AircraftYaw(dNewHeading);
}

void CHsiMeter::UpdateCourseknod( double dCourseAngle )
{
	//①旋转按钮
	//②改变指针角度依据旋转按钮的角度
}
