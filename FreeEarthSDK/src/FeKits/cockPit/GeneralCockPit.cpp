#include <FeKits/cockPit/GeneralCockPit.h>


//Test
#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osg/Image>
#include <osg/BlendColor>
#include <osg/BlendFunc>

#include <iostream>
#include <math.h>

#include <FeUtils/PathRegistry.h>

namespace FeKit
{

#ifndef METER_NAME
#define METER_NAME
#define ALTI      "AltiMeter"
#define ADI       "AdiMeter"
#define AOA       "AoaMeter"
#define HSI       "HsiMeter"
#define TIMEMETER "TimeMeter"
#define VSI       "VsiMeter"
#endif

#ifndef METER_XML_PATH
#define METER_XML_PATH
#endif
}

namespace FeKit{

    CGeneralCockPit::CGeneralCockPit(int nPosX, int nPosY, int nWidth, int nHeight, const std::string& strRSPath)
        :CCockPit()
        ,osg::Camera()
        ,m_MeterManager(NULL)
        ,m_Altimeter(NULL)
        ,m_TimeMeter(NULL)
        ,m_VsiMeter(NULL)
        ,m_HsiMeter(NULL)
        ,m_AdiMeter(NULL)
        ,m_AoaMeter(NULL)
        ,m_strRSPath(strRSPath)
    {
        m_vecBackColor.set(0.4, 0.4, 0.4, 0.8);

        InitCockPit(nPosX, nPosY, nWidth, nHeight);
    }

    CGeneralCockPit::~CGeneralCockPit()
    {

    }

    void CGeneralCockPit::InitCockPit(int nPosX, int nPosY, int nWidth, int nHeight)
    {
        setViewport(nPosX, nPosY, nWidth, nHeight);
        setViewMatrix(osg::Matrix::identity());

        setRenderOrder(osg::Camera::POST_RENDER);
        setClearMask(GL_DEPTH_BUFFER_BIT);
        setAllowEventFocus(false);
        setReferenceFrame(osg::Transform::ABSOLUTE_RF);

        setProjectionMatrix(osg::Matrix::ortho(-1.0, 1.0, -1.0, 1.0, -10.0, 10.0));

        getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

        setClearColor(m_vecBackColor);

        std::string strMeterXmlPath = FeFileReg->GetFullPath("meter/MeterXml/MeterXml.xml");
        m_MeterManager = new CMeterManager(strMeterXmlPath.c_str());
        if (!m_MeterManager)
        {
            osg::notify(osg::NOTICE)<<"仪表配置文件读取错误！仪表配置文件存放路径："
                + strMeterXmlPath<<std::endl;
            return;
        }
        //创建仪表
        BuildMeter();

        //背景板
        osg::Node* pNode = osgDB::readNodeFile(FeFileReg->GetFullPath("meter/background.ive"));
        if(pNode)
        {
            pNode->getOrCreateStateSet()->setAttributeAndModes(new osg::BlendColor(osg::Vec4(0.5, 0.5, 0.5, 0.5)), osg::StateAttribute::ON);
            pNode->getOrCreateStateSet()->setAttributeAndModes(new osg::BlendFunc(GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR), osg::StateAttribute::ON);

            osg::MatrixTransform* pMt = new osg::MatrixTransform;
            pMt->addChild(pNode);
            pMt->setMatrix(osg::Matrix::scale(0.05,0.06,0.05) 
                * osg::Matrix::translate(osg::Vec3(0,-0.4,-2.5)));
            addChild(pMt);
        }

        getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    }


