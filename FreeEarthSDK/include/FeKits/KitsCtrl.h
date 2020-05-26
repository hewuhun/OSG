/**************************************************************************************************
* @file KitsCtrl.h
* @note 小工具的外部访问接口，定义了所有的工具挂件必须实现的接口，如果实现了这些接口
*       在外部直接的控制其显隐和获得其显示名称和标识名称
* @author l00008
* @data 2014-03-13
**************************************************************************************************/

#ifndef FE_KITS_CTRL_H
#define FE_KITS_CTRL_H

#include <string>
#include <FeKits/Export.h>
#include <FeUtils/RenderContext.h>

namespace FeKit
{
    /**
    * @class CKitsCtrl
    * @brief 挂件控制接口
    * @note 挂件控制接口
    * @author l00008
    */
    class FEKIT_EXPORT CKitsCtrl : public FeUtil::CRenderContextObserver
    {
    public:
       CKitsCtrl(FeUtil::CRenderContext* pContext);

       virtual ~CKitsCtrl();

    public:
        /**
        *@note: 显示工具
        */
        virtual void Show() = 0;

        /**
        *@note: 隐藏工具
        */
        virtual void Hide() = 0;

        /**
        *@note: 判断工具是否处于隐藏状态
        */
        virtual bool IsHide() const;

        /**
        *@note: 获得显示在界面上的功能的名称，这个名称不能作为工具的唯一标示符
        */
        virtual std::string GetTitle() const;

        /**
        *@note: 设置显示在界面上的功能的名称，这个名称不能作为工具的唯一标示符
        */
        virtual void SetTitle(const std::string& strTitle);
        
        /**
        *@note: 获得唯一标示工具的字符串
        */
        virtual std::string GetKey() const;

        /**
        *@note: 设置唯一标示工具的字符串
        */
        virtual void SetKey(const std::string& strKey);

        /**
        *@note: 重新设置窗口的位置大小,如果有显示界面的工具需要重新这个函数
        */
        virtual void ResizeKit(int nW, int nH);

    public:
		///工具的显隐
        bool					m_bShow;        

		///工具的唯一标示符
        std::string				m_strKey;       
		
		///工具的显示名称
        std::string				m_strTitle;    
    };
}

#endif //FE_KITS_CTRL_H
