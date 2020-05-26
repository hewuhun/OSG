/**************************************************************************************************
* @file SystemService.h
* @note 系统服务类，主要提供系统功能模块管理，以及一些常用功能接口
* @author c00005
* @data 2017-2-15
**************************************************************************************************/
#ifndef FESHELL_SYSTEM_SERVICE_H
#define FESHELL_SYSTEM_SERVICE_H

#include <FeUtils/RenderContext.h>
#include <FeUtils/FreeViewPoint.h>
#include <FeUtils/SceneSysCall.h>

#include <FeKits/manipulator/ManipulatorUtil.h>

#include <FeShell/Export.h>
#include <FeShell/SystemSerializer.h>
#include <FeShell/EnvironmentSys.h>
#include <FeShell/SmartToolSys.h>

namespace FeShell
{
	/**
	* @class CSystemService
	* @brief 系统服务类
	* @note 定义了系统服务，主要提供系统功能模块管理，以及一些常用功能接口
	* @author l00003
	*/
	class FESHELL_EXPORT CSystemService : public osg::Referenced
	{
	public:
		/**  
		  * @note 系统服务类无参数的构造函数  
		*/
		CSystemService( );

		/**  
		  * @note 析构函数  
		*/
		virtual ~CSystemService(void);

	public:
		/**  
		  * @brief 初始化服务
		  * @note 系统启动后调用初始化服务，其中需要传入服务需要的参数以及启动默认的服务  
		  * @param pView [in] 视口
		  * @param pRoot [in] root节点
		  * @param pMapNode [in] 地球节点
		  * @param sysConfig [in] 系统配置
		  * @return 返回值
		*/
		bool InitService(FeUtil::CRenderContext* pRenderContext, const CSysConfig& sysConfig);

		/**
		*@note:获得三维渲染环境配置，主要保存了三维的基础对象
		*/
		FeUtil::CRenderContext* GetRenderContext(){return m_opRenderContext.get();};

		/**
		*@note: 获得系统配置信息，其中可以获得系统的名称、所属公司、其他配置信息
		*/
		virtual const CSysConfig& GetSysConfig() const;
		
		/**
		*@note: 设置系统配置信息
		*/
		virtual void SetSysConifig(const CSysConfig& sysConfig);

	public:
		/**  
		* @brief 定位
		* @note 根据提供的参数来定位位置  
		* @param strValue [in] 经纬高字符串，中间使用“，”隔开；
		* @return 返回值
		*/
		bool Locate( const std::string& strValue);

		/**  
		* @brief 定位
		* @note 根据提供的参数来定位位置  
		* @param sViewPoint [in] 视口
		* @param fTime [in] 定位时间
		* @param bSetHome [in] 是否设置此视点为Home视点
		* @return 返回值
		*/
		bool Locate( const FeUtil::CFreeViewPoint& sViewPoint, bool bSetHome = false);

		/**  
		* @brief home定位
		* @note 使用配置文件中的经纬高以及旋转信息来定位到地球的某一点  
		* @return 返回值
		*/
		bool Home();

		/**  
		* @brief 获得视口
		* @note 获得当前视口  
		* @return 返回值，视口
		*/
		FeUtil::CFreeViewPoint GetViewPoint();

		///获得漫游器管理器
		FeKit::CManipulatorManager* GetManipulatorManager();

		/**  
		* @brief 注册和移除事件处理Handler
		* @note 统一管理注册和移除处理事件的Handler
		* @return 成功true，失败false
		*/
		bool AddEventHandler(osgGA::GUIEventHandler* pHandler);
		bool RemoveEventHandler(osgGA::GUIEventHandler* pHandler);

		/**  
		* @brief 保存系统配置
		* @note 将系统配置保存到默认的位置 
		* @return 成功返返回true，否则返回false
		*/
		bool Save();

		/**  
		* @brief 保存系统配置到指定的路径
		* @note 指定位置，将系统配置保存
		* @return 成功返返回true，否则返回false
		*/
		bool SaveAs();
		
		/**  
		* @brief 获取立体显示效果
		*/
		bool GetStereo() const;
		
		/**  
		* @brief 设置立体显示效果
		*/
		void SetStereo(bool bState);
		
		/**  
		* @brief 获取渲染窗口的宽度
		*/
		int GetRenderWidth() const;
		
		/**  
		* @brief 获取渲染窗口的高度
		*/
		int GetRenderHeight() const;

	public:
		/**
		*@note: 获得预加载的环境管理
		*/
		CEnvironmentSys* GetEnvironmentSys();

		/**
		*@note: 获得预加载的视图挂件管理
		*/
		CSmartToolSys* GetSmartToolSys();

		/**
		*@note: 添加一个系统模块到平台中
		*/
		bool AddAppModuleSys(FeUtil::CSceneSysCall* pSysCall);

		/**
		*@note: 移除一个应用模块从平台中
		*/
		bool RemoveModuleSys(FeUtil::CSceneSysCall* pSysCall);

		/**
		*@note: 移除所有的系统调用,此接口只能在系统结束时调用
		*/
		bool RemoveAllModule();

		/**
		*@note: 获得系统模块
		*/
		FeUtil::CSceneSysCall* GetModuleSys(const std::string& strTitle);

		/**
		*@note: 窗口大小改变事件分发
		*/
		virtual void ResizeGraphicsContext(int x,int y,int w,int h);

	protected:
		/**
		*@note: 初始化系统预定义的模块系统
		*/
		virtual bool InitSysModule();

	protected:
		///系统的配置信息对象
		CSysConfig										m_sysConfig; 

		///管理视图挂件
		osg::observer_ptr<CEnvironmentSys>				m_opEnvironmentSys;
		
		///管理视图挂件
		osg::observer_ptr<CSmartToolSys>				m_opSmartToolSys;  

		///渲染图形上下文
		osg::observer_ptr<FeUtil::CRenderContext>		m_opRenderContext;

		///应用系统的列表，其中应用系统使用其文件名称字符串作为唯一的标示符
		typedef std::map< std::string, osg::ref_ptr<FeUtil::CSceneSysCall> >  VecSysCall;
		
		///业务模块列表
		VecSysCall										m_mapSysCall;    
	};
}

#endif//FESHELL_SYSTEM_SERVICE_H
