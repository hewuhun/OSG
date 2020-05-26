#include <FeKits/manipulator/ManipulatorUtil.h>

#include <FeKits/manipulator/FreeEarthManipulator.h>
#include <FeUtils/CoordConverter.h>

#include <osg/Vec3d>
#include <osgGA/KeySwitchMatrixManipulator>
#include "osg/ShapeDrawable"
#include "osg/Geode"


namespace FeKit
{
	CManipulatorManager::CManipulatorManager(FeUtil::CRenderContext* pRenderContext)
		:osgGA::KeySwitchMatrixManipulator()
		,m_opRenderContext(pRenderContext)
		,m_bHandleEnabled(true)
		,m_bLocked(false)
		,m_nCameraManipulatorIndex(0)
	{
		if (m_opRenderContext.valid())
		{
			if (m_opRenderContext->GetViewer())
			{
				m_rpCameraAltCallback = new CCameraAltCallback(m_opRenderContext.get());
				m_opRenderContext->GetViewer()->addPostUpdateCallback(m_rpCameraAltCallback);

				if ( m_opRenderContext->GetCamera(0))
				{
					m_rpClampProjCallback = new CEarthClampProjectionMatrixCallback(m_opRenderContext.get());
					m_opRenderContext->GetCamera(0)->setClampProjectionMatrixCallback(m_rpClampProjCallback.get());
				}
			}
		}
	}

	CManipulatorManager::~CManipulatorManager()
	{

	}

	void CManipulatorManager::Locate(const osgEarth::Viewpoint& pViewPoint, double dTime )
	{
		FeKit::FreeEarthManipulator* pFEManipulator = GetOrCreateFEManipulator();
		if(pFEManipulator)
		{
			pFEManipulator->flyToViewpoint(pViewPoint, dTime);
		}
	}

	osgEarth::Viewpoint CManipulatorManager::GetViewPoint()
	{
		FeKit::FreeEarthManipulator* pFEManipulator = GetOrCreateFEManipulator(false);
		if(pFEManipulator)
		{
			return pFEManipulator->getViewpoint();
		}

		return osgEarth::Viewpoint();
	}

	void CManipulatorManager::Home(double dTime)
	{
		osgGA::CameraManipulator* pBaseManipulator = getCurrentMatrixManipulator();
		if(pBaseManipulator)
		{
			pBaseManipulator->home(dTime);
		}
	}

	void CManipulatorManager::SetHome( const osgEarth::Viewpoint& pViewPoint, double dTime )
	{
		FeKit::FreeEarthManipulator* pFEManipulator = GetOrCreateFEManipulator(false);
		if(pFEManipulator)
		{
			pFEManipulator->setHomeViewpoint(pViewPoint, dTime);
		}
	}

	void CManipulatorManager::ToNorthViewPoint( double dTime )
	{
		osgEarth::Viewpoint vp = GetViewPoint();
		vp.setHeading(0.0);

		Locate(vp, dTime);
	}

	//osg::Geode* CreateBall(osg::Vec3d center, osg::Vec4d color)
	//{
	//	osg::ShapeDrawable* pShapeDrawable = new osg::ShapeDrawable;
	//	osg::Sphere* pSphere = new osg::Sphere(center, 100000.0);
	//	pShapeDrawable->setShape(pSphere);
	//	pShapeDrawable->setColor(color);

	//	osg::Geode* pGeode = new osg::Geode;
	//	pGeode->addDrawable(pShapeDrawable);

	//	return pGeode;
	//}

