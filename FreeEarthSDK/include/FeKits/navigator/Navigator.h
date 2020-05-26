/**************************************************************************************************
* @file CFENavigator.h
* @note 导航器功能代码
* @author l00008
* @data 2014-02-20
**************************************************************************************************/

#ifndef  FE_NAVIGATOR_FE_NAVIGATOR_H
#define FE_NAVIGATOR_FE_NAVIGATOR_H

#include <osgViewer/View>

#include <FeKits/Export.h>

#include <FeKits/ScreenKits.h>

namespace FeKit
{
	class CNavigatorPart;
	class CNavigatorCtrl;

	/**
	* @class CFENavigator
	* @brief 导航器
	* @note 导航器主要是一个HUD相机，相机下包含了需要显示的组件，本类提供了相机的创建、
	*       以及组件的创建过程。并提供了外界可操作的接口。
	* @author l00008
	*/
	class FEKIT_EXPORT CNavigator : public CScreenKits
	{
	public:
		/**
		* @brief 创建导航器
		* @note 通过提供导航器的起始屏幕坐标、宽度以及起始位置偏移来创建导航器
		* @param pView [in]（添加导航器的View）
		* @param pNavigatorCtrl [in]（控制器，用来控制场景）
		* @param nXOffset [in] X偏移
		* @param nYOffset [in] Y偏移
		* @param nWidth [in] 宽度
		* @param nHeight [in] 高度
		* @param ePlace [in] 位置
		*/
		CNavigator(
			FeUtil::CRenderContext* pContext,
			int nXOffset, 
			int nYOffset, 
			int nWidth, 
			int nHeight, 
			CPlaceStrategy::KIT_PLACE ePlace,
			CNavigatorCtrl* pNavigatorCtrl,
			const std::string& strRes = "");

		/**
		*@note: 设置相机的位置和宽高
		*/
		virtual void SetPosition(int nX, int nY, int nWidth, int nHeight);

		/**
		*@note: 显示导航器
		*/
		virtual void Show();

		/**
		*@note: 隐藏导航器
		*/
		virtual void Hide();

	protected:
		~CNavigator(void);

	protected:
		///控制器，和操作器相关联
		osg::ref_ptr<CNavigatorCtrl>            m_rpCtrl;   

		///导航器的组件
		osg::observer_ptr<CNavigatorPart>       m_opNVPart;

		///场景的View
		osg::observer_ptr<osgViewer::View>      m_opView;
	};
}

#endif //FE_NAVIGATOR_FE_NAVIGATOR_H
