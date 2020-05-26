#include <FeShell/SmartToolSys.h>


namespace FeShell
{
	CSmartToolSys::CSmartToolSys( )
		:FeUtil::CSceneSysCall()
	{
		SetSysKey(SMART_TOOL_SYSTEM_CALL_DEFAULT_KEY);
	}


	CSmartToolSys::~CSmartToolSys()
	{

	}

	bool CSmartToolSys::InitiliazeImplement()
	{
		m_mapTool.clear();

		return true;
	}

	bool CSmartToolSys::UnInitiliazeImplement()
	{
		m_mapTool.clear();

		return true;
	}

	bool CSmartToolSys::AddSmartTool( FeKit::CKitsCtrl* pKitsCtrl , bool flag )
	{
		if(pKitsCtrl && !m_mapTool.count(pKitsCtrl->GetKey()))
		{
			if (flag)
			{
				m_mapTool[pKitsCtrl->GetKey()] = pKitsCtrl;
			}

			osg::Node* pNode = dynamic_cast<osg::Node*>(pKitsCtrl);
			m_opSystemCallRoot->addChild(pNode);

			return true;
		}

		return false;
	}

	FeKit::CKitsCtrl* CSmartToolSys::GetSmartTool(const std::string& strKey)
	{
		if(m_mapTool.count(strKey))
		{
			return m_mapTool[strKey];
		}

		return NULL;
	}

	bool CSmartToolSys::RemoveSmartTool(FeKit::CKitsCtrl* pKitsCtrl)
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

	FeShell::CSmartToolSys::SmartToolList CSmartToolSys::GetSmartTools() const
	{
		return m_mapTool;
	}

	void CSmartToolSys::ResizeSys( int nW, int nH )
	{
		SmartToolList::iterator itr = m_mapTool.begin();
		for(itr; itr != m_mapTool.end(); ++itr)
		{
			if(itr->second)
			{
				itr->second->ResizeKit(nW, nH);
			}
		}
	}

}