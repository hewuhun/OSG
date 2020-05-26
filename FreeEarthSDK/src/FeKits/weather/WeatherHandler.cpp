#include <FeKits/weather/WeatherHandler.h>

#include <FeKits/weather/WeatherEffectNode.h>

namespace FeKit
{
    CWeatherHandler::CWeatherHandler( CWeatherEffectNode * effect )
    {
        m_pWeatherEffect = effect;
    }


    CWeatherHandler::~CWeatherHandler(void)
    {
    }

    bool CWeatherHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        if (ea.getEventType() == ea.KEYUP)
        {
            switch (ea.getKey())
            {
            case osgGA::GUIEventAdapter::KEY_1:
                m_pWeatherEffect->SetWeatherType(FeKit::LIGHT_RIAN);
                break;
            case osgGA::GUIEventAdapter::KEY_2:
                m_pWeatherEffect->SetWeatherType(FeKit::HEAVY_RAIN);
                break;
            case osgGA::GUIEventAdapter::KEY_3:
                m_pWeatherEffect->SetWeatherType(FeKit::LIGHT_SNOW);
                break;
            case osgGA::GUIEventAdapter::KEY_4:
                m_pWeatherEffect->SetWeatherType(FeKit::HEAVY_SNOW);
                break;
            case osgGA::GUIEventAdapter::KEY_0:
                m_pWeatherEffect->SetWeatherType(FeKit::NO_WEATHER);
                break;
            default:
                break;
            }
        }

        return false;
    }

}
