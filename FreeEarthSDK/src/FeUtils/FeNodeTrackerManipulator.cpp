
#include <FeUtils/FeNodeTrackerManipulator.h>

namespace FeUtil
{
	CNodeTrackerManipulator::CNodeTrackerManipulator(osgViewer::Viewer* v,osgEarth::MapNode* n,FeUtil::CRenderContext *pRenderContext)
		:osgGA::NodeTrackerManipulator()
		,FeUtil::FreeEarthManipulatorBase(v,pRenderContext)
		,m_dDefalutDistance(1000.0)
		,m_bEnable(true)
	{
		setMapNode(n);
	}

	CNodeTrackerManipulator::~CNodeTrackerManipulator()
	{

	}

	bool CNodeTrackerManipulator::performMovementLeftMouseButton( const double eventTimeDelta, const double dx, const double dy )
	{
		if(!m_bEnable)
		{
			return false;
		}

		osg::Quat  oldRotation = getRotation();
		osg::Vec3d oldCenter = getCenter();

		osg::Vec3d nodeCenter;
		osg::Quat nodeRotation;
		computeNodeCenterAndRotation(nodeCenter, nodeRotation);

		// rotate camera
		//if( getVerticalAxisFixed() ) 
		{

			osg::Matrix rotation_matrix;
			rotation_matrix.makeRotate(_rotation);

			osg::Vec3d sideVector = getSideVector(rotation_matrix);
			osg::Vec3d localUp(0.0f,0.0f,1.0f);

			osg::Vec3d forwardVector = localUp^sideVector;
			sideVector = forwardVector^localUp;

			forwardVector.normalize();
			sideVector.normalize();

			osg::Quat rotate_elevation;
			rotate_elevation.makeRotate(dy,sideVector);

			osg::Quat rotate_azim;
			rotate_azim.makeRotate(-dx,localUp);

			_rotation = _rotation * rotate_elevation * rotate_azim;

		} 		

		osg::Matrixd m = getInverseMatrix();
		osg::Vec3d  eye,tmp;
		m.getLookAt(eye,tmp,tmp);
		if(checkCollision(tmp,eye,tmp,tmp))
		{
			setRotation(oldRotation);
			setCenter(oldCenter);
			return false;
		}

		return true;
	}

	void CNodeTrackerManipulator::InitRotate( const float px0, const float py0,
		const float px1, const float py1)
	{
		osg::Vec3d axis;
		float angle;

		trackball( axis, angle, px1, py1, px0, py0 );

		osg::Quat new_rotate;
		new_rotate.makeRotate( angle, axis );

		_rotation = _rotation * new_rotate;
	}

	bool CNodeTrackerManipulator::handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
	{
		if(!m_bEnable)
		{
			return false;
		}

		switch(ea.getKey())
		{
 		case osgGA::GUIEventAdapter::KEY_Space:
 			{
				flushMouseEventStack();
				_thrown = false;
				home(ea,us);
				SetHorizontalAngle(0.0,90.0);
				setDistance(m_dDefalutDistance);
				return true;
 			} 
		return false;
		}
	}

	void CNodeTrackerManipulator::SetHorizontalAngle(double dAngle_X,double dAngle_Y)
	{
		osg::Matrix rotation_matrix;
		rotation_matrix.makeRotate(_rotation);

		osg::Vec3d sideVector = getSideVector(rotation_matrix);
		osg::Vec3d localUp(0.0f,0.0f,1.0f);

		osg::Vec3d forwardVector = localUp^sideVector;
		sideVector = forwardVector^localUp;

		forwardVector.normalize();
		sideVector.normalize();

		osg::Quat rotate_elevation;
		rotate_elevation.makeRotate(osg::DegreesToRadians(-dAngle_Y),sideVector);

		osg::Quat rotate_azim;
		rotate_azim.makeRotate(osg::DegreesToRadians(-dAngle_X),localUp);

		_rotation = _rotation * rotate_elevation * rotate_azim;//
	}

	void CNodeTrackerManipulator::SetDefalutDistance( double distance )
	{
		m_dDefalutDistance = distance;
		setDistance(m_dDefalutDistance);
	}

