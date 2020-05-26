#include <plot/PlotPluginInterface.h>
#include <plot/PlotPropertyBuilder.h>

#include <mainWindow/FreeMainWindow.h>
#include <mainWindow/FreeToolBar.h>
#include <mainWindow/FreeUtil.h>

#include <QVariant>
#include <QMetaType>

#include <FeUtils/PathRegistry.h>
#include <FeUtils/EventListener.h>

#include <mark/PropertyWidget.h>

namespace FreeViewer
{
	CPlotPluginInterface::CPlotPluginInterface( CFreeMainWindow* pMainWindow)
		: CUIObserver(pMainWindow)
		, m_plotProcessor()
		, m_plotMenuManager(this)
		, m_rpCurPlotNode(NULL)
		, m_p3DTreeWidget(NULL)
		, m_pPlotTreeItemRoot(NULL)
		, m_pActiveItem(NULL)
		, m_bCreatingPlot(false)
		, m_bPropertyWidgetShow(false)
		, m_pCurrentWidget(NULL)
	{
		m_strMenuTitle = QString(tr("Plot"));

		InitWidget();
	}

	CPlotPluginInterface::~CPlotPluginInterface()
	{

	}

	void CPlotPluginInterface::BuildContext()
	{
		/// 军标菜单
		InitPlotMenu();

		/// 树形控件
		InitPlotTree();
	}

	void CPlotPluginInterface::InitWidget()
	{
		if (NULL == m_pMainWindow && false == m_opSystemService.valid())
		{
			return ;
		}

		//创建根节点
		m_p3DTreeWidget = m_pMainWindow->Get3DTreeWidget();
		if (NULL == m_p3DTreeWidget)
		{
			return ;
		}
		m_p3DTreeWidget->PushBackSystem(E_PLOT_LAYER, this);

		m_rpMarkSys = new FeManager::CFreeMarkSys();

		if(m_rpMarkSys.valid() && m_opSystemService.valid())
		{
			m_rpMarkSys->Initialize(m_opSystemService->GetRenderContext());
			m_opSystemService->AddAppModuleSys(m_rpMarkSys);
			m_rpMarkSys->SetDefaultMarkFilePath(m_opSystemService->GetSysConfig().GetPlotsConfig());
			m_rpMarkSys->Load();

			m_rpPlotHandler = new CPlotEventHandler(this);

			BuildContext();
		}

		m_rpEventListener = new CPlotEventListener;
		connect(m_rpEventListener, SIGNAL(NodeStateEventTrigger(unsigned int)), this, SLOT(SlotNodeStateChanged(unsigned int)));
	}

	void CPlotPluginInterface::InitPlotTree()
	{
		if (m_rpMarkSys.valid())
		{
			FeExtNode::CExternNode* pExternNode = m_rpMarkSys->GetRootMark();

			m_pPlotTreeItemRoot = new QTreeWidgetItem(m_p3DTreeWidget->GetTreeWidget(), QStringList(tr("Plot Layer")));
			m_pPlotTreeItemRoot->setData(0, Qt::UserRole, QVariant(E_PLOT_LAYER_ROOT));
			//m_pPlotTreeItemRoot->setData(1, Qt::UserRole, QVariant::fromValue(pExternNode));
			m_pPlotTreeItemRoot->setCheckState(0, Qt::Unchecked);
			m_p3DTreeWidget->GetTreeWidget()->AddTopTreeItem(m_pPlotTreeItemRoot);
			m_pActiveItem = m_pPlotTreeItemRoot;

			if (pExternNode->AsComposeNode())
			{
				FeExtNode::CExComposeNodeOption::VecExternNodes vecChildren = pExternNode->AsComposeNode()->GetChildren();
				FeExtNode::CExComposeNodeOption::VecExternNodes::iterator itr = vecChildren.begin();
				for (; itr != vecChildren.end(); ++itr)
				{
					AddTreeItem(m_pPlotTreeItemRoot, (*itr).get());
				}
			}

			m_pPlotTreeItemRoot->setExpanded(true);
		}
	}

