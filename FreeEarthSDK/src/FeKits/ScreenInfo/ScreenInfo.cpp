#include <iostream>
#include <sstream>
#include <iomanip>
#include <osgGA/EventVisitor>
#include <osg/LineWidth>
#include <osgDB/ConvertUTF>
#include <osgEarth/GeoMath>
#include <osgEarthUtil/EarthManipulator>

#include <FeKits/screenInfo/ScreenInfo.h>
#include <FeKits/manipulator/ManipulatorUtil.h>
#include <FeUtils/MathUtil.h>

namespace FeKit
{

    CScreenInfo::CScreenInfo( 
		FeUtil::CRenderContext* pContext,
        int nXOffset, 
        int nYOffset, 
        int nWidth, 
        int nHeight, 
        CPlaceStrategy::KIT_PLACE ePlace)
        :CScreenKits(pContext, nXOffset, nYOffset, nWidth, nHeight, ePlace)
        ,m_textLon(NULL)
        ,m_textLat(NULL)
        ,m_textHei(NULL)
        ,m_textHeading(NULL)
        ,m_textPitch(NULL)
        ,m_textRange(NULL)
        ,m_rpHandler(NULL)
        ,m_dPreHeading(0.0)
        ,m_dPrePitch(0.0)
        ,m_dPreRange(0.0)

        ,m_dPreMouseX(0.0)
        ,m_dPreMouseY(0.0)
        ,m_dPreMouseHei(0.0)
    {
        addChild(CreateSIUI());
		m_opInfoMaker = new CCommonInfo(m_opRenderContext.get());
        m_rpHandler = new CScreenInfoHandler(this);

        if(!IsHide() && m_opRenderContext.valid())
        {
            m_opRenderContext->GetView()->addEventHandler(m_rpHandler.get());
        }

    }


    CScreenInfo::~CScreenInfo(void)
    {

    }


