
#ifndef FE_FIRST_TRACKER_MANIPULATOR
#define FE_FIRST_TRACKER_MANIPULATOR

#include <osgGA/NodeTrackerManipulator>

#include <FeKits/Export.h>


namespace FeKit
{
	class FEKIT_EXPORT CFirstTrackerManipulator : public osgGA::NodeTrackerManipulator
	{
	public:
		CFirstTrackerManipulator(osg::Camera * pCamera);
		virtual ~CFirstTrackerManipulator();

	public:
		/**  
          * @brief 初始化第一视角相机
          * @note 根据出入节点，及视点到节点中心的距离，初始化第一视角漫游器； 
          * @param pNode [in] 该节点为跟踪模型节点（.ive等数字模型）；
          * @param distance [in] 视点到节点中心的距离；
          * @return 返回值
        */
		bool InitFirstView(osg::Node *pNode,bool bEnable=true);

		/**  
          * @brief 第一视角左右视角，俯仰视角的范围；
          * @note 设置第一视角的可视范围； 
          * @param dMaxYaw [in] 左右视角的范围 dMaxYaw ~ -dMaxYaw；
          * @param dMaxPitch [in] 俯仰视角范围dMaxPitch ~-dMaxPitch；
          * @return 返回值
        */
		void SetVisualRange(double dMaxYaw,double dMaxPitch);

		/**
		*note:设置鼠标键盘等外设事件开关；
		*/
		void setEnable(bool bEnable);

		void SetViewRange(double dAngle);

	protected:
		virtual bool performMovementLeftMouseButton(const double eventTimeDelta, const double dx, const double dy);

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

        /*virtual bool  handleKeyDown (const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us) ;*/

        virtual bool  handleKeyUp (const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us) ;

        virtual bool  handleMouseWheel (const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us) ;
          
        virtual bool  handleMouseDeltaMovement (const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us) ;



	private:

        bool m_bEnable;

		double												m_dMaxYaw;//偏航视角范围
		double												m_dYawAngle;//偏航视角；

		double												m_dMaxPitch;//俯仰视角范围；
		double												m_dPitchAngle;//俯仰视角；

		osg::observer_ptr<osg::Camera>						m_opCamera;
		

	};
}

#endif // FE_FIRST_TRACKER_MANIPULATOR
