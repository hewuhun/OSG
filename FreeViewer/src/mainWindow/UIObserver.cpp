#include <mainWindow/UIObserver.h>

#include <mainWindow/FreeSceneWidget.h>

#include <mainWindow/FreeMainWindow.h>
#include <mainWindow/FreeMenuWidget.h>
#include <mainWindow/FreeToolBar.h>
#include <mainWindow/FreeToolButton.h>
#include <mainWindow/FreeStatusBar.h>
#include <mainWindow/FreeDockFrame.h>
#include <mainWindow/FreeUtil.h>

namespace FreeViewer
{
	CUIObserver::CUIObserver( CFreeMainWindow* pMainWindow )
		:QObject()
		,m_pMainWindow(pMainWindow)
	{
		if (m_pMainWindow)
		{
			FeEarth::C3DSceneWidget* p3DSceneWidget = m_pMainWindow->Get3DSceneWidget();
			if (p3DSceneWidget)
			{
				FeShell::CSystemManager* pSystemManager = p3DSceneWidget->GetSystemManager();
				if (pSystemManager)
				{
					m_opSystemService = pSystemManager->GetSystemService();
				}
			}
		}
	}

	CUIObserver::~CUIObserver()
	{
		m_opSystemService = NULL;
	}

	CFreeStatusBar* CUIObserver::GetStatusBar()
	{
		if (NULL == m_pMainWindow)
		{
			return NULL;
		}
		
		return m_pMainWindow->GetStatusBar();
	}

	FeShell::CSystemService* CUIObserver::GetSystemService()
	{
		return m_opSystemService.get();
	}

	CDockFrame* CUIObserver::GetDockWidget()
	{
		if (NULL == m_pMainWindow)
		{
			return NULL;
		}

		return m_pMainWindow->GetDockWidget();
	}

	QAction* CUIObserver::CreateMenuAndToolAction( 
		QString strName,
		QString strNormalIcon, 
		QString strHoverIcon,
		QString strPressIcon, 
		QString strDisableIcon,
		bool bCheckable,
		bool bChecked )
	{
		if (NULL == m_pMainWindow)
		{
			return NULL;
		}

		CFreeMenuWidget* pMenuWidget = m_pMainWindow->GetMenuWidget();
		if (NULL ==pMenuWidget)
		{
			return NULL;
		}

		//创建菜单栏的QAction，并添加到菜单中
		QAction* pAction = NULL;
		QMenu* pMenu = pMenuWidget->GetOrCreateMenu(m_strMenuTitle);
		if (pMenu)
		{
			pAction = new QAction(strName, pMenu);
			pAction->setCheckable(bCheckable);
			pAction->setChecked(bChecked);
			pMenu->addAction(pAction);
		}

		//创建工具栏按钮，并添加到工具栏中
		CFreeToolButton* pToolBtn = NULL;
		CFreeToolBar* pToolBar = pMenuWidget->GetToolBar();
		if (pToolBar)
		{
			pToolBtn = new CFreeToolButton(
				strName,
				strNormalIcon,
				strHoverIcon,
				strPressIcon,
				strDisableIcon,
				pToolBar
				);
			pToolBtn->setCheckable(bCheckable);
			pToolBtn->setChecked(bChecked);
			pToolBar->AddToolButton(pToolBtn);
		}

		//同步工具栏按钮和菜单栏QAction的状态
		connect(pToolBtn, SIGNAL(clicked(bool)), pAction, SLOT(setChecked(bool)));
		connect(pToolBtn, SIGNAL(clicked(bool)), pAction, SIGNAL(triggered(bool)));
		connect(pAction, SIGNAL(toggled(bool)), pToolBtn, SLOT(setChecked(bool)));
	
		return pAction;
	}

	QAction* CUIObserver::CreateMenuAction( 
		QString strName,
		bool bCheckable,
		bool bChecked )
	{
		if (NULL == m_pMainWindow)
		{
			return NULL;
		}

		CFreeMenuWidget* pMenuWidget = m_pMainWindow->GetMenuWidget();
		if (NULL ==pMenuWidget)
		{
			return NULL;
		}

		//创建菜单栏的QAction，并添加到菜单中
		QAction* pAction = NULL;
		QMenu* pMenu = pMenuWidget->GetOrCreateMenu(m_strMenuTitle);
		if (pMenu)
		{
			pAction = new QAction(strName, pMenu);
			pAction->setCheckable(bCheckable);
			pAction->setChecked(bChecked);
			pMenu->addAction(pAction);
		}

		return pAction;
	}

	CFreeToolButton* CUIObserver::CreateToolBtn( 
		QString strName,
		QString strNormalIcon,
		QString strHoverIcon, 
		QString strPressIcon,
		QString strDisableIcon,
		bool bCheckable,
		bool bChecked )
	{
		if (NULL == m_pMainWindow)
		{
			return NULL;
		}

		CFreeMenuWidget* pMenuWidget = m_pMainWindow->GetMenuWidget();
		if (NULL ==pMenuWidget)
		{
			return NULL;
		}

		//创建工具栏按钮，并添加到工具栏中
		CFreeToolButton* pToolBtn = NULL;
		CFreeToolBar* pToolBar = pMenuWidget->GetToolBar();
		if (pToolBar)
		{
			pToolBtn = new CFreeToolButton(
				strName,
				strNormalIcon,
				strHoverIcon,
				strPressIcon,
				strDisableIcon,
				pToolBar
				);
			pToolBtn->setCheckable(bCheckable);
			pToolBtn->setChecked(bChecked);
			pToolBar->AddToolButton(pToolBtn);
		}

		return pToolBtn;
	}

	CFreeToolBar* CUIObserver::GetToolBar()
	{
		if(NULL == m_pMainWindow)
		{
			return NULL;
		}

		CFreeMenuWidget* pMenuWidget = m_pMainWindow->GetMenuWidget();
		if (NULL == pMenuWidget)
		{
			return NULL;
		}

		return pMenuWidget->GetToolBar();
	}

}