	void CPlotPluginInterface::InitPlotMenu()
	{
		m_plotMenuManager.InitMenuAndActions();

		connect(&m_plotMenuManager, SIGNAL(SignalOpenPropertyWidget()), this, SLOT(SlotMenuOpenPropertyWidget()));
		connect(&m_plotMenuManager, SIGNAL(SignalDeletePlot()), this, SLOT(SlotMenuDeletePlot()));
		connect(&m_plotMenuManager, SIGNAL(SignalClearPlot()), this, SLOT(SlotMenuClearPlot()));

		//添加工具条分割线
		CFreeToolBar* pToolBar = GetToolBar();
		pToolBar->AddSperator();
	}

	void CPlotPluginInterface::AddTreeItem(QTreeWidgetItem* pParentItem, FeExtNode::CExternNode* pExternNode)
	{
		if (NULL == pExternNode) 
		{
			return;
		}

		//获取图标
		QString strIcon;
		CPlotVisitProcessor::CProcessData<QString> data(strIcon);
		m_plotProcessor.DoProcess(pExternNode, CPlotVisitProcessor::E_PLOT_GET_ICON_PATH, data);

		//判断当前添加的节点类型
		EItemType type = E_PLOT_NODE;
		if (pExternNode->AsComposeNode())
		{
			type = E_PLOT_FOLDER;
		}

		//创建并添加树节点
		QTreeWidgetItem* pItem = new QTreeWidgetItem(QStringList(ConvertToCurrentEncoding(pExternNode->GetName())));
		pItem->setIcon(0, QIcon(strIcon));
		pItem->setData(0, Qt::UserRole, QVariant(type));
		pItem->setData(1, Qt::UserRole, QVariant::fromValue(pExternNode));
		if (!pExternNode->AsComposeNode())
		{
			//该节点不存在子节点时 初始化该节点的visible
			if (pExternNode->GetVisible())
			{
				pItem->setCheckState(0, Qt::Checked);
			}
			else
			{
				pItem->setCheckState(0, Qt::Unchecked);
			}
		}

		m_p3DTreeWidget->GetTreeWidget()->AddTreeItem(pItem, pParentItem);
		m_pActiveItem = pItem;

		//遍历组节点下的所有子节点
		if (pExternNode->AsComposeNode())
		{
			FeExtNode::CExComposeNodeOption::VecExternNodes vecChildren = pExternNode->AsComposeNode()->GetChildren();
			FeExtNode::CExComposeNodeOption::VecExternNodes::iterator itr = vecChildren.begin();
			for (; itr != vecChildren.end(); ++itr)
			{
				AddTreeItem(pItem, (*itr).get());
			}
		}

		pParentItem->setExpanded(true);
	}

	FeExtNode::CExternNode* CPlotPluginInterface::GetPlotByItem( QTreeWidgetItem* pItem)
	{
		FeExtNode::CExternNode* pNode = NULL;
		if(pItem)
		{
			if(pItem == m_pPlotTreeItemRoot)
			{
				pNode = m_rpMarkSys->GetRootMark();
			}
			else
			{
				pNode = pItem->data(1, Qt::UserRole).value<FeExtNode::CExternNode*>();
			}
			
		}

		return pNode;
	}

	void CPlotPluginInterface::StartDrawPlot()
	{
		if( !IsDrawingOrEditing() )
		{
			m_bCreatingPlot = true; 

			/// 将当前绘制的军标添加到场景管理器，并开启编辑
			m_rpMarkSys.get()->AddMark(m_rpCurPlotNode.get(), true);
			m_rpMarkSys.get()->StartEdit(m_rpCurPlotNode.get());
		}
	}

	void CPlotPluginInterface::SuccessToDraw()
	{
		if( IsDrawingOrEditing() )
		{
			/// 关闭菜单选中状态 
			m_plotMenuManager.ResetMenu();

			/// 停止事件捕获 
			m_rpPlotHandler->StopCapture();

			/// 显示标记属性对话框
			if(!m_bPropertyWidgetShow)
			{
				ShowPlotPropertyWidget(m_rpCurPlotNode.get()); 
			}
		}
	}

