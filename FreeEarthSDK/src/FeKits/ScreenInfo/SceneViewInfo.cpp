#include <FeKits/screenInfo/SceneViewInfo.h>

#include <FeKits/manipulator/ManipulatorUtil.h>

#include <FeUtils/MathUtil.h>

namespace FeKit
{
	CSceneViewInfo::CSceneViewInfo(FeUtil::CRenderContext* pContext)
		:osg::Referenced()
		,m_opRenderContext(pContext)
		,m_rpHandler(NULL)
		,m_dPreMouseX(0.0)
		,m_dPreMouseY(0.0)
		,m_dLongitude(0.0)
		,m_dLatitude(0.0)
		,m_dHeight(0.0)
	{
		if(m_opRenderContext.valid())
		{
			m_opInfoMaker = new CCommonInfo(m_opRenderContext.get());

			if (m_opRenderContext->GetView())
			{
				m_rpHandler = new CSceneViewInfoHandler(this);
				m_opRenderContext->GetView()->addEventHandler(m_rpHandler.get());
			}
		}
	}

	CSceneViewInfo::~CSceneViewInfo(void)
	{
		m_listCallback.clear();
	}

	void CSceneViewInfo::AddCallback( CSceneViewInfoCallback* pCallback )
	{
		m_listCallback.push_back(pCallback);
	}

	void CSceneViewInfo::RemoveCallback( CSceneViewInfoCallback* pCallback )
	{
		SceneViewInfoCallbacks::iterator it = m_listCallback.begin();
		while(it != m_listCallback.end())
		{
			if (*it == pCallback)
			{
				m_listCallback.erase(it);
				break;
			}
			it++;
		}
	}

	void CSceneViewInfo::UpdateInfo(double dMouseX, double dMouseY)
	{
		if(!m_opInfoMaker.valid()) return;

		//获取相机信息
		double dHeading=0, dPitch=0, dRange=0;
		m_opInfoMaker->GetPosture(dPitch, dHeading, dRange);

		//获取鼠标当前位置的地理信息
		if(!FeMath::Equal(m_dPreMouseX, dMouseX) ||!FeMath::Equal(m_dPreMouseY, dMouseY))
		{
			m_dPreMouseX = dMouseX;
			m_dPreMouseY = dMouseY;
			m_opInfoMaker->GetLLH(m_dPreMouseX, m_dPreMouseY, m_dLongitude, m_dLatitude, m_dHeight);
		}

		//开始回调
		for (int i = 0; i < m_listCallback.size(); i++)
		{
			m_listCallback.at(i)->UpdateViewInfo(m_dLongitude, m_dLatitude, m_dHeight, dHeading, dPitch, dRange);
		}
	}

}

namespace FeKit
{
	/////////////////////////////////////////////
	//CSceneViewInfoHandler
	/////////////////////////////////////////////
	CSceneViewInfoHandler::CSceneViewInfoHandler( CSceneViewInfo* pSInfo )
		:m_opSceneViewInfo(pSInfo)
	{

	}

	CSceneViewInfoHandler::~CSceneViewInfoHandler()
	{

	}


	bool CSceneViewInfoHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
	{
		if (ea.getEventType() != osgGA::GUIEventAdapter::FRAME)
		{
			if (m_opSceneViewInfo.valid())
			{
				m_opSceneViewInfo->UpdateInfo(ea.getX(), ea.getY());
			}
		}

		return false;
	}
}