    osg::Geode* CScreenInfo::CreateSIUI()
    {
        osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;

        //simfang.ttf msyh.ttf
        osg::ref_ptr<osgText::Font> pFont = osgText::readFontFile("simfang.ttf");

        m_textLon = new osgText::Text;
		m_textLon->setText("0.00000");
        m_textLon->setFont(pFont.get());
        m_textLon->setFontResolution(64.0f,64.0f);
        m_textLon->setColor(osg::Vec4d(1,1,1,1.0));
        m_textLon->setCharacterSize(m_dRightRate * 0.095);
        m_textLon->setDataVariance(osg::Object::DYNAMIC);
        m_textLon->setAlignment(osgText::Text::LEFT_BOTTOM);//设置对其方式
        m_textLon->setPosition(osg::Vec3(m_dLeftRate*0.8, m_dTopRate*0.8, 0.0));
        m_textLon->setText(osgDB::convertStringFromCurrentCodePageToUTF8("经 度:"), osgText::String::ENCODING_UTF8);


        m_textLat = new osgText::Text;
		m_textLat->setText("0.00000");
        m_textLat->setFont(pFont.get());
        m_textLat->setFontResolution(64.0f,64.0f);
        m_textLat->setColor(osg::Vec4d(1,1,1,1.0));
        m_textLat->setCharacterSize(m_dRightRate * 0.095);
        m_textLat->setDataVariance(osg::Object::DYNAMIC);
        m_textLat->setAlignment(osgText::Text::LEFT_BOTTOM);//设置对其方式
        m_textLat->setPosition(osg::Vec3(m_dLeftRate*0.8, m_dTopRate*0.7, 0.0));
        m_textLat->setText(osgDB::convertStringFromCurrentCodePageToUTF8("纬 度:"), osgText::String::ENCODING_UTF8);


        m_textHei = new osgText::Text;
		m_textHei->setText("0.00000");
        m_textHei->setFont(pFont.get());
        m_textHei->setFontResolution(64.0f,64.0f);
        m_textHei->setColor(osg::Vec4d(1,1,1,1.0));
        m_textHei->setCharacterSize(m_dRightRate * 0.095);
        m_textHei->setDataVariance(osg::Object::DYNAMIC);
        m_textHei->setAlignment(osgText::Text::LEFT_BOTTOM);//设置对其方式
        m_textHei->setPosition(osg::Vec3(m_dLeftRate*0.8, m_dTopRate*0.6, 0.0));
        m_textHei->setText(osgDB::convertStringFromCurrentCodePageToUTF8("高 度:"), osgText::String::ENCODING_UTF8);


        m_textRange = new osgText::Text;
		m_textRange->setText("0.00000");
        m_textRange->setFont(pFont.get());
        m_textRange->setFontResolution(64.0f,64.0f);
        m_textRange->setColor(osg::Vec4d(1,1,1,1.0));
        m_textRange->setCharacterSize(m_dRightRate * 0.095);
        m_textRange->setDataVariance(osg::Object::DYNAMIC);
        m_textRange->setAlignment(osgText::Text::LEFT_BOTTOM);//设置对其方式
        m_textRange->setPosition(osg::Vec3(m_dLeftRate*0.8, m_dTopRate*0.5, 0.0));
        m_textRange->setText(osgDB::convertStringFromCurrentCodePageToUTF8("视点高:"), osgText::String::ENCODING_UTF8);


        m_textHeading = new osgText::Text;
		m_textHeading->setText("0.00000");
        m_textHeading->setFont(pFont.get());
        m_textHeading->setFontResolution(64.0f,64.0f);
        m_textHeading->setColor(osg::Vec4d(1,1,1,1.0));
        m_textHeading->setCharacterSize(m_dRightRate * 0.095);
        m_textHeading->setDataVariance(osg::Object::DYNAMIC);
        m_textHeading->setAlignment(osgText::Text::LEFT_BOTTOM);//设置对其方式
        m_textHeading->setPosition(osg::Vec3(m_dLeftRate*0.8, m_dTopRate*0.4, 0.0));
        m_textHeading->setText(osgDB::convertStringFromCurrentCodePageToUTF8("俯仰角:"), osgText::String::ENCODING_UTF8);


        m_textPitch = new osgText::Text;
		m_textPitch->setText("0.00000");
        m_textPitch->setFont(pFont.get());
        m_textPitch->setFontResolution(64.0f,64.0f);
        m_textPitch->setColor(osg::Vec4d(1,1,1,1.0));
        m_textPitch->setCharacterSize(m_dRightRate * 0.095);
        m_textPitch->setDataVariance(osg::Object::DYNAMIC);
        m_textPitch->setAlignment(osgText::Text::LEFT_BOTTOM);//设置对其方式
        m_textPitch->setPosition(osg::Vec3(m_dLeftRate*0.8, m_dTopRate*0.3, 0.0));
        m_textPitch->setText(osgDB::convertStringFromCurrentCodePageToUTF8("横滚角:"), osgText::String::ENCODING_UTF8);


        pGeode->addDrawable(m_textLon.get());
        pGeode->addDrawable(m_textLat.get());
        pGeode->addDrawable(m_textHei.get());
        pGeode->addDrawable(m_textHeading.get());
        pGeode->addDrawable(m_textPitch.get());
        pGeode->addDrawable(m_textRange.get());

        return pGeode.release();
    }

    void CScreenInfo::UpdateInfo(double dMouseX, double dMouseY)
    {
        UpdateCameraInfo();

        UpdateMouseInfo(dMouseX, dMouseY);
    }

