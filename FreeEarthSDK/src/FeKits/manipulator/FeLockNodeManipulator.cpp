#include <FeKits/manipulator/FeLockNodeManipulator.h>
#include <FeKits/manipulator/FreeEarthManipulator.h>
#include <osgViewer/Renderer>

std::string g_strFirstLockNode = "first_Lock_Node"; // 第一人称时，锁定的节点名称。
std::string g_strThirdLockNode = "third_Lock_Node"; // 第三人称时，锁定的节点名称。

FeKit::FeLockNodeManipulator::FeLockNodeManipulator( FeUtil::CRenderContext* rc )
	:FeKit::FreeEarthManipulatorBase(rc)
    ,m_opLockNode(NULL)
    ,m_homeHead(0.0)
    ,m_homeTilt(0.0)
    ,m_homeLockDis(0.0)
    ,m_curHead(0.0)
    ,m_curTilt(0.0)
    ,m_curLockDis(1.0)
	,m_lastX(0.0)
	,m_lastY(0.0)
	,m_dirRate(0.1)
	,m_zoomRate(0.1)
	,m_minLockDis(0.1)
	,m_maxLockDis(20000000.0)
	,m_lastHead(0.0)
    ,m_lastTilt(0.0)
    ,m_lastLockDis(0.1)
	,m_lockType(THIRDPERSON)
	,m_firstHomeHead(0.0)
	,m_firstHomeTilt(-90.0)
	,m_xDirParam(1)
	,m_yDirParam(1)
	,m_vecCenterOffset(0,0,0)
{
	if(m_opRenderContext.valid())
	{
		osg::Camera* cam = m_opRenderContext->GetCamera();
		if(cam)
		{
			m_viewMatrix = cam->getViewMatrix();
		}
	}
}

FeKit::FeLockNodeManipulator::~FeLockNodeManipulator()
{

}

osg::Matrixd FeKit::FeLockNodeManipulator::getMatrix() const
{
	osg::Matrix mat;
	mat = osg::Matrix::inverse(m_viewMatrix);
	return mat;
}

osg::Matrixd FeKit::FeLockNodeManipulator::getInverseMatrix() const
{
	return m_viewMatrix;
}

void FeKit::FeLockNodeManipulator::home( const osgGA::GUIEventAdapter& ,osgGA::GUIActionAdapter& )
{
	resetCamera();
}

void FeKit::FeLockNodeManipulator::setThirdPersonHomeParam( double lockDis,float degreeHead /*= 0*/,float degreeTilt /*= 0*/ )
{
	m_homeLockDis = lockDis;
	m_homeHead = degreeHead;
	m_homeTilt = degreeTilt;
}

void FeKit::FeLockNodeManipulator::resetCamera()
{
	if(m_lockType == THIRDPERSON)
	{
		m_curLockDis = m_homeLockDis;
		m_curHead = m_homeHead;
		m_curTilt = m_homeTilt;
		m_xDirParam = -1;
	}
	else if(m_lockType == FIRSTPERSON)
	{
		m_curHead = m_firstHomeHead;
		m_curTilt = m_firstHomeTilt;
		m_xDirParam = 1;
	}
}

