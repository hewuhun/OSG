#include <FeUtils/GeneratorID.h>

namespace FeUtil
{
	CUIDGenerator* CUIDGenerator::m_pInstance = NULL;
	CUIDGenerator::CDestoryInstance CUIDGenerator::destroy;

	CUIDGenerator* CUIDGenerator::Instance()
	{
		if(!m_pInstance)
		{
			m_pInstance = new CUIDGenerator();
		}

		return m_pInstance;
	}

	FeUtil::FEID CUIDGenerator::CreateUID()
	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> exclusive( m_IDGenMutex );
		return (FEID)( m_unId++ );
	}

	CUIDGenerator::CUIDGenerator()
		:m_unId(0)
	{

	}

	CUIDGenerator::~CUIDGenerator()
	{

	}

	bool CUIDGenerator::SetOriginalID( FEID id )
	{
		if (m_unId > id)
		{
			return false;
		}
		m_unId = id;

		return true;
	}
}
