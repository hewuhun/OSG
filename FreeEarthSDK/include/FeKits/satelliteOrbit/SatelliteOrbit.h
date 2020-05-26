/**************************************************************************************************
* @file CSatelliteOrbit.h
* @note 定义了卫星轨道平面
* @author l00008
* @data 2014-8-26
**************************************************************************************************/

#ifndef HG_SATELLITE_ORBIT_H
#define HG_SATELLITE_ORBIT_H

#include <osg/Group>

#include <FeKits/Export.h>

#include <FeKits/KitsCtrl.h>

namespace FeKit
{
    /**
      * @class CSynSatelliteOrbit
      * @brief 同步卫星轨道
      * @note 同步卫星轨道
      * @author l00008
    */

    class FEKIT_EXPORT CSynSatelliteOrbit : public osg::Group, public CKitsCtrl
    {
    public:
        CSynSatelliteOrbit(FeUtil::CRenderContext* pContext, const osg::Vec4d& vecColor);

    public:
        /**
        *@note: 显隐控制
        */
        virtual void Show();

        virtual void Hide();

    protected:
        virtual ~CSynSatelliteOrbit(void);

    protected:
        double          m_dOrbitRadius; //地球同步卫星轨道半径
        osg::Vec4d      m_vecColor;  //轨道颜色
    };

}
#endif //HG_SATELLITE_ORBIT_H
