/**************************************************************************************************
* @file AoaMeter.h
* @note AOA仪表是飞机座舱中显示俯仰角度的指示器
* @author ty00007
* @data 2014-3-26
**************************************************************************************************/
#ifndef AOAMETER_H
#define AOAMETER_H

#include <osg/MatrixTransform>
#include <osg/Geometry>
#include <osgDB/ReadFile>
#include <iostream>
/**
  * @class CAoaMeter
  * @brief AOA仪表
  * @note AOA仪表是飞机座舱中显示俯仰角度的指示器，
  根据飞机当前的飞行姿态的俯仰角改变仪表显示数值
  * @author ty00007
*/
class CAoaMeter : public osg::MatrixTransform
{
public:
	CAoaMeter(void);
	~CAoaMeter(void);
public:
	/**  
	  * @brief 更新俯仰角
	  * @note 根据传入的俯仰角度改变AOA仪表上显示的数值
	  * @param dPitch [in] 俯仰角
	  * @return void
	*/
	void UpdatePitch( double dPitch );
	/**  
	  * @brief 记录俯仰角
	  * @note 记录AOA仪表俯仰角上显示的数值
	  * @param dPitch [in] 俯仰角
	  * @return void
	*/
	void RecordPitch( double dPitch) { m_dPitch = dPitch; };
	/**  
	  * @brief AOA仪表是否可用
	  * @note AOA仪表是否可用
	  * @param bflag [in] 标示仪表是否使用
	  * @return void
	*/
	void AoaMeterOFF( bool bflag);
protected:
	/**  
	  * @brief 重建仪表
	  * @note 分析仪表内部结构重新组合仪表，试用本类 
	  * @return osg::Group
	*/
	osg::Group* RebuildAoaMeter();
protected:
	//俯仰角指示器标示的几何节点
	osg::Geometry* m_pAoatape;
	//是否可用的标示节点隔板
	osg::Group* m_pAoa_off_flag;
	//俯仰角
	double m_dPitch;
	//仪表能表示的最大范围
	double m_dMaxPitchRange;
	//仪表能表示的最小范围
	double m_dMinPitchRange;
};

#endif