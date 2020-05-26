#include <FeKits/manipulator/FirstPersonManipulator.h>
#include <FeKits/manipulator/FreeEarthManipulator.h>
#include <FeUtils/CoordConverter.h>
#include <osg/CullFace>
#include <osgViewer/Renderer>
#include <fstream>

namespace FeKit
{
	FirstPersonManipulator::FirstPersonManipulator(FeUtil::CRenderContext* rc)
		:m_opRenderContext(rc)
		,m_initialized(false)
		,m_left(false)
		,m_right(false)
		,m_up(false)
		,m_down(false)
		,m_head(0.0)
		,m_tilt(0.0)
		,m_lastX(0.0)
		,m_lastY(0.0)
		,m_dirRate(0.1)
		,m_speedRate(0.05)
		,m_homeHead(0.0)
		,m_homeTilt(0.0)
		,m_radius(0.2,0.2,0.85)
		,m_isFalling(false)
		,m_slidingSpeed(0.02)
		,m_gravity(0,0,-0.08f)
		,m_localNorth(0,1,0)
	    ,m_localSide(1,0,0)
	    ,m_localUp(0,0,1)
		,m_forwardDir(0,1,0)
		,m_sideDir(1,0,0)
		,m_initLocalCenter(false)
		,m_nearClip(0.1)
		,m_farClip(10000)
		,m_stepPerMove(1)
		,m_curStep(0)
		,m_needJump(false)
		,m_jumpStep(0)
		,m_pathSaving(false)
		,m_maxSavePathSize(360000)
		,m_pathPlaying(false)
		,m_curSavePathStep(0)
		,m_strSavePath("./savepath.txt")
		,m_heightOffset(0.8)
	{
	}

	FirstPersonManipulator::~FirstPersonManipulator()
	{

	}

	void FirstPersonManipulator::setByMatrix( const osg::Matrixd& matrix )
	{

	}

	void FirstPersonManipulator::setByInverseMatrix( const osg::Matrixd& matrix )
	{

	}

	osg::Matrixd FirstPersonManipulator::getMatrix() const
	{
		/*return 
		osg::Matrixd::rotate(m_rotation)*
		osg::Matrixd::rotate(m_centerRotation)*
		osg::Matrixd::translate(m_center);*/
		osg::Matrix mat;
		mat = osg::Matrix::inverse(m_viewMat);
		return mat;
	}

	osg::Matrixd FirstPersonManipulator::getInverseMatrix() const
	{
		/*return osg::Matrixd::translate(-m_center)*
			osg::Matrixd::rotate(m_centerRotation.inverse() ) *
			osg::Matrixd::rotate(m_rotation.inverse());*/
		return m_viewMat;
	}

	void FirstPersonManipulator::home( const osgGA::GUIEventAdapter& ,osgGA::GUIActionAdapter& )
	{
		if(m_opRenderContext.valid())
		{
			m_center = m_homeCenter;
			m_head = m_homeHead;
			m_tilt = m_homeTilt;
		}
	}

	void FirstPersonManipulator::setByLookAt( const osg::Vec3d& eye, const osg::Vec3d& center, const osg::Vec3d& up )
	{
		osg::Matrixd rotation_matrix = osg::Matrixd::lookAt(eye,center,up);

		m_centerRotation = getRotation( eye ).getRotate().inverse();
		m_rotation = rotation_matrix.getRotate().inverse() * m_centerRotation.inverse();	
	}

	osg::Matrixd FirstPersonManipulator::getRotation( const osg::Vec3d& center )
	{
		osg::Vec3d lookVector = center;
		lookVector.normalize();

		osg::Vec3d up = getLocalNorth(center);
		up.normalize();

		double offset = 1000;

		return osg::Matrixd::lookAt( center,center - (lookVector * offset), up);
	}

