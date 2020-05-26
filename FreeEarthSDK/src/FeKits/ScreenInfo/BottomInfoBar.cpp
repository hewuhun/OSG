#include <iostream>
#include <sstream>
#include <iomanip>
#include <osgGA/EventVisitor>
#include <osg/LineWidth>
#include <osgDB/ConvertUTF>
#include <osgEarth/GeoMath>

//#include <FeUtils/FreeEarthManipulator.h>
#include <osgEarthUtil/EarthManipulator>

#include <FeKits/screenInfo/BottomInfoBar.h>
#include <FeUtils/MathUtil.h>
#include <FeUtils/StrUtil.h>
#include <FeKits/manipulator/ManipulatorUtil.h>
#include <FeUtils/UtilityGeom.h>


namespace FeKit
{

    CBottomInfoBar::CBottomInfoBar( 
		FeUtil::CRenderContext* pContext,
        int nXOffset, 
        int nYOffset, 
        int nWidth, 
        int nHeight, 
        CPlaceStrategy::KIT_PLACE ePlace)
        :CScreenKits(pContext, nXOffset, nYOffset, nWidth, nHeight, ePlace)
        ,m_bottomInfoText(NULL)
        ,m_dPreHeading(0.0)
        ,m_dPrePitch(0.0)
        ,m_dPreRange(0.0)

        ,m_dPreMouseX(0.0)
        ,m_dPreMouseY(0.0)
        ,m_dPreMouseHei(0.0)
    {
		setNumChildrenRequiringEventTraversal(getNumChildrenRequiringEventTraversal() + 1);

		osg::Vec4d vecBColor(0.0, 0.0, 0.0, 0.3);
		double dWidth = m_dRightRate - m_dLeftRate;
		double dHeight = m_dTopRate - m_dBottomRate;
		osg::Vec3d vecCorner(m_dLeftRate, m_dBottomRate, -1.0);

		osg::Geode* pGeode = 
			FeUtil::CreateBasePlate("", vecCorner, dWidth, dHeight, osg::Vec4d(0.0, 0.0, 0.0, 0.2));

		addChild(pGeode);

        addChild(CreateSIUI());

		m_opInfoMaker = new CCommonInfo(m_opRenderContext.get());
    }


    CBottomInfoBar::~CBottomInfoBar(void)
    {

    }

    osg::Geode* CBottomInfoBar::CreateSIUI()
    {
        osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;

        //simfang.ttf msyh.ttf
        osg::ref_ptr<osgText::Font> pFont = osgText::readFontFile("fonts/simhei.ttf");

        m_bottomInfoText = new osgText::Text;
        m_bottomInfoText->setFont(pFont.get());
       // m_bottomInfoText->setFontResolution(128.0f,128.0f);
        m_bottomInfoText->setColor(osg::Vec4d(1,1,1,1.0));
        m_bottomInfoText->setCharacterSize(m_dHeightRate * 0.6);
        m_bottomInfoText->setDataVariance(osg::Object::DYNAMIC);
        m_bottomInfoText->setAlignment(osgText::Text::RIGHT_CENTER);//设置对其方式
    //    m_bottomInfoText->setPosition(osg::Vec3(m_dRightRate, m_dBottomRate, 1.0));
        m_bottomInfoText->setPosition(osg::Vec3(m_dRightRate, 0, 0.0));
        m_bottomInfoText->setBackdropColor(osg::Vec4(0,0,0,1));
        m_bottomInfoText->setBackdropImplementation(osgText::Text::DELAYED_DEPTH_WRITES );
        m_bottomInfoText->setBackdropType(osgText::Text::OUTLINE);
        m_bottomInfoText->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
        m_bottomInfoText->getOrCreateStateSet()->setRenderingHint(osg::StateSet::OPAQUE_BIN);
        pGeode->addDrawable(m_bottomInfoText.get());

        return pGeode.release();
    }

    void CBottomInfoBar::UpdateInfo(double dMouseX, double dMouseY)
    {
		if(!m_opInfoMaker.valid()) return;

		bool bChanged = false; //判断是否有数值改变

		double dHeading=0, dPitch=0, dRange=0;
		m_opInfoMaker->GetPosture(dPitch, dHeading, dRange);

		if(!FeMath::Equal(dRange, m_dPreRange)||
		   !FeMath::Equal(m_dPreMouseX, dMouseX) ||
		   !FeMath::Equal(m_dPreMouseY, dMouseY))
		{
			m_dPreHeading = dHeading;
			m_dPrePitch = dPitch;
			m_dPreRange = dRange;
			m_dPreMouseX = dMouseX;
			m_dPreMouseY = dMouseY;
			bChanged = true;
		}

		if(bChanged)
		{
			double dLon(0.0), dLat(0.0), dHei(0.0);
			m_opInfoMaker->GetLLH(m_dPreMouseX, m_dPreMouseY, dLon, dLat, dHei);

			std::stringstream os;
            os.setf(std::ios::fixed);
			if(dLon < 0.0)
			{
				os<<"经度 "<<std::setprecision(4)<<-dLon<<"°W, ";
			}
			else
			{
				os<<"经度 "<<std::setprecision(4)<<dLon<<"°E, ";
			}

			if(dLat < 0.0)
			{
				os<<" 纬度 "<<std::setprecision(4)<<-dLat<<"°S, ";
			}
			else
			{
				os<<" 纬度 "<<std::setprecision(4)<<dLat<<"°N, ";
			}

			if(dRange < 10000000.0)
			{
				os<<" 海拔高度 "<<std::setprecision(4)<<dHei<<"米,  ";
			}
			
			//os<<"ViewPointHei: ";
			//os<<"Camera Pitch: "<<dPitch<<"°,"<<"Heading: "<<dHeading<<"°,";

			if(dRange > 1000.0 || dRange < -1000.0)
			{
				os<<"视角海拔高度 "<<std::setprecision(1)<<(dRange / 1000.0)<<"千米";
			}
			else
			{
				os<<"视角海拔高度 "<<std::setprecision(4)<<dRange<<"米";
			}

			m_bottomInfoText->setText(FeUtil::ToUTF(os.str()), osgText::String::ENCODING_UTF8);
		}
    }

    void CBottomInfoBar::Show()
    {
        CScreenKits::Show();
    }

    void CBottomInfoBar::Hide()
    {
        CScreenKits::Hide();
    }

	void CBottomInfoBar::SetInfoMaker( unsigned unKey )
	{
		if(unKey == 0)
		{
			m_opInfoMaker = new CCommonInfo(m_opRenderContext.get());
		}
		else
		{
			m_opInfoMaker = new CArmyInfo(m_opRenderContext.get());
		}
	}

	void CBottomInfoBar::traverse( osg::NodeVisitor& nv )
	{
		if (nv.getVisitorType()==osg::NodeVisitor::EVENT_VISITOR)
		{
			osgGA::EventVisitor* ev = dynamic_cast<osgGA::EventVisitor*>(&nv);
			if (ev)
			{
				for(osgGA::EventQueue::Events::iterator itr = ev->getEvents().begin();
					itr != ev->getEvents().end();
					++itr)
				{
					osgGA::GUIEventAdapter* ea = itr->get()->asGUIEventAdapter();
					UpdateInfo(ea->getX(), ea->getY());
				}
			}
		}

		CScreenKits::traverse(nv);
	}
}