    void CScreenInfo::UpdateCameraInfo()
    {
		if(!m_opInfoMaker.valid()) return;

		double dHeading=0, dPitch=0, dRange=0;
		m_opInfoMaker->GetPosture(dPitch, dHeading, dRange);

		if(!FeMath::Equal(dHeading, m_dPreHeading) ||
			!FeMath::Equal(dPitch, m_dPrePitch) ||
			!FeMath::Equal(dRange, m_dPreRange))
		{
			m_dPreHeading = dHeading;
			m_dPrePitch = dPitch;
			m_dPreRange = dRange;

			char cTemp[100];
			std::sprintf(cTemp, "视点高: %.3f", dRange);
			std::string strRange = cTemp;
			m_textRange->setText(
				osgDB::convertStringFromCurrentCodePageToUTF8(strRange), 
				osgText::String::ENCODING_UTF8);

			std::sprintf(cTemp, "俯仰角: %.6f°", dHeading);
			std::string strHeading = cTemp;
			m_textHeading->setText(
				osgDB::convertStringFromCurrentCodePageToUTF8(strHeading), 
				osgText::String::ENCODING_UTF8);

			std::sprintf(cTemp, "横滚角: %.6f°", dPitch);
			std::string strPitch = cTemp;
			m_textPitch->setText(
				osgDB::convertStringFromCurrentCodePageToUTF8(strPitch), 
				osgText::String::ENCODING_UTF8);
		}
    }

    void CScreenInfo::UpdateMouseInfo(double dMouseX, double dMouseY)
    {
		if(!m_opInfoMaker.valid()) return;

		if(!FeMath::Equal(m_dPreMouseX, dMouseX) ||
			!FeMath::Equal(m_dPreMouseY, dMouseY))
		{
			m_dPreMouseX = dMouseX;
			m_dPreMouseY = dMouseY;
#if 0
			double dLat=0, dLon=0, dHei=0;
			m_opInfoMaker->GetLLH(m_dPreMouseX, m_dPreMouseY, dLon, dLat, dHei);

			char cTemp[100];
			std::sprintf(cTemp, "经  度: %.6f°", dLon);
			std::string strLon = cTemp;
			m_textLon->setText(
				osgDB::convertStringFromCurrentCodePageToUTF8(strLon), 
				osgText::String::ENCODING_UTF8);

			std::sprintf(cTemp, "纬  度: %0.6f°", dLat);
			std::string strLat = cTemp;
			m_textLat->setText(
				osgDB::convertStringFromCurrentCodePageToUTF8(strLat), 
				osgText::String::ENCODING_UTF8);

			std::sprintf(cTemp, "海  拔: %.3f", dHei);
			std::string strHei = cTemp;
			m_textHei->setText(
				osgDB::convertStringFromCurrentCodePageToUTF8(strHei), 
				osgText::String::ENCODING_UTF8);

#else
			std::string strLon("");
			std::string strLat("");
			std::string strHei("");
			m_opInfoMaker->GetLLH(m_dPreMouseX, m_dPreMouseY, strLon, strLat, strHei);

			m_textLon->setText(
				osgDB::convertStringFromCurrentCodePageToUTF8(strLon), 
				osgText::String::ENCODING_UTF8);

			m_textLat->setText(
				osgDB::convertStringFromCurrentCodePageToUTF8(strLat), 
				osgText::String::ENCODING_UTF8);

			m_textHei->setText(
				osgDB::convertStringFromCurrentCodePageToUTF8(strHei), 
				osgText::String::ENCODING_UTF8);
#endif
		}
    }

    void CScreenInfo::Show()
    {
        CScreenKits::Show();

        if(m_opRenderContext.valid() && m_rpHandler.valid())
        {
            m_opRenderContext->GetView()->addEventHandler(m_rpHandler.get());
        }
    }

    void CScreenInfo::Hide()
    {
        if(m_opRenderContext.valid() && m_rpHandler.valid())
        {
            m_opRenderContext->GetView()->removeEventHandler(m_rpHandler.get());
        }

        CScreenKits::Hide();
    }

	void CScreenInfo::SetInfoMaker( unsigned unKey )
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

}

namespace FeKit
{

    /////////////////////////////////////////////
    //CScreenInfoHandler
    /////////////////////////////////////////////

    CScreenInfoHandler::CScreenInfoHandler( CScreenInfo* pSInfo )
        :m_opSInfo(pSInfo)
    {

    }

    CScreenInfoHandler::~CScreenInfoHandler()
    {

    }


    bool CScreenInfoHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        if (ea.getEventType() != osgGA::GUIEventAdapter::FRAME)
        {
            if (m_opSInfo.valid())
            {
                m_opSInfo->UpdateInfo(ea.getX(), ea.getY());
            }
        }

        return false;
    }
}

