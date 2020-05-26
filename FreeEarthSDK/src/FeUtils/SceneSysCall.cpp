
#include <FeUtils/SceneSysCall.h>

namespace FeUtil
{

	CSceneSysCall::CSceneSysCall( )
		:m_opRenderContext(NULL)
		,m_opSystemCallRoot(NULL)
		,m_strKey(SYSTEM_CALL_DEFAULT_KEY)
		,m_bInitlized(false)
	{

	}

	CSceneSysCall::~CSceneSysCall()
	{

	}

	bool CSceneSysCall::Initialize(FeUtil::CRenderContext* pRenderContext)
	{
		m_opRenderContext = pRenderContext;
		if(!m_opRenderContext.valid()) return false;
		if(!m_opRenderContext->GetRoot()) return false;

		m_opSystemCallRoot = new osg::Group();
		m_opSystemCallRoot->setName("SceneSystemCallName");
		m_opRenderContext->GetRoot()->addChild(m_opSystemCallRoot.get());

		if(!InitiliazeImplement()) return false;
		m_bInitlized = true;

		return true;
	}

	bool CSceneSysCall::UnInitialize()
	{
		if(!m_opRenderContext.valid()) return false;
		if(!m_opRenderContext->GetRoot()) return false;

		if(!UnInitiliazeImplement()) return false;

		m_opRenderContext->GetRoot()->removeChild(m_opSystemCallRoot.get());
		m_bInitlized = false;
		m_opSystemCallRoot = NULL;
		m_opRenderContext = NULL;
		return true;
	}

	void CSceneSysCall::SetSysKey( const std::string& strKey )
	{
		m_strKey = strKey;
	}

	std::string CSceneSysCall::GetSysKey() const
	{
		return m_strKey;
	}

	void CSceneSysCall::SetInit( bool bState )
	{
		m_bInitlized = bState;
	}

	bool CSceneSysCall::IsInit() const
	{
		return m_bInitlized;
	}

	osg::Group* CSceneSysCall::GetSysGroup()
	{
		return m_opSystemCallRoot.get();
	}



}