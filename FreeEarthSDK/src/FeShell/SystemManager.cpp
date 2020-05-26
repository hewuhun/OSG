#include <algorithm>
#include <assert.h>

#include <osgDB/ReadFile>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>

#include <FeKits/manipulator/ManipulatorUtil.h>
#include <FeUtils/logger/LoggerDef.h>
#include <FeUtils/PathRegistry.h>
#include <FeUtils/CoordConverter.h>

#include <FeKits/IR/SulIR.h>

#include <FeUtils/EnvironmentVariableReader.h>

#include <FeShell/SystemManager.h>
#include <FeShell/SystemService.h>
#include <FeShell/SmartToolSys.h>
#include <FeShell/EnvironmentSys.h>

#include <FeEarth/FreeEarth.h>
#include <osgEarth/DPLineSegmentIntersector>


namespace FeShell
{
	CSystemManager::CSystemManager(const std::string& strResorcePath)
		:osg::Referenced()
		, m_bFirstFrame(true)
		, m_strSystemResoucePath(strResorcePath)
		, m_rpViewer(NULL)
		, m_rpCompositeViewer(NULL)
		, m_rpRenderContext(NULL)
		, m_rpSystemService(NULL)
		, m_strEarthPath("")
	{

	}

	CSystemManager::~CSystemManager()
	{

	}

	bool CSystemManager::Initialize()
	{
		//如果用户设置的路径为空，则读取环境变量中的路径
		FeUtil::CFileRegInitAgent agent;
		if (m_strSystemResoucePath.empty())
		{
			FeUtil::CEnvironmentVariableReader reader;
			agent.InitDataPath(reader.DoRead("FREE_EARTH_FILE_PATH"));
		}
		else
		{
			agent.InitDataPath(m_strSystemResoucePath);
		}

		if (!InitBegin())
		{
			return false;
		}

		if (!PreLoadSystemConfig())
		{
			LOG(LOG_ERROR) << "系统配置文件读取错误，程序退出";
			return false;
		}

		if (!InitViewerScene())
		{
			LOG(LOG_ERROR) << "系统场景初始化失败，程序退出";
			return false;
		}

		if (!InitService())
		{
			LOG(LOG_WARN) << "系统加载服务模块失败";
		}

		if (!InitCameraSetting())
		{
			LOG(LOG_ERROR) << "系统相机初始化失败，程序退出";
			return false;
		}

		if (!InitEventHandles())
		{
			LOG(LOG_WARN) << "系统事件初始化失败";
		}

		if (!InitEnd())
		{
			return false;
		}

		return true;
	}

	bool CSystemManager::UnInitialize()
	{
		if (m_rpSystemService.valid())
		{
			m_rpSystemService->RemoveAllModule();
			LOG(LOG_NOTICE) << "系统服务模块成功卸载";
		}

		if (m_rpCompositeViewer.valid())
		{
			m_rpCompositeViewer->setDone(true);
		}

		m_bFirstFrame = true;

		return false;
	}

	bool CSystemManager::PreLoadSystemConfig()
	{
		FeShell::CSystemConfigLoader configLoader;
		if (configLoader.Execute(FeFileReg->GetFullPath("config\\systemConfig.xml"), m_systemConfig))
		{
			return true;
		}

		//return false;
	}

