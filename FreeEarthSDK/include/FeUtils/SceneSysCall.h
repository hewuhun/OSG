/**************************************************************************************************
* @file SceneSysCall.h
* @note 定义了组成系统的模块的接口，这些模块通过使用共同的接口注册到SHell层中，可以获得
*       场景的主节点、事件处理系统、渲染系统等一系列的资源并互相可以交互。
* @author 00008
* @data 
**************************************************************************************************/

#ifndef HG_SCENE_SYSTEM_CALL_H
#define HG_SCENE_SYSTEM_CALL_H

#include <osg/observer_ptr>
#include <osg/Group>

#include <FeUtils/Export.h>
#include <FeUtils/RenderContext.h>

namespace FeUtil
{
	const std::string SYSTEM_CALL_DEFAULT_KEY = "SceneSystemCall";

    /**
    * @class CSceneSysCall
    * @brief 系统调用
    * @note 组成渲染系统的子系统
    * @author 00008
    */
    class FEUTIL_EXPORT CSceneSysCall : public osg::Referenced
    {
    public:
        CSceneSysCall();

    public:
        /**
        *@note: 初始化系统
        */
        virtual bool Initialize(FeUtil::CRenderContext* pRenderContext);
		
        /**
        *@note: 反初始化系统
        */
        virtual bool UnInitialize();

        /**
        *@note: 开始系统
        */
		virtual bool Start(){return true;}
		
        /**
        *@note: 结束系统
        */
        virtual bool Stop(){return true;}

        /**
        *@note: 获得功能对应的Group节点
        */
        virtual osg::Group* GetSysGroup();

        /**
        *@note: 设置模块的名称，名称可以作为唯一的标识使用
        */
        virtual void SetSysKey(const std::string& strKey);

        /**
        *@note: 获得模块的名称，名称可以作为唯一的标识使用
        */
        virtual std::string GetSysKey() const;
		
        /**
        *@note: 改变系统大小
        */
		virtual void ResizeSys(int nW, int nH){};

    protected:
        virtual ~CSceneSysCall();

		/**  
		  * @brief 设置初始化状态
		*/
		void SetInit( bool bState );
		
		/**  
		  * @brief 是否已经初始化
		*/
		bool IsInit() const;
		
		/**  
		  * @brief 继承自此类的系统调用模块需要实现的函数，此函数通过Initializ最后调用
		*/
		virtual bool InitiliazeImplement() = 0;
		
		/**  
		  * @brief 继承自此类的系统调用模块需要实现的函数，此函数通过UnInitializ最先调用
		*/
		virtual bool UnInitiliazeImplement() = 0;

    protected:
		///渲染环境服务接口
        osg::observer_ptr<FeUtil::CRenderContext>  m_opRenderContext; 
		///系统模块的主Root
		osg::observer_ptr<osg::Group>   m_opSystemCallRoot;  

		///唯一标识系统的字符串
        std::string                     m_strKey; 
		///是否初始化完成
        bool                            m_bInitlized; 
    };
}

#endif//HG_SCENE_SYSTEM_CALL_H
