/**************************************************************************************************
* @file ZoomComponent.h
* @note 缩放组件的定义
* @author l00008
* @data 2014-02-20
**************************************************************************************************/
#ifndef FE_NAVIGATOR_ZOOM_COMPONENT_H
#define FE_NAVIGATOR_ZOOM_COMPONENT_H

#include <FeKits/navigator/BasePart.h>

namespace FeKit
{
	class CZoomPole;
	class CZoomInButton;
	class CZoomOutButton;
	class CZoomSlidBlock;

	/**
	* @class CZoomPart
	* @brief 缩放组件
	* @note 缩放组件由放大、缩小等一系列组件构成，在本类中提供了创建一系列组件的接口
	* @author l00008
	*/
	class CZoomPart: public CCompositPart
	{
	public:
		/**
		* @brief 缩放组件构造函数
		* @param strRes [in] 组件的资源路径
		*/
		CZoomPart(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes);

		~CZoomPart(void);
	};

	/**
	* @class CZoomEdge
	* @brief 缩放组件的边缘
	* @author c00005
	*/
	class CZoomEdge : public CLeafPart
	{
	public:
		/**
		* @brief 缩放组件边缘的构造函数
		* @param strRes [in] 组件的资源路径
		*/
		CZoomEdge(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes);

		virtual ~CZoomEdge(){};
	};

	/**
	* @class CZoomPole
	* @brief 滑竿组件
	* @author c00005
	*/
	class CZoomPole: public CLeafPart
	{
	public:
		/**
		* @brief 滑竿组件的构造函数
		* @param strRes [in] 组件的资源路径
		*/
		CZoomPole(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes);

		~CZoomPole(void){};

	public:
		/**
		*@note 判断屏幕坐标点是否在空间的可拣选范围中
		*/
		virtual bool WithinMe(int nX, int nY);
	};

	/**
	* @class CZoomInButton
	* @brief 放大组件
	* @author c00005
	*/
	class CZoomInButton: public CLeafPart
	{
	public:
		/**
		* @brief 放大组件构造函数
		* @param strRes [in] 组件的资源路径
		*/
		CZoomInButton(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes);

		~CZoomInButton(void){};

	protected:
		/**
		*@note: 处理鼠标按下事件
		*/
		virtual bool PushHandle(int nX, int nY);

		/**
		*@note: 处理鼠标释放事件
		*/
		virtual bool ReleaseHandle(int nX, int nY);

		/**
		*@note: 处理鼠标双击事件
		*/
		virtual bool DoubleClickHandle(int nX, int nY);
	};

	/**
	* @class CZoomOutButton
	* @brief 缩小组件
	* @author c00005
	*/
	class CZoomOutButton: public CLeafPart
	{
	public:
		/**
		* @brief 缩小组件构造函数
		* @param strRes [in] 组件的资源路径
		*/
		CZoomOutButton(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes);

		~CZoomOutButton(void){};

	protected:
		/**
		*@note: 处理鼠标按下事件
		*/
		virtual bool PushHandle(int nX, int nY);

		/**
		*@note: 处理鼠标释放事件
		*/
		virtual bool ReleaseHandle(int nX, int nY);

		/**
		*@note: 处理鼠标双击事件
		*/
		virtual bool DoubleClickHandle(int nX, int nY);
	};

	/**
	* @class CZoomSlidBlock
	* @brief 滑块组件
	* @author c00005
	*/
	class CZoomSlidBlock: public CLeafPart
	{
	public:
		///缩放状态
		enum EZoomState
		{
			///正常
			NOON,       
			///放大
			ZOOM_IN,
			///缩小
			ZOOM_OUT
		};

	public:
		/**
		* @brief 滑块组件构造函数
		* @param strRes [in] 组件的资源路径
		*/
		CZoomSlidBlock(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes);

		~CZoomSlidBlock(void){};

	public:
		/**
		* @brief 设置滑块的缩放范围
		*/
		void SetRange(double dRangeRate);
		
		/**
		* @brief 更新位置
		*/
		virtual void UpdatePos(int nX, int nY, int nWidth, int nHeight, double dWR, double dHR);

	protected:
		/**
		* @brief 获取Delta值
		*/
		void GetDeltaValue(int nX, int nY, EZoomState& eState, double& dRate);
		
		/**
		* @brief 设置滑块当前的位置
		*/
		void SetCurrentPos();

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

	protected:
		///滑块的状态
		EZoomState                                 m_eZoomState;    

		///滑块的范围比例和导航器屏幕位置一起确定滑块的范围
		double                                     m_dRangeRate;    

		//滑块的范围
		int                                        m_nRange;       
		
		//滑块滑动的速度，0-1
		double                                     m_dRate;         
	};
}

#endif //FE_NAVIGATOR_ZOOM_COMPONENT_H