	void CPlotPluginInterface::FailedToDraw()
	{
		if( IsDrawingOrEditing() )
		{
			/// 关闭菜单选中状态 
			m_plotMenuManager.ResetMenu();

			/// 停止事件捕获 
			m_rpPlotHandler->StopCapture();

			if(m_rpMarkSys.valid() && m_bCreatingPlot)
			{
				m_rpMarkSys->RemoveMark(m_rpCurPlotNode);
			}

			ResetAllState();
		}
	}

	void CPlotPluginInterface::StartEventCapture()
	{
		if(GetSystemService())
		{
			GetSystemService()->AddEventHandler(m_rpPlotHandler.get());

			m_rpPlotHandler->SetActivePlotNode(m_rpCurPlotNode);
			m_rpPlotHandler->StartCapture();
		}
	}

	void CPlotPluginInterface::StopEventCapture()
	{
		if(GetSystemService())
		{
			m_rpPlotHandler->SetActivePlotNode(NULL);
			m_rpPlotHandler->StopCapture();

			GetSystemService()->RemoveEventHandler(m_rpPlotHandler.get());
		}
	}

	void CPlotPluginInterface::ShowPlotPropertyWidget( FeExtNode::CExternNode* pNode )
	{
		if(!pNode) return;

		CPlotPropertyBuilder builder(m_opSystemService.get(), pNode, true);
		QWidget* pPropertyWidget = builder.CreatePropertyWidget(m_pMainWindow);	
		m_pCurrentWidget = pPropertyWidget;
		if (pPropertyWidget)
		{
			connect(pPropertyWidget, SIGNAL(sig_accept(FeExtNode::CExternNode*, bool)),this, SLOT(SlotPropertyOK(FeExtNode::CExternNode*))); 
			connect(pPropertyWidget, SIGNAL(sig_reject(FeExtNode::CExternNode*, bool)),this, SLOT(SlotPropertyCancel(FeExtNode::CExternNode*)));
			pPropertyWidget->show();

			if(m_rpMarkSys.valid())
			{
				m_rpMarkSys->Start();
				m_rpMarkSys->StartEdit(pNode);
				
				if(m_rpMarkSys->GetEventListenManager())
				{
					m_rpMarkSys->GetEventListenManager()->RegistEventListener(m_rpEventListener, pNode);
				}
			}
			m_rpCurPlotNode = pNode;
			m_bPropertyWidgetShow = true;
		}
	}

	void CPlotPluginInterface::SlotPropertyOK( FeExtNode::CExternNode* pNode )
	{
		SlotRemoveEventListener(pNode);

		if(m_bCreatingPlot)
		{
			QTreeWidgetItem* pParentItem = GetCurrentGroupItem();
			AddTreeItem(pParentItem, pNode);

			FeExtNode::CExternNode* pParentNode = GetPlotByItem(pParentItem);
			if(pParentNode && pParentNode->AsComposeNode() /*&&  (pParentNode != m_rpMarkSys->GetRootMark())*/)
			{
				m_rpMarkSys->RemoveMark(pNode);
				m_rpMarkSys->AddMark(GetPlotByItem(pParentItem)->AsComposeNode(), pNode);
			}
		}
		else
		{
			if(m_pActiveItem)
			{
				m_pActiveItem->setText(0, ConvertToCurrentEncoding(pNode->GetName()));
			}
		}

		m_rpMarkSys->Save();
		ResetAllState();
	}

	void CPlotPluginInterface::SlotPropertyCancel( FeExtNode::CExternNode* pNode )
	{
 		SlotRemoveEventListener(pNode);
 
 		if(m_rpMarkSys.valid())
 		{
 			/// 用户取消，移除当前图元
 			if(m_bCreatingPlot)
 			{
 				m_rpMarkSys->RemoveMark(pNode);	
 			}
 			else if(m_pActiveItem)
 			{
 				/// 恢复图元之前的设置
 				m_pActiveItem->setText(0, ConvertToCurrentEncoding(pNode->GetName()));
 				m_rpMarkSys->Save();
 			}
 		}
 
 		ResetAllState();
	}

