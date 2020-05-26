/**************************************************************************************************
* @file AirViewHandler.h
* @note 更新鸟瞰图中的相机的位置信息并做不同的坐标系统的转换
* @author l00008
* @data 2014-04-29
**************************************************************************************************/

#ifndef FE_AIR_VIEW_HANDLER_H
#define FE_AIR_VIEW_HANDLER_H

#include <osgGA/GUIEventHandler>

#include <osgEarth/MapNode>

namespace FeKit
{

/**
  * @class CAirViewHandler
  * @brief 更新鸟瞰图的事件处理器
  * @note 更新鸟瞰图的事件处理器
  * @author l00008
*/
class CAirViewFocus;
class CAirViewGround;
class CAirViewHandler : public osgGA::GUIEventHandler
{
public:
    CAirViewHandler(CAirViewGround* pGround, CAirViewFocus* pFocus, osgEarth::MapNode* pMapNode);

    virtual bool handle(
        const osgGA::GUIEventAdapter& ea, 
        osgGA::GUIActionAdapter& aa, 
        osg::Object* pObject, 
        osg::NodeVisitor* pNV);

protected:
    ~CAirViewHandler();

protected:
    osg::observer_ptr<CAirViewGround>           m_opGround;
    osg::observer_ptr<CAirViewFocus>            m_opFocus;

    osg::observer_ptr<osgEarth::MapNode>        m_opMapNode;
};

}

#endif //FE_AIR_VIEW_HANDLER_H