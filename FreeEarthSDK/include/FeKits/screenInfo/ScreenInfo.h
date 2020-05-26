
#ifndef FE_SCREENINFO_H
#define FE_SCREENINFO_H

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
    class CScreenInfoHandler;
    /**
    * @class CScreenInfo
    * @brief 屏幕信息显示
    * @author c00005
    */
    class FEKIT_EXPORT CScreenInfo : public CScreenKits
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
        CScreenInfo(
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
        ~CScreenInfo(void);

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

    protected:
        osg::ref_ptr<osgText::Text>             m_textLon;
        osg::ref_ptr<osgText::Text>             m_textLat;
        osg::ref_ptr<osgText::Text>             m_textHei;
        osg::ref_ptr<osgText::Text>             m_textHeading;
        osg::ref_ptr<osgText::Text>             m_textPitch;
        osg::ref_ptr<osgText::Text>             m_textRange;

		osg::ref_ptr<CinfoMaker>                m_opInfoMaker;
        osg::ref_ptr<CScreenInfoHandler>        m_rpHandler;

        double                                  m_dPrePitch; //旧Pitch数据
        double                                  m_dPreRange; //旧Range数据
        double                                  m_dPreHeading; //旧Heading数据

        double                                  m_dPreMouseX;
        double                                  m_dPreMouseY;
        double                                  m_dPreMouseHei;

        
    };

    class CScreenInfoHandler : public osgGA::GUIEventHandler
    {
    public:
        CScreenInfoHandler(CScreenInfo* pSInfo);

        virtual bool handle(
            const osgGA::GUIEventAdapter& ea,
            osgGA::GUIActionAdapter& aa);

    protected:
        ~CScreenInfoHandler();

    protected:
        osg::observer_ptr<CScreenInfo>           m_opSInfo;
    };



}

#endif //FE_SCREENINFO_H