bool FeKit::FeLockNodeManipulator::handle( const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us )
{
	bool handled = false;
	if(m_opLockNode.valid())
	{
		//checkClampProjectionMatrixCallback();

		switch( ea.getEventType() )
		{
		case osgGA::GUIEventAdapter::DOUBLECLICK:
		{
			resetCamera();
			break;
		}
		case osgGA::GUIEventAdapter::PUSH:
			if(ea.getButtonMask() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON || ea.getButtonMask() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
			{
				m_lastX = ea.getX();
				m_lastY = ea.getY();
				handled = true;
			}
			break;

		case osgGA::GUIEventAdapter::DRAG:
			if(ea.getButtonMask() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
			{
				int dx = ea.getX() - m_lastX;
				int dy = ea.getY() - m_lastY;

				m_curHead += dx * m_dirRate * m_xDirParam;
				m_curTilt += dy * m_dirRate * m_yDirParam;

                m_curHead = std::fmod(m_curHead,360.0f);
                m_curTilt = std::fmod(m_curTilt,360.0f);

				m_lastX = ea.getX();
				m_lastY = ea.getY();

				handled = true;
			}
			else if(ea.getButtonMask() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
			{
				int dy = ea.getY() - m_lastY;
				if(dy > 0)
					m_curLockDis *= (1.0 + m_zoomRate);
				else
					m_curLockDis *= (1.0 - m_zoomRate);

				m_lastX = ea.getX();
				m_lastY = ea.getY();

				handled = true;
			}
			break;

		case osgGA::GUIEventAdapter::SCROLL:
			if(ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_DOWN)
			{
				m_curLockDis *= (1.0 + m_zoomRate);
				handled = true;
			}
			else if(ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_UP)
			{
				m_curLockDis *= (1.0 - m_zoomRate);
				handled = true;
			}
			break;

		case osgGA::GUIEventAdapter::FRAME:
			modifyFirstPersonNearFar();
			break;

		//case osgGA::GUIEventAdapter::KEYUP:
		//	if (ea.getKey() == 'v')
		//	{
		//		if (getLockType() == THIRDPERSON)
		//		{
		//			setLockType(FIRSTPERSON);
		//		}
		//		else
		//		{
		//			setLockType(THIRDPERSON);
		//		}
		//		handled = true;
		//	}

		default: break;
		}
	}
	return handled;
}

void FeKit::FeLockNodeManipulator::computeViewMatrix()
{
	if(m_opLockNode.valid())
	{
		osg::Vec3d localCenter = m_opLockNode->getBound().center();

		if (m_lockType == THIRDPERSON)
		{
			osg::Node* pLocalNode = RecursionNode(m_opLockNode.get(), g_strThirdLockNode);
			if (pLocalNode)
			{
				osg::MatrixTransform *thirdMatrix = dynamic_cast<osg::MatrixTransform*>(pLocalNode);
				if (thirdMatrix)
				{
					localCenter = thirdMatrix->getMatrix().getTrans();
				}
			}
		}
		else if(m_lockType == FIRSTPERSON)
		{
			m_curHead = osg::clampBetween(m_curHead, -89.0f, 89.0f);
			m_curTilt = osg::clampBetween(m_curTilt, -179.0f, 0.0f);
			m_curLockDis = 0.1;

			localCenter = osg::Vec3d(0, -0.5, 0);
			osg::Node *pLocalNode = RecursionNode(m_opLockNode.get(), g_strFirstLockNode);
			if (pLocalNode)
			{
				osg::MatrixTransform *firstMatrix = dynamic_cast<osg::MatrixTransform*>(pLocalNode);
				if (firstMatrix)
				{
					localCenter = firstMatrix->getMatrix().getTrans();
				}
			}
			localCenter += m_vecCenterOffset;
		}

		m_worldTransMat.makeIdentity();
		osg::MatrixList transMatList = m_opLockNode->getWorldMatrices();
		if(transMatList.size() > 0)
			m_worldTransMat = transMatList[0];

		if (getLockType() == THIRDPERSON)
		{
			m_worldTransMat.preMultRotate(osg::Quat(-s_dRollRadianAngle, osg::Y_AXIS));
		}

		osg::Vec3d lockCenter = localCenter * m_worldTransMat;

		osg::Vec3d localNorth,localUp,localRight;
		computeLocalAxis(lockCenter,localNorth,localUp,localRight);

		osg::Quat q(osg::DegreesToRadians(m_curHead),localUp);
		osg::Matrix mat(q);

		m_rightDir = localRight * mat;
		m_rightDir.normalize();

		m_upDir = localNorth * mat;
		m_upDir.normalize();

		osg::Quat q1(osg::DegreesToRadians(m_curTilt),m_rightDir);
		osg::Matrix mat1(q1);

		m_upDir = m_upDir * mat1;
		m_upDir.normalize();

		m_lookDir = localUp * mat1;
		m_lookDir.normalize();

		if(m_curLockDis < m_minLockDis)
			m_curLockDis = m_minLockDis;

		if (m_curLockDis > m_maxLockDis)
			m_curLockDis = m_maxLockDis;

		osg::Vec3d camPos = lockCenter + m_lookDir * m_curLockDis;
		osg::Vec3d checkPos = camPos + m_lookDir * 3;
		osg::Vec3d intersectPos;
		osg::Vec3d safePos;
		osg::Vec3d tmp;

		if (CheckCollision(tmp, camPos, safePos, tmp))
		{
			/*if (rayToWorld(lockCenter, checkPos, intersectPos,tmp))
			{
			if (checkCollisionWithCheckPoint(lockCenter, camPos, safePos, intersectPos))
			{
			double dis = (safePos - lockCenter).length();
			if (dis < m_curLockDis)
			{
			m_curLockDis = dis;
			camPos = lockCenter + m_lookDir * m_curLockDis;
			}	
			}
			}
			else
			{
			if (checkCollisionWithCheckPoint(lockCenter, camPos, safePos, camPos))
			{
			double dis = (safePos - lockCenter).length();
			if (dis < m_curLockDis)
			{
			m_curLockDis = dis;
			camPos = lockCenter + m_lookDir * m_curLockDis;
			}
			}
			}*/
			m_viewMatrix.makeLookAt(safePos, lockCenter, m_upDir);

			m_curHead = m_lastHead;
			m_curTilt = m_lastTilt;
			m_curLockDis = m_lastLockDis;
		}
		else
		{
			m_viewMatrix.makeLookAt(camPos, lockCenter, m_upDir);
			
			m_lastHead = m_curHead;
			m_lastTilt = m_curTilt;
			m_lastLockDis = m_curLockDis;
		}

		

		
	}
}

void FeKit::FeLockNodeManipulator::computeLocalAxis( const osg::Vec3d& center,osg::Vec3d& northDir,osg::Vec3d& upDir,osg::Vec3d& rightDir )
{
	/*upDir = center;
	upDir.normalize();

	osg::Vec3d worldUp1(0,0,1);

	double dot = osg::absolute(worldUp1 * upDir);
	if (osg::equivalent(dot, 1.0))
	{
	worldUp1 = osg::Vec3d(0, 1, 0);
	}

	rightDir = upDir ^ worldUp1;
	rightDir.normalize();
	northDir = rightDir ^ upDir;
	northDir.normalize();*/

	osg::Vec3d localNorth(0,1,0);
	osg::Vec3d localUp(0,0,1);
	osg::Vec3d localRight(-1,0,0);
	osg::Vec3d localCenter(0,0,0);

	osg::Vec3d worldNorth = localCenter * m_worldTransMat;
	osg::Vec3d worldUp = localUp * m_worldTransMat;
	osg::Vec3d worldRight = localRight * m_worldTransMat;
	osg::Vec3d worldCenter = localCenter * m_worldTransMat;

	upDir = worldUp - worldCenter;
	upDir.normalize();

	rightDir = worldRight - worldCenter;
	rightDir.normalize();
   
	northDir = rightDir ^ upDir;
	northDir.normalize();

	rightDir = upDir ^ northDir;
	rightDir.normalize();
}

void FeKit::FeLockNodeManipulator::checkClampProjectionMatrixCallback()
{
	/*if(m_opRenderContext.valid() && m_rpProjCallback.valid())
	{
		osg::Camera* cam = m_opRenderContext->GetCamera();
		if(cam)
		{
			osgViewer::Renderer* r = dynamic_cast<osgViewer::Renderer*>(cam->getRenderer());
			if(r)
			{				
				osgUtil::SceneView* sv = r->getSceneView(0);
				if(sv)
				{
					osgUtil::CullVisitor* cv = sv->getCullVisitor();
					if(cv)
					{
						if(cv->getClampProjectionMatrixCallback() == NULL || cv->getClampProjectionMatrixCallback() != m_rpProjCallback.get())
							cv->setClampProjectionMatrixCallback(m_rpProjCallback.get());
					}
				}
			}
		}	
	}	*/
}

void FeKit::FeLockNodeManipulator::modifyNearFar( double& inputNear,double &inputFar )
{
	switch(m_lockType)
	{
	case THIRDPERSON:
		break;
	case FIRSTPERSON:
		break;

	default: break;
	}
}

void FeKit::FeLockNodeManipulator::modifyFirstPersonNearFar()
{
    if(m_lockType == FIRSTPERSON && m_opRenderContext.valid())
	{
		osg::Camera* cam = m_opRenderContext->GetCamera();
		if(cam)
		{
			FeKit::CEarthClampProjectionMatrixCallback* cb = dynamic_cast<FeKit::CEarthClampProjectionMatrixCallback*>(cam->getClampProjectionMatrixCallback());
			if(cb)
			{
				cb->SetUseCustomNearFar(true);
				cb->SetCustomNear(0.5);
				cb->SetCustomFar(0.5 * 100000);
			}
		}
	}
}

void FeKit::FeLockNodeManipulator::setFirstPersonHomeParam( float degreeTilt /*= -90*/,float degreeHead /*= 0*/ )
{
	m_firstHomeHead = degreeHead;
	m_firstHomeTilt = degreeTilt;
}

void FeKit::FeLockNodeManipulator::updateCamera(osg::Camera& camera)
{
	computeViewMatrix();
	CameraManipulator::updateCamera(camera);
}

osg::Node * FeKit::FeLockNodeManipulator::RecursionNode(osg::Node *node, const std::string &strName)
{
	if (strName.empty())
	{
		return NULL;
	}

	osg::Group *gp = node->asGroup();

	if (gp == NULL)
	{
		return NULL;
	}

	int numChild = gp->getNumChildren();
	for (int i = 0; i < numChild; ++i)
	{
		if (gp->getChild(i)->getName() == strName)
		{
			osg::Node *result = gp->getChild(i);
			return result;
		}
	}
	for (int i = 0; i < numChild; ++i)
	{
		if (RecursionNode(gp->getChild(i), strName))
		{
			return RecursionNode(gp->getChild(i), strName);
		}
	}
	return NULL;
}

double FeKit::FeLockNodeManipulator::s_dRollRadianAngle = 0.0;



