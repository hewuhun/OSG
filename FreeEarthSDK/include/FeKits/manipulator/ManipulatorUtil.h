/**************************************************************************************************
* @file ManipulatorUtil.h
* @note 操作器管理，定义与场景漫游器以及场景相机相关的函数
* @author l00008
* @data 2014-07-07
**************************************************************************************************/
#ifndef MANIPULATOR_UTIL_H
#define MANIPULATOR_UTIL_H

#include <FeKits/Export.h>

#include <osgViewer/View>
#include <osgGA/KeySwitchMatrixManipulator>

#include <FeKits/manipulator/FreeEarthManipulator.h>
#include <FeKits/manipulator/FeLockNodeManipulator.h>

namespace FeKit
{
	const std::string FREE_EARTH_MANIPULATOR_KEY = "FreeEarthManipulatorKey";
	const std::string FIRST_LOCK_MANIPULATOR_KEY = "FirstPersonManipulatorKey";
	const std::string THIRD_LOCK_MANIPULATOR_KEY = "ThirdPersonManipulatorKey";

	/**
	  * @class CManipulatorManager
	  * @brief 操作器管理类
	  * @note 用于管理场景操作器，包括切换、添加操作器，同时还定义了一些常用的与相机相关的函数
	  * @author c00005
	*/
	class FEKIT_EXPORT CManipulatorManager : public osgGA::KeySwitchMatrixManipulator
	{
	public:
		/**  
		  * @note 构造函数  
		  * @param pRenderContext [in] 场景渲染上下文
		*/
		CManipulatorManager(FeUtil::CRenderContext* pRenderContext);

		/**  
		  * @note 析构函数  
		*/
		virtual ~CManipulatorManager();

	public:
		/**  
		  * @brief 设置视点位置为当前视点位置，该方法用于地球操作器
		  * @param viewPoint [in] 视点位置
		  * @param dTime [in] 移动到该视点的时间
		*/
		void Locate(const osgEarth::Viewpoint& viewPoint, double dTime);

		/**  
		  * @brief 获取当前视点位置
		*/
		osgEarth::Viewpoint GetViewPoint();

		/**  
		  * @brief 设置Home视点位置，该方法用于地球操作器
		  * @param viewPoint [in] 视点位置
		  * @param dTime [in] 移动到该视点的时间
		*/
		void SetHome(const osgEarth::Viewpoint& viewPoint, double dTime);

		/**  
		  * @brief 回归到Home视点位置
		*/
		void Home(double dTime = 0.5);

		/**  
		* @brief 将视点切换到正北方向视角
		* @param dTime [in] 移动到该视点的时间
		*/
		void ToNorthViewPoint(double dTime);

		/**  
		* @brief 将当前视点切换到垂直视角
		* @param dTime [in] 移动到该视点的时间
		*/
		void ToVerticalViewPoint(double dTime);
		
		/**  
		* @brief 锁定视点
		* @param bLock [in] 是否锁定视点
		*/
		void LockViewPoint(bool bLock = true);

	public:
		/**  
		* @brief 获取FreeEarth地球操作器
		*/
		FeKit::FreeEarthManipulator* GetOrCreateFEManipulator(bool bActive = true);
		
		/**  
		* @brief 获取第三人称锁定操作器
		*/
		FeKit::FeLockNodeManipulator* GetOrCreateLockManipulator(bool bActive = true);
		
		/**  
		* @brief 获取第一人称的锁定操作器
		*/
		FeKit::FeLockNodeManipulator* GetOrCreateFirstPersonManipulator(bool bActive = true);

		/**  
		  * @note 添加相机操作器以及对应的关键字，关键字为自定义字符串
		  * @param pCameraManp [in] 相机操作器
		  * @param strKey [in] 相机操作器的关键字
		  * @return bool 是否添加成功
		*/
		bool AddCameraManipulator(osgGA::CameraManipulator* pCameraManp, std::string strKey);

		/**  
		  * @note 移除关键字对应的相机操作器
		  * @param strKey [in] 相机操作器的关键字
		  * @return bool 是否移除成功
		*/
		bool RemoveCameraManipulator(std::string strKey);

		/**  
		  * @note 通过相机操作器关键字获取到相机操作器，同时可控制是否激活
		  * @param strKey [in] 相机操作器的关键字
		  * @param bActive [in] 是否激活
		  * @return osgGA::CameraManipulator* 相机操作器
		*/
		osgGA::CameraManipulator* GetCameraManipulatorByKey(std::string strKey, bool bActive = false);

		/**  
		  * @note 激活与关键字对应的相机操作器
		  * @param strKey [in] 相机操作器的关键字
		  * @return bool 是否激活成功
		*/
		bool ActivateCameraManipulator(std::string strKey);

		/**  
		  * @note 判断关键字对应的操作器是否为激活状态
		  * @param strKey [in] 相机操作器的关键字
		  * @return bool 是否激活
		*/
		bool IsActiveCameraManipulator(std::string strKey);

		/**  
		  * @note 设置第三人称操作器的参数
		  * @param dLockDis [in] 锁定距离
		  * @param fDegreeHead [in] 俯仰角
		  * @param fDegreeTilt [in] 偏航角
		*/
		void SetThirdPersonHomeParam(double dLockDis, float fDegreeHead = 0.0, float fDegreeTilt = -80.0);

		/**  
		  * @note 设置切换相机操作器的事件处理器是否可用
		  * @param bEnabled [in] 是否可用
		*/
		void SetHandleEnabled(bool bEnabled);

		/**  
		  * @note 获取当前切换相机操作器的事件处理器是否可用
		*/
		bool GetHandleEnabled();

	protected:
		/**  
		  * @note 通过字符串关键字获取相机操作器的索引以及操作器指针
		  * @param strKey [in] 字符串关键字
		  * @param nKey [inout] 索引
		  * @param pCameraManip [inout] 相机操作器
		  * @return bool 是否获取成功
		*/
		bool GetCameraManipulatorInfo(const std::string& strKey, int& nKey, osgGA::CameraManipulator** pCameraManip);

		/**  
		* @brief 处理切换操作器的事件处理机制
		*/
		virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);

	protected:
		///是否锁定操作器，即禁用操作器
		bool												m_bLocked;

		///切换操作器事件是否被激活
		bool												m_bHandleEnabled;

		///渲染上下文
		osg::observer_ptr<FeUtil::CRenderContext>			m_opRenderContext;

		///相机操作器的关键数字，每个操作器对应不同的关键字
		int													m_nCameraManipulatorIndex;

		osg::ref_ptr<CCameraAltCallback>					m_rpCameraAltCallback;

		osg::ref_ptr<CEarthClampProjectionMatrixCallback>	m_rpClampProjCallback;
	};

} 


#endif //MANIPULATOR_UTIL_H
