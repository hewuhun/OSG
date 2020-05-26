/**************************************************************************************************
* @file CCockPitShell.h
* @note 定义了驱动座舱仪表的shell，这些shell是公用的接口来操纵座舱。
* @author l00008
* @data 2013-03-16
**************************************************************************************************/

#ifndef HG_COCKPIT_H
#define HG_COCKPIT_H

#include <string>

#include <osg/Referenced>

#include <FeKits/Export.h>

namespace FeKit
{

/**
  * @class CCockPit
  * @brief 座舱操纵器
  * @note 定义了操纵通用座舱的接口，其中定义了座舱需要显示的信息的接口，座舱的设计需要满足
  *       这几个接口定义的功能，表的数据映射需要根据不同的飞行设备自定义显示。
  * @author l00008
*/


class FEKIT_EXPORT CCockPit : public osg::Referenced
{
public:
    CCockPit();
// 
//     typedef enum
//     {
//         STOP = 0, //停机状态
//         RUN,      //正常的飞行状态
//         SHOT,       //被击中
//     };

public:
    /**  
    * @note 显示和隐藏座舱
    */
    virtual void ShowCockPit(bool bShow) = 0;

    /**  
    * @note 是否显示座舱
    */
    virtual bool IsHidden() const;

public:
    /**  
    * @note 设置需要显示座舱所属设备的名称和其他的信息，信息不宜过长。 
    * @param strTip[in] 显示的具体信息
    */
    virtual void UpdateTip( const std::string& strTip ) = 0;


    /**
    * @note 设置飞机位置，使用经度、维度、高度信息
    * @param dLon[in] 经度信息
    * @param dLat[in] 纬度信息
    * @param dHeight[in] 高度信息
    */
    virtual void UpdatePosition( double dLon, double dlat, double dHeight ) = 0;

    /**  
    * @note 设置设备的飞行航向、速度、航程
    * @param dHeading[in] 飞行航向
    * @param dVelocity[in] 飞行速度
    * @param dMile[in] 飞行航程
    */
    virtual void UpdateHsiMeter( double dHeading, double dVelocity , double dMile) = 0;

	 /**  
    * @note 设置设备的飞行姿态，主要是俯仰、滚转、偏航
    * @param dPitch[in] 俯仰信息
    * @param dRoll[in] 滚转信息
    * @param dYaw[in] 偏航信息
    */
    virtual void UpdatePosture( double dPitch, double dRoll, double dYaw, double dVelocity ) = 0;

    /**  
    * @note 设置显示飞机的油量
    * @param dValue[in] 油量
    */
    virtual void UpdateOilMass(double dValue) = 0;

    /**  
    * @note 设置显示飞机气温表
    * @param dValue[in] 气温
    */
    virtual void UpdateTemperature(double dValue) = 0;

    /**
    * @note 设置飞机是否被击中，即是否处于安全状态
    */
    virtual void UpdateSafeState(bool bState) = 0;


protected:
    virtual ~CCockPit();

protected:
    bool                m_bSHow;
};

}

#endif //HG_COCKPIT_H
