#include <FeKits/cockPit/meter/AdiMeter.h>

#include <osgDB/ReadFile>

#include <FeUtils/PathRegistry.h>

CAdiMeter::CAdiMeter(void) : osg::MatrixTransform(),
m_pAdi_aux_flag(NULL),
m_pAdi_gs_flag(NULL),
m_pAdi_loc_flag(NULL),
m_pAdi_off_flag(NULL),
m_pAdi_bank_indexMT(NULL),
m_pAdi_glide_slope_indexMT(NULL),
m_pAdi_pitch_steering_barMT(NULL),
m_pAdi_slipe_ballMT(NULL),
m_pAdi_sphereMT(NULL),
m_pAdi_turn_indicatorMT(NULL),
m_pAdi_yaw_steering_barMT(NULL),
m_dMaxPitchAngle(90.0),
m_dMinPitchAngle(-90.0),
m_dMaxRollAngle(180),
m_dMinRollAngle(-180),
m_dMaxYawAngle(180),
m_dMinYawAngle(-180)
{
	RebuildAdiMeter();
}


CAdiMeter::~CAdiMeter(void)
{
}

osg::Group* CAdiMeter::RebuildAdiMeter()
{
	osg::Group* pNewAdiMeter = new osg::Group;
	std::string strNodePath = FeFileReg->GetFullPath("meter/adi/adi.ac");
	osg::Node* pAdiMeter = osgDB::readNodeFile(strNodePath);
	if (!pAdiMeter)
	{
		osg::notify(osg::INFO)<<" 读取模型出错，模型正确路径：" + strNodePath<<std::endl;
		return NULL;
	}
	//分析拆分Adi仪表，并重组仪表以适用于程序控制
	osg::Node* pChildLevelOne = pAdiMeter->asGroup()->getChild(0);
	//第一级的六个子节点
	osg::Group* pAdi_instr = pChildLevelOne->asGroup()->getChild(0)->asGroup();
	osg::Group* pAdi_arrow_knod_ccw = pChildLevelOne->asGroup()->getChild(1)->asGroup();
	osg::Group* pAdi_arrow_knod_cw = pChildLevelOne->asGroup()->getChild(2)->asGroup();
	osg::Group* pAdi_bkg = pChildLevelOne->asGroup()->getChild(3)->asGroup();
	osg::Group* pAdi_knod = pChildLevelOne->asGroup()->getChild(4)->asGroup();
	osg::Group* pAdi_mount = pChildLevelOne->asGroup()->getChild(5)->asGroup();
	//Adi仪表第二级第一个节点（Adi Flags）的是十个子节点
	osg::Group* pNewAdi_instr = new osg::Group;
	//|---->aux  
	//|---->gs  
	//|---->loc  
	//|---->off  
//	osg::Group* pNewAdi_flags = new osg::Group;
	osg::Group* pAdi_flags = pAdi_instr->getChild(0)->asGroup();
	osg::Group* pAdi_aux_flag = pAdi_flags->getChild(0)->asGroup();  
	osg::Group* pAdi_gs_flag = pAdi_flags->getChild(1)->asGroup(); 
	osg::Group* pAdi_loc_flag = pAdi_flags->getChild(2)->asGroup(); 
	osg::Group* pAdi_off_flag = pAdi_flags->getChild(3)->asGroup();

	osg::Group* pAdi_bank_index = pAdi_instr->getChild(1)->asGroup();         // 转
	osg::Group* pAdi_face = pAdi_instr->getChild(2)->asGroup();  
	osg::Group* pAdi_glide_slope_index = pAdi_instr->getChild(3)->asGroup();  //选、转
	osg::Group* pAdi_pitch_steering_bar = pAdi_instr->getChild(4)->asGroup(); //选、移
	osg::Group* pAdi_reticle = pAdi_instr->getChild(5)->asGroup(); 
	osg::Group* pAdi_slipe_ball = pAdi_instr->getChild(6)->asGroup();         //转 [-2,2]
	osg::Group* pAdi_sphere = pAdi_instr->getChild(7)->asGroup();             //转XY 
	osg::Group* pAdi_turn_indicator = pAdi_instr->getChild(8)->asGroup();     //移
	osg::Group* pAdi_yaw_steering_bar = pAdi_instr->getChild(9)->asGroup();   //选、移

	m_pAdi_bank_indexMT = new osg::MatrixTransform;
	m_pAdi_bank_indexMT->addChild(pAdi_bank_index);
	pNewAdi_instr->addChild(m_pAdi_bank_indexMT);
	
	pNewAdi_instr->addChild(pAdi_face);
	
	m_pAdi_glide_slope_indexMT = new osg::MatrixTransform;
	m_pAdi_glide_slope_indexMT->addChild(pAdi_glide_slope_index);
	pNewAdi_instr->addChild(m_pAdi_glide_slope_indexMT);
	
	m_pAdi_pitch_steering_barMT = new osg::MatrixTransform;
	m_pAdi_pitch_steering_barMT->addChild(pAdi_pitch_steering_bar);
	pNewAdi_instr->addChild(m_pAdi_pitch_steering_barMT);

	pNewAdi_instr->addChild(pAdi_reticle);

	m_pAdi_slipe_ballMT = new osg::MatrixTransform;
	m_pAdi_slipe_ballMT->addChild(pAdi_slipe_ball);
	pNewAdi_instr->addChild(m_pAdi_slipe_ballMT);

	m_pAdi_sphereMT = new osg::MatrixTransform;
	m_pAdi_sphereMT->addChild(pAdi_sphere);
	pNewAdi_instr->addChild(m_pAdi_sphereMT);

	m_pAdi_turn_indicatorMT = new osg::MatrixTransform;
	m_pAdi_turn_indicatorMT->addChild(pAdi_turn_indicator);
	pNewAdi_instr->addChild(m_pAdi_turn_indicatorMT);

	m_pAdi_yaw_steering_barMT = new osg::MatrixTransform;
	m_pAdi_yaw_steering_barMT->addChild(pAdi_yaw_steering_bar);
	pNewAdi_instr->addChild(m_pAdi_yaw_steering_barMT);

	pNewAdiMeter->addChild(pNewAdi_instr);
	pNewAdiMeter->addChild(pAdi_arrow_knod_ccw);
	pNewAdiMeter->addChild(pAdi_arrow_knod_cw);
	pNewAdiMeter->addChild(pAdi_bkg);
	pNewAdiMeter->addChild(pAdi_knod);
	pNewAdiMeter->addChild(pAdi_mount);

	addChild(pNewAdiMeter);

	return NULL;
}

