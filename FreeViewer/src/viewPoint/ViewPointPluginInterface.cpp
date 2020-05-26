#include <viewPoint/ViewPointPluginInterface.h>

#include <mainWindow/FreeToolBar.h>
#include <mainWindow/FreeMainWindow.h>

#include <iostream>

namespace FreeViewer
{
	CViewPointPluginInterface::CViewPointPluginInterface(CFreeMainWindow* pMainWindow)
		: CUIObserver(pMainWindow)
		, m_pSearchWidget(NULL)
	{
		InitWidget();
	}

	void CViewPointPluginInterface::InitWidget()
	{
		m_strMenuTitle = tr("ViewPoint");

		if (m_pMainWindow)
		{
			//视点复位
			QAction* pResetAction = CreateMenuAndToolAction(
				tr("ViewPoint Reset"),
				":/images/icon/viewpoint_reset.png", 
				":/images/icon/viewpoint_reset.png", 
				":/images/icon/viewpoint_reset_press.png",
				":/images/icon/viewpoint_reset.png",
				false);
			connect(pResetAction, SIGNAL(triggered()), this, SLOT(SlotViewPointReset()));

			//视点正北
			QAction* pNorthAction = CreateMenuAndToolAction(
				tr("ViewPoint North"),
				":/images/icon/viewpoint_north.png", 
				":/images/icon/viewpoint_north.png", 
				":/images/icon/viewpoint_north_press.png",
				":/images/icon/viewpoint_north.png",
				false);
			connect(pNorthAction, SIGNAL(triggered()), this, SLOT(SlotViewPointNorth()));

			//视点垂直
			QAction	*pVerticalAction = CreateMenuAndToolAction(
				tr("ViewPoint Vertical"),
				":/images/icon/viewpoint_vert.png", 
				":/images/icon/viewpoint_vert.png", 
				":/images/icon/viewpoint_vert_press.png",
				":/images/icon/viewpoint_vert.png",
				false);
			connect(pVerticalAction, SIGNAL(triggered()), this, SLOT(SlotViewPointVertical()));

			//视点定位
			QAction* pLocalAction = CreateMenuAndToolAction(
				tr("ViewPoint Local"),
				":/images/icon/viewpoint_locate.png", 
				":/images/icon/viewpoint_locate.png", 
				":/images/icon/viewpoint_locate_press.png",
				":/images/icon/viewpoint_locate.png",
				false);
			connect(pLocalAction, SIGNAL(triggered()), this, SLOT(SlotViewPointLocal()));

			//视角锁定
			QAction* pLockAction = CreateMenuAndToolAction(
				tr("ViewPoint Lock"),
				":/images/icon/lock_open.png",
				":/images/icon/lock_open.png",
				":/images/icon/lock.png",
				":/images/icon/lock_press.png", 
				true,
				false);
			connect(pLockAction, SIGNAL(triggered(bool)), this, SLOT(SlotViewPointLock(bool)));

			//添加工具条分割线
			CFreeToolBar* pToolBar = GetToolBar();
			pToolBar->AddSperator();
		}

		BuildContext();
	}

	void CViewPointPluginInterface::BuildContext()
	{
		if (m_opSystemService.valid())
		{
			m_opManipulatorManager = m_opSystemService->GetManipulatorManager();
		}
	}

	void CViewPointPluginInterface::SlotViewPointReset()
	{
		m_opManipulatorManager->Home();
	}

	void CViewPointPluginInterface::SlotViewPointNorth()
	{
		m_opManipulatorManager->ToNorthViewPoint(0.5);
	}

	void CViewPointPluginInterface::SlotViewPointVertical()
	{
		m_opManipulatorManager->ToVerticalViewPoint(0.5);
	}

	void CViewPointPluginInterface::SlotViewPointLocal()
	{
		if(!m_pSearchWidget && m_opSystemService.valid())
		{
			m_pSearchWidget = new CLocateWidget(m_pMainWindow);
			m_pSearchWidget->SetSystemService(m_opSystemService.get());
			m_pSearchWidget->ShowDialogNormal();
		}
		else
		{
			m_pSearchWidget->ShowDialogNormal();
		}
	}

	void CViewPointPluginInterface::SlotViewPointLock(bool bLock)
	{
		m_opManipulatorManager->LockViewPoint(bLock);
	}

	CViewPointPluginInterface::~CViewPointPluginInterface(void)
	{
		if (m_pSearchWidget)
		{
			delete m_pSearchWidget;
			m_pSearchWidget = NULL;
		}
	}
}

