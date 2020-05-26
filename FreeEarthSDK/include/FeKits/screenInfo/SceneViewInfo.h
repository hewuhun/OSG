/**************************************************************************************************
* @file SceneViewInfo.h
* @note 场景信息获取，随着鼠标位置以及相机位置的变化，用户可实现自定义功能
* @author c00005
* @data 2017-1-18
**************************************************************************************************/
#ifndef FE_SCENE_VIEW_INFO_H
#define FE_SCENE_VIEW_INFO_H

#include <osgGA/GUIEventHandler>

#include <FeKits/Export.h>
#include <FeKits/screenInfo/InfoMaker.h>

#include <vector>

namespace FeKit
{
	class CSceneViewInfoHandler;

	/**
	* @class CSceneViewInfoCallback
	* @brief 场景信息获取回调类
	* @note 场景信息获取回调类，用户可继承该类，实现当鼠标移动或者相机变化时的操作
	* @author c00005
	*/
	class FEKIT_EXPORT CSceneViewInfoCallback: public osg::Referenced
	{
	public:
		/**  
		* @brief 构造函数
		*/
		CSceneViewInfoCallback()
			:osg::Referenced()
		{

		}

		/**  
		* @brief 析构函数
		*/
		virtual ~CSceneViewInfoCallback()
		{

		}

	public:
		/**  
		* @brief 更新鼠标所在地理位置信息以及当前相机的姿态信息
		* @param dLon [in] 经度
		* @param dLat [in] 纬度
		* @param dHei [in] 高度
		* @param dLon [in] 偏航角度
		* @param dLat [in] 俯仰角度
		* @param dHei [in] 相机的高度
		*/
		virtual void UpdateViewInfo(double dLon, double dLat, double dHei, double dHeading, double dPitch, double dCamHeight) = 0;
	};

	/**
	* @class CSceneViewInfo
	* @brief 场景信息获取
	* @note 场景信息包括鼠标所在的位置以及相机信息的获取功能
	* @author c00005
	*/
	class FEKIT_EXPORT CSceneViewInfo: public osg::Referenced
	{
		friend class CSceneViewInfoHandler;

	public:
		/**  
		* @brief 构造函数
		* @note 场景信息包括鼠标所在的位置以及相机信息的获取功能
		* @param pContext [in] 渲染上下文
		*/
		CSceneViewInfo(FeUtil::CRenderContext* pContext);

		/**  
		* @brief 析构函数
		*/
		~CSceneViewInfo(void);

	public:
		/**  
		* @note 添加场景信息获取回调对象  
		*/
		void AddCallback(CSceneViewInfoCallback* pCallback);

		/**  
		* @note 移除场景信息获取回调对象  
		*/
		void RemoveCallback(CSceneViewInfoCallback* pCallback);

	protected:
		/**
		@note: 更新屏显信息
		*/
		void UpdateInfo(double dMouseX, double dMouseY);

	protected:
		///场景渲染上下文
		osg::observer_ptr<FeUtil::CRenderContext>	m_opRenderContext;

		///场景信息回调类列表
		typedef std::vector<osg::ref_ptr<CSceneViewInfoCallback>> SceneViewInfoCallbacks;		
		SceneViewInfoCallbacks						m_listCallback;

		///场景信息获取的对象
		osg::ref_ptr<CInfoMaker>					m_opInfoMaker;

		///事件回调，用于每帧进行响应
		osg::ref_ptr<CSceneViewInfoHandler>			m_rpHandler;

		///鼠标上一次移动的X位置
		double										m_dPreMouseX;
		///鼠标上一次移动的Y位置
		double										m_dPreMouseY;

		///经度信息
		double										m_dLongitude;
		///纬度信息
		double										m_dLatitude;
		///高度信息
		double										m_dHeight;
	};

	/**
	* @class CSceneViewInfoHandler
	* @brief 时间回调，用于调用获取场景信息
	* @author c00005
	*/
	class CSceneViewInfoHandler : public osgGA::GUIEventHandler
	{
	public:
		/**  
		* @brief 构造函数
		*/
		CSceneViewInfoHandler(CSceneViewInfo* pSInfo);

		/**  
		* @brief 析构函数
		*/
		~CSceneViewInfoHandler();

	public:
		/**  
		* @brief 事件处理
		*/
		virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	protected:
		///场景信息获取的对象
		osg::observer_ptr<CSceneViewInfo>		m_opSceneViewInfo;
	};
}

#endif //FE_SCENE_VIEW_INFO_H