	void CManipulatorManager::ToVerticalViewPoint( double dTime )
	{
		osgEarth::Viewpoint vp = GetViewPoint();
		vp.setPitch(-90.0);

		Locate(vp, dTime);

	//	osg::Vec3d eye, center, up, out;
	//	if (m_opRenderContext.valid())
	//	{
	//		m_opRenderContext->GetCamera()->getViewMatrixAsLookAt(eye, center, up);

	//		m_opRenderContext->GetRoot()->addChild(CreateBall(eye, osg::Vec4d(1.0, 0.0, 0.0, 1.0)));
	//		m_opRenderContext->GetRoot()->addChild(CreateBall(center, osg::Vec4d(1.0, 1.0, 0.0, 1.0)));

	//	}
	//	out = eye;
	//	out.normalize();
	//	center.normalize();
	//	out = eye-center;
	//	out.normalize();
	//	up.normalize();

 //		//osgEarth::Viewpoint vp = GetViewPoint();
 //		osg::Vec3d llh = osg::Vec3d(vp.focalPoint()->x(), vp.focalPoint()->y(), vp.focalPoint()->z());
 //		osg::Vec3d xyz;

	//	FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(), llh, xyz);
	//	xyz = xyz*osg::Matrix::rotate(-osg::DegreesToRadians(vp.getPitch()), out^up);//*osg::Matrix::rotate(-osg::DegreesToRadians(vp.getHeading()), osg::Z_AXIS);
	//	m_opRenderContext->GetRoot()->addChild(CreateBall(xyz, osg::Vec4d(0.0, 1.0, 0.0, 1.0)));
	//	FeUtil::XYZ2DegreeLLH(m_opRenderContext.get(), xyz, llh);

	//	vp.focalPoint()->x() = llh.x();
	//	vp.focalPoint()->y() = llh.y();
	//	//vp.setPitch(-90.0);

	//	std::cout<<"Pitch:"<<vp.getPitch()<<std::endl;
	//	std::cout<<"Heading:"<<vp.getHeading()<<std::endl;
	//	Locate(vp, dTime);
	}

	void CManipulatorManager::LockViewPoint( bool bLock )
	{
		m_bLocked = bLock;
	}

	FeKit::FreeEarthManipulator* CManipulatorManager::GetOrCreateFEManipulator(bool bActive)
	{
		FreeEarthManipulator* pEarthManip = dynamic_cast<FreeEarthManipulator*>(GetCameraManipulatorByKey(FREE_EARTH_MANIPULATOR_KEY, bActive));

		//如果未获取到，则新建操作器
		if (NULL == pEarthManip && m_opRenderContext.valid())
		{
			pEarthManip = new FeKit::FreeEarthManipulator(m_opRenderContext.get());
			pEarthManip->setNode(m_opRenderContext->GetMapNode());
			pEarthManip->getSettings()->setThrowingEnabled(true);
			pEarthManip->getSettings()->setLockAzimuthWhilePanning(false);
			AddCameraManipulator(pEarthManip, FREE_EARTH_MANIPULATOR_KEY);

			if (bActive)
			{
				ActivateCameraManipulator(FREE_EARTH_MANIPULATOR_KEY);
			}
		}

		return pEarthManip;
	}

	FeKit::FeLockNodeManipulator* CManipulatorManager::GetOrCreateLockManipulator(bool bActive)
	{
		FeLockNodeManipulator* pLockManip = dynamic_cast<FeLockNodeManipulator*>(GetCameraManipulatorByKey(THIRD_LOCK_MANIPULATOR_KEY, bActive));

		//如果未获取到，则新建操作器
		if (NULL == pLockManip && m_opRenderContext.valid())
		{
			pLockManip = new FeLockNodeManipulator(m_opRenderContext.get());
			pLockManip->setLockType(FeLockNodeManipulator::THIRDPERSON);
			pLockManip->setThirdPersonHomeParam(100.0, 0.0, -80.0);
			pLockManip->resetCamera();
			AddCameraManipulator(pLockManip, THIRD_LOCK_MANIPULATOR_KEY);

			if (bActive)
			{
				ActivateCameraManipulator(THIRD_LOCK_MANIPULATOR_KEY);
			}
		}

		return pLockManip;
	}

