
#ifndef FREE_STATUS_INFO_H
#define FREE_STATUS_INFO_H 1

#include <mainWindow/UIObserver.h>
#include <mainWindow/FreeStatusBar.h>

#include <FeKits/screenInfo/SceneViewInfo.h>

namespace FreeViewer
{
	class CFreeStatusInfoCallback: public FeKit::CSceneViewInfoCallback
	{
	public:
		CFreeStatusInfoCallback(CFreeStatusBar* pStatusBar);

		virtual ~CFreeStatusInfoCallback();

	public:
		virtual void UpdateViewInfo(double dLon, double dLat, double dHei, double dHeading, double dPitch, double dCamHeight);

	protected:
		CFreeStatusBar*					m_pFreeStatusBar;
	};
}


#endif