	bool CSystemManager::InitViewerScene()
	{
		osg::DisplaySettings::instance()->setNumOfDatabaseThreadsHint(4);
		osg::DisplaySettings::instance()->setNumOfHttpDatabaseThreadsHint(2);

		m_rpViewer = new osgViewer::Viewer();
		m_rpCompositeViewer = new osgViewer::CompositeViewer();
		m_rpCompositeViewer->addView(m_rpViewer);
		m_rpCompositeViewer->setRunMaxFrameRate(60);
		
		// 设置增量编译
		osg::ref_ptr<osgUtil::IncrementalCompileOperation> ipIncrementalCompile = new osgUtil::IncrementalCompileOperation;
		ipIncrementalCompile->setTargetFrameRate(60);
		ipIncrementalCompile->setMaximumNumOfObjectsToCompilePerFrame(80);
		ipIncrementalCompile->assignForceTextureDownloadGeometry();
		m_rpCompositeViewer->setIncrementalCompileOperation(ipIncrementalCompile.get());

		osg::DisplaySettings* ds = osg::DisplaySettings::instance();
		ds->setNumMultiSamples(4);
		m_rpViewer->setDisplaySettings(ds);

		//首先读取用户定义的earth文件
		osg::ref_ptr<osgEarth::MapNode> rpMapNode =
			osgEarth::MapNode::findMapNode(osgDB::readNodeFile(m_strEarthPath));
		if (false == rpMapNode.valid())
		{
			rpMapNode = osgEarth::MapNode::findMapNode(osgDB::readNodeFile(m_systemConfig.GetEarthFile()));
		}

		if (rpMapNode.valid())
		{
			m_rpViewer->getDatabasePager()->setDoPreCompile(true);
			//m_rpViewer->getDatabasePager()->setUpThreads(20);

			osg::ref_ptr<osg::MatrixTransform>	rpEarthRotationMT = new osg::MatrixTransform;
			rpEarthRotationMT->addChild(rpMapNode.get());

			osg::ref_ptr<osg::Group> rpRoot = new osg::Group;
			rpRoot->addChild(rpEarthRotationMT.get());
			m_rpViewer->setSceneData(rpRoot.get());

			m_rpRenderContext = new FeUtil::CRenderContext();
			m_rpRenderContext->SetMapNode(rpMapNode.get());
			m_rpRenderContext->SetRoot(rpRoot.get());
			m_rpRenderContext->SetViewer(m_rpCompositeViewer.get());
			m_rpRenderContext->SetEarthRotationMT(rpEarthRotationMT);

			return true;
		}

		LOG(LOG_ERROR) << "无法读取地球文件，无法搭建三维场景";

		//return false;
	}

	bool CSystemManager::InitCameraSetting()
	{
		m_rpViewer->getCamera()->setNearFarRatio(0.0000003);
		m_rpViewer->getCamera()->setClearMask(m_rpViewer->getCamera()->getClearMask() | GL_STENCIL_BUFFER_BIT);
		m_rpViewer->getCamera()->setClearStencil(0);
		m_rpViewer->getCamera()->setSmallFeatureCullingPixelSize(-1.0f);
		//m_rpViewer->setUpViewOnSingleScreen(0);

#ifdef WIN32
#ifdef QT4_VERSION
		m_rpCompositeViewer->setThreadingModel(osgViewer::ViewerBase::DrawThreadPerContext);
#else
		m_rpCompositeViewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
#endif
#else
		m_rpCompositeViewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);	
#endif

