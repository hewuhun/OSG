#include <osgEarth/DateTime>

#include <FeUtils/PathRegistry.h>
#include <FeUtils/TimeUtil.h>
#include <FeShell/EnvironmentSys.h>

namespace FeShell
{
	CEnvironmentSys::CEnvironmentSys( )
		:FeUtil::CSceneSysCall()
		,m_opFreeSky(NULL)
	{
		SetSysKey(ENVIRONMNET_SYSTEM_CALL_DEFAULT_KEY);
	}


	CEnvironmentSys::~CEnvironmentSys()
	{

	}

	bool CEnvironmentSys::InitiliazeImplement()
	{
		m_mapTool.clear();

		m_opFreeSky = new FeKit::CFreeSky(m_opRenderContext.get());
		if(m_opFreeSky->Initialize())
		{
			std::string strTitle = "SkyNode";
			m_opFreeSky->Attach(m_opRenderContext->GetView(), 0);
			m_opFreeSky->SetAmbient(osg::Vec4(0.35, 0.35, 0.35, 1.0));
			m_opFreeSky->SetDateTime(osgEarth::DateTime());
			m_opFreeSky->SetKey(strTitle);
			m_opFreeSky->SetTitle(strTitle);
			AddSkyTool(m_opFreeSky.get());
		}

		return true;
	}

	bool CEnvironmentSys::UnInitiliazeImplement()
	{
		m_mapTool.clear();
		return true;
	}

	bool CEnvironmentSys::AddSkyTool( osg::Node* pNode, bool bAddToRoot )
	{
		FeKit::CKitsCtrl* pKit = dynamic_cast<FeKit::CKitsCtrl*>(pNode);

		if(pKit)
		{
			if(!m_mapTool.count(pKit->GetKey()))
			{
				if(m_opSystemCallRoot.valid())
				{
					if(bAddToRoot)
					{
						m_opSystemCallRoot->addChild(pNode);
					}

					m_mapTool[pKit->GetKey()] = pKit;
					return true;
				}
			}
		}

		return false;
	}


	bool CEnvironmentSys::RemoveSkyTool(FeKit::CKitsCtrl* pKitsCtrl)
	{
		if(pKitsCtrl && m_mapTool.count(pKitsCtrl->GetKey()))
		{
			m_mapTool.erase(pKitsCtrl->GetKey());

			osg::Node* pNode = dynamic_cast<osg::Node*>(pKitsCtrl);
			m_opSystemCallRoot->removeChild(pNode);

			return true;
		}

		return false;
	}

	FeShell::CEnvironmentSys::SkyToolList CEnvironmentSys::GetSkyTools() const
	{
		return m_mapTool;
	}

	FeKit::CKitsCtrl* CEnvironmentSys::GetSkyTool(const std::string& strKey)
	{
		if(m_mapTool.count(strKey))
		{
			return m_mapTool[strKey];
		}

		return NULL;
	}

	FeKit::CFreeSky* CEnvironmentSys::GetSkyNode()
	{
		return m_opFreeSky.get();
	}

}




