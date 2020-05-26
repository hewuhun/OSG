/**************************************************************************************************
* @file NavigatorCtrlBase.h
* @note 导航器控制器的基类，主要定义了基类的接口
* @author l00008
* @data 2014-02-20
**************************************************************************************************/
#ifndef FE_NAVIGATOR_NAVIGATOR_CTRL_BASE_H
#define FE_NAVIGATOR_NAVIGATOR_CTRL_BASE_H

#include <osgGA/GUIEventHandler>

#include <FeKits/Export.h>

namespace FeKit
{
	class CBasePart;

	/**
	* @class CNavigatorCtrlBase
	* @brief 导航器的控制器基类
	* @note 导航器需要控制场景，控制器作为中间的类实现了场景和导航器之间的桥接，主要提供了
	*       控制场景的接口
	* @author l00008
	*/
	class FEKIT_EXPORT CNavigatorCtrl : public osgGA::GUIEventHandler
	{
	public:
		CNavigatorCtrl()
			:osgGA::GUIEventHandler()
		{
		};

		virtual ~CNavigatorCtrl(void){};

	public:
		/**
		* @brief 更新北方方向
		* @note 提供了更新场景中的北方方向接口，其中参数是弧度
		* @param dValue [in]（北方的方向，从导航器传入的北方方向是以弧度传入并且值的范围是0-360之间，
		其中角度的起始位置是正北（正Y轴）逆时针计算）
		*/
		virtual void UpdateHeading(double dValue) = 0;

		/**
		*@note: 停止更新北方方向
		*/
		virtual void StopUpdateHeading() = 0;

		/**
		* @brief 更新pan值
		* @note 提供了更新场景中的pan的接口
		* @param dX [in]（X方向的位置差值，0-1之间的坐标）
		* @param dX [in]（Y方向的位置差值，0-1之间的坐标）
		*/
		virtual void UpdatePan(double dX, double dY) = 0;

		/**
		*@note: 停止更新位置
		*/
		virtual void StopUpdatePan() = 0;

		/**
		* @brief 更新场景的旋转
		* @note 提供了场景旋转的接口
		* @param dAzimuth [in]（方位度差值，0-1之间的坐标）
		* @param dPitch [in]（偏转角差值，0-1之间的坐标）
		*/
		virtual void UpdateRotate(double dAzimuth, double dPitch) = 0;

		/**
		*@note: 停止更新旋转
		*/
		virtual void StopUpdateRotate() = 0;

		/**
		* @brief 更新缩放
		* @note 提供了场景缩放的接口
		* @param dRate [in]（缩放比例，正放大，负缩小）
		*/
		virtual void UpdateZoom(double dRate) = 0;

		/**
		*@note: 停止更新缩放
		*/
		virtual void StopUpdateZoom() = 0;

		/**
		*@note: 添加交互的指北针引用，用来通过场景更新导航器
		*/
		virtual void AddActiveComponent(CBasePart* pComponent){};
	};

}

#endif //FE_NAVIGATOR_NAVIGATOR_CTRL_BASE_H

