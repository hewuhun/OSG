#include <mainWindow/FreeMainWindow.h>

#include <mainWindow/FreeUtil.h>
#include <mainWindow/FreeMenuWidget.h>
#include <mainWindow/FreeStatusBar.h>
#include <mainWindow/FreeSceneWidget.h>
#include <mainWindow/FreeDockFrame.h>
#include <mainWindow/Free3DDockTreeWidget.h>
#include <help/HelpMenu.h>

#include <tool/ToolPluginInterface.h>
#include <view/ViewPluginInterface.h>
#include <layer/SystemLayerTree.h>
#include <layer/CustomLayerTree.h>
#include <viewPoint/ViewPointPluginInterface.h>
#include <environment/EnvironmentPluginInterface.h>
#include <viewPoint/ViewPointTree.h>

#include <FeUtils/CoordConverter.h>
#include <FeShell/SystemManager.h>

#include <QStatusBar>
#include <QPainter>

namespace FreeViewer
{
	FreeViewer::CFreeMainWindow::CFreeMainWindow(QWidget *parent)
		: CFreeFramelessWidget(parent)
		, m_pBootScreenFrame(NULL)
		, m_pFreeMenuWidget(NULL)
		, m_pStatusBar(NULL)
		, m_pDockWidget(NULL)
		, m_pSceneWidget(NULL)
		, m_p3DSceneWidget(NULL)
		, m_p3DTreeWidget(NULL)
		, m_pGraphicScene(NULL)
		, m_pGraphicView(NULL)
		, m_pMeasureWidget(NULL)
		, m_pMarkWidget(NULL)
		, m_pPlotWidget(NULL)
		, m_pServiceDockTree(NULL)
		, m_bDragLeaveFlag(false)
	{
		InitWidget();
	}

	CFreeMainWindow::~CFreeMainWindow()
	{
		PluginList::iterator itr = m_listPlugins.begin();
		for(; itr != m_listPlugins.end(); ++itr)
		{
			if(*itr)
			{
				delete (*itr);
			}
		}
		m_listPlugins.clear();

		if (m_pBootScreenFrame)
		{
			delete m_pBootScreenFrame;
			m_pBootScreenFrame = NULL;
		}

		if (m_pFreeMenuWidget)
		{
			delete m_pFreeMenuWidget;
			m_pFreeMenuWidget = NULL;
		}

		if (m_pStatusBar)
		{
			delete m_pStatusBar;
			m_pStatusBar = NULL;
		}

		if (m_pDockWidget)
		{
			delete m_pDockWidget;
			m_pDockWidget = NULL;
		}

		if (m_p3DSceneWidget)
		{
			delete m_p3DSceneWidget;
			m_p3DSceneWidget = NULL;
		}

		if (m_pSceneWidget)
		{
			delete m_pSceneWidget;
			m_pSceneWidget = NULL;
		}
	}

	void CFreeMainWindow::showEvent(QShowEvent *event)
	{
		if (m_pGraphicView)
		{
			m_pGraphicView->show();
		}

		CFreeFramelessWidget::showEvent(event);
	}

	void CFreeMainWindow::hideEvent(QHideEvent *event)
	{
		static bool g_bHidden = true;

		if (g_bHidden)
		{
			if (m_pGraphicView)
			{
				m_pGraphicView->hide();
			}

			g_bHidden = false;
		}

		CFreeFramelessWidget::hideEvent(event);
	}

