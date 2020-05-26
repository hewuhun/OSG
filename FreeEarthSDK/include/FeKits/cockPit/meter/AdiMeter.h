/**************************************************************************************************
* @file AdiMeter.h
* @note 飞行姿态仪表，包含偏航角、俯仰角、横滚角的显示
* @author ty00007
* @data 2014-3-25
**************************************************************************************************/
#ifndef ADIMETER_H
#define ADIMETER_H

#include <osg/MatrixTransform>
#include <osg/Geometry>

#include <iostream>
/**
  * @class 
  * @brief 飞行姿态仪表
  * @note 姿态表包括：包含偏航角、俯仰角、横滚角的显示
  * @author ty00007
*/
class CAdiMeter : public osg::MatrixTransform
{
public:
	CAdiMeter(void);
	~CAdiMeter(void);
	/**  
    * @note 设置设备的飞行姿态，主要是俯仰、滚转、偏航
    * @param dPitch[in] 俯仰信息
    * @param dRoll[in] 滚转信息
    * @param dYaw[in] 偏航信息
	* @param dVelocity[in] 速度信息
    */
	void UpdatePosture(double dPitch, double dRoll, double dYaw);

protected:
	/**  
	  * @brief 构建仪表
	  * @note 拆分仪表部件重新组合 
	  * @return osg::Group*
	*/
	osg::Group* RebuildAdiMeter();

protected:
	/**  
	  * @brief 改变仪表显示的俯仰、横滚角
	  * @note 根据传入的俯仰、横滚角改变仪表指针显示的数值
	  * @param dPitch [in] 俯仰角
	  * @param dRoll[in] 横滚角
	  * @return void
	*/
	void UpdatePicthAndRoll(double dPitch, double dRoll);
	/**  
	  * @brief 改变仪表显示的偏航角
	  * @note 根据传入的偏航角改变仪表指针显示的数值
	  * @param dYaw [in] 偏航角
	  * @return void
	*/
	void UpdateYaw(double dYaw);

protected:
	//Aux指示灯
	osg::Group* m_pAdi_aux_flag;
	//gs指示灯
	osg::Group* m_pAdi_gs_flag; 
	//loc指示灯
	osg::Group* m_pAdi_loc_flag; 
	//off指示灯
	osg::Group* m_pAdi_off_flag;
	//
	osg::MatrixTransform* m_pAdi_bank_indexMT;
	//ADI坡度指示器
	osg::MatrixTransform* m_pAdi_glide_slope_indexMT;
	//ADI俯仰指示器
	osg::MatrixTransform* m_pAdi_pitch_steering_barMT;
	//ADI平衡滚球
	osg::MatrixTransform* m_pAdi_slipe_ballMT;
	//ADI俯仰、横滚角指示球
	osg::MatrixTransform* m_pAdi_sphereMT;
	//ADI转向指示
	osg::MatrixTransform* m_pAdi_turn_indicatorMT;
	//偏航指示器
	osg::MatrixTransform* m_pAdi_yaw_steering_barMT;
	//俯仰角能表示最大、小角度
	double m_dMaxPitchAngle;
	double m_dMinPitchAngle;
	//横滚角能表示的最大、小值
	double m_dMaxRollAngle;
	double m_dMinRollAngle;
	//偏航角表示范围
	double m_dMaxYawAngle;
	double m_dMinYawAngle;

};

#endif