/**************************************************************************************************
* @file EarthShowKits.h
* @note 定义了所有的地球特效的控制器
* @author l00008
* @data 2014-06-10
**************************************************************************************************/

#ifndef HG_EARTH_SHOW_VIEW_KIT_CAMERA_H
#define HG_EARTH_SHOW_VIEW_KIT_CAMERA_H

#include <FeKits/Export.h>
#include <FeKits/KitsCtrl.h>

namespace FeKit
{
    class FEKIT_EXPORT CEarthShowKits : public CKitsCtrl
    {
    public:
        CEarthShowKits();

        virtual ~CEarthShowKits();

    public:
        /**
        *@note: 显隐控制
        */
        virtual void Show();
        virtual void Hide();
        virtual bool IsHide() const;

        /**
        *@note: 改变大小，此函数在此空实现
        */
        virtual void ResizeKit(int nW, int nH);
    };

}

#endif //HG_EARTH_SHOW_VIEW_KIT_CAMERA_H
