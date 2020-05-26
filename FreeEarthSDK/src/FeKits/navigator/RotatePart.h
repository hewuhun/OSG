/**************************************************************************************************
* @file RotateComponent.h
* @note 移动旋转的定义
* @author l00008
* @data 2014-02-20
**************************************************************************************************/
#ifndef FE_NAVIGATOR_TOTATE_COMPONENT_H
#define FE_NAVIGATOR_TOTATE_COMPONENT_H

#include <FeKits/navigator/BasePart.h>

namespace FeKit
{
	/**
	* @class CRotatePart
	* @brief 旋转场景的组件
	* @note 定义可响应消息的旋转场景组件
	* @author l00008
	*/
	class CRotatePart : public CCompositPart
	{
	public:
		/**
		* @brief 旋转场景的组件构造函数
		* @param strRes [in] 组件的资源路径
		*/
		CRotatePart(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes);

		virtual ~CRotatePart(void){};
	};

	/**
	* @class CRotateEdge
	* @brief 旋转场景组件的边缘
	* @author c00005
	*/
	class CRotateEdge : public CLeafPart
	{
	public:
		/**
		* @brief 旋转场景组件的边缘构造函数
		* @param strRes [in] 组件的资源路径
		*/
		CRotateEdge(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes);

		virtual ~CRotateEdge(){};

	protected:
		/**
		*@note 判断屏幕坐标点是否在空间的可拣选范围中
		*/
		virtual bool WithinMe(int nX, int nY);
	};

	/**
	* @class CRotatePanel
	* @brief 旋转场景的组件
	* @note 定义可响应消息的旋转场景组件
	* @author l00008
	*/
	class CRotatePanel : public CLeafPart
	{
	public:
		/**
		* @brief 旋转场景的组件构造函数
		* @param strRes [in] 组件的资源路径
		*/
		CRotatePanel(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes);

		~CRotatePanel(void);

	protected:
		/**
		*@note: 处理鼠标按下事件
		*/
		virtual bool PushHandle(int nX, int nY);

		/**
		*@note: 处理鼠标拖拽事件
		*/
		virtual bool DragHandle(int nX, int nY);

		/**
		*@note: 处理鼠标释放事件
		*/
		virtual bool ReleaseHandle(int nX, int nY);

		/**
		*@note: 处理鼠标双击事件
		*/
		virtual bool DoubleClickHandle(int nX, int nY);

		/**
		*@note: 通过当前鼠标屏幕的坐标获得方位角和偏转角
		*/
		void GetDirction(int nX, int nY, double& dAzimuth, double& dPitch);

		/**
		*@note: 更新旋转角度
		*/
		bool UpdateRotate(int nX, int nY);

	protected:
		///旋转的比例，0-1
		double                          m_dRate;  

		///旋转方位角
		double                          m_dAzimuth;

		///旋转俯仰角
		double                          m_dPitch;
	};
}
#endif //FE_NAVIGATOR_TOTATE_COMPONENT_H