    void CGeneralCockPit::AddChildNodeList(const vecMeterAttributeList MeterAttributeList, const setLoadMeterID loadMeterID)
    {
        if (!m_MeterManager)
        {
            return;
        }
        int nIndex = 0;
        SMeterAttribute sMeterAttribute;

        for (setLoadMeterID::iterator itr = loadMeterID.begin(); itr != loadMeterID.end(); itr++)
        {
            switch(*itr)
            {
            case AltiMeter:
                nIndex = m_MeterManager->ReturnMeterIndex(ALTI);
                sMeterAttribute = MeterAttributeList.at(nIndex);
                //定义海拔仪表
                m_Altimeter = new CAltiMeter;
                m_Altimeter->setMatrix(osg::Matrix::scale(sMeterAttribute.scale.x, sMeterAttribute.scale.y, sMeterAttribute.scale.z) * 
                    osg::Matrix::rotate(-osg::PI_2, osg::Z_AXIS) * 
                    osg::Matrix::rotate(-osg::PI_2 , osg::Y_AXIS) *
                    osg::Matrix::rotate(-osg::PI_2 * 0.85, osg::X_AXIS) * 
                    osg::Matrix::translate(sMeterAttribute.position.x, sMeterAttribute.position.y, sMeterAttribute.position.z));
                addChild(m_Altimeter);
                break;
            case AdiMeter:
                nIndex = m_MeterManager->ReturnMeterIndex(ADI);
                sMeterAttribute = MeterAttributeList.at(nIndex);
                m_AdiMeter = new CAdiMeter;
                m_AdiMeter->setMatrix(osg::Matrix::scale(sMeterAttribute.scale.x, sMeterAttribute.scale.y, sMeterAttribute.scale.z) * 
                    osg::Matrix::rotate(-osg::PI_2, osg::Z_AXIS) * 
                    osg::Matrix::rotate(-osg::PI_2 , osg::Y_AXIS) *
                    osg::Matrix::rotate(-osg::PI_2 * 0.95, osg::X_AXIS) * 
                    osg::Matrix::translate(sMeterAttribute.position.x, sMeterAttribute.position.y, sMeterAttribute.position.z));
                addChild(m_AdiMeter);
                break;
            case AoaMeter:
                nIndex = m_MeterManager->ReturnMeterIndex(AOA);
                sMeterAttribute = MeterAttributeList.at(nIndex);
                m_AoaMeter = new CAoaMeter;
                m_AoaMeter->setMatrix(osg::Matrix::scale(sMeterAttribute.scale.x, sMeterAttribute.scale.y, sMeterAttribute.scale.z) * 
                    osg::Matrix::rotate(-osg::PI_2, osg::Z_AXIS) * 
                    osg::Matrix::rotate(-osg::PI_2 , osg::Y_AXIS) *
                    osg::Matrix::rotate(-osg::PI_2 * 0.85, osg::X_AXIS) * 
                    osg::Matrix::translate(sMeterAttribute.position.x, sMeterAttribute.position.y, sMeterAttribute.position.z));
                addChild(m_AoaMeter);
                break;
            case HsiMeter:
                nIndex = m_MeterManager->ReturnMeterIndex(HSI);
                sMeterAttribute = MeterAttributeList.at(nIndex);
                m_HsiMeter = new CHsiMeter;
                m_HsiMeter->setMatrix(osg::Matrix::scale(sMeterAttribute.scale.x, sMeterAttribute.scale.y, sMeterAttribute.scale.z) * 
                    osg::Matrix::rotate(-osg::PI_2, osg::Z_AXIS) * 
                    osg::Matrix::rotate(-osg::PI_2 , osg::Y_AXIS) *
                    osg::Matrix::rotate(-osg::PI_2 * 0.85, osg::X_AXIS) * 
                    osg::Matrix::translate(sMeterAttribute.position.x, sMeterAttribute.position.y, sMeterAttribute.position.z));
                addChild(m_HsiMeter);
                break;
            case TimeMeter:
                nIndex = m_MeterManager->ReturnMeterIndex(TIMEMETER);
                sMeterAttribute = MeterAttributeList.at(nIndex);
                m_TimeMeter = new CTimeMeter;
                m_TimeMeter->setMatrix(osg::Matrix::scale(sMeterAttribute.scale.x, sMeterAttribute.scale.y, sMeterAttribute.scale.z) * 
                    osg::Matrix::rotate(-osg::PI_2, osg::Z_AXIS) * 
                    osg::Matrix::rotate(-osg::PI_2 , osg::Y_AXIS) *
                    osg::Matrix::rotate(-osg::PI_2 * 0.85, osg::X_AXIS) * 
                    osg::Matrix::translate(sMeterAttribute.position.x, sMeterAttribute.position.y, sMeterAttribute.position.z));
                addChild(m_TimeMeter);
                break;
            case VsiMeter:
                nIndex = m_MeterManager->ReturnMeterIndex(VSI);
                sMeterAttribute = MeterAttributeList.at(nIndex);
                m_VsiMeter = new CVsiMeter;
                m_VsiMeter->setMatrix(osg::Matrix::scale(sMeterAttribute.scale.x, sMeterAttribute.scale.y, sMeterAttribute.scale.z) * 
                    osg::Matrix::rotate(-osg::PI_2, osg::Z_AXIS) * 
                    osg::Matrix::rotate(-osg::PI_2 , osg::Y_AXIS) *
                    osg::Matrix::rotate(-osg::PI_2 * 0.85, osg::X_AXIS) * 
                    osg::Matrix::translate(sMeterAttribute.position.x, sMeterAttribute.position.y, sMeterAttribute.position.z));
                addChild(m_VsiMeter);
                break;
            default:
                break;
            }
        }
    }

