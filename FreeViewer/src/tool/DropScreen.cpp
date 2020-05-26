#include <tool/DropScreen.h>

namespace FreeViewer	
{
	CDropScreen::CDropScreen(FeShell::CSystemService* pSystemService)
		:m_rpCaptureCallback(NULL)
		,m_opSystemService(pSystemService)
		,m_opLastDrawCallback(NULL)
	{
		osg::Camera* pCamera = m_opSystemService->GetRenderContext()->GetCamera();
		if (pCamera)
		{
			m_rpCaptureCallback = new FeKit::CCaptureDrawCallback();

			m_opLastDrawCallback = pCamera->getPostDrawCallback();
			pCamera->setPostDrawCallback(m_rpCaptureCallback.get());	
		}
	}

	CDropScreen::~CDropScreen()
	{
		m_rpCaptureCallback = NULL;
	}

	void CDropScreen::DropScrren( const std::string& strPath)
	{
		if (m_rpCaptureCallback.valid())
		{	
			m_rpCaptureCallback->WriteImage(strPath);
		}

		osg::Camera* camera = m_opSystemService->GetRenderContext()->GetCamera();
		if (camera)
		{
			camera->setPostDrawCallback(m_opLastDrawCallback.get());	
		}
	}
}