	void CPlotPluginInterface::SlotRemoveEventListener( FeExtNode::CExternNode* pNode )
	{
		if(pNode && m_rpMarkSys.valid() && m_rpEventListener.valid() && m_rpMarkSys->GetEventListenManager())
		{
			m_rpMarkSys->GetEventListenManager()->UnRegistEventListener(m_rpEventListener, pNode);
		}
	}

	QTreeWidgetItem* CPlotPluginInterface::GetCurrentGroupItem()
	{
		QTreeWidgetItem* pParentItem = NULL;

		FeExtNode::CExternNode* pNode = GetPlotByItem(m_pActiveItem);
		if(pNode && pNode->AsComposeNode())
		{
			pParentItem = m_pActiveItem;
		}
		else if(m_pActiveItem && GetPlotByItem(m_pActiveItem->parent()))
		{
			pParentItem = m_pActiveItem->parent();
		}
		else
		{
			pParentItem = m_pPlotTreeItemRoot;
		}

		return pParentItem;
	}

	void CPlotPluginInterface::ResetAllState()
	{
		m_bCreatingPlot = false;
		m_bPropertyWidgetShow = false;

		if (m_pCurrentWidget)
		{
			m_pCurrentWidget->close();
			m_pCurrentWidget = NULL;
		}

		if(m_rpMarkSys.valid())
		{
			m_rpMarkSys->StopEdit(m_rpCurPlotNode.get());
			m_rpMarkSys->Stop();
			m_rpCurPlotNode = NULL;
		}
	}

	void CPlotPluginInterface::DeleteNodeFromMarkSys( FeExtNode::CExternNode* pNode, FeExtNode::CExternNode* pParentNode )
	{
		if(!pNode || !pParentNode || !pParentNode->AsComposeNode() || !m_rpMarkSys.valid())
		{
			return;
		}

		if(pNode->AsComposeNode())
		{
			FeExtNode::CExComposeNodeOption::VecExternNodes children = pNode->AsComposeNode()->GetChildren();
			for(int n = 0; n < children.size(); n++)
			{
				DeleteNodeFromMarkSys(children.at(n), pNode);
			}
		}

		m_rpMarkSys->RemoveMark(pParentNode->AsComposeNode(), pNode);
	}

	void CPlotPluginInterface::SlotContextMenuItem( QTreeWidgetItem* pItem, QPoint pt)
	{
		if (NULL == pItem || IsDrawingOrEditing())
		{
			return ;
		}

		m_pActiveItem = pItem;

		EItemType type = (EItemType)pItem->data(0, Qt::UserRole).toInt();
		switch (type)
		{
		case E_PLOT_FOLDER:
		case E_PLOT_NODE:
			{
				QMenu* pPopMenu = m_plotMenuManager.CreatePopMenu(GetPlotByItem(pItem), m_p3DTreeWidget);
				if(pPopMenu)
				{
					pPopMenu->setAttribute(Qt::WA_DeleteOnClose);
					pPopMenu->exec(pt);
				}
			}
			break;
		case E_PLOT_LAYER_ROOT:
			{
				QMenu* pPopMenu = m_plotMenuManager.CreateRootPopMenu(GetPlotByItem(pItem), m_p3DTreeWidget);
				if(pPopMenu)
				{
					pPopMenu->setAttribute(Qt::WA_DeleteOnClose);
					pPopMenu->exec(pt);
				}
			}
			break;
		default:
			break;
		}
	}

	void CPlotPluginInterface::SlotItemsVisibleChange( QTreeWidgetItem* pItem, bool bVisible)
	{
		if(NULL == pItem)
		{
			return ;
		}

		if(!IsDrawingOrEditing())
		{
			m_pActiveItem = pItem;
		}
	
		FeExtNode::CExternNode* pNode = GetPlotByItem(pItem);
		if(pNode)
		{
			pNode->SetVisible(bVisible);
			m_rpMarkSys->Save();
		}
	}