	void CFreeMainWindow::InitWidget()
	{
		m_pBootScreenFrame = new FreeViewer::CBootScreenFrame(NULL);

		GetScreenGeometry();
		m_pBootScreenFrame->move(
			(GetScreenGeometry(GetPrimaryScreenIndex()).width() - m_pBootScreenFrame->width())/2,
			(GetScreenGeometry(GetPrimaryScreenIndex()).height() - m_pBootScreenFrame->height())/2);
		m_pBootScreenFrame->show();
		QApplication::processEvents();

		setAutoFillBackground(true);

		//背景图片
		QPalette palette;
		QPixmap imgTitle = QPixmap(":/images/background.png");
		imgTitle = imgTitle.scaled(GetScreenGeometry().width(), GetScreenGeometry().height());
		palette.setBrush(QPalette::Background, QBrush(imgTitle));
		setPalette(palette);

		//窗体整体布局
		QVBoxLayout* pVLayout = new QVBoxLayout(this);
		pVLayout->setContentsMargins(0, 0, 0, 0);
		pVLayout->setSpacing(0);
		setLayout(pVLayout);

		//主菜单
		m_pFreeMenuWidget = new FreeViewer::CFreeMenuWidget(this);
		pVLayout->addWidget(m_pFreeMenuWidget);
		SetDoubleClickedArea(QRect(0, 0, m_pFreeMenuWidget->width(), m_pFreeMenuWidget->height()));
		SetMovableArea(QRect(0, 0, m_pFreeMenuWidget->width(), m_pFreeMenuWidget->height()));

		//关闭、最小化、最大化
		connect(m_pFreeMenuWidget, SIGNAL(SignalCloseWidget()), this, SLOT(SlotCloseWidget()));
		connect(m_pFreeMenuWidget, SIGNAL(SignalMinisizeWidget()), this, SLOT(SlotMinisizeWidget()));
		connect(m_pFreeMenuWidget, SIGNAL(SignalMaxisizeWidget()), this, SLOT(SlotMaxisizeWidget()));

		//主显示窗体布局
		QHBoxLayout* pSceneLayout = new QHBoxLayout();
		pSceneLayout->setContentsMargins(2, 2, 2, 2);
		pSceneLayout->setSpacing(0);
		pVLayout->addLayout(pSceneLayout);

		//树列表
		m_pDockWidget = new CDockFrame(this);
		pSceneLayout->addWidget(m_pDockWidget);
		m_pDockWidget->GetTitleBar()->setMouseTracking(true);
		m_pDockWidget->GetTitleBar()->installEventFilter(this);

		//计算并设置树列表的显示宽度
		//计算并设置树列表的显示宽度
		double dOutterDis = (m_pFreeMenuWidget->height()-g_nToolBarHeight)/tan(g_dToolBarBorderAngle);
		int nWith = g_sizeMenuLogo.width()/g_sizeDefaultScreen.width()*GetScreenGeometry().width()-dOutterDis;
		m_pDockWidget->SetFixedWidth(nWith);


		if (NULL == m_p3DTreeWidget)
		{
			m_p3DTreeWidget = new CFree3DDockTreeWidget(tr("3D Scene"), this);
			m_p3DTreeWidget->GetTreeWidget()->setMouseTracking(true);
			m_p3DTreeWidget->GetTreeWidget()->viewport()->installEventFilter(this);
			m_pDockWidget->AddDockWidget(m_p3DTreeWidget);
		}

		// 添加服务管理树
		if (NULL == m_pServiceDockTree)
		{
			m_pServiceDockTree = new CFreeServiceDockTree(tr("Service Manager"), this);
			m_pDockWidget->AddDockWidget(m_pServiceDockTree);
		}


		//添加信息提示框
		m_pStatusBar = new CFreeStatusBar(this);
		m_pStatusBar->setFixedHeight(32);
		pVLayout->addWidget(m_pStatusBar);

		//二三维显示窗体
		m_pSceneWidget = new FreeViewer::CFreeSceneWidget(this);
		pSceneLayout->addWidget(m_pSceneWidget);

		//初始化三维场景
		m_p3DSceneWidget = new FeEarth::C3DSceneWidget(FeFileReg->GetDataPath(), true);
		m_p3DSceneWidget->Initialize();
		connect(m_p3DSceneWidget, SIGNAL(Signal3DSceneInitDone(bool)),this, SLOT(Slot3DInitDone(bool)), Qt::QueuedConnection);

		m_pBootScreenFrame->SetLoadText(tr("Loading 3DSceneWidget"), "", "");
		m_pBootScreenFrame->SetRoundProgress(0, 30, false);


		// 服务模块和二维场景交互信号和槽
		connect(m_pServiceDockTree, SIGNAL(SignalDragItem(bool)), this, SLOT(SlotDragItem(bool)));
		connect(m_p3DSceneWidget, SIGNAL(SignalDragEnterEvent(QDragEnterEvent*)), this, SLOT(SlotDragEnterEvent(QDragEnterEvent*)));
		connect(m_p3DSceneWidget, SIGNAL(SignalDropEvent(QDropEvent *)), this, SLOT(SlotDropEvent(QDropEvent *)));
		connect(this, SIGNAL(SiganlAddLayerItem(QTreeWidgetItem*)), m_pServicePluginInterface, SLOT(SlotAddLayer()));

	}