		return true;
	}

	bool CSystemManager::InitService()
	{
		m_rpSystemService = new FeShell::CSystemService();

		return m_rpSystemService->InitService(m_rpRenderContext.get(), m_systemConfig);
	}

	bool CSystemManager::InitEventHandles()
	{
		m_rpCompositeViewer->setKeyEventSetsDone(0);
		m_rpViewer->setKeyEventSetsDone(0);

#ifndef RELEASE_ON
		m_rpViewer->addEventHandler(new osgViewer::WindowSizeHandler);
		m_rpViewer->addEventHandler(new osgViewer::StatsHandler);
		m_rpViewer->addEventHandler(new osgGA::StateSetManipulator(m_rpViewer->getCamera()->getOrCreateStateSet()));
#endif
		//初始化操作器，包括地球操作器和跟踪操作器
		FeKit::CManipulatorManager* pSwitch = new FeKit::CManipulatorManager(m_rpRenderContext);

		//设置后创建第一人称、第三人称操作器, 默认执行setNode(getSceneData());
		pSwitch->GetOrCreateFEManipulator();
		m_rpViewer->setCameraManipulator(pSwitch);

		//FeUtil::CFreeViewPoint viewPoint(m_systemConfig.GetEndViewPoint().GetEarthVP());
		//viewPoint.SetTime(m_systemConfig.GetEndViewPoint().GetTime());
		//m_rpSystemService->Locate(viewPoint, true);
		//pSwitch->SetHome(m_systemConfig.GetEndViewPoint().GetEarthVP(), m_systemConfig.GetEndViewPoint().GetTime());

		return true;
	}

	void CSystemManager::Frame()
	{
		if (false == m_rpCompositeViewer.valid())
		{
			return ;
		}

		PreFrame();

		m_rpCompositeViewer->frame();
		//if (!m_rpCompositeViewer->isRealized())
		//{
		//	m_rpCompositeViewer->realize();
		//}

		//if(!m_rpCompositeViewer->done())
		//{
		//	m_dMinFrameTime = m_rpCompositeViewer->getRunMaxFrameRate()>0.0 ? 1.0/m_rpCompositeViewer->getRunMaxFrameRate() : 0.0;
		//	osg::Timer_t startFrameTick = osg::Timer::instance()->tick();
		//	if (m_rpCompositeViewer->getRunFrameScheme()==osgViewer::ViewerBase::ON_DEMAND)
		//	{
		//		if (m_rpCompositeViewer->checkNeedToDoFrame())
		//		{
		//			m_rpCompositeViewer->frame();
		//		}
		//		else
		//		{
		//			// we don't need to render a frame but we don't want to spin the run loop so make sure the minimum
		//			// loop time is 1/100th of second, if not otherwise set, so enabling the frame microSleep below to
		//			// avoid consume excessive CPU resources.
		//			if (m_dMinFrameTime==0.0) m_dMinFrameTime=0.01;
		//		}
		//	}
		//	else
		//	{
		//		m_rpCompositeViewer->frame();
		//	}

		//	// work out if we need to force a sleep to hold back the frame rate
		//	osg::Timer_t endFrameTick = osg::Timer::instance()->tick();
		//	double frameTime = osg::Timer::instance()->delta_s(startFrameTick, endFrameTick);
		//	if (frameTime < m_dMinFrameTime) OpenThreads::Thread::microSleep(static_cast<unsigned int>(1000000.0*(m_dMinFrameTime-frameTime)));
		//}

		PostFrame();

		//if ((m_rpCompositeViewer->getRunFrameScheme() == osgViewer::ViewerBase::CONTINUOUS || m_rpCompositeViewer->checkNeedToDoFrame())
		//	&& !m_rpCompositeViewer->done())
		//{
		//	PreFrame();
		//	m_rpCompositeViewer->frame();
		//	PostFrame();
		//}
	}

	bool CSystemManager::SetGraphicContext(osg::GraphicsContext* pContext)
	{
		if (pContext && m_rpViewer.valid())
		{
			osg::Camera* pCamera = m_rpViewer->getCamera();
			if (pCamera->getGraphicsContext() != pContext)
			{
				pCamera->setGraphicsContext(pContext);
				if (!pCamera->getViewport())
				{
					pCamera->setViewport(new osg::Viewport(0, 0, pContext->getTraits()->width, pContext->getTraits()->height));
				}

				pCamera->setProjectionMatrixAsPerspective(30.0f, pCamera->getViewport()->width() / pCamera->getViewport()->height(), 1.0f, 10000.0f);
			}

			return true;
		}

		return false;
	}

	void CSystemManager::SetESCState(bool bKey)
	{
		if (m_rpCompositeViewer.valid())
		{
			if (bKey)
			{
				m_rpCompositeViewer->setKeyEventSetsDone(osgGA::GUIEventAdapter::KEY_Escape);
			}
			else
			{
				m_rpCompositeViewer->setKeyEventSetsDone(0);
			}
		}
	}

	FeShell::CSystemService* CSystemManager::GetSystemService()
	{
		return m_rpSystemService.get();
	}

	void CSystemManager::ResizeGraphicsContext(int x, int y, int w, int h)
	{
		if (m_rpSystemService.valid())
		{
			m_rpSystemService->ResizeGraphicsContext(x, y, w, h);
		}
	}

	FeUtil::CRenderContext* CSystemManager::GetRenderContext()
	{
		if (m_rpRenderContext.valid())
			return m_rpRenderContext.get();
		else
			return NULL;
	}

	osgViewer::Viewer *CSystemManager::GetViewer()
	{
		return m_rpViewer.get();
	}

	osgViewer::CompositeViewer * CSystemManager::GetCompositeViewer()
	{
		return m_rpCompositeViewer.get();
	}

	void CSystemManager::PreFrame()
	{

	}

	void CSystemManager::PostFrame()
	{

	}

	void CSystemManager::SetEarthPath( std::string strPath )
	{
		m_strEarthPath = strPath;
	}

}