	FeKit::FeLockNodeManipulator* CManipulatorManager::GetOrCreateFirstPersonManipulator(bool bActive)
	{
		FeLockNodeManipulator* pFirstManip = dynamic_cast<FeLockNodeManipulator*>(GetCameraManipulatorByKey(FIRST_LOCK_MANIPULATOR_KEY, bActive));

		//如果未获取到，则新建操作器
		if (NULL == pFirstManip && m_opRenderContext.valid())
		{
			pFirstManip = new FeLockNodeManipulator(m_opRenderContext.get());
			pFirstManip->setLockType(FeLockNodeManipulator::FIRSTPERSON);
			pFirstManip->resetCamera();
			AddCameraManipulator(pFirstManip, FIRST_LOCK_MANIPULATOR_KEY);

			if (bActive)
			{
				ActivateCameraManipulator(FIRST_LOCK_MANIPULATOR_KEY);
			}
		}

		return pFirstManip;
	}

	bool CManipulatorManager::AddCameraManipulator(osgGA::CameraManipulator* pCameraManp, std::string strKey)
	{
		if (NULL == pCameraManp)
		{
			return false;
		}

		addMatrixManipulator(m_nCameraManipulatorIndex++, strKey, pCameraManp);

		return true;
	}

	bool CManipulatorManager::RemoveCameraManipulator( std::string strKey )
	{
		int nKey = 0;
		osgGA::CameraManipulator* pCameraManipulator = NULL;

		if (GetCameraManipulatorInfo(strKey, nKey, &pCameraManipulator))
		{
			KeyManipMap& km = getKeyManipMap();
			KeyManipMap::iterator itr = km.find(nKey);
			if (itr != km.end())
			{
				km.erase(itr);
				return true;
			}
		}

		return false;
	}

	osgGA::CameraManipulator* CManipulatorManager::GetCameraManipulatorByKey( std::string strKey, bool bActive )
	{
		int nKey = 0;
		osgGA::CameraManipulator* pCameraManipulator = NULL;

		if (GetCameraManipulatorInfo(strKey, nKey, &pCameraManipulator))
		{
			if (bActive)
			{
				selectMatrixManipulatorWithKey(nKey);
			}
		}

		return pCameraManipulator;
	}

	bool CManipulatorManager::ActivateCameraManipulator( std::string strKey )
	{
		int nKey = 0;
		osgGA::CameraManipulator* pCameraManipulator = NULL;

		if (GetCameraManipulatorInfo(strKey, nKey, &pCameraManipulator))
		{
			selectMatrixManipulatorWithKey(nKey);

			return true;
		}

		return false;
	}

	bool CManipulatorManager::IsActiveCameraManipulator( std::string strKey )
	{
		int nKey = 0;
		osgGA::CameraManipulator* pCameraManipulator = NULL;

		if (GetCameraManipulatorInfo(strKey, nKey, &pCameraManipulator))
		{
			if (getCurrentMatrixManipulator() == pCameraManipulator)
			{
				return true;
			}
		}

		return false;
	}

	void CManipulatorManager::SetThirdPersonHomeParam(double lockDis, float degreeHead, float degreeTilt)
	{
		FeKit::FeLockNodeManipulator *pThird = GetOrCreateLockManipulator();
		if (pThird)
		{
			pThird->setThirdPersonHomeParam(lockDis, degreeHead, degreeTilt);
			pThird->resetCamera();
		}
	}

	void CManipulatorManager::SetHandleEnabled( bool bEnabled )
	{
		m_bHandleEnabled = bEnabled;
	}

	bool CManipulatorManager::GetHandleEnabled()
	{ 
		return m_bHandleEnabled;
	}

	bool CManipulatorManager::GetCameraManipulatorInfo( const std::string& strKey, int& nKey, osgGA::CameraManipulator** pCameraManip )
	{
		KeyManipMap& km = getKeyManipMap();
		KeyManipMap::iterator itr = km.begin();
		while(itr != km.end())
		{
			NamedManipulator namedManip = itr->second;
			if (0 == std::strcmp(namedManip.first.c_str(), strKey.c_str()))
			{
				nKey = itr->first;
				*pCameraManip = namedManip.second.get();

				return true;
			}

			itr++;
		}

		return false;
	}

	bool CManipulatorManager::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
	{
		if (m_bLocked) 
		{ 
			return false; 
		}

		return osgGA::KeySwitchMatrixManipulator::handle(ea, us);
	}
}
