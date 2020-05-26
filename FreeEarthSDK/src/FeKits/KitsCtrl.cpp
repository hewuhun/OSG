#include <FeKits/KitsCtrl.h>

namespace FeKit
{
	CKitsCtrl::CKitsCtrl(FeUtil::CRenderContext* pContext)
		:FeUtil::CRenderContextObserver(pContext)
		,m_bShow(false)
		,m_strTitle("")
		,m_strKey("")
	{
		
	}

	CKitsCtrl::~CKitsCtrl()
	{

	}

	bool CKitsCtrl::IsHide() const
	{
		return !m_bShow;
	}

	std::string CKitsCtrl::GetTitle() const
	{
		return m_strTitle;
	}

	void CKitsCtrl::SetTitle( const std::string& strTitle )
	{
		m_strTitle = strTitle;
	}

	void CKitsCtrl::SetKey( const std::string& strKey )
	{
		m_strKey = strKey;
	}

	std::string CKitsCtrl::GetKey() const
	{
		return m_strKey;
	}

	void CKitsCtrl::ResizeKit( int nW, int nH )
	{

	}
}
