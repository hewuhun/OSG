/**************************************************************************************************
* @file AirView.h
* @note 鸟瞰图功能实现类
* @author l00008
* @data 2014-04-29
**************************************************************************************************/
#ifndef FE_AIR_VIEW_H
#define FE_AIR_VIEW_H

#include <osg/NodeVisitor>

#include <osgViewer/View>

#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIActionAdapter>

#include <osgEarth/MapNode>

#include <FeKits/ScreenKits.h>
#include <FeKits/airview/AirViewKits.h>
#include <FeKits/Export.h>
#include <FeKits/manipulator/ManipulatorUtil.h>

namespace FeKit
{
	class CLocateCallback;
    /**
    * @class CAirView
    * @brief 鸟瞰图
    * @note 提供一个鸟瞰图的功能类，用于全局观察场景当前的相机位置和视点位置
    * @author l00008
    */
    class FEKIT_EXPORT CAirView : public CScreenKits
    {
    public:
        /**
        * @brief 创建鸟瞰图
        * @note 通过提供鸟瞰图的起始屏幕坐标、宽度以及起始位置偏移来创建鸟瞰图
        * @param pView [in]（添加鸟瞰图的View）
        * @param nXOffset [in] X偏移
        * @param nYOffset [in] Y偏移
        * @param nWidth [in] 宽度
        * @param nHeight [in] 高度
        * @param ePlace [in] 位置
        */
        CAirView(
			FeUtil::CRenderContext* pContext,
            int nXOffset, 
            int nYOffset, 
            int nWidth, 
            int nHeight, 
            CPlaceStrategy::KIT_PLACE ePlace, 
            const std::string& strDataPath = "");

		~CAirView(void);

	public:
		/**
		*@note: 添加一个标记,标记有圆环、矩形、三角形
		*/
		bool AddAirViewMark(CAirViewMark* pMark);

		/**
		*@note: 删除一个标记,标记有圆环、矩形、三角形
		*/
		bool RemoveAirViewMark(CAirViewMark* pMark);

		/**
		*@note: 清空标记，删除所有的标记
		*/
		bool ClearAirViewMarks();

    protected:
		/**
		*@note: 遍历，内部启用了事件处理器
		*/
        virtual void traverse(osg::NodeVisitor& nv);

		/**
		*@note: 事件处理器，用于处理鼠标事件
		*/
		virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

		/**
		*@note: 双击事件的响应
		*/
        virtual bool DoubleClick(osgViewer::View* pView, unsigned unX, unsigned unY);

		/**
		*@note: 创建鸟瞰图
		* @param strDataPath [in] 鸟瞰图所需的资源路径
		*/
        void CreateAirView(const std::string& strDataPath);

    protected:
		///是否显示
		bool										m_bShow;
		
		///定位飞行时间
		float										m_fLocateTime;    
		
		///最小的定位高度
		double										m_dMinDistance;   

		///鸟瞰图中的定位框
        osg::observer_ptr<CAirViewFocus>            m_opFocus;
		
		//鸟瞰图的主平面
        osg::observer_ptr<CAirViewGround>           m_opGround;

		///存放标记的组节点
		osg::observer_ptr<osg::Group>               m_opMarkGroup;

    };
}

#endif //FE_AIR_VIEW_H
