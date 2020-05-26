
#ifndef FE_KIT_TRACKER_MANIPULATOR
#define FE_KIT_TRACKER_MANIPULATOR

#include <osgGA/NodeTrackerManipulator>
#include <FeKits/manipulator/FreeEarthManipulatorBase.h>
#include <FeKits/Export.h>
#include <osgEarth/MapNode>


namespace FeKit
{
	class FEKIT_EXPORT CNodeTrackerManipulator : public osgGA::NodeTrackerManipulator,public FeKit::FreeEarthManipulatorBase
	{
	public:
		CNodeTrackerManipulator(osgViewer::Viewer* v,osgEarth::MapNode* n,FeUtil::CRenderContext *pRenderContext);
		virtual ~CNodeTrackerManipulator();

	public:
		/**  
          * @brief 初始化跟踪相机
          * @note 根据出入节点，及视点到节点中心的距离，初始化跟踪漫游器； 
          * @param pNode [in] 该节点为跟踪模型节点（.ive等数字模型）；
          * @param distance [in] 视点到节点中心的距离；
          * @return 返回值
        */
		bool InitTrackView(osg::Node *pNode,double distance =1000.0,bool bEnable=true);

		/**
		*note:设置视点到节点中心的默认距离；
		*/
		void SetDefalutDistance( double distance );

		/**
		*note:设置鼠标键盘等外设事件开关；
		*/
		void setEnable(bool bEnable);

	protected:
		virtual bool performMovementLeftMouseButton(const double eventTimeDelta, const double dx, const double dy);
		virtual bool performMovementRightMouseButton( const double eventTimeDelta, const double dx, const double dy );

		virtual bool handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
		
		void InitRotate( const float px0, const float py0,
			const float px1, const float py1 );

		void SetHorizontalAngle(double dAngle_X,double dAngle_Y);

	protected:
		virtual bool  handleResize (const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us) ;

		virtual bool  handleMouseMove (const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us) ;

		virtual bool  handleMouseDrag (const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us) ;

		virtual bool  handleMousePush (const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us) ;

		virtual bool  handleMouseRelease (const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us) ;

		virtual bool  handleKeyUp (const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us) ;

		virtual bool  handleMouseWheel (const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us) ;

		virtual bool  handleMouseDeltaMovement (const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us) ;

		virtual osg::Matrixd getBaseMatrix() const;
	protected: 
		bool						m_bEnable;         //鼠标键盘等外设事件开关；
		double						m_dDefalutDistance;//保存用户设置跟随距离；
	};
}

#endif // FE_KIT_TRACKER_MANIPULATOR