	void CPlotPluginInterface::SlotItemDBClicked( QTreeWidgetItem* pItem)
	{
		if(NULL == pItem)
		{
			return ;
		}

		if(!IsDrawingOrEditing())
		{
			m_pActiveItem = pItem;
		}

		FeExtNode::CExternNode* pPlot = GetPlotByItem(pItem);

		if (pPlot && !pPlot->AsComposeNode() && m_rpMarkSys.valid() && m_opSystemService.valid())
		{	
			FeUtil::CFreeViewPoint pVP = pPlot->GetViewPoint();
			m_opSystemService->Locate(pVP);
		}
	}

	void CPlotPluginInterface::SlotMenuOpenPropertyWidget()
	{
		if(m_p3DTreeWidget && !m_bPropertyWidgetShow)
		{
			ShowPlotPropertyWidget(GetPlotByItem(m_pActiveItem));
		}
	}

	void CPlotPluginInterface::SlotMenuDeletePlot()
	{
		FeExtNode::CExternNode* pNode = GetPlotByItem(m_pActiveItem);
		if(pNode && m_rpMarkSys.valid())
		{
			FeExtNode::CExternNode* pParentGuess = 0;
			if(m_pActiveItem && m_pActiveItem->parent())
			{
				pParentGuess = GetPlotByItem(m_pActiveItem->parent());
			}

			FeExtNode::CExComposeNode* pParentNode = pParentGuess ? 
				pParentGuess->AsComposeNode() : m_rpMarkSys->GetRootMark();

			DeleteNodeFromMarkSys(pNode, pParentNode);
			m_p3DTreeWidget->GetTreeWidget()->RemoveTreeItem(m_pActiveItem);
			m_rpMarkSys->Save();

			m_pActiveItem = m_p3DTreeWidget->GetTreeWidget()->currentItem();
		}
	}


	void CPlotPluginInterface::SlotMenuClearPlot()
	{
		FeExtNode::CExternNode* pNode = GetPlotByItem(m_pActiveItem);
		if(pNode)
		{
			FeExtNode::CExComposeNode* pParent = pNode->AsComposeNode();
			if(pParent && m_rpMarkSys.valid())
			{
				m_p3DTreeWidget->GetTreeWidget()->RemoveTreeItemChilds(m_pActiveItem);
				FeExtNode::CExComposeNodeOption::VecExternNodes children = pParent->GetChildren();
				for(int n = 0; n < children.size(); n++)
				{
					DeleteNodeFromMarkSys(children.at(n), pParent);
				}

				m_rpMarkSys->Save();
			}
		}
	}

	void CPlotPluginInterface::SlotNodeStateChanged( unsigned int eType)
	{
		CPropertyWidget* pWidget = dynamic_cast<CPropertyWidget*>(m_pCurrentWidget);
		if(pWidget)
		{
			pWidget->HandleSDKEvent(eType);
		}
	}

	CFreeMainWindow* CPlotPluginInterface::GetMainWindow()
	{
		return m_pMainWindow;
	}

	void CPlotPluginInterface::DeleteCurrentPlot()
	{
		/// 关闭菜单选中状态 
		m_plotMenuManager.SetMutex(NULL);

		/// 停止事件捕获 
		m_rpPlotHandler->StopCapture();

		if(m_bPropertyWidgetShow)
		{
			m_pCurrentWidget->close();
		}

		if (m_rpCurPlotNode.valid())
		{
			SlotPropertyCancel(m_rpCurPlotNode.get());
		}
	}

	bool CPlotPluginInterface::IsDrawingOrEditing()
	{
		return (m_bCreatingPlot || m_bPropertyWidgetShow);
	}

	CPlotVisitProcessor& CPlotPluginInterface::GetPlotProcessor()
	{
		return m_plotProcessor;
	}

	FeManager::CFreeMarkSys* CPlotPluginInterface::GetMarkSys()
	{ 
		return m_rpMarkSys.get();
	}

	CFree3DDockTreeWidget* CPlotPluginInterface::GetTreeDockWidget()
	{ 
		return m_p3DTreeWidget; 
	}

	void CPlotPluginInterface::SetActivePlotNode( FeExtNode::CExternNode* pNode )
	{
		if(pNode) 
		{
			m_rpCurPlotNode = pNode; 
		}
	}

}


