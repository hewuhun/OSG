#include <mark/MarkPluginInterface.h>

#include <mark/MarkPropertyBuilder.h>

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
	CMarkPluginInterface::CMarkPluginInterface( CFreeMainWindow* pMainWindow)
		:CUIObserver(pMainWindow)
		, m_markProcessor()
		, m_markMenuMgr(this)
		, m_rpCurMarkNode(NULL)
		, m_p3DTreeWidget(NULL)
		, m_pMarkTreeItemRoot(NULL)
		, m_pActiveItem(NULL)
		, m_bCreatingMark(false)
		, m_bPropertyWidgetShow(false)
		, m_pCurrentWidget(NULL)
	{
		m_strMenuTitle = QString(tr("Mark"));

		InitWidget();
	}

	CMarkPluginInterface::~CMarkPluginInterface()
	{

	}

	void CMarkPluginInterface::BuildContext()
	{
		/// 标记菜单
		InitMarkMenu();

		/// 树形控件
		InitMarkTree();
	}

	void CMarkPluginInterface::InitWidget()
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
		m_p3DTreeWidget->PushBackSystem(E_MARK_LAYER, this);

		//connect(m_p3DTreeWidget->GetTreeWidget(), SIGNAL(SignalContextMenuItem(QTreeWidgetItem*, QPoint)), 
		//	this, SLOT(SlotTreeContextMenuItem(QTreeWidgetItem*, QPoint)));
		//connect(m_p3DTreeWidget->GetTreeWidget(), SIGNAL(SignalItemsVisibleChange(QTreeWidgetItem*, bool)), 
		//	this, SLOT(SlotTreeItemsVisibleChange(QTreeWidgetItem*, bool)));
		//connect(m_p3DTreeWidget->GetTreeWidget(), SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), 
		//	this, SLOT(SlotTreeItemDBClicked(QTreeWidgetItem*)));

		m_rpMarkSys = new FeManager::CFreeMarkSys();

		if(m_rpMarkSys.valid() && m_opSystemService.valid())
		{
			m_rpMarkSys->Initialize(m_opSystemService->GetRenderContext());
			m_opSystemService->AddAppModuleSys(m_rpMarkSys);
			m_rpMarkSys->SetDefaultMarkFilePath(m_opSystemService->GetSysConfig().GetMarksConfig());
			m_rpMarkSys->Load();

			m_rpMarkHandler = new CMarkEventHandler(this);

			BuildContext();
		}

		m_rpEventListener = new CMarkEventListener;
		connect(m_rpEventListener, SIGNAL(NodeStateEventTrigger(unsigned int)), this, SLOT(SlotNodeStateChanged(unsigned int)));
	}

	void CMarkPluginInterface::InitMarkTree()
	{
		if (m_rpMarkSys.valid())
		{
			FeExtNode::CExternNode* pExternNode = m_rpMarkSys->GetRootMark();

			m_pMarkTreeItemRoot = new QTreeWidgetItem(m_p3DTreeWidget->GetTreeWidget(), QStringList(tr("Mark Layer")));
			m_pMarkTreeItemRoot->setData(0, Qt::UserRole, QVariant(E_MARK_LAYER_ROOT));
			//m_pMarkTreeItemRoot->setData(1, Qt::UserRole, QVariant::fromValue(pExternNode));
			m_pMarkTreeItemRoot->setCheckState(0, Qt::Unchecked);
			m_p3DTreeWidget->GetTreeWidget()->AddTopTreeItem(m_pMarkTreeItemRoot);
			m_pActiveItem = m_pMarkTreeItemRoot;

			if (pExternNode->AsComposeNode())
			{
				FeExtNode::CExComposeNodeOption::VecExternNodes vecChildren = pExternNode->AsComposeNode()->GetChildren();
				FeExtNode::CExComposeNodeOption::VecExternNodes::iterator itr = vecChildren.begin();
				for (; itr != vecChildren.end(); ++itr)
				{
					AddTreeItem(m_pMarkTreeItemRoot, (*itr).get());
				}
			}

			m_pMarkTreeItemRoot->setExpanded(true);
		}
	}

	void CMarkPluginInterface::InitMarkMenu()
	{
		m_markMenuMgr.InitMenuAndActions();

		connect(&m_markMenuMgr, SIGNAL(SignalOpenPropertyWidget()), this, SLOT(SlotMenuOpenPropertyWidget()));
		connect(&m_markMenuMgr, SIGNAL(SignalDeleteMark()), this, SLOT(SlotMenuDeleteMark()));
		connect(&m_markMenuMgr, SIGNAL(SignalClearMark()), this, SLOT(SlotMenuClearMark()));

		//添加工具条分割线
		CFreeToolBar* pToolBar = GetToolBar();
		pToolBar->AddSperator();
	}

	void CMarkPluginInterface::AddTreeItem(QTreeWidgetItem* pParentItem, FeExtNode::CExternNode* pExternNode)
	{
		if (NULL == pExternNode) 
		{
			return;
		}

		//获取图标
		QString strIcon;
		CMarkVisitProcessor::CProcessData<QString> data(strIcon);
		m_markProcessor.DoProcess(pExternNode, CMarkVisitProcessor::E_MARK_GET_ICON_PATH, data);

		//判断当前添加的节点类型
		EItemType type = E_MARK_NODE;
		if (pExternNode->AsComposeNode())
		{
			type = E_MARK_FOLDER;
		}

		//创建并添加树节点
		QTreeWidgetItem* pItem = new QTreeWidgetItem(QStringList(ConvertToCurrentEncoding(pExternNode->GetName())));
		//pItem->setCheckState(0, Qt::Checked);
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

	FeExtNode::CExternNode* CMarkPluginInterface::GetMarkByItem( QTreeWidgetItem* pItem)
	{
		FeExtNode::CExternNode* pNode = NULL;
		if(pItem)
		{
			if(pItem == m_pMarkTreeItemRoot)
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

	void CMarkPluginInterface::StartDrawMark()
	{
		if( !IsDrawingOrEditing() )
		{
			m_bCreatingMark = true; 

			/// 将当前绘制的标记添加到场景管理器，并开启编辑
			m_rpMarkSys.get()->AddMark(m_rpCurMarkNode.get(), true);
			m_rpMarkSys.get()->StartEdit(m_rpCurMarkNode.get());
		}
	}

	void CMarkPluginInterface::StartEventCapture()
	{
		if(GetSystemService())
		{
			GetSystemService()->AddEventHandler(m_rpMarkHandler.get());

			m_rpMarkHandler->SetActiveMarkNode(m_rpCurMarkNode);
			m_rpMarkHandler->StartCapture();
		}
	}

	void CMarkPluginInterface::StopEventCapture()
	{
		if(GetSystemService())
		{
			m_rpMarkHandler->SetActiveMarkNode(NULL);
			m_rpMarkHandler->StopCapture();

			GetSystemService()->RemoveEventHandler(m_rpMarkHandler.get());
		}
	}

	void CMarkPluginInterface::ShowMarkPropertyWidget( FeExtNode::CExternNode* pMarkNode , bool bCreate)
	{
		if(!pMarkNode) return;

		CMarkPropertyBuilder builder(m_opSystemService.get(), pMarkNode, bCreate);
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
				m_rpMarkSys->StartEdit(pMarkNode);

				if(m_rpMarkSys->GetEventListenManager())
				{
					m_rpMarkSys->GetEventListenManager()->RegistEventListener(m_rpEventListener, pMarkNode);
				}
			}
			m_rpCurMarkNode = pMarkNode;
			m_bPropertyWidgetShow = true;
		}
	}

	void CMarkPluginInterface::SlotPropertyOK( FeExtNode::CExternNode* pNode )
	{
		SlotRemoveEventListener(pNode);

		if(m_bCreatingMark)
		{
			QTreeWidgetItem* pParentItem = GetCurrentGroupItem();
			AddTreeItem(pParentItem, pNode);

			FeExtNode::CExternNode* pParentNode = GetMarkByItem(pParentItem);
			if(pParentNode && pParentNode->AsComposeNode() /*&&  (pParentNode != m_rpMarkSys->GetRootMark())*/)
			{
				m_rpMarkSys->RemoveMark(pNode);
				m_rpMarkSys->AddMark(GetMarkByItem(pParentItem)->AsComposeNode(), pNode);
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

	void CMarkPluginInterface::SlotPropertyCancel( FeExtNode::CExternNode* pNode )
	{
		SlotRemoveEventListener(pNode);

		if(m_rpMarkSys.valid())
		{
			/// 用户取消，移除当前图元
			if(m_bCreatingMark)
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

	void CMarkPluginInterface::SlotRemoveEventListener( FeExtNode::CExternNode* pNode )
	{
		if(pNode && m_rpMarkSys.valid() && m_rpEventListener.valid() && m_rpMarkSys->GetEventListenManager())
		{
			m_rpMarkSys->GetEventListenManager()->UnRegistEventListener(m_rpEventListener, pNode);
		}
	}

	QTreeWidgetItem* CMarkPluginInterface::GetCurrentGroupItem()
	{
		QTreeWidgetItem* pParentItem = NULL;

		FeExtNode::CExternNode* pNode = GetMarkByItem(m_pActiveItem);
		if(pNode && pNode->AsComposeNode())
		{
			pParentItem = m_pActiveItem;
		}
		else if(m_pActiveItem && GetMarkByItem(m_pActiveItem->parent()))
		{
			pParentItem = m_pActiveItem->parent();
		}
		else
		{
			pParentItem = m_pMarkTreeItemRoot;
		}

		return pParentItem;
	}

	void CMarkPluginInterface::ResetAllState()
	{
		m_bCreatingMark = false;
		m_bPropertyWidgetShow = false;

		if (m_pCurrentWidget)
		{
			m_pCurrentWidget->close();
			m_pCurrentWidget = NULL;
		}

		if(m_rpMarkSys.valid())
		{
			m_rpMarkSys->StopEdit(m_rpCurMarkNode.get());
			m_rpMarkSys->Stop();
			m_rpCurMarkNode = NULL;
		}
	}

	void CMarkPluginInterface::DeleteNodeFromMarkSys( FeExtNode::CExternNode* pNode, FeExtNode::CExternNode* pParentNode )
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

	void CMarkPluginInterface::SlotContextMenuItem( QTreeWidgetItem* pItem, QPoint pt)
	{
		if (NULL == pItem || IsDrawingOrEditing())
		{
			return ;
		}

		m_pActiveItem = pItem;

		EItemType type = (EItemType)pItem->data(0, Qt::UserRole).toInt();
		switch (type)
		{
		case E_MARK_FOLDER:
		case E_MARK_NODE:
			{
				QMenu* pPopMenu = m_markMenuMgr.CreatePopMenu(GetMarkByItem(pItem), m_p3DTreeWidget);
				if(pPopMenu)
				{
					pPopMenu->setAttribute(Qt::WA_DeleteOnClose);
					pPopMenu->exec(pt);
				}
			}
			break;
		case E_MARK_LAYER_ROOT:
			{
				QMenu* pPopMenu = m_markMenuMgr.CreateRootPopMenu(GetMarkByItem(pItem), m_p3DTreeWidget);
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

	void CMarkPluginInterface::SlotItemsVisibleChange( QTreeWidgetItem* pItem, bool bVisible)
	{
		if(NULL == pItem)
		{
			return ;
		}

		if(!IsDrawingOrEditing())
		{
			m_pActiveItem = pItem;
		}
	
		FeExtNode::CExternNode* pNode = GetMarkByItem(pItem);
		if(pNode)
		{
			pNode->SetVisible(bVisible);
			m_rpMarkSys->Save();
		}
	}

	void CMarkPluginInterface::SlotItemDBClicked( QTreeWidgetItem* pItem)
	{
		if (NULL == pItem)
		{
			return;
		}

		if(!IsDrawingOrEditing())
		{
			m_pActiveItem = pItem;
		}

		FeExtNode::CExternNode* pMark = GetMarkByItem(pItem);

		if (pMark && !pMark->AsComposeNode() && m_rpMarkSys.valid() && m_opSystemService.valid())
		{	
			FeUtil::CFreeViewPoint pVP = pMark->GetViewPoint();
			m_opSystemService->Locate(pVP);
		}
	}

	void CMarkPluginInterface::SlotMenuOpenPropertyWidget()
	{
		if(m_p3DTreeWidget && !m_bPropertyWidgetShow)
		{
			ShowMarkPropertyWidget(GetMarkByItem(m_pActiveItem), false);
		}
	}

	void CMarkPluginInterface::SlotMenuDeleteMark()
	{
		FeExtNode::CExternNode* pNode = GetMarkByItem(m_pActiveItem);
		if(pNode && m_rpMarkSys.valid())
		{
			FeExtNode::CExternNode* pParentGuess = 0;
			if(m_pActiveItem && m_pActiveItem->parent())
			{
				pParentGuess = GetMarkByItem(m_pActiveItem->parent());
			}

			FeExtNode::CExComposeNode* pParentNode = pParentGuess ? 
				pParentGuess->AsComposeNode() : m_rpMarkSys->GetRootMark();

			DeleteNodeFromMarkSys(pNode, pParentNode);
			//if(m_rpMarkSys->RemoveMark(pParentNode, pNode))
			{
				m_p3DTreeWidget->GetTreeWidget()->RemoveTreeItem(m_pActiveItem);
				m_rpMarkSys->Save();
			}

			m_pActiveItem = m_p3DTreeWidget->GetTreeWidget()->currentItem();
		}
	}

	
	void CMarkPluginInterface::SlotMenuClearMark()
	{
		FeExtNode::CExternNode* pNode = GetMarkByItem(m_pActiveItem);
		if(pNode)
		{
			FeExtNode::CExComposeNode* pParent = pNode->AsComposeNode();
			if(pParent && m_rpMarkSys.valid())
			{
				m_p3DTreeWidget->GetTreeWidget()->RemoveTreeItemChilds(m_pActiveItem);
				//pParent->Clear();
				FeExtNode::CExComposeNodeOption::VecExternNodes children = pParent->GetChildren();
				for(int n = 0; n < children.size(); n++)
				{
					DeleteNodeFromMarkSys(children.at(n), pParent);
				}

				m_rpMarkSys->Save();
			}
		}
	}

	void CMarkPluginInterface::SlotNodeStateChanged( unsigned int eType)
	{
		CPropertyWidget* pWidget = dynamic_cast<CPropertyWidget*>(m_pCurrentWidget);
		if(pWidget)
		{
			pWidget->HandleSDKEvent(eType);
		}
	}

	CFreeMainWindow* CMarkPluginInterface::GetMainWindow()
	{
		return m_pMainWindow;
	}

	void CMarkPluginInterface::DeleteCurrentMark()
	{
		/// 关闭菜单选中状态 
		m_markMenuMgr.SetMutex(NULL);

		/// 停止事件捕获 
		m_rpMarkHandler->StopCapture();

		if(m_bPropertyWidgetShow)
		{
			m_pCurrentWidget->close();
		}

		if (m_rpCurMarkNode.valid())
		{
			SlotPropertyCancel(m_rpCurMarkNode.get());
		}
	}

	bool CMarkPluginInterface::IsDrawingOrEditing()
	{
		return (m_bCreatingMark || m_bPropertyWidgetShow);
	}

	CMarkVisitProcessor& CMarkPluginInterface::GetMarkProcessor()
	{
		return m_markProcessor;
	}

	FeManager::CFreeMarkSys* CMarkPluginInterface::GetMarkSys()
	{ 
		return m_rpMarkSys.get();
	}

	CFree3DDockTreeWidget* CMarkPluginInterface::GetTreeDockWidget()
	{ 
		return m_p3DTreeWidget; 
	}

	void CMarkPluginInterface::SetActiveMarkNode( FeExtNode::CExternNode* pNode )
	{
		if(pNode) 
		{
			m_rpCurMarkNode = pNode; 
		}
	}

	void CMarkPluginInterface::SuccessToDraw()
	{
		if( IsDrawingOrEditing() )
		{
			/// 关闭菜单选中状态 
			m_markMenuMgr.ResetMenu();

			/// 停止事件捕获 
			m_rpMarkHandler->StopCapture();

			/// 显示标记属性对话框
			if(!m_bPropertyWidgetShow)
			{
				ShowMarkPropertyWidget(m_rpCurMarkNode.get(), true); 
			}
		}
	}

	void CMarkPluginInterface::FailedToDraw()
	{
		if( IsDrawingOrEditing() )
		{
			/// 关闭菜单选中状态 
			m_markMenuMgr.ResetMenu();

			/// 停止事件捕获 
			m_rpMarkHandler->StopCapture();

			if(m_rpMarkSys.valid() && m_bCreatingMark)
			{
				m_rpMarkSys->RemoveMark(m_rpCurMarkNode);
			}
			
			ResetAllState();
		}
	}

}


