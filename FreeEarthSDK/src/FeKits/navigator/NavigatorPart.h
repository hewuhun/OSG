/**************************************************************************************************
* @file NavigatorPart.h
* @note 导航器的总组件
* @author c00005
* @data 2017-4-1
**************************************************************************************************/
#ifndef FE_NAVIGATOR_NAVIGATOR_COMPONENT_H
#define FE_NAVIGATOR_NAVIGATOR_COMPONENT_H

#include <FeKits/navigator/BasePart.h>

namespace FeKit
{
	/**
	* @class CRotateComponent
	* @brief 导航器的总组件
	* @note 导航器的总组件，创建了导航器所需的所有组件
	* @author l00008
	*/
	class CNavigatorPart : public CCompositPart
	{
	public:
		/**  
		* @brief 构造函数
		* @note 导航器总组件的构造函数  
		* @param strRes [in] 资源路径
		* @return 返回值
		*/
		CNavigatorPart(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes);

		~CNavigatorPart(void);

	public:
		/**
		*@note: 更新组件需要的值
		*/
		virtual void UpdateValue(double dValue);

		/**
		*@note 判断屏幕坐标点是否在空间的可拣选范围中
		*/
		virtual bool WithinMe( int nX, int nY );

		/**
		*@note 是否为只显示指北针
		*/
		void ShowOnlyCompass(bool bShow);

	protected:
		/**
		*@note 创建指北针部件
		*/
		bool CreateCompassPart();

		/**
		*@note 创建旋转部件
		*/
		bool CreateRotatePart();

		/**
		*@note 创建移动部件
		*/
		bool CreateMovePart();

		/**
		*@note 创建缩放部件
		*/
		bool CreateZoomPart();

	protected:
		/**
		*@note 鼠标移动事件处理
		*/
		virtual bool MoveHandle(int nX, int nY);

	protected:
		///是否只显示指北针
		bool                               m_bOnlyShowCompass;

		///组件
		osg::observer_ptr<CBasePart>       m_opCompass;

		///资源路径
		std::string                        m_strRes;
	};
}

#endif //FE_NAVIGATOR_NAVIGATOR_COMPONENT_H
