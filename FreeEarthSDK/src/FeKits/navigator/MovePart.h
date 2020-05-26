/**************************************************************************************************
* @file MoveComponent.h
* @note 移动场景的组件
* @author l00008
* @data 2014-02-20
**************************************************************************************************/
#ifndef FE_NAVIGATOR_MOVE_COMPONENT_H
#define FE_NAVIGATOR_MOVE_COMPONENT_H

#include <FeKits/navigator/BasePart.h>

namespace FeKit
{
	/**
	* @class CMoveComponent
	* @brief 移动场景组件
	* @note 定义可响应消息的移动场景组件
	* @author l00008
	*/
	class CMovePart : public CCompositPart
	{
	public:
		/**
		* @brief 移动场景组件构造函数
		* @param strRes [in] 组件的资源路径
		*/
		CMovePart(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes);

		~CMovePart(void);
	};

	/**
	* @class CMoveEdge
	* @brief 移动场景组件的边缘
	* @author c00005
	*/
	class CMoveEdge : public CLeafPart
	{
	public:
		/**
		* @brief 移动场景组件构造函数
		* @param strRes [in] 组件的资源路径
		*/
		CMoveEdge(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes);

		virtual ~CMoveEdge();

	protected:
		/**
		*@note 判断屏幕坐标点是否在空间的可拣选范围中
		*/
		virtual bool WithinMe(int nX, int nY);
	};

	/**
	* @class CMovePanel
	* @brief 旋转场景的组件
	* @note 定义可响应消息的旋转场景组件
	* @author l00008
	*/
	class CMovePanel : public CLeafPart
	{
	public:
		/**
		* @brief 移动场景组件构造函数
		* @param strRes [in] 组件的资源路径
		*/
		CMovePanel(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes);

		~CMovePanel(void);

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
		virtual bool DoubleClickHandle( int nX, int nY );

		/**
		*@note: 通过当前鼠标屏幕的坐标获得方位角和偏转角
		*/
		void GetDirction(int nX, int nY, double& dX, double& dY);

		/**
		*@note: 更新移动的位置
		*/
		void UpdateMove(int nX, int nY);

	protected:
		///旋转的比例，0-1
		double                          m_dRate;  

		///拖动的X坐标
		double                          m_dX;

		///拖动的Y坐标
		double                          m_dY;
	};
}

#endif //FE_NAVIGATOR_MOVE_COMPONENT_H