	void CFreeMainWindow::InitContext()
	{

		// 加载服务管理
		m_pServicePluginInterface = new CServicePluginInterface(this);
		m_listPlugins.push_back(m_pServicePluginInterface);
		m_pBootScreenFrame->SetLoadText(tr("Loading Service"), tr("Loading Layer"), tr(""));
		m_pBootScreenFrame->SetRoundProgress(30, 50);

		//三维初始化完成
		m_p3DSceneWidget->setMinimumSize(1, 1);
		m_pSceneWidget->AddSubWidget(m_p3DSceneWidget);

		//三维系统图层
		m_pBootScreenFrame->SetLoadText(tr("Loading SystemLayer"), tr("Loading CustomLayer"), "");
		m_pBootScreenFrame->SetRoundProgress(50, 60);
		CSystemLayerTree* pSystemLayer = new CSystemLayerTree(this);
		m_listPlugins.push_back(pSystemLayer);

		//三维用户自定义图层
		m_pBootScreenFrame->SetLoadText(tr("Loading CustomLayer"), tr("Loading EnvironmentWidget"), "");
		m_pBootScreenFrame->SetRoundProgress(60, 70);
		CCustomLayerTree* pCustomLayer = new CCustomLayerTree(this);
		m_listPlugins.push_back(pCustomLayer);
		m_pServicePluginInterface->SetCustomLayerTree(pCustomLayer);


		//环境菜单
		m_pBootScreenFrame->SetLoadText(tr("Loading EnvironmentWidget"), tr("Loading MeasureWidget"), "");
		m_pBootScreenFrame->SetRoundProgress(70, 75);
		CEnvironmentWidgetPlugin* pEnvironmentWidget = new CEnvironmentWidgetPlugin(this);
		m_listPlugins.push_back(pEnvironmentWidget);

		//测量菜单
		m_pBootScreenFrame->SetLoadText(tr("Loading MeasureWidget"), tr("Loading MarkWidget"), "");
		m_pBootScreenFrame->SetRoundProgress(75, 80);
		m_pMeasureWidget = new CMeasurePluginInterface(this);
		m_listPlugins.push_back(m_pMeasureWidget);

		//标记菜单
		m_pBootScreenFrame->SetLoadText(tr("Loading MarkWidget"), tr("Loading PlotWidget"), "");
		m_pBootScreenFrame->SetRoundProgress(80, 85);
		m_pMarkWidget = new CMarkPluginInterface(this);
		m_listPlugins.push_back(m_pMarkWidget);

		//军标菜单
		m_pBootScreenFrame->SetLoadText(tr("Loading PlotWidget"), tr("Loading ToolWidget"), "");
		m_pBootScreenFrame->SetRoundProgress(85, 90);
		m_pPlotWidget = new CPlotPluginInterface(this);
		m_listPlugins.push_back(m_pPlotWidget);

		//视点菜单
		CViewPointPluginInterface* pViewPointInterface = new CViewPointPluginInterface(this);
		m_listPlugins.push_back(pViewPointInterface);

		//视点图层
		CViewPointTree* pViewPointLayer = new CViewPointTree(this, FeFileReg->GetFullPath("config/viewPointConfig.xml"));
		m_listPlugins.push_back(pViewPointLayer);

		//视图菜单
		m_pBootScreenFrame->SetLoadText(tr("Loading ToolWidget"), "", "");
		m_pBootScreenFrame->SetRoundProgress(90, 95);

		//工具菜单
		CViewPluginInterface* pViewWidget = new CViewPluginInterface(this);
		m_listPlugins.push_back(pViewWidget);

		CToolPluginInterface* pToolPluginInterface = new CToolPluginInterface(this);
		m_listPlugins.push_back(pToolPluginInterface);

		//帮助菜单
		CHelpMenu* pHelpMenu = new CHelpMenu(this);
		m_listPlugins.push_back(pHelpMenu);

		////二维地图图层树
		//	m_pBootScreenFrame->SetLoadText(tr("Loading 2DLayer"), tr("Loading Done"), "");
		//	m_pBootScreenFrame->SetRoundProgress(95, 100);
		//	CFreeMapLayerTree * pFreeMapLayerTree = new CFreeMapLayerTree(this);
		//	m_listPlugins.push_back(pFreeMapLayerTree);
		//	pFreeMapLayerTree->GetLayerTreeView()->setMenuProvider(
		//		new CFreeMapLayerTreeMenuProvider(pFreeMapLayerTree->GetLayerTreeView(), m_p2DSceneWidget->GetMapCanvas(), this));

		//定位
		FeShell::CSystemManager* pManager = m_p3DSceneWidget->GetSystemManager();
		if (pManager)
		{
			FeShell::CSystemService* pService = pManager->GetSystemService();
			if (pService)
			{
				if (pService->GetManipulatorManager())
				{
					pService->GetManipulatorManager()->SetHome(pService->GetSysConfig().GetEndViewPoint().GetEarthVP(), 3);
					pService->GetManipulatorManager()->Home(3);
				}
			}
		}

		m_pBootScreenFrame->hide();
	}

