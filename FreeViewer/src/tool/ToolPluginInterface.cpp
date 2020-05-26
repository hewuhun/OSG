#include <QMessageBox>
#include <QStatusBar>
#include <QFileDialog>
#include <QDesktopWidget>
#include <FeUtils/StrUtil.h>

#include <mainWindow/FreeMainWindow.h>

#include <tool/ToolPluginInterface.h>

namespace FreeViewer
{
	CToolPluginInterface::CToolPluginInterface(CFreeMainWindow* pMainWindow)
		: CUIObserver(pMainWindow)
		, m_pLanguageWidget(NULL)
		, m_pBlackBoard(NULL)
		, m_pControlBtns(NULL)
		, m_pDropScreen(NULL)
	{
		m_strMenuTitle = tr("Tool");

		BuildContext();
	}

	void CToolPluginInterface::BuildContext()
	{
		if (m_pMainWindow)
		{
			//黑板功能选项框显示
			QAction* pBlackBoardAction = CreateMenuAction(tr("BlackBoard"), false, false);
			connect(pBlackBoardAction, SIGNAL(triggered(bool)), this, SLOT(SlotBlackBoard(bool)));

			//截屏功能
			QAction	*pDropScreenAction = CreateMenuAction(tr("DropScreen"), false, false);
			connect(pDropScreenAction, SIGNAL(triggered()), this, SLOT(SlotDropScreen()));

			//语言选项功能选项框显示
			QAction* pOptionAction = CreateMenuAction(tr("Language"), false, false);
			connect(pOptionAction, SIGNAL(triggered()), this, SLOT(SlotOpenOptionsSet()));
		}
	}

	//语言选择功能选项显示
	void CToolPluginInterface::SlotOpenOptionsSet()
	{
		if(!m_pLanguageWidget && m_opSystemService.valid())
		{
			m_pLanguageWidget = new CLanguageWidget(m_pMainWindow);
			m_pLanguageWidget->show();
		}
		else
		{
			m_pLanguageWidget->show();
		}
	}
	
	void CToolPluginInterface::SlotBlackBoard(bool bAction)
	{
		if(!m_pBlackBoard)
		{
			m_pBlackBoard = new CBlackBoard(m_pMainWindow);
			m_pBlackBoard->setGeometry(m_pMainWindow->x(), m_pMainWindow->y(), m_pMainWindow->width(), m_pMainWindow->height());
			m_pBlackBoard->setAttribute(Qt::WA_TranslucentBackground, true);
			m_pBlackBoard->setWindowFlags(Qt::Tool);

			m_pControlBtns = new CBlackBoardControlBtns(m_pMainWindow);
			m_pControlBtns->setGeometry(m_pMainWindow->x(), m_pMainWindow->y(), m_pMainWindow->width(), m_pMainWindow->height());

			m_pControlBtns->SetBlackBoardWidget(m_pBlackBoard);
		}

		if(m_pBlackBoard->isHidden())
		{
			m_pBlackBoard->setGeometry(m_pMainWindow->x(), m_pMainWindow->y(), m_pMainWindow->width(), m_pMainWindow->height());
			m_pControlBtns->setGeometry(m_pMainWindow->x(), m_pMainWindow->y(), m_pMainWindow->width(), m_pMainWindow->height());

			m_pBlackBoard->show();

			m_pControlBtns->setFixedWidth(50);
			m_pControlBtns->move(m_pMainWindow->mapToGlobal(
				QPoint(m_pMainWindow->width() - m_pControlBtns->width(), m_pMainWindow->height() - m_pControlBtns->height())));
			m_pControlBtns->show();
		}
		else
		{
			m_pBlackBoard->hide();
			m_pControlBtns->hide();
		}
	}

	void CToolPluginInterface::SlotDropScreen()
	{
		if (!m_pDropScreen)
		{
			m_pDropScreen = new CDropScreen(m_opSystemService.get());		
		}
		QString filePath;
#ifdef WIN32
		QFileDialog* pFileDialog = new QFileDialog;
		pFileDialog->setAttribute(Qt::WA_DeleteOnClose, true);
		filePath = pFileDialog->getSaveFileName(0, tr("Save File"), FeFileReg->GetDataPath().c_str(), tr("(*.bmp)"));
#else
		QFileDialog fileDialog(0, tr("Save File"), FeFileReg->GetDataPath().c_str(), tr("(*.bmp)"));
		if (fileDialog.exec())
		{
			filePath = fileDialog.selectedFiles().first();
			filePath += ".bmp";
		}
#endif
		m_pDropScreen->DropScrren(filePath.toStdString());

		delete m_pDropScreen;
		m_pDropScreen = NULL;
	}

	CToolPluginInterface::~CToolPluginInterface()
	{
		if (m_pLanguageWidget)
		{
			delete m_pLanguageWidget;
			m_pLanguageWidget = NULL;
		}
		if (m_pBlackBoard)
		{
			delete m_pBlackBoard;
			m_pBlackBoard = NULL;
		}
		if (m_pControlBtns)
		{
			delete m_pControlBtns;
			m_pControlBtns = NULL;
		}
		if (m_pDropScreen)
		{
			delete m_pDropScreen;
			m_pDropScreen = NULL;
		}
	}
}

