
#include <FeEarth/FreeEarth.h>
#include <osgViewer/Renderer>
#include <FeKits/manipulator/FreeEarthManipulator.h>

namespace FeEarth
{
	CFreeEarth::CFreeEarth( const std::string& strResorcePath )
		:FeShell::CSystemManager(strResorcePath)
	{

	}

	CFreeEarth::~CFreeEarth( void )
	{

	}

	bool CFreeEarth::Initialize()
	{
		if(FeShell::CSystemManager::Initialize())
		{
			if(m_rpSystemService.valid() && m_rpSystemService->GetManipulatorManager())
			{
				m_rpSystemService->Home();
			}

			return true;
		}

		return false;
	}

	bool CFreeEarth::UnInitialize()
	{
		return FeShell::CSystemManager::UnInitialize();
	}

	void CFreeEarth::PreFrame()
	{
		CSystemManager::PreFrame();

		//resetUseCustomNearFar();
	}

	void CFreeEarth::resetClampProjectionMatrixCallback()
	{
		osg::Camera* cam = m_rpRenderContext->GetCamera();
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
						cv->setClampProjectionMatrixCallback(NULL);
					}
				}
			}
		}	
	}

	void CFreeEarth::resetUseCustomNearFar()
	{
		//if(m_rpRenderContext.valid())
		//{
		//	osg::Camera* cam = m_rpRenderContext->GetCamera();
		//	if(cam)
		//	{
		//		FeKit::CEarthClampProjectionMatrixCallback* pc = dynamic_cast<FeKit::CEarthClampProjectionMatrixCallback*>(cam->getClampProjectionMatrixCallback());
		//		if(pc)
		//		{
		//			pc->SetUseCustomNearFar(false);
		//		}
		//	}
		//}
	}

	void CFreeEarth::PostFrame()
	{
		CSystemManager::PostFrame();
	}


}


