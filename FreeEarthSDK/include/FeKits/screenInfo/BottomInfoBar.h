/**************************************************************************************************
* @file BottomInfoBar.h
* @note 场景下方显示信息框
* @author c00005
* @data 2017-4-1
**************************************************************************************************/
#ifndef FE_BOTTOM_INFOMATION_BAR_H
#define FE_BOTTOM_INFOMATION_BAR_H 1

#include <osg/NodeVisitor>
#include <osgText/Text>
#include <osgViewer/View>

#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIActionAdapter>

#include <osgEarth/MapNode>

#include <FeKits/ScreenKits.h>
#include <FeKits/screenInfo/InfoMaker.h>

#include <FeKits/Export.h>

namespace FeKit
{
	/**
	* @class CBottomInfoBar
	* @brief 渲染窗口底层的显示框
	* @note 主要显示鼠标的信息、视点的信息等。
	* @author c00005
	*/
	class FEKIT_EXPORT CBottomInfoBar : public CScreenKits
	{
	public:
		/**  
		* @brief 构造一个HUD相机的父类
		* @note 通过指定位置以及相对于其位置的偏移值
		* @param nXOffset [in] X偏移
		* @param nYOffset [in] Y偏移
		* @param nWidth [in] 宽度
		* @param nHeight [in] 高度
		* @param ePlace [in] 位置
		* @return 返回值
		*/
		CBottomInfoBar(
			FeUtil::CRenderContext* pContext,
			int nXOffset, 
			int nYOffset, 
			int nWidth, 
			int nHeight, 
			CPlaceStrategy::KIT_PLACE ePlace);

		/**
		@note: 更新屏显信息
		*/
		void UpdateInfo(double dMouseX, double dMouseY);

	public:
		/**
		*@note: 显示屏幕信息显示，除了将其NodeMask设置为0x1之外还添加处理事件handler
		*/
		virtual void Show();

		/**
		*@note: 隐藏屏幕信息显示，除了将其NodeMask设置为0x0之外还移除处理事件handler
		*/
		virtual void Hide();

		/**
		*@Note: 设置计算方法
		*/
		void SetInfoMaker(unsigned unKey);

	protected:
		~CBottomInfoBar(void);

		/**  
		* @note 创建屏显信息显示框  
		*/
		osg::Geode* CreateSIUI();

		/**
		*@note: 更新相机信息，如果相机的信息没有变化就无需修改
		*/
		void UpdateCameraInfo();

		/**
		*@note: 更新鼠标的信息，如果鼠标的位置没有改变就无需修改
		*/
		void UpdateMouseInfo(double dMouseX, double dMouseY);

		/**
		*@note: 遍历，用于更新信息
		*/
		virtual void traverse(osg::NodeVisitor& nv);

	protected:
		///文本显示
		osg::ref_ptr<osgText::Text>             m_bottomInfoText;

		///鼠标以及相机信息获取器
		osg::ref_ptr<CInfoMaker>                m_opInfoMaker;

		///场景View
		osg::observer_ptr<osgViewer::View>      m_opView;

		///场景地球节点
		osg::observer_ptr<osgEarth::MapNode>    m_opMapNode;

		///旧Pitch数据
		double                                  m_dPrePitch; 
		///旧Range数据
		double                                  m_dPreRange; 
		///旧Heading数据
		double                                  m_dPreHeading; 
		///旧鼠标X位置
		double                                  m_dPreMouseX;
		///旧鼠标Y位置
		double                                  m_dPreMouseY;
		///旧鼠标高度信息
		double                                  m_dPreMouseHei;
	};
}

#endif //FE_SCREENINFO_H