	void CFreeMainWindow::paintEvent( QPaintEvent * event )
	{
		// 		//背景图片
		// 		QPalette palette;
		// 		QPixmap imgTitle = QPixmap(":/images/background.png");
		// 		palette.setBrush(QPalette::Background, QBrush(imgTitle.scaled(width(), height())));
		// 		setPalette(palette);
		// 
		// 		//绘制边框线
		// 		QPainter painter(this);
		// 
		// 		QVector<QPoint> points;
		// 		//左上角
		// 		points.push_back(QPoint(0, 0));
		// 		//右上角
		// 		points.push_back(QPoint(width()-g_nDeltaWidth, 0));
		// 		//右下角
		// 		points.push_back(QPoint(width()-g_nDeltaWidth, height()-g_nDeltaWidth));
		// 		//左下角
		// 		points.push_back(QPoint(0, height()-g_nDeltaWidth));
		// 		//左上角
		// 		points.push_back(QPoint(0, 0));
		// 
		// 		painter.setPen(g_colorFrameLine);
		// 		painter.drawPolyline(points);
		// 
		// 		CFreeFramelessWidget::paintEvent(event);
	}


	CFreeMenuWidget* CFreeMainWindow::GetMenuWidget()
	{
		return m_pFreeMenuWidget;
	}

	CFreeStatusBar* CFreeMainWindow::GetStatusBar()
	{
		return m_pStatusBar;
	}

	CDockFrame* CFreeMainWindow::GetDockWidget()
	{
		return m_pDockWidget;
	}

	FeEarth::C3DSceneWidget* CFreeMainWindow::Get3DSceneWidget()
	{
		return m_p3DSceneWidget;
	}

	void CFreeMainWindow::ClearMark()
	{
		m_pMarkWidget->DeleteCurrentMark();
	}

	void CFreeMainWindow::ClearPlot()
	{
		m_pPlotWidget->DeleteCurrentPlot();
	}

