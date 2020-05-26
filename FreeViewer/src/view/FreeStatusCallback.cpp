#include <view/FreeStatusCallback.h>

namespace FreeViewer
{
	CFreeStatusInfoCallback::CFreeStatusInfoCallback( CFreeStatusBar* pStatusBar )
		:CSceneViewInfoCallback()
		,m_pFreeStatusBar(pStatusBar)
	{

	}

	CFreeStatusInfoCallback::~CFreeStatusInfoCallback()
	{

	}

	void CFreeStatusInfoCallback::UpdateViewInfo( double dLon, double dLat, double dHei, double dHeading, double dPitch, double dCamHeight )
	{
		if (m_pFreeStatusBar)
		{
			QString strLon = QString("%1").arg(QString::number(dLon, 'f', 6), 11);
			QString strLat = QString("%1").arg(QString::number(dLat, 'f', 6), 10);
			QString strHei = QString("%1").arg(QString::number(dHei, 'f', 2), 8);
			QString strCamHei = QString("%1").arg(QString::number(dCamHeight, 'f', 2), 11);

			m_pFreeStatusBar->SetViewText(strLon, strLat, strHei, strCamHei);
		}
	}

}