	bool CNodeTrackerManipulator::InitTrackView( osg::Node *pNode ,double distance,bool bEnable )
	{
		if(pNode)
		{
			m_bEnable = bEnable;
			m_dDefalutDistance = distance;
			this->setByMatrix(
				osg::Matrix::rotate(
				0.0,osg::X_AXIS,
				0.0,osg::Y_AXIS,
				0.0,osg::Z_AXIS));

			this->setTrackNode(pNode);

			this->computeHomePosition();
			this->setAutoComputeHomePosition(false);
			this->setDistance(0.0);
			this->SetHorizontalAngle(0.0,0.0);
			this->setDistance(m_dDefalutDistance);
			return true;
		}
		else
		{
			return false;
		}
	}

	bool CNodeTrackerManipulator::handleResize( const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us )
	{
		if (m_bEnable)
		{
			return osgGA::NodeTrackerManipulator::handleResize(ea, us);
		}
		return false;
	}

	bool CNodeTrackerManipulator::handleMouseMove( const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us )
	{
		if (m_bEnable)
		{
			bool flag =  osgGA::NodeTrackerManipulator::handleMouseMove(ea, us);
			osg::Matrixd m = getInverseMatrix();
			osg::Vec3d  eye,tmp;
			m.getLookAt(eye,tmp,tmp);
			if(checkCollision(tmp,eye,tmp,tmp))
			{
				return false;
			}
			return flag;
		}
		return false;
	}

	bool CNodeTrackerManipulator::handleMouseDrag( const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us )
	{
		if (m_bEnable)
		{
			double dis = getDistance();
			osg::Quat  oldRotation = getRotation();
			osg::Vec3d oldCenter = getCenter();
			bool flag = osgGA::NodeTrackerManipulator::handleMouseDrag(ea, us);
			osg::Matrixd m = getInverseMatrix();
			osg::Vec3d  eye,tmp;
			m.getLookAt(eye,tmp,tmp);
			if(checkCollision(tmp,eye,tmp,tmp))
			{
				setDistance(dis);
				setRotation(oldRotation);
				setCenter(oldCenter);
				return false;
			}
			return flag;
		}
		return false;
	}

	bool CNodeTrackerManipulator::handleMousePush( const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us )
	{
		if (m_bEnable)
		{
			return  osgGA::NodeTrackerManipulator::handleMousePush(ea, us);
		}
		return false;
	}

	bool CNodeTrackerManipulator::handleMouseRelease( const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us )
	{
		if (m_bEnable)
		{
			return  osgGA::NodeTrackerManipulator::handleMouseRelease(ea, us);
		}
		return false;
	}

	bool CNodeTrackerManipulator::handleKeyUp( const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us )
	{
		if (m_bEnable)
		{
			return  osgGA::NodeTrackerManipulator::handleKeyUp(ea, us);
		}
		return false;
	}

	bool CNodeTrackerManipulator::handleMouseWheel( const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us )
	{
		if (m_bEnable)
		{
			double dis = getDistance();
			bool flag = osgGA::NodeTrackerManipulator::handleMouseWheel(ea, us);
			osg::Matrixd m = getInverseMatrix();
			osg::Vec3d  eye,tmp;
			m.getLookAt(eye,tmp,tmp);
			if(checkCollision(tmp,eye,tmp,tmp))
			{
				setDistance(dis);
				return false;
			}
			return flag;
		}
		return false;
	}

	bool CNodeTrackerManipulator::handleMouseDeltaMovement( const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us )
	{
		if (m_bEnable)
		{
			return  osgGA::NodeTrackerManipulator::handleMouseDeltaMovement(ea, us);
		}
		return false;
	}

	void CNodeTrackerManipulator::setEnable( bool bEnable )
	{

		m_bEnable = bEnable;

	}

	osg::Matrixd CNodeTrackerManipulator::getBaseMatrix() const
	{
		return getMatrix();
	}

	bool CNodeTrackerManipulator::performMovementRightMouseButton( const double eventTimeDelta, const double dx, const double dy )
	{
		if (m_bEnable)
		{
			double dis = getDistance();
			bool flag = osgGA::NodeTrackerManipulator::performMovementRightMouseButton(eventTimeDelta,dx,dy);
			osg::Matrixd m = getInverseMatrix();
			osg::Vec3d  eye,tmp;
			m.getLookAt(eye,tmp,tmp);
			if(checkCollision(tmp,eye,tmp,tmp))
			{
				setDistance(dis);
				return false;
			}
			return flag;
		}
		return false;
	}




}





