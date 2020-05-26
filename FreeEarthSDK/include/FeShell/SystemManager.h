/**************************************************************************************************
* @file SystemManager.h
* @note 三维渲染管理器，引导管理接口，用于管理三维的渲染初始化等工作
* @author c00005
* @data 2017-4-1
**************************************************************************************************/
#ifndef FE_SYSTEM_MANAGER_H
#define FE_SYSTEM_MANAGER_H

#include <osg/Referenced>
#include <osg/MatrixTransform>
#include <osgViewer/Viewer>
#include <osgViewer/CompositeViewer>
#include <OpenThreads/Mutex>

#include <FeShell/SystemSerializer.h>
#include <FeShell/SystemService.h>

namespace FeShell
{
	/**
	* @class CSystemManager
	* @brief 引导管理接口
	* @note 引导管理接口，定义了系统的启动和关闭系统接口
	* @author l00008
	*/
	class FESHELL_EXPORT CSystemManager : public osg::Referenced
	{
		friend struct MyUpdateCallback;

	public:
		/**  
		  * @brief 构造函数
		  * @param strResorcePath [in] 三维场景资源路径
		*/
		CSystemManager( const std::string& strResorcePath );

		virtual ~CSystemManager(void);

	public:
		/**
		*@note: 初始化渲染系统，指定系统默认的配置资源路径
		*/
		virtual bool Initialize( );

		/**
		*@note: 反初始化场景，将场景中的要素归位，并完成销毁
		*/
		virtual bool UnInitialize();

	public:
		/**
		*@note: 获得系统服务接口，通过此接口可以添加系统服务模块
		*/
		FeShell::CSystemService* GetSystemService();

		/**
        *@note: 是否打开按“ESC”退出系统
        */
		virtual void SetESCState(bool bKey);

		/**
		* @note 获得主视景器
		*/
		virtual osgViewer::Viewer *GetViewer();
		
		/**
		* @note 获得复杂视景器
		*/
		virtual osgViewer::CompositeViewer *GetCompositeViewer();

		/**
		*@note: 设置渲染系统需要渲染的图形上下文
		*/
		virtual bool SetGraphicContext(osg::GraphicsContext* pContext);

		/**
		*@note: 窗口大小改变事件分发
		*/
		virtual void ResizeGraphicsContext(int x,int y,int w,int h);

		/**
		*@note: 获取渲染上下文接口
		*/
		FeUtil::CRenderContext* GetRenderContext();

		/**
		*@note: 设置用户earth文件的接口，优先使用该路径
		*/
		void SetEarthPath(std::string strPath);

	protected:
		/**
		*@note: 预加载系统，加载系统的配置文件
		*/
		virtual bool PreLoadSystemConfig( );

		/**
		*@note: 初始化相机信息，设置相机的基础属性并获得渲染窗口
		*/
		virtual bool InitCameraSetting();

		/**
		*@note: 初始化场景视口信息，根据配置文件初始化地球场景、场景元素系统等
		*/
		virtual bool InitViewerScene();

		/**
		*@note: 初始化事件回调系统以及漫游器，在此可以添加缺省的功能回调
		*/
		virtual bool InitEventHandles();

		/**
		*@note: 初始化服务
		*/
		virtual bool InitService();

		/**
		*@note: 初始化完成之后需要个性化的收尾工作
		*/
		virtual bool InitEnd() = 0;

		/**
		*@note: 初始化完成之后需要个性化开始的准备工作;
		*/
		virtual bool InitBegin() = 0;

	public:
		/**
		*@note: 一帧开始前的操作，此函数在一帧开始之前调用，有系统自动调用，用户不能够自行调用
		*/
		virtual void PreFrame();

		/**
		*@note: 一帧结束之后的操作，此函数在一帧结束之后调用，有系统自动调用，用户不能够自行调用
		*/
		virtual void PostFrame();
		
		/**
		*@note: 开始场景的渲染事件
		*/
		virtual void Frame();
	
	protected:
		///是否为第一帧
		bool												m_bFirstFrame;				

		///系统的资源路径
		std::string											m_strSystemResoucePath;		

		///系统配置文件信息
		CSysConfig											m_systemConfig;				

		///渲染的主视口，用于渲染主场景
		osg::ref_ptr<osgViewer::Viewer>						m_rpViewer;					
		
		///渲染的多视口，用于管理多个渲染视口，默认会自动生成一个主视口
		osg::ref_ptr<osgViewer::CompositeViewer>			m_rpCompositeViewer;		

		///系统模块服务接口，主要提供给系统外部
		osg::ref_ptr<CSystemService>						m_rpSystemService;			

		///渲染服务接口，主要正对系统内部
		osg::ref_ptr<FeUtil::CRenderContext>				m_rpRenderContext;			      

		///用户earth文件路径
		std::string											m_strEarthPath;

		OpenThreads::Mutex									m_mutex;

		double												m_dMinFrameTime;
	};

}

#endif//FE_SYSTEM_MANAGER_H
