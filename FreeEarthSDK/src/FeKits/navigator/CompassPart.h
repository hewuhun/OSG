/**************************************************************************************************
* @file CompassComponent.h
* @note 指北针组件，主要由表针和表盘组成
* @author l00008
* @data 2014-02-20
**************************************************************************************************/
#ifndef FE_NAVIGATOR_COMPASS_COMPONENT_H
#define FE_NAVIGATOR_COMPASS_COMPONENT_H

#include <osg/Geode>

#include <FeKits/navigator/BasePart.h>

namespace FeKit
{
	class CCompassPlate;
	class CCompassNeedle;

	/**
	* @class CCompassComponent
	* @brief 指北针组件
	* @note 指北针组件由表针和表盘组成并提供了场景更新的接口
	* @author l00008
	*/
	class CCompassPart : public CCompositPart
	{
	public:
		/**
		* @brief 指北针组件构造函数
		* @param strRes [in] 组件的资源路径
		*/
		CCompassPart(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes);

		~CCompassPart(void);

	public:
		/**
		*@note: 更新组件的值
		*/
		void UpdateValue(double dValue);

		/**
		*@note 设置组件对应的处理控制器，每个空间都保存一个处理器的引用用来更新场景
		*/
		virtual void SetCtrl(CNavigatorCtrl* pCtrl);

	protected:
		/**
		*@note 判断屏幕坐标点是否在空间的可拣选范围中
		*/
		virtual bool WithinMe(int nX, int nY);

	protected:
		///指北针组件的表盘
		osg::observer_ptr<CCompassPlate>          m_opPlate;

		///指北针组件的指针
		osg::observer_ptr<CCompassNeedle>         m_opNeedle;

		///指北针的角度
		double                                    m_dNorthDegree;
	};

	/**
	* @class CCompassPlate
	* @brief 指北针表盘
	* @note 提供了表盘，可以贴图
	* @author l00008
	*/
	class CCompassPlate : public CLeafPart
	{
	public:
		/**
		* @brief 指北针组件构造函数
		* @param strRes [in] 组件的资源路径
		*/
		CCompassPlate(
			double dLeft, 
			double dRight, 
			double dBottom, 
			double dTop, 
			const std::string& strRes);

		~CCompassPlate(void){};

	protected:
		/**
		*@note 判断屏幕坐标点是否在空间的可拣选范围中
		*/
		virtual bool WithinMe(int nX, int nY);
	};

	/**
	* @class CCompassNeedle
	* @brief 指北针表针
	* @note 提供了指北针的表针并提供了操作接口
	* @author l00008
	*/
	class CCompassNeedle : public CLeafPart
	{
	public:
		/**
		* @brief 指北针表针组件构造函数
		* @param strRes [in] 组件的资源路径
		*/
		CCompassNeedle(
			CCompassPart* pParent,
			double dLeft, 
			double dRight, 
			double dBottom, 
			double dTop, 
			const std::string& strRes);

		~CCompassNeedle(void){};

	public:
		/**
		*@note: 更新组件的值
		*/
		virtual void UpdateValue(double dValue);

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
		*@note: 获得北方偏转角度
		*/
		double GetDeltaNorthDegree(int nX, int nY);
		
		/**
		*@note: 设置当前的北方角度，更新场景
		*/
		void SetCurrentDirection(double dValue);

	protected:
		///北方角度值
		double									m_dNorthDegree;

		///上一次的X位置值
		int										m_nPrePosX;

		///上一次的Y位置值
		int										m_nPrePosY;

		///组件的父节点
		osg::observer_ptr<CCompassPart>         m_opParent;
	};
}
#endif //FE_NAVIGATOR_COMPASS_COMPONENT_H
