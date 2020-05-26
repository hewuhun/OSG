#include <iostream>
#include <sstream>
#include <iomanip>
#include <osgGA/EventVisitor>
#include <osg/LineWidth>
#include <osgDB/ConvertUTF>
#include <osgEarth/GeoMath>
#include <osgEarthUtil/EarthManipulator>
//#include <FeUtils/FreeEarthManipulator.h>

#include <FeKits/miniLogo/MiniLogo.h>
#include <FeUtils/UtilityGeom.h>
#include <FeUtils/PathRegistry.h>
namespace FeKit
{

    CMiniLogo::CMiniLogo( 
		int nXOffset, 
		int nYOffset, 
		int nWidth, 
		int nHeight, 
		CPlaceStrategy::KIT_PLACE ePlace, 
		osgViewer::View* pView, 
		const std::string& strTitle,
		const std::string& strIconPath)
        :CScreenKits(NULL, nXOffset, nYOffset, nWidth, nHeight, ePlace)
        ,m_strIconPath(strIconPath)
    {
        osg::Geode* pGeode = CreateLogoUI(strTitle);

        addChild(pGeode);
    }


    CMiniLogo::~CMiniLogo(void)
    {

    }

    void CMiniLogo::SetIconPath( const std::string& strPath )
    {
        m_strIconPath = strPath;
    }

    std::string CMiniLogo::GetIconPath()
    {
        return m_strIconPath;
    }

    osg::Geode* CMiniLogo::CreateLogoUI(const std::string& strTitle)
    {
        osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;

        osg::ref_ptr<osgText::Font> pFont = osgText::readFontFile(FeFileReg->GetFullPath("fonts/LCALLIG.TTF"));

        m_rpTextLogo = new osgText::Text;
        m_rpTextLogo->setFont(pFont.get());
   //     m_rpTextLogo->setFontResolution(40.0f,40.0f);
        m_rpTextLogo->setColor(osg::Vec4d(1,1,1,1.0));
        m_rpTextLogo->setCharacterSize(m_dRightRate * 0.2, 1.3);
        m_rpTextLogo->setDataVariance(osg::Object::DYNAMIC);
        m_rpTextLogo->setAlignment(osgText::Text::LEFT_CENTER);//设置对其方式
        m_rpTextLogo->setPosition(osg::Vec3(m_dLeftRate*1.0, m_dTopRate*0.5, -1.0));
        m_rpTextLogo->setBackdropColor(osg::Vec4(0,0,0,1));
        m_rpTextLogo->setBackdropImplementation(osgText::Text::STENCIL_BUFFER);
        m_rpTextLogo->setBackdropType(osgText::Text::OUTLINE);
        m_rpTextLogo->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
        m_rpTextLogo->getOrCreateStateSet()->setRenderingHint(osg::StateSet::OPAQUE_BIN);
//         m_rpTextLogo->setText(
// 			osgDB::convertStringFromCurrentCodePageToUTF8("实兵行动推演系统"), 
//             osgText::String::ENCODING_UTF8);

		m_rpTextLogo->setText(strTitle, osgText::String::ENCODING_UTF8);

        pGeode->addDrawable(m_rpTextLogo.get());

        return pGeode.release();
    }

}

