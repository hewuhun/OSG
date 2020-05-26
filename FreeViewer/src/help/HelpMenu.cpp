#include <help/HelpMenu.h>

#include <mainWindow/FreeMainWindow.h>
#include <mainWindow/FreeDialog.h>
#include <FeUtils/PathRegistry.h>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>

namespace FreeViewer
{
	CHelpMenu::CHelpMenu( CFreeMainWindow* pMainWindow)
		: CUIObserver(pMainWindow)
		, m_pAboutDlg(NULL)
	{
		m_strMenuTitle = QString(tr("Help"));

		InitWidget();		
	}

	CHelpMenu::~CHelpMenu()
	{

	}

	void CHelpMenu::InitWidget()
	{
		m_pActionHelp = CreateMenuAction(tr("FreeViewer Help"));
		//m_pActionHelp->setShortcut(Qt::Key_F1);
		connect(m_pActionHelp, SIGNAL(triggered(bool)), this, SLOT(SlotOpenHelp()));

		m_pOfficeWebsite = CreateMenuAction(tr("office website"));
	//	m_pOfficeWebsite->setShortcut(Qt::ALT | Qt::Key_W);
		connect(m_pOfficeWebsite, SIGNAL(triggered(bool)), this, SLOT(SlotWebsite()));

		m_pAboutDlg = new CAboutWidget(m_pMainWindow);

#if _WIN32
		m_pActionUpdate = CreateMenuAction(tr("online update"));
	//	m_pActionUpdate->setShortcut(Qt::ALT | Qt::Key_U);
		connect(m_pActionUpdate, SIGNAL(triggered(bool)), this, SLOT(SlotOnlineUpdate()));
#endif

		m_pActionAbout = CreateMenuAction(tr("About"));
	//	m_pActionAbout->setShortcut(Qt::ALT | Qt::Key_O);
		connect(m_pActionAbout, SIGNAL(triggered(bool)), this, SLOT(SlotAbout()));
	}

	void CHelpMenu::SlotAbout()
	{
		if(m_pAboutDlg->isHidden())
		{
			m_pAboutDlg->ShowDialogNormal();
		}
	}

	void CHelpMenu::SlotOpenHelp()
	{
		QString strHelp = FeFileReg->GetFullPath("doc/FreeViewer_Help.pdf").c_str();
		QDesktopServices::openUrl(QUrl::fromLocalFile(strHelp));	
	}

	void CHelpMenu::SlotOnlineUpdate()
	{
		QProcess pro;
		QString strUpdate = "../updater.exe";//FeFileReg->GetFullPath("update/updater.exe").c_str();
		pro.startDetached(strUpdate);
	}

	void CHelpMenu::SlotWebsite()
	{
		QString strWeb= "www.henggetec.com";
		QDesktopServices::openUrl(QUrl(strWeb));
	}

}