void CAdiMeter::UpdatePicthAndRoll( double dPitch, double dRoll )
{
	if (dPitch > m_dMaxPitchAngle || dPitch < m_dMinPitchAngle)
	{
		if (dPitch > (m_dMaxPitchAngle+m_dMinPitchAngle)/2.0)
		{
			dPitch = m_dMaxPitchAngle;
		}
		else
		{
			dPitch = m_dMinPitchAngle;
		}
	}
	//中点请查看adi.xml相应配置
	osg::Vec3 center = osg::Vec3( 0.0291,0,  0);
	osg::Vec3 nagetiveCenter = osg::Vec3(-0.0291, 0, 0);
	double dAngle1 = osg::DegreesToRadians(-dPitch);

	if (dRoll > m_dMaxRollAngle || dRoll < m_dMinRollAngle)
	{
		if (dRoll > (m_dMaxRollAngle+m_dMinRollAngle)/2.0)
		{
			dRoll = m_dMaxRollAngle;
		}
		else
		{
			dRoll = m_dMinRollAngle;
		}
	}
	double dAngle2 = osg::DegreesToRadians(dRoll);
	m_pAdi_sphereMT->setMatrix(osg::Matrix::translate(center) 
		* osg::Matrix::rotate(dAngle1, osg::Z_AXIS)
		* osg::Matrix::translate(nagetiveCenter)
		* osg::Matrix::rotate(dAngle2, osg::X_AXIS));

}
void CAdiMeter::UpdateYaw( double dYaw )
{
	if (dYaw > m_dMaxYawAngle || dYaw < m_dMinYawAngle)
	{
		if (dYaw > (m_dMaxYawAngle+m_dMinYawAngle)/2.0)
		{
			dYaw = m_dMaxYawAngle;
		}
		else
		{
			dYaw = m_dMinYawAngle;
		}
	}
#ifndef BASERANGE
#define BASERANGE
#define BASERANGE_ANGLE 180.0
#define BASERANGE_TRANSFORM 0.0180
#endif
	double dTransRange = dYaw / BASERANGE_ANGLE;
	double dTransform = dTransRange * BASERANGE_TRANSFORM;

	m_pAdi_yaw_steering_barMT->setMatrix(osg::Matrix::translate(osg::Vec3(0, 0, dTransform)));
}

void CAdiMeter::UpdatePosture( double dPitch, double dRoll, double dYaw )
{
	UpdatePicthAndRoll(dPitch, dRoll);
	UpdateYaw(dYaw);
}


