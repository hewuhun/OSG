#include <osgViewer/View>

#include <FeKits/airview/AirViewKits.h>
#include <FeKits/airview/AirViewHandler.h>

namespace FeKit
{

 CAirViewHandler::CAirViewHandler
     (CAirViewGround* pGround, CAirViewFocus* pFocus, osgEarth::MapNode* pMapNode)
     :osgGA::GUIEventHandler()
     ,m_opGround(pGround)
     ,m_opFocus(pFocus)
     ,m_opMapNode(pMapNode)
 {

 }

 CAirViewHandler::~CAirViewHandler()
 {

 }

 bool CAirViewHandler::handle( 
     const osgGA::GUIEventAdapter& ea, 
     osgGA::GUIActionAdapter& aa, 
     osg::Object* pObject, 
     osg::NodeVisitor* pNV )
 {
     osgViewer::View* pView = dynamic_cast<osgViewer::View*>(aa.asView());
     if(!pView)
     {
         return false;
     }

     if(m_opFocus.valid() && m_opGround.valid() && m_opMapNode.valid())
     {
         osg::Vec3d vecEye;
         osg::Vec3d vecUp;
         osg::Vec3d vecCenter;
         pView->getCamera()->getViewMatrixAsLookAt(vecEye, vecUp, vecCenter);

         osgEarth::GeoPoint eyeGeo;
         eyeGeo.fromWorld( m_opMapNode->getMapSRS()->getGeographicSRS(), vecEye );

         osg::Vec3d vecWorldLLH(eyeGeo.x(), eyeGeo.y(), 0.0);
         double dDistance = eyeGeo.z();

         m_opFocus->UpdateFocus(vecWorldLLH, dDistance);
     }


     return false;
 }

}