    bool CGeneralCockPit::BuildMeter()
    {
        if (!m_MeterManager)
        {
            return false;
        }
        vecMeterAttributeList MeterAttributeList = m_MeterManager->GetMeterAttributeList();
        setLoadMeterID loadMeterID = m_MeterManager->GetLoadMeterIDList();
        AddChildNodeList(MeterAttributeList, loadMeterID);
        return true;
    }

    void CGeneralCockPit::UpdateTip( const std::string& strTip )
    {

    }

    void CGeneralCockPit::UpdatePosition( double dLon, double dlat, double dHeight ) 
    {
        if (!m_Altimeter)
        {
            return;
        }
        m_Altimeter->UpdateAltitude(dHeight);
    }

    void CGeneralCockPit::UpdateMiles( double dMile )
    {
        if (!m_HsiMeter)
        {
            return;
        }
        //航程
        m_HsiMeter->UpdateMiles(dMile);
    }

    void CGeneralCockPit::UpdateVelocity( double dVelocity )
    {
        if (!m_HsiMeter)
        {
            return;
        }
        //实时飞行速度
        m_HsiMeter->UpdateVelocity(dVelocity);
    }

    void CGeneralCockPit::UpdateClimbRate( double dClimbRate )
    {
        if (!m_VsiMeter)
        {
            return;
        }
        m_VsiMeter->UpdateClimbRate(dClimbRate);
    }

    void CGeneralCockPit::UpdateHeading( double dHeading )
    {
        if (!m_HsiMeter)
        {
            return;
        }
        m_HsiMeter->UpdateHeading(dHeading);
    }

    void CGeneralCockPit::UpdateHsiMeter( double dHeading, double dVelocity /*= 1000*/, double dMile /*= 0*/ )
    {
        //更新速度
        UpdateVelocity(dVelocity);
        //更新航程距离
        UpdateMiles(dMile);
        //HSI仪表显示航向角度
        UpdateHeading( dHeading );
    }

    void CGeneralCockPit::ShowCockPit( bool bShow )
    {
        if(m_bSHow)
        {
            setNodeMask(0x11111111);
        }
        else
        {
            setNodeMask(0x00000000);
        }

        m_bSHow = !m_bSHow;
    }

    void CGeneralCockPit::UpdatePosture( double dPitch, double dRoll, double dYaw, double dVelocity ) 
    {
        if (m_VsiMeter)
        {
            //更新攀升下降幅度 速度单位：节， 1节 = 1852/3600 m/s  1英尺 = 0.3048米
            double dClimbRate = dVelocity * 1852 / 3600 /0.3048 * std::cos(dPitch); //英尺每秒
            m_VsiMeter->UpdateClimbRate( dClimbRate );
        }
        if (m_AdiMeter)
        {
            //更新飞行的姿态
            m_AdiMeter->UpdatePosture(dPitch, dRoll, dYaw);
        }
        if (m_AoaMeter)
        {
            //更新AOA仪表的俯仰角度
            m_AoaMeter->UpdatePitch(dPitch);
            m_AoaMeter->RecordPitch(dPitch);
        }
    }

    void CGeneralCockPit::UpdateOilMass( double dValue )
    {

    }

    void CGeneralCockPit::UpdateTemperature( double dValue )
    {

    }

    void CGeneralCockPit::UpdateSafeState( bool bState )
    {

    }


}