	void CFreeMainWindow::ClearMeasure()
	{
		m_pMeasureWidget->SlotDeleteMeasure(true);
	}

	void CFreeMainWindow::Slot3DInitDone(bool bState)
	{
		if (bState)
		{
			m_pBootScreenFrame->SetRoundProgress(0, 0, true);
			InitContext();
		}

		m_pBootScreenFrame->SetRoundProgress(100, 100, true);
		m_pBootScreenFrame->hide();
	}


	void CFreeMainWindow::resizeEvent( QResizeEvent * event )
	{
		CFreeFramelessWidget::resizeEvent(event);
#ifndef WIN32
		move(x(), y());
#endif
		MoveWindows();
	}

	void CFreeMainWindow::MoveWindows()
	{
		if (m_pGraphicView && m_pSceneWidget )
		{
			int nSceneWidgetX = m_pSceneWidget->geometry().x();
			int nSceneWidgetY = m_pSceneWidget->geometry().y();
			int nSceneWidgetW = m_pSceneWidget->geometry().width();
			int nSceneWidgetH = m_pSceneWidget->geometry().height();

			int nViewW = m_pGraphicView->size().width();
			int nViewH = m_pGraphicView->size().height();

			int nViewX = this->geometry().x() + nSceneWidgetX + (nSceneWidgetW - nViewW)/2;
			int nViewY = this->geometry().y() + nSceneWidgetY + (nSceneWidgetH - nViewH);

			m_pGraphicView->setGeometry(nViewX, nViewY, nViewW, nViewH);
			m_pGraphicScene->setSceneRect(0, 0, nViewW, nViewH);
			m_pGraphicView->fitInView(m_pGraphicScene->sceneRect(), Qt::KeepAspectRatio);
		}
	}

	CFree3DDockTreeWidget* CFreeMainWindow::Get3DTreeWidget()
	{
		if (NULL == m_p3DTreeWidget)
		{
			m_p3DTreeWidget = new CFree3DDockTreeWidget(tr("3D Scene"), this);
			m_p3DTreeWidget->GetTreeWidget()->setMouseTracking(true);
			m_p3DTreeWidget->GetTreeWidget()->viewport()->installEventFilter(this);
			m_pDockWidget->AddDockWidget(m_p3DTreeWidget);
		}

		return m_p3DTreeWidget;
	}

	CFreeServiceDockTree* CFreeMainWindow::GetServiceDockTree()
	{
		return m_pServiceDockTree;
	}

	bool CFreeMainWindow::eventFilter( QObject* watched, QEvent* event )
	{
		if (event->type() == QEvent::MouseMove)
		{
			QMouseEvent* pEvent = dynamic_cast<QMouseEvent*>(event);
			if (pEvent)
			{
				this->mouseMoveEvent(pEvent);
			}
		}

		return false;
	}

	void CFreeMainWindow::SlotDragItem(bool bStatus)
	{
		m_bDragLeaveFlag = bStatus;
	}

	void CFreeMainWindow::SlotDragEnterEvent( QDragEnterEvent *event )
	{
		if (m_bDragLeaveFlag)
		{
			// 拖放消息只有来自本程序中的控件才有效，避免外部拖放（bug#347）
			if(event->source())
			{
				event->acceptProposedAction();
			}
		}
		else
		{
			event->ignore();
			QWidget::dragEnterEvent(event);
		}
	}

	void CFreeMainWindow::SlotDropEvent(QDropEvent *event)
	{
		if (NULL == m_pServicePluginInterface || NULL == m_pServicePluginInterface->GetCurrentItem())
		{
			return;
		}

		emit SiganlAddLayerItem( m_pServicePluginInterface->GetCurrentItem() );

		m_pServicePluginInterface->SetCurrentItem(NULL);
	}

	void CFreeMainWindow::moveEvent( QMoveEvent *event )
	{
		CFreeFramelessWidget::moveEvent(event);
#ifndef WIN32
		move(x(), y());
#endif
		MoveWindows();
	}

}


