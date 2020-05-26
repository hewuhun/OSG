#include <FeKits/navigator/Navigator.h>

#include <osgDB/ReadFile>
#include <osg/Texture2D>

#include <FeKits/navigator/NavigatorCtrl.h>
#include "NavigatorPart.h"

namespace FeKit
{
	CNavigator::CNavigator( 
		FeUtil::CRenderContext* pContext,
		int nXOffset, 
		int nYOffset, 
		int nWidth, 
		int nHeight, 
		CPlaceStrategy::KIT_PLACE ePlace, 
		CNavigatorCtrl* pNavigatorCtrl, 
		const std::string& strRes /*= ""*/ )
		:CScreenKits(pContext, nXOffset, nYOffset, nWidth, nHeight, ePlace)
		,m_rpCtrl(pNavigatorCtrl)
		,m_opNVPart(NULL)
	{
		m_opNVPart = new CNavigatorPart(m_dLeftRate, m_dRightRate, m_dBottomRate, m_dTopRate, strRes);
		m_opNVPart->UpdatePos(m_nX, m_nY, m_nWidth, m_nHeight, m_dWidthRate, m_dHeightRate);
		addChild(m_opNVPart.get());

		if(m_rpCtrl.valid())
		{
			m_opNVPart->SetCtrl(m_rpCtrl.get());
			m_rpCtrl->AddActiveComponent(m_opNVPart.get());

			if(!IsHide())
			{
				Show();
			}
		}
		if(m_opRenderContext.valid()) 
		{
			m_opView = m_opRenderContext->GetView();
		}
	}

	CNavigator::~CNavigator( void )
	{

	}

	void CNavigator::SetPosition( int nX, int nY, int nWidth, int nHeight )
	{
		CScreenKits::SetPosition(nX, nY, nWidth, nHeight);

		if(m_opNVPart.valid())
		{
			m_opNVPart->UpdatePos(m_nX, m_nY, m_nWidth, m_nHeight, m_dWidthRate, m_dHeightRate);
		}
	}

	void CNavigator::Show()
	{
		CScreenKits::Show();

		if(m_rpCtrl.valid() && m_opView.valid())
		{
			m_opView->addEventHandler(m_rpCtrl.get());
		}
	}

	void CNavigator::Hide()
	{
		if(m_rpCtrl.valid() && m_opView.valid())
		{
			m_opView->removeEventHandler(m_rpCtrl.get());
		}
		CScreenKits::Hide();
	}
}