	void FirstPersonManipulator::setHomePosition( osg::Vec3d spherePos,float degreeHead,float degreeTilt)
	{
		FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),spherePos,m_homeCenter);
		m_homeHead = degreeHead;
		m_homeTilt = degreeTilt;
	}

	osg::Vec3d FirstPersonManipulator::getLocalNorth( osg::Vec3d c )
	{
		osg::Vec3d lookVector = c;
		lookVector.normalize();

		osg::Vec3d side;

		//Force the side vector to be orthogonal to north
		osg::Vec3d worldUp(0,0,1);

		double dot = osg::absolute(worldUp * lookVector);
		if (osg::equivalent(dot, 1.0))
		{
			//We are looking nearly straight down the up vector, so use the Y vector for world up instead
			worldUp = osg::Vec3d(0, 1, 0);
			//OE_NOTICE << "using y vector victor" << std::endl;
		}

		side = lookVector ^ worldUp;
		osg::Vec3d up = side ^ lookVector;
		up.normalize();

		return up;
	}

	void FirstPersonManipulator::checkClampProjectionMatrixCallback()
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
		}*/	
	}

	bool FirstPersonManipulator::handle( const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us )
	{
		bool handled = false;

		if(m_initialized)
		{
			//checkClampProjectionMatrixCallback();

			switch( ea.getEventType() )
			{
			case osgGA::GUIEventAdapter::PUSH:
				if(ea.getButtonMask() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
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
					
					m_head += dx * m_dirRate;
					m_tilt += dy * m_dirRate;
					if(m_head >= 360) m_head -= 360;
					if(m_tilt >= 89) m_tilt = 89;
					if(m_tilt <= -89) m_tilt = -89;

					m_lastX = ea.getX();
					m_lastY = ea.getY();

					updateLookDir();

					handled = true;
				}
				break;


			case osgGA::GUIEventAdapter::KEYDOWN:

				if(ea.getKey() == 'l' || ea.getKey() == 'L')
				{
					resetCamera();
					handled = true;
					break;
				}
				if(ea.getKey() == osgGA::GUIEventAdapter::KEY_Space && !m_isFalling)
				{
					m_needJump = true;
					handled = true;
					break;
				}
				if(ea.getKey() == 'u' || ea.getKey() == 'U')
				{
					m_up = true;
					handled = true;
					break;
				}
				if(ea.getKey() == 'j' || ea.getKey() == 'J')
				{
					m_down = true;
					handled = true;
					break;
				}
				if(ea.getKey() == 'h' || ea.getKey() == 'H')
				{
					m_left = true;
					handled = true;
					break;
				}
				if(ea.getKey() == 'k' || ea.getKey() == 'K')
				{
					m_right = true;
					handled = true;
					break;
				}
				break;

			case osgGA::GUIEventAdapter::KEYUP:
				if(ea.getKey() == 'u' || ea.getKey() == 'U')
				{
					m_up = false;
					handled = true;
					break;
				}
				if(ea.getKey() == 'j' || ea.getKey() == 'J')
				{
					m_down = false;
					handled = true;
					break;
				}
				if(ea.getKey() == 'h' || ea.getKey() == 'H')
				{
					m_left = false;
					handled = true;
					break;
				}
				if(ea.getKey() == 'k' || ea.getKey() == 'K')
				{
					m_right = false;
					handled = true;
					break;
				}
				if(ea.getKey() == osgGA::GUIEventAdapter::KEY_F9)
				{
					startSavePath();
					handled = true;
					break;
				}

				if(ea.getKey() == osgGA::GUIEventAdapter::KEY_F10)
				{
					savePathToFile(m_strSavePath);
					handled = true;
					break;
				}

				if(ea.getKey() == osgGA::GUIEventAdapter::KEY_F11)
				{
					readPathFromFile(m_strSavePath);
					playSavePath();
					handled = true;
					break;
				}
				break;
			case osgGA::GUIEventAdapter::FRAME:
				modifyNearFar();
				break;



			default: break;
			}
		}

		return handled;
	}

	bool FirstPersonManipulator::init( osg::Node* modelNode,std::string octFileName /*= ""*/ )
	{
		if(modelNode && m_opRenderContext.valid())
		{
			osg::Group* pn = NULL;
			osg::ref_ptr<osg::Group> tp = new osg::Group();
			int np = modelNode->getNumParents();
			if(np > 1)
			{
				return false;
			}
			else if(np == 1)
			{
				pn = modelNode->getParent(0);
			}

			osg::Matrix wm;
			osg::MatrixList ml = modelNode->getWorldMatrices();
			if(ml.size() > 0)
				wm = ml[0];

			m_transMat = wm;
			osg::Vec3d modelPos = m_transMat.getTrans();
			osg::Vec3d tm;
			FeUtil::XYZ2DegreeLLH(m_opRenderContext.get(),modelPos,tm);

			m_localInitUp = modelPos;
			m_localInitUp.normalize();

			osg::Matrix mat;
			mat.makeIdentity();

			modelNode->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT_AND_BACK),osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
			modelNode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

			bool fromFile = false;
			if(octFileName != "")
			{
                std::fstream fs(octFileName.c_str(),ios::in|ios::binary);
				if(fs.is_open())
				{
					fromFile = true;
					m_initialized = m_triangleSelector.ConstructTriangleOctreeFromFile(fs);
					fs.close();
				}
			}
		
			if(!fromFile)
			{
				if(pn != NULL)
				{
					tp->addChild(modelNode);
					pn->removeChild(modelNode);
				}

				m_initialized = m_triangleSelector.ConstructTriangleOctree(modelNode,mat);

				if(pn != NULL)
				{
					pn->addChild(modelNode);
					tp->removeChild(modelNode);
				}
			}
			
			resetCamera();

			return m_initialized;
		}
		return false;
	}

	osg::Vec3d FirstPersonManipulator::getLocalSide( osg::Vec3d c )
	{
		osg::Vec3d lookVector = c;
		lookVector.normalize();

		osg::Vec3d side;

		//Force the side vector to be orthogonal to north
		osg::Vec3d worldUp(0,0,1);

		double dot = osg::absolute(worldUp * lookVector);
		if (osg::equivalent(dot, 1.0))
		{
			//We are looking nearly straight down the up vector, so use the Y vector for world up instead
			worldUp = osg::Vec3d(0, 1, 0);
			//OE_NOTICE << "using y vector victor" << std::endl;
		}

		side = lookVector ^ worldUp;
		side.normalize();

		return side;
	}

	void FirstPersonManipulator::updateCamera()
	{
		float offset = 1;
		osg::Vec3d localLookAt = m_eyePos + m_localNorth * offset;
		osg::Vec3d worLookAt = localLookAt * m_transMat;

		m_viewMat.makeLookAt(m_center,worLookAt,m_localInitUp);
	}

	static double lastZ;

	void FirstPersonManipulator::updatePosition()
	{
		osg::Vec3d upDir(0,0,0);
		osg::Vec3d downDir(0,0,0);
		osg::Vec3d leftDir(0,0,0);
		osg::Vec3d rightDir(0,0,0);

		if(m_up)
			upDir = m_forwardDir;
		if(m_down)
			downDir = -m_forwardDir;
		if(m_left)
			leftDir = -m_sideDir;
		if(m_right)
			rightDir = m_sideDir;

		m_curMoveDir = upDir + downDir + leftDir + rightDir;
		m_curMoveDir.normalize();
		m_curMoveDir = m_curMoveDir * m_speedRate;

		if(m_needJump)
		{
			m_curMoveDir += osg::Vec3d(0.0,0.0,0.2);	
			m_jumpStep = 5;

			m_needJump = false;
		}
		if(m_isFalling && m_jumpStep > 0)
		{
			m_curMoveDir += osg::Vec3d(0.0,0.0,0.2);
			m_jumpStep--;
		}

		if(!m_initLocalCenter)
		{
			osg::Matrix invMat = osg::Matrix::inverse(m_transMat);
			m_localCenter = m_center * invMat;
			m_initLocalCenter = true;
		}
		
		FeMath::triangle3d outTriangle;
		osg::Vec3d hitPosition;
		bool foundCollision = false;

		if(m_pathPlaying )
		{
			if(m_curSavePathStep < m_savePathList.size())
			{
				MyPathData d = m_savePathList.at(m_curSavePathStep);
				m_head = d.pathHead;
				m_tilt = d.pathTilt;
				m_localCenter = d.pathPosition;

				updateLookDir();

				m_curSavePathStep++;
			}
			else
				m_pathPlaying = false;
		}
		else
		{
			m_localCenter = m_collisionManager.getCollisionResultPositionWithModel(&m_triangleSelector,m_localCenter,m_radius,
				m_curMoveDir,outTriangle,hitPosition,
				m_isFalling,m_slidingSpeed,m_gravity,foundCollision);
		}

		if(m_pathSaving && m_savePathList.size() < m_maxSavePathSize)
		{
			MyPathData d;
			d.pathPosition = m_localCenter;
			d.pathHead = m_head;
			d.pathTilt = m_tilt;
			m_savePathList.push_back(d);
		}

		if(foundCollision)
			m_jumpStep = 0;

		m_eyePos = m_localCenter;
		m_eyePos.z() = m_eyePos.z() + m_heightOffset;

		m_center = m_eyePos * m_transMat;
			
		updateCamera();	
	}

	void FirstPersonManipulator::updateLookDir()
	{
		osg::Vec3d north(0,1,0);
		osg::Vec3d upDir(0,0,1);
		osg::Vec3d sideDir(1,0,0);

		osg::Quat q(osg::DegreesToRadians(m_head),upDir);
		osg::Matrix mat(q);

		m_sideDir = sideDir * mat;
		m_sideDir.normalize();
		m_localSide = m_sideDir;

		m_forwardDir = north * mat;
		m_forwardDir.normalize();
		m_localNorth = m_forwardDir;

		osg::Quat q1(osg::DegreesToRadians(m_tilt),m_sideDir);
		osg::Matrix mat1(q1);

		m_localNorth = m_localNorth * mat1;
		m_localNorth.normalize();
		m_localUp = m_localUp * mat1;
		m_localUp.normalize();
	}

	void FirstPersonManipulator::resetCamera()
	{
		m_center = m_homeCenter;
		m_head = m_homeHead;
		m_tilt = m_homeTilt;
		updateLookDir();
		m_initLocalCenter = false;
	}

	void FirstPersonManipulator::updateCamera(osg::Camera& camera)
	{
		updatePosition();
		CameraManipulator::updateCamera(camera);
	}

	void FirstPersonManipulator::startSavePath()
	{
		m_savePathList.clear();
		m_savePathList.resize(0);
		m_pathSaving = true;
	}

	bool FirstPersonManipulator::savePathToFile(std::string fileName)
	{
		m_pathSaving = false;

		if(m_savePathList.size() > 0)
		{
            std::fstream iofile(fileName.c_str(),ios::out|ios::binary|ios::trunc);
			if(iofile.is_open())
			{
				SavePathList::size_type s = m_savePathList.size();
				iofile.write((char*)(&s),sizeof(s));

				for (SavePathList::iterator it = m_savePathList.begin(); it != m_savePathList.end(); it++)
				{
					MyPathData p = *it;
					iofile.write((char*)(&p),sizeof(p));
				}

				iofile.close();

				return true;
			}
		}

		return false;
	}

	bool FirstPersonManipulator::readPathFromFile( std::string fileName )
	{
        std::fstream fs(fileName.c_str(),ios::in|ios::binary);
		if(fs.is_open())
		{
			m_savePathList.clear();
			m_savePathList.resize(0);

			SavePathList::size_type s;
			fs.read((char*)(&s),sizeof(s));

			for (SavePathList::size_type i = 0; i < s; i++)
			{
				MyPathData p;
				fs.read((char*)(&p),sizeof(p));

				m_savePathList.push_back(p);
			}

			fs.close();

			return true;
		}
		
		return false;
	}

	void FirstPersonManipulator::playSavePath()
	{
		m_pathPlaying = true;
		m_curSavePathStep = 0;
	}

	void FirstPersonManipulator::modifyNearFar()
	{
		if(m_opRenderContext.valid())
		{
			osg::Camera* cam = m_opRenderContext->GetCamera();
			if(cam)
			{
				FeKit::CEarthClampProjectionMatrixCallback* pc = dynamic_cast<FeKit::CEarthClampProjectionMatrixCallback*>(cam->getClampProjectionMatrixCallback());
				if(pc)
				{
					pc->SetUseCustomNearFar(true);
					pc->SetCustomNear(m_nearClip);
					pc->SetCustomFar(m_farClip);
				}
			}
		}
	}

	void FirstPersonManipulator::getHomePositionFromCam()
	{
		if(m_opRenderContext.valid())
		{
			osg::Camera* cam = m_opRenderContext->GetCamera();
			if(cam)
			{
				osg::Vec3d eye,center,up;
				cam->getViewMatrixAsLookAt(eye,center,up);

				osg::Vec3d zDir = up;
				zDir.normalize();

				osg::Vec3d yDir = center - eye;
				yDir.normalize();

				osg::Vec3d xDir = yDir ^ zDir;
				xDir.normalize();

				zDir = xDir ^ yDir;
				zDir.normalize();

				osg::Matrix invMat = osg::Matrix::inverse(m_transMat);
                osg::Matrix invRotateMat(invMat.getRotate());
	
				osg::Vec3d localZDir = zDir * invRotateMat;
				localZDir.normalize();
				osg::Vec3d localYDir = yDir * invRotateMat;
				localYDir.normalize();
				osg::Vec3d localXDir = localYDir ^ localZDir;
				localXDir.normalize();

				double tilt = acos(osg::clampBetween(localZDir * osg::Vec3d(0,0,1),-1.0,1.0));
				if(localYDir.z() < 0)
					tilt = -tilt;

				double head = acos(osg::clampBetween(localXDir * osg::Vec3d(1,0,0),-1.0,1.0));
				if(localYDir.x() > 0)
					head = -head;

				tilt = osg::RadiansToDegrees(tilt);
				head = osg::RadiansToDegrees(head);

				osg::Vec3d localCenter = eye * invMat;
				localCenter.z() -= m_heightOffset;

				m_homeCenter = localCenter * m_transMat;;
				m_homeHead = head;
				m_homeTilt = tilt;

				resetCamera();
			}
		}
	}
}

