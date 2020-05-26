/**************************************************************************************************
* @file CGeneralCockPit.h
* @note 定义了公用的座舱，本通用的座舱是屏幕座舱，提供了基本的飞行设备姿态和位置显示。
* @author l00008
* @data 2013-03-16
**************************************************************************************************/

#ifndef HG_GENERAL_COCKPIT_H
#define HG_GENERAL_COCKPIT_H

//#include <utils/HgUtil/UtilityCamera.h>

#include <osg/Camera>

#include <FeKits/cockPit/meter/MeterManager.h>
#include <FeKits/cockPit/CockPit.h>

#include <FeKits/Export.h>


namespace FeKit
{

/**
  * @class CGeneralCockPit
  * @brief 基本座舱
  * @note 基本的座舱没有嵌入到飞行器设备中，只是一个HUD的显示设备，适用于初级的通用飞行设备。
  * @author l00008
*/


class FEKIT_EXPORT CGeneralCockPit : public CCockPit, public osg::Camera
{
public:
    CGeneralCockPit(int nPosX, int nPosY, int nWidth, int nHeight, const std::string& strRSPath);

public:
    /**  
    * @note 显示和隐藏座舱
    */
    void ShowCockPit(bool bShow);

public:
    /**  
    * @note 设置需要显示座舱所属设备的名称和其他的信息，信息不宜过长。 
    * @param strTip[in] 显示的具体信息
    */
    virtual void UpdateTip( const std::string& strTip );


    /**
    * @note 设置飞机位置，使用经度、维度、高度信息
    * @param dLon[in] 经度信息
    * @param dLat[in] 纬度信息
    * @param dHeight[in] 高度信息
    */
    virtual void UpdatePosition( double dLon, double dlat, double dHeight);

    /**  
    * @note 设置设备的飞行姿态，主要是俯仰、滚转、偏航
    * @param dPitch[in] 俯仰信息
    * @param dRoll[in] 滚转信息
    * @param dYaw[in] 偏航信息
	* @param dVelocity[in] 速度信息
    */
    virtual void UpdatePosture( double dPitch, double dRoll, double dYaw, double dVelocity = 0 );
	
	/**  
    * @note 设置设备的飞行航向、速度、航程
    * @param dHeading[in] 飞行航向
    * @param dVelocity[in] 飞行速度
    * @param dMile[in] 飞行航程
    */
	virtual void UpdateHsiMeter( double dHeading, double dVelocity = 1000, double dMile = 0);

    /**  
    * @note 设置显示飞机的油量
    * @param dValue[in] 油量
    */
    virtual void UpdateOilMass(double dValue);

    /**  
    * @note 设置显示飞机气温表
    * @param dValue[in] 气温
    */
    virtual void UpdateTemperature(double dValue);

    /**
    * @note 设置飞机是否被击中，即是否处于安全状态
    */
    virtual void UpdateSafeState(bool bState);

	

protected:
    virtual ~CGeneralCockPit();

    void InitCockPit(int nPosX, int nPosY, int nWidth, int nHeight);
	
	void UpdateVelocity(double dVelocity);

	void UpdateClimbRate(double dClimbRate);

	void UpdateHeading(double dHeading);

	void UpdateMiles(double dMile);

	bool BuildMeter();

	void AddChildNodeList(const vecMeterAttributeList MeterAttributeList, const setLoadMeterID loadMeterID);

protected:
    osg::Vec4                       m_vecBackColor;
	CAltiMeter*                     m_Altimeter;
	CTimeMeter*                     m_TimeMeter;
	CVsiMeter*                      m_VsiMeter;
	CHsiMeter*                      m_HsiMeter;
	CAdiMeter*                      m_AdiMeter;
	CAoaMeter*                      m_AoaMeter;
	CMeterManager*                  m_MeterManager;

    std::string                     m_strRSPath; //资源的父路径
};

}

#endif //HG_COCKPIT_SHELL_H
