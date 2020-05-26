#include <FeShell/SystemService.h>

#include <FeUtils/StrUtil.h>
#include <FeUtils/CoordConverter.h>

#include <FeKits/manipulator/ManipulatorUtil.h>
#include <FeKits/manipulator/FreeEarthManipulator.h>

namespace FeShell
{
	CSystemService::CSystemService( )
		:osg::Referenced()
		,m_opEnvironmentSys(NULL)
		,m_opSmartToolSys(NULL)
		,m_opRenderContext(NULL)
	{

	}

	CSystemService::~CSystemService()
	{

	}

	bool CSystemService::InitService( FeUtil::CRenderContext* pRenderContext, const CSysConfig& sysConfig )
	{
		m_opRenderContext = pRenderContext;
		m_sysConfig = sysConfig;

		return InitSysModule();
	}

	FeUtil::CFreeViewPoint CSystemService::GetViewPoint()
	{
		FeUtil::CFreeViewPoint freeViewPoint;

		if(GetManipulatorManager())
		{
			freeViewPoint.SetViewPoint(GetManipulatorManager()->GetViewPoint());
		}

		return freeViewPoint;
	}

	bool CSystemService::Locate( const std::string& strValue )
	{
		double dRange = 0.0;
		osg::Vec3d vecLLH(0.0, 0.0, 0.0);
		std::vector<std::string> vectempLLH;
		FeUtil::StringSplit(strValue, ",", vectempLLH);

		if(vectempLLH.size() >= 2) //有经纬度
		{
			vecLLH.x() = osgEarth::as<double>(*vectempLLH.begin(), 0.0);
			vecLLH.y() = osgEarth::as<double>(*(vectempLLH.begin() + 1), 0.0);

			if(vectempLLH.size() >= 3) //有经纬高
			{
				dRange = osgEarth::as<double>(*(vectempLLH.begin() + 2), 0.0);
			}

			FeUtil::CFreeViewPoint viewPoint;

			if (vectempLLH.size() >= 4) // 设置运行时间
			{
				viewPoint.SetTime(osgEarth::as<double>(*(vectempLLH.begin() + 3), 0.0));
			}

			//ViewPoint中dHei代表的是相机的高度
			viewPoint.SetViewPoint(osg::Vec4d(vecLLH.x(), vecLLH.y(), dRange, dRange));
			return Locate(viewPoint);
		}
		return false;
	}

	bool CSystemService::Locate( const FeUtil::CFreeViewPoint& sViewPoint, bool bSetHome /*= false*/ )
	{
		if(GetManipulatorManager())
		{
			GetManipulatorManager()->Locate(sViewPoint.GetEarthVP(), sViewPoint.GetTime());
			if (bSetHome)
			{
				GetManipulatorManager()->SetHome(sViewPoint.GetEarthVP(), sViewPoint.GetTime());
			}
		}
		return true;
	}

	bool CSystemService::Home()
	{
		if(GetManipulatorManager())
		{
			GetManipulatorManager()->Home();
		}

		return true;
	}

	const CSysConfig& CSystemService::GetSysConfig() const
	{
		return m_sysConfig;
	}

	void CSystemService::SetSysConifig( const CSysConfig& sysConfig )
	{
		m_sysConfig = sysConfig;
	}

	bool CSystemService::AddEventHandler(osgGA::GUIEventHandler* pHandler)
	{
		return m_opRenderContext->AddEventHandler(pHandler);
	}
	bool CSystemService::RemoveEventHandler(osgGA::GUIEventHandler* pHandler)
	{
		return m_opRenderContext->RemoveEventHandler(pHandler);
	}

	bool CSystemService::AddAppModuleSys( FeUtil::CSceneSysCall* pSysCall )
	{
		if(!pSysCall || m_mapSysCall.count(pSysCall->GetSysKey()))
		{
			return false;
		}

		m_mapSysCall[pSysCall->GetSysKey()] = pSysCall;

		return true;
	}

	bool CSystemService::RemoveModuleSys( FeUtil::CSceneSysCall* pSysCall )
	{
		if(pSysCall && m_mapSysCall.count(pSysCall->GetSysKey()))
		{
			m_mapSysCall.erase(pSysCall->GetSysKey());
			return true;
		}

		return false;
	}

	FeUtil::CSceneSysCall* CSystemService::GetModuleSys( const std::string& strTitle )
	{
		if(m_mapSysCall.count(strTitle))
		{
			return m_mapSysCall[strTitle];
		}

		return NULL;
	}

	void CSystemService::ResizeGraphicsContext(int x,int y,int w,int h)
	{
		for(VecSysCall::iterator itr = m_mapSysCall.begin(); itr != m_mapSysCall.end(); ++itr)
		{
			if(itr->second.valid())
			{
				itr->second->ResizeSys(w, h);
			}
		}
	}

	bool CSystemService::RemoveAllModule()
	{
		for(VecSysCall::iterator itr = m_mapSysCall.begin(); itr != m_mapSysCall.end(); ++itr)
		{
			if((itr->second).valid())
			{
				(itr->second)->Stop();
				(itr->second)->UnInitialize();
			}
		}

		m_mapSysCall.clear();

		return true;
	}

	bool CSystemService::Save()
	{
		return true;
	}

	bool CSystemService::SaveAs()
	{
		return true;
	}

	void CSystemService::SetStereo(bool bState)
	{
		osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
		ds->setStereo(!osg::DisplaySettings::instance()->getStereo());
	}

	bool CSystemService::GetStereo() const
	{
		osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
		return ds->getStereo();
	}

	CEnvironmentSys* CSystemService::GetEnvironmentSys()
	{
		return m_opEnvironmentSys.get();
	}

	CSmartToolSys* CSystemService::GetSmartToolSys()
	{
		return m_opSmartToolSys.get();
	}

	bool CSystemService::InitSysModule()
	{
		m_opEnvironmentSys = new FeShell::CEnvironmentSys();
		m_opEnvironmentSys->Initialize(m_opRenderContext.get());
		AddAppModuleSys(m_opEnvironmentSys.get());

		m_opSmartToolSys = new FeShell::CSmartToolSys();
		m_opSmartToolSys->Initialize(m_opRenderContext.get());
		AddAppModuleSys(m_opSmartToolSys.get());

		return true;
	}

	int CSystemService::GetRenderWidth() const
	{
		if(m_opRenderContext.valid())
		{
			return m_opRenderContext->GetGraphicsWindow()->getTraits()->width;
		}

		return 0;
	}

	int CSystemService::GetRenderHeight() const
	{
		if(m_opRenderContext.valid())
		{
			return m_opRenderContext->GetGraphicsWindow()->getTraits()->height;
		}

		return 0;
	}

	FeKit::CManipulatorManager* CSystemService::GetManipulatorManager()
	{
		if(m_opRenderContext.valid() && m_opRenderContext->GetView())
		{
			return dynamic_cast<FeKit::CManipulatorManager*>(m_opRenderContext->GetView()->getCameraManipulator());
		}

		return NULL;
	}
}