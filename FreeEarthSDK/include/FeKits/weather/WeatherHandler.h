/**************************************************************************************************
* @file WeatherHandler.h
* @note 天气操作事件处理器
* @author c00005
* @data 2014-2-27
**************************************************************************************************/
#ifndef FE_WEATHER_HANDLER_H
#define FE_WEATHER_HANDLER_H

#include <osgViewer/Viewer>
#include <osgGA/GUIEventHandler>
#include <osgEarthUtil/EarthManipulator>

#include <FeKits/Export.h>

namespace FeKit
{
    class CWeatherEffectNode;

    /**
    * @class CWeatherHandler
    * @brief 处理天气的变化的事件
    * @note 该类主要用于天气的变换，包括雨雪的大小几种效果
    * @author c00005
    */
    class FEKIT_EXPORT CWeatherHandler : public osgGA::GUIEventHandler
    {
    public:
        CWeatherHandler(CWeatherEffectNode * effect);
        
		~CWeatherHandler(void);

    public:
		/**  
		  * @brief 事件处理器入口函数
		*/
        virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

    protected:
        ///天气效果的观察者指针
        osg::observer_ptr<CWeatherEffectNode> m_pWeatherEffect;
    };
}
#endif //FE_WEATHER_HANDLER_H
