
#include <osgGA/StandardManipulator>
#include <FeKits/manipulator/FeFirstTrackerManipulator.h>


namespace FeKit
{
	CFirstTrackerManipulator::CFirstTrackerManipulator(osg::Camera * pCamera)
		:osgGA::NodeTrackerManipulator()
        ,m_bEnable(true)
		,m_dMaxYaw(osg::PI_2)
		,m_dMaxPitch(osg::PI_2)
		,m_dPitchAngle(0.0)
		,m_dYawAngle(0.0)
		,m_opCamera(pCamera)
	{

	}

	CFirstTrackerManipulator::~CFirstTrackerManipulator()
	{

	}

	bool CFirstTrackerManipulator::performMovementLeftMouseButton( const double eventTimeDelta, const double dx, const double dy )
	{		
		if(!m_bEnable)
		{
			return false;
		}
		m_dYawAngle += dx;
		m_dPitchAngle += dy;

		double dyNew = dy;
		double dxNew = dx;

		if(m_dPitchAngle >= m_dMaxPitch || m_dPitchAngle <= -m_dMaxPitch)
		{
			m_dPitchAngle -= dy;
			dyNew = 0.0;
			
		}

		if(m_dYawAngle >= m_dMaxYaw || m_dYawAngle <= -m_dMaxYaw)
		{
			m_dYawAngle -= dx;
			dxNew =0.0;

		}
		
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
			rotate_elevation.makeRotate(dyNew,sideVector);

			osg::Quat rotate_azim;
			rotate_azim.makeRotate(-dxNew,localUp);

			_rotation = _rotation * rotate_elevation * rotate_azim;
		}


		
		return true;
	}

	void CFirstTrackerManipulator::InitRotate( const float px0, const float py0,
		const float px1, const float py1)
	{
		osg::Vec3d axis;
		float angle;

		trackball( axis, angle, px1, py1, px0, py0 );

		osg::Quat new_rotate;
		new_rotate.makeRotate( angle, axis );

		_rotation = _rotation * new_rotate;
	}

	bool CFirstTrackerManipulator::handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
	{
        if (!m_bEnable)
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
				setDistance(0.0);
				m_dYawAngle = 0.0;
				m_dPitchAngle = 0.0;
				return true;
 			} 
		}

		return false;
	}


	void CFirstTrackerManipulator::SetHorizontalAngle(double dAngle_X,double dAngle_Y)
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

    bool CFirstTrackerManipulator::handleResize( const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us )
    {
        if (m_bEnable)
        {
             return osgGA::NodeTrackerManipulator::handleResize(ea, us);
        }
        return false;
    }

    bool CFirstTrackerManipulator::handleMouseMove( const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us )
    {
        if (m_bEnable)
        {
            return   osgGA::NodeTrackerManipulator::handleMouseMove(ea, us);
        }
        return false;
    }

    bool CFirstTrackerManipulator::handleMouseDrag( const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us )
    {
        if (m_bEnable)
        {
             return  osgGA::NodeTrackerManipulator::handleMouseDrag(ea, us);
        }
        return false;
    }

    bool CFirstTrackerManipulator::handleMousePush( const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us )
    {
        if (m_bEnable)
        {
            return  osgGA::NodeTrackerManipulator::handleMousePush(ea, us);
        }
        return false;
    }

    bool CFirstTrackerManipulator::handleMouseRelease( const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us )
    {
        if (m_bEnable)
        {
             return  osgGA::NodeTrackerManipulator::handleMouseRelease(ea, us);
        }
        return false;
    }

    bool CFirstTrackerManipulator::handleKeyUp( const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us )
    {
        if (m_bEnable)
        {
             return  osgGA::NodeTrackerManipulator::handleKeyUp(ea, us);
        }
        return false;
    }

    bool CFirstTrackerManipulator::handleMouseWheel( const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us )
    {
        if (m_bEnable)
        {
             return osgGA::NodeTrackerManipulator::handleMouseWheel(ea, us);
        }
        return false;
    }

    bool CFirstTrackerManipulator::handleMouseDeltaMovement( const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us )
    {
        if (m_bEnable)
        {
            return  osgGA::NodeTrackerManipulator::handleMouseDeltaMovement(ea, us);
        }
        return false;
    }

	bool CFirstTrackerManipulator::InitFirstView( osg::Node *pNode ,bool bEnable)
	{
		if(pNode)
		{
			m_bEnable = bEnable;
			this->setByMatrix(
				osg::Matrix::rotate(
				0.0,osg::X_AXIS,
				0.0,osg::Y_AXIS,
				0.0,osg::Z_AXIS));

			this->setTrackNode(pNode);
			this->computeHomePosition();
			this->setAutoComputeHomePosition(false);
			this->setDistance(0.0);
			this->SetHorizontalAngle(0.0,-90.0);
			m_dPitchAngle = 0.0;
			m_dYawAngle = 0.0;

			m_dMaxYaw = osg::PI_2;
			m_dMaxPitch = osg::PI_2;

			return true;
		}
		else
		{
			return false;
		}	
	}

	void CFirstTrackerManipulator::setEnable( bool bEnable )
	{
		m_bEnable = bEnable;
	}

	void CFirstTrackerManipulator::SetViewRange(double dAngle)
	{
		if(m_opCamera.valid())
		{
			double fovy, aspectRatio, zNear, zFar;
			m_opCamera->getProjectionMatrixAsPerspective(fovy,aspectRatio,zNear,zFar);
			m_opCamera->setProjectionMatrixAsPerspective(dAngle,aspectRatio,zNear,zFar);
		}
	}

	void CFirstTrackerManipulator::SetVisualRange( double dMaxYaw,double dMaxPitch )
	{
		m_dMaxYaw = osg::DegreesToRadians(dMaxYaw);
		m_dMaxPitch = osg::DegreesToRadians(dMaxPitch);

		osg::Matrix rotation_matrix;
		rotation_matrix.makeRotate(_rotation);

		osg::Vec3d sideVector = getSideVector(rotation_matrix);
		osg::Vec3d localUp(0.0f,0.0f,1.0f);

		osg::Vec3d forwardVector = localUp^sideVector;
		sideVector = forwardVector^localUp;

		forwardVector.normalize();
		sideVector.normalize();

		osg::Quat rotate_elevation;
		rotate_elevation.makeRotate(m_dPitchAngle,sideVector);

		osg::Quat rotate_azim;
		rotate_azim.makeRotate(m_dYawAngle,localUp);

		_rotation = _rotation * rotate_elevation * rotate_azim;

		m_dYawAngle = 0.0;
		m_dPitchAngle = 0.0;


	}

}


