#include <service/ServicePluginInterface.h>
#include <mainWindow/FreeToolBar.h>
#include <mainWindow/FreeMainWindow.h>
#include <mainWindow/FreeDockFrame.h>
#include <mainWindow/FreeUtil.h>

#include <FeUtils/PathRegistry.h>

#include <QMenu>
#include <QFileDialog>
#include <FeServiceProvider/ServiceLayer.h>
namespace FreeViewer
{
	CServicePluginInterface::CServicePluginInterface(CFreeMainWindow *pMainWindow)
		: CUIObserver(pMainWindow)
		, m_pServiceDockTree(NULL)
		, m_pOGCServiceWidget(NULL)
		, m_pTileServiceWidget(NULL)
		, m_pDataServiceWidget(NULL)
		, m_pOGCServicePropertyWidget(NULL)
		, m_pOGCLayerPropertyWidget(NULL)
		, m_pServiceLayerPropertyWidget(NULL)
		, m_pItemCurrent(NULL)
		, m_pServiceManager(NULL)
		, m_bUpdateFlag(false)
		//, m_pWarningWidget(NULL)
		, m_bIsShowProperty(false)
		, m_bItemEditState(false)
		, m_pCustomLayerTree(NULL)
	{
		// 添加菜单栏
		m_strMenuTitle = tr("Service");

		InitWidget();
	}

	CServicePluginInterface::~CServicePluginInterface()
	{
		SaveData();
		m_pServiceManager->CleanService();

		if(m_pLocalDiskTree)
		{
			delete m_pLocalDiskTree;
			m_pLocalDiskTree = NULL;
		}
	}

	void CServicePluginInterface::InitWidget()
	{
		// 实例化服务系统树
		m_pServiceDockTree = m_pMainWindow->GetServiceDockTree();
		if (NULL == m_pServiceDockTree)
		{
			return;
		}
		m_pServiceDockTree->PushBackSystem(E_SERVICE, this);

		m_pActionOGC = CreateMenuAndToolAction(
			tr("Add OGC") /*+ "(&O)"*/,
			":/images/icon/OGCService_normal.png",
			":/images/icon/OGCService_normal.png",
			":/images/icon/OGCService_pressed.png",
			":/images/icon/OGCService_normal.png",
			false
			);
		connect(m_pActionOGC, SIGNAL(triggered()), this, SLOT(SlotAddOGCService()));

		m_pActionTile = CreateMenuAndToolAction(
			tr("Add Tile") /*+ "(&X)"*/,
			":/images/icon/TileServer_normal.png",
			":/images/icon/TileServer_normal.png",
			":/images/icon/TileServer_pressed.png",
			":/images/icon/TileServer_normal.png",
			false
			);
		connect(m_pActionTile, SIGNAL(triggered()), this, SLOT(SlotAddTileService()));

		m_pActionData = CreateMenuAndToolAction(
			tr("Add Data") /*+ "(&D)"*/,
			":/images/icon/DataService_normal.png",
			":/images/icon/DataService_normal.png",
			":/images/icon/DataService_pressed.png",
			":/images/icon/DataService_normal.png",
			false
			);
		connect(m_pActionData, SIGNAL(triggered()), this, SLOT(SlotAddDataService()));

		// 添加工具条分割线
		CFreeToolBar* pToolBar = GetToolBar();
		pToolBar->AddSperator();

		// 右键添加图层到场景
		m_pActionAddLayer = new QAction(tr("Add To Scene") + "(&A)", this);
		connect(m_pActionAddLayer, SIGNAL(triggered()), this, SLOT(SlotAddLayer()));

		// 重命名
		m_pActionRename = new QAction(tr("Rename"), this);
		connect(m_pActionRename, SIGNAL(triggered()), this, SLOT(SlotRename()));

		// 右键删除
		m_pActionDelete = new QAction(tr("Delete") + "(&D)", this);
		connect(m_pActionDelete, SIGNAL(triggered()), this, SLOT(SlotDelete()));

		// 右键属性
		m_pActionProperty = new QAction(tr("Property") + "(&R)", this);
		connect(m_pActionProperty, SIGNAL(triggered()), this, SLOT(SlotProperty()));

		// 右键刷新
		m_pActionUpdate = new QAction(tr("Update") + "(&E)", this);
		connect(m_pActionUpdate, SIGNAL(triggered()), this, SLOT(SlotUpdate()));

		InitContext();
	}

	void CServicePluginInterface::InitContext()
	{
		// 本地磁盘服务根节点
		m_pLocalDiskTree = new CLocalDiskTree(m_pServiceDockTree->GetTreeWidget(), QStringList(tr("Local Service")));
		//m_pTreeRootItemLocal = new QTreeWidgetItem(QStringList(tr("Local Service")));
		m_pTreeRootItemLocal = m_pLocalDiskTree->GetTreeRootItem();
		m_pTreeRootItemLocal->setData(0, Qt::UserRole, E_LOCAL_SERVICE_ROOT);
		m_pTreeRootItemLocal->setFlags(Qt::ItemIsEnabled);
		m_pTreeRootItemLocal->setData(0, Qt::CheckStateRole, QVariant());
		m_pServiceDockTree->GetTreeWidget()->AddTopTreeItem(m_pTreeRootItemLocal);
		

		// OGC标准服务根节点
		m_pTreeRootItemOGC = new QTreeWidgetItem(QStringList(tr("OGC Service")));
		m_pTreeRootItemOGC->setData(0, Qt::UserRole, E_OGC_SERVICE_ROOT);
		m_pServiceDockTree->GetTreeWidget()->AddTopTreeItem(m_pTreeRootItemOGC);

		// 实例化WMS根节点
		m_pTreeRootItemWMS = new QTreeWidgetItem(QStringList(tr("WMS")));
		m_pTreeRootItemWMS->setData(0, Qt::UserRole, E_WMS_SERVICE_ROOT);
		m_pTreeRootItemWMS->setIcon(0, QIcon(":/images/icon/service.png"));
		m_pServiceDockTree->GetTreeWidget()->AddTreeItem(m_pTreeRootItemWMS, m_pTreeRootItemOGC);

		// 实例化WMTS根节点
		m_pTreeRootItemWMTS = new QTreeWidgetItem(QStringList(tr("WMTS")));
		m_pTreeRootItemWMTS->setData(0, Qt::UserRole, E_WMTS_SERVICE_ROOT);
		m_pTreeRootItemWMTS->setIcon(0, QIcon(":/images/icon/service.png"));
		m_pServiceDockTree->GetTreeWidget()->AddTreeItem(m_pTreeRootItemWMTS, m_pTreeRootItemOGC);

		// 实例化WFS根节点
		m_pTreeRootItemWFS = new QTreeWidgetItem(QStringList(tr("WFS")));
		m_pTreeRootItemWFS->setData(0, Qt::UserRole, E_WFS_SERVICE_ROOT);
		m_pTreeRootItemWFS->setIcon(0, QIcon(":/images/icon/service.png"));
		m_pServiceDockTree->GetTreeWidget()->AddTreeItem(m_pTreeRootItemWFS, m_pTreeRootItemOGC);

		// 实例化WCS根节点
		m_pTreeRootItemWCS = new QTreeWidgetItem(QStringList(tr("WCS")));
		m_pTreeRootItemWCS->setData(0, Qt::UserRole, E_WCS_SERVICE_ROOT);
		m_pTreeRootItemWCS->setIcon(0, QIcon(":/images/icon/service.png"));
		m_pServiceDockTree->GetTreeWidget()->AddTreeItem(m_pTreeRootItemWCS, m_pTreeRootItemOGC);

		// Tile服务根节点
		m_pTreeRootItemTile = new QTreeWidgetItem(QStringList(tr("Tile Service")));
		m_pTreeRootItemTile->setData(0, Qt::UserRole, E_TILE_SERVICE_ROOT);
		m_pServiceDockTree->GetTreeWidget()->AddTopTreeItem(m_pTreeRootItemTile);

		// 实例化TMS根节点
		m_pTreeRootItemTMS = new QTreeWidgetItem(QStringList(tr("TMS")));
		m_pTreeRootItemTMS->setData(0, Qt::UserRole, E_TMS_SERVICE_ROOT);
		m_pTreeRootItemTMS->setIcon(0, QIcon(":/images/icon/service.png"));
		m_pServiceDockTree->GetTreeWidget()->AddTreeItem(m_pTreeRootItemTMS, m_pTreeRootItemTile);

		// 实例化XYZ根节点
		m_pTreeRootItemXYZ = new QTreeWidgetItem(QStringList(tr("XYZ")));
		m_pTreeRootItemXYZ->setData(0, Qt::UserRole, E_XYZ_SERVICE_ROOT);
		m_pTreeRootItemXYZ->setIcon(0, QIcon(":/images/icon/service.png"));
		m_pServiceDockTree->GetTreeWidget()->AddTreeItem(m_pTreeRootItemXYZ, m_pTreeRootItemTile);

		// 数据集服务根节点
		m_pTreeRootItemData = new QTreeWidgetItem(QStringList(tr("Data Service")));
		m_pTreeRootItemData->setData(0, Qt::UserRole, E_DATA_SERVICE_ROOT);
		m_pServiceDockTree->GetTreeWidget()->AddTopTreeItem(m_pTreeRootItemData);

		// 实例化服务序列化和反序列化管理
		m_pServiceManager = new FeServiceProvider::ServiceManager();

		// 初始化本地磁盘服务
		m_pLocalDiskTree->Initialize();

		// 初始化加载服务
		LoadData();

		m_vecService.push_back(m_pTreeRootItemWMS);
		m_vecService.push_back(m_pTreeRootItemWMTS);
		m_vecService.push_back(m_pTreeRootItemWFS);
		m_vecService.push_back(m_pTreeRootItemWCS);

		InitExpandTree();
	}

	void CServicePluginInterface::SetCustomLayerTree(CCustomLayerTree * pCustomLayerTree)
	{
		m_pCustomLayerTree = pCustomLayerTree;
	}

	void CServicePluginInterface::InitExpandTree()
	{
		m_pTreeRootItemLocal->setExpanded(true);
		m_pTreeRootItemOGC->setExpanded(true);
		m_pTreeRootItemTile->setExpanded(true);
		m_pTreeRootItemData->setExpanded(true);
	}

	void CServicePluginInterface::SlotAddOGCService()
	{
		if (NULL == m_pOGCServiceWidget)
		{
			m_pOGCServiceWidget = new COGCServiceWidget(m_pMainWindow);
			m_pOGCServiceWidget->Reset(m_pItemCurrent);
			m_pOGCServiceWidget->ShowDialogNormal();

			connect(m_pOGCServiceWidget, SIGNAL(SignalAddOGCService(FeServiceProvider::ServiceProvider *)), this, SLOT(SlotAddServiceToTree( FeServiceProvider::ServiceProvider *)));
			connect(m_pOGCServiceWidget, SIGNAL(SignalAddServiceFinished()), this, SIGNAL(SignalAddServiceFinished()));
			connect(m_pOGCServiceWidget, SIGNAL(SignalResetCurrentItem()), this, SLOT(SlotResetCurrentItem()));
		}
		else
		{
			m_pOGCServiceWidget->Reset(m_pItemCurrent);
			m_pOGCServiceWidget->ShowDialogNormal();
		}
	}

	void CServicePluginInterface::SlotAddTileService()
	{
		if (NULL == m_pTileServiceWidget)
		{
			m_pTileServiceWidget = new CTileServiceWidget(m_pMainWindow);

			m_pTileServiceWidget->Reset(m_pItemCurrent);
			m_pTileServiceWidget->ShowDialogNormal();

			connect(m_pTileServiceWidget, SIGNAL(SignalAddTileService(FeServiceProvider::ServiceProvider *)), this, SLOT(SlotAddServiceToTree( FeServiceProvider::ServiceProvider *)));
			connect(m_pTileServiceWidget, SIGNAL(SignalAddServiceFinished()), this, SIGNAL(SignalAddServiceFinished()));
			connect(m_pTileServiceWidget, SIGNAL(SignalResetCurrentItem()), this, SLOT(SlotResetCurrentItem()));
		}
		else
		{
			m_pTileServiceWidget->Reset(m_pItemCurrent);
			m_pTileServiceWidget->ShowDialogNormal();
		}
	}

	void CServicePluginInterface::SlotAddDataService()
	{
		if (NULL == m_pDataServiceWidget)
		{
			m_pDataServiceWidget = new CDataServiceWidget(m_pMainWindow);
			m_pDataServiceWidget->Reset();
			m_pDataServiceWidget->ShowDialogNormal();

			connect(m_pDataServiceWidget, SIGNAL(SignalAddDataService(FeServiceProvider::ServiceProvider *)), this, SLOT(SlotAddServiceToTree( FeServiceProvider::ServiceProvider *)));
			connect(m_pDataServiceWidget, SIGNAL(SignalAddServiceFinished()), this, SIGNAL(SignalAddServiceFinished()));
		}
		else
		{
			m_pDataServiceWidget->Reset();
			m_pDataServiceWidget->ShowDialogNormal();
		}
	}

	void CServicePluginInterface::SlotAddServiceToTree( FeServiceProvider::ServiceProvider *pServiceProvider )
	{
		FeServiceProvider::WMSServiceProvider *pWMS = dynamic_cast<FeServiceProvider::WMSServiceProvider *>(pServiceProvider);
		if (pWMS)
		{
			AddItemService(pWMS, m_pTreeRootItemWMS, E_WMS_SERVICE_NODE);
		}
		
		FeServiceProvider::WMTSServiceProvider *pWMTS = dynamic_cast<FeServiceProvider::WMTSServiceProvider *>(pServiceProvider);
		if (pWMTS)
		{
			AddItemService(pWMTS, m_pTreeRootItemWMTS, E_WMTS_SERVICE_NODE);
		}

		FeServiceProvider::WFSServiceProvider *pWFS = dynamic_cast<FeServiceProvider::WFSServiceProvider *>(pServiceProvider);
		if (pWFS)
		{
			AddItemService(pWFS, m_pTreeRootItemWFS, E_WFS_SERVICE_NODE);
		}

		FeServiceProvider::WCSServiceProvider *pWCS = dynamic_cast<FeServiceProvider::WCSServiceProvider *>(pServiceProvider);
		if (pWCS)
		{
			AddItemService(pWCS, m_pTreeRootItemWCS, E_WCS_SERVICE_NODE);
		}

		FeServiceProvider::TMSServiceProvider *pTMS = dynamic_cast<FeServiceProvider::TMSServiceProvider *>(pServiceProvider);
		if (pTMS)
		{
			AddItemService(pTMS, m_pTreeRootItemTMS, E_TMS_SERVICE_NODE);
		}

		FeServiceProvider::XYZServiceProvider *pXYZ = dynamic_cast<FeServiceProvider::XYZServiceProvider *>(pServiceProvider);
		if (pXYZ)
		{
			AddItemService(pXYZ, m_pTreeRootItemXYZ, E_XYZ_SERVICE_NODE);
		}

		FeServiceProvider::LocalDataServiceProvider *pData = dynamic_cast<FeServiceProvider::LocalDataServiceProvider *>(pServiceProvider);
		if (pData)
		{
			AddItemService(pData, m_pTreeRootItemData, E_DATA_SERVICE_NODE);
		}
	}

	void CServicePluginInterface::AddItemService( FeServiceProvider::ServiceProvider *pServiceProvider, QTreeWidgetItem *pParent, EServiceItemType type )
	{
		// 服务序列化和反序列化管理添加子服务
		if (m_pServiceManager)
		{
			m_pServiceManager->AddService(pServiceProvider);
		}

		// 添加子服务节点
		QString strServiceName = pServiceProvider->GetName().c_str();
		QTreeWidgetItem *pItemService = new QTreeWidgetItem(QStringList(strServiceName));
		switch (type)
		{
		case E_WMS_SERVICE_NODE:
		case E_WMTS_SERVICE_NODE:
		case E_WFS_SERVICE_NODE:
		case E_WCS_SERVICE_NODE:
			{
				pItemService->setData(0, Qt::UserRole, E_OGC_SERVICE_FOLDER);
				pItemService->setData(1, Qt::UserRole, QVariant::fromValue(pServiceProvider));
				pItemService->setFlags(pItemService->flags() | Qt::ItemIsEditable);

				if (pServiceProvider->TestNet())
				{
					pItemService->setIcon(0, QIcon(":/images/icon/serviceLink.png"));

					// 添加子图层节点
					pServiceProvider->GetAttr();
					for (int i = 0; i<pServiceProvider->GetLayerCount(); ++i)
					{
						FeServiceProvider::ServiceLayer *pServiceLayer = pServiceProvider->GetLayers().at(i);

						QString strLayerName = ConvertToCurrentEncoding(pServiceLayer->GetName());
						QTreeWidgetItem *pItemLayer = new QTreeWidgetItem(QStringList(strLayerName));
						pItemLayer->setData(0, Qt::UserRole, type);
						pItemLayer->setData(1, Qt::UserRole, QVariant::fromValue(pServiceLayer));
						pItemLayer->setIcon(0, QIcon(":/images/icon/serviceLayer.png"));

						m_pServiceDockTree->GetTreeWidget()->AddTreeItem(pItemLayer, pItemService);
					}
				}
				else
				{
					pItemService->setIcon(0, QIcon(":/images/icon/serviceUnlink.png"));
				}

				m_pServiceDockTree->GetTreeWidget()->AddTreeItem(pItemService, pParent);
			}
			break;

		case E_TMS_SERVICE_NODE:
		case E_XYZ_SERVICE_NODE:
			{
				pItemService->setData(0, Qt::UserRole, type);
				pItemService->setData(1, Qt::UserRole, QVariant::fromValue(pServiceProvider));
				pItemService->setIcon(0, QIcon(":/images/icon/serviceLayer.png"));
				pItemService->setFlags(pItemService->flags() | Qt::ItemIsEditable);
				m_pServiceDockTree->GetTreeWidget()->AddTreeItem(pItemService, pParent);
			}
			break;

		case E_DATA_SERVICE_NODE:
			{
				pItemService->setData(0, Qt::UserRole, type);
				pItemService->setData(1, Qt::UserRole, QVariant::fromValue(pServiceProvider));
				pItemService->setIcon(0, QIcon(":/images/icon/serviceLayer.png"));
				pItemService->setFlags(pItemService->flags() | Qt::ItemIsEditable);
				m_pServiceDockTree->GetTreeWidget()->AddTreeItem(pItemService, pParent);
			}
			break;

		default:
			break;
		}
	}

	void CServicePluginInterface::ContextMenuItem( QTreeWidgetItem* pItem, QPoint point )
	{
		if (NULL == pItem || m_bIsShowProperty)
		{
			return;
		}

		m_pItemCurrent = pItem;

		// 右键菜单
		QMenu *pMenu = NULL;

		EServiceItemType type = (EServiceItemType)pItem->data(0, Qt::UserRole).toInt();
		switch (type)
		{
		case E_LOCAL_SERVICE_FOLDER:
			{
				// 本地磁盘文件夹节点
				pMenu = new QMenu(m_pServiceDockTree->GetTreeWidget());
				pMenu->addAction(m_pActionUpdate);
				pMenu->exec(point);
			}
			break;

		case E_LOCAL_SERVICE_IMAGE_NODE:
		case E_LOCAL_SERVICE_MODEL_NODE:
			{
				// 本地磁盘图层节点
				pMenu = new QMenu(m_pServiceDockTree->GetTreeWidget());
				pMenu->addAction(m_pActionAddLayer);
				pMenu->exec(point);
			}
			break;

		case E_WMS_SERVICE_ROOT:
		case E_WMTS_SERVICE_ROOT:
		case E_WFS_SERVICE_ROOT:
		case E_WCS_SERVICE_ROOT:
			{
				// OGC服务根节点
				pMenu = new QMenu(m_pServiceDockTree->GetTreeWidget());
				pMenu->addAction(m_pActionOGC);
				pMenu->addAction(m_pActionUpdate);
				pMenu->exec(point);
			}
			break;

		case E_OGC_SERVICE_FOLDER:
			{
				// OGC子服务节点
				pMenu = new QMenu(m_pServiceDockTree->GetTreeWidget());
				pMenu->addAction(m_pActionRename);
				pMenu->addAction(m_pActionDelete);
				pMenu->addAction(m_pActionProperty);
				pMenu->exec(point);
			}
			break;

		case E_WMS_SERVICE_NODE:
		case E_WMTS_SERVICE_NODE:
		case E_WFS_SERVICE_NODE:
		case E_WCS_SERVICE_NODE:
			{
				// OGC服务图层节点
				pMenu = new QMenu(m_pServiceDockTree->GetTreeWidget());
				pMenu->addAction(m_pActionAddLayer);
				pMenu->addAction(m_pActionProperty);
				pMenu->exec(point);
			}
			break;

		case E_TMS_SERVICE_ROOT:
		case E_XYZ_SERVICE_ROOT:
			{
				// Tile服务根节点
				pMenu = new QMenu(m_pServiceDockTree->GetTreeWidget());
				pMenu->addAction(m_pActionTile);
				pMenu->exec(point);
			}
			break;

		case E_TMS_SERVICE_NODE:
		case E_XYZ_SERVICE_NODE:
			{
				// Tile服务图层节点
				pMenu = new QMenu(m_pServiceDockTree->GetTreeWidget());
				pMenu->addAction(m_pActionAddLayer);
				pMenu->addAction(m_pActionRename);
				pMenu->addAction(m_pActionDelete);
				pMenu->addAction(m_pActionProperty);
				pMenu->exec(point);
			}
			break;

		case E_DATA_SERVICE_ROOT:
			{
				// 添加的数据根节点
				pMenu = new QMenu(m_pServiceDockTree->GetTreeWidget());
				pMenu->addAction(m_pActionData);
				pMenu->exec(point);
			}
			break;

		case E_DATA_SERVICE_NODE:
			{
				// 数据集图层节点
				pMenu = new QMenu(m_pServiceDockTree->GetTreeWidget());
				pMenu->addAction(m_pActionAddLayer);
				pMenu->addAction(m_pActionRename);
				pMenu->addAction(m_pActionDelete);
				pMenu->addAction(m_pActionProperty);
				pMenu->exec(point);
			}
			break;

		default:
			break;
		}
	}

	void CServicePluginInterface::SlotProperty()
	{
		if (NULL == m_pItemCurrent || m_bIsShowProperty)
		{
			return;
		}

		EServiceItemType type = (EServiceItemType)m_pItemCurrent->data(0, Qt::UserRole).toInt();
		switch (type)
		{
		case E_OGC_SERVICE_FOLDER:
			{
				if (m_pOGCServicePropertyWidget)
				{
					m_pOGCServicePropertyWidget->SetItemCurrent(m_pItemCurrent);
					m_pOGCServicePropertyWidget->ShowDialogNormal();
				}
				else
				{
					m_pOGCServicePropertyWidget = new COGCServicePropertyWidget(m_pMainWindow);
					m_pOGCServicePropertyWidget->SetItemCurrent(m_pItemCurrent);
					m_pOGCServicePropertyWidget->ShowDialogNormal();
				}
			}
			break;

		case E_WMS_SERVICE_NODE:
		case E_WMTS_SERVICE_NODE:
		case E_WFS_SERVICE_NODE:
		case E_WCS_SERVICE_NODE:
			{
				if (m_pOGCLayerPropertyWidget)
				{
					m_pOGCLayerPropertyWidget->SetItemCurrent(m_pItemCurrent);
					m_pOGCLayerPropertyWidget->ShowDialogNormal();
				}
				else
				{
					m_pOGCLayerPropertyWidget = new COGCLayerPropertyWidget(m_pMainWindow);
					m_pOGCLayerPropertyWidget->SetItemCurrent(m_pItemCurrent);
					m_pOGCLayerPropertyWidget->ShowDialogNormal();
				}
			}
			break;

		case E_TMS_SERVICE_NODE:
		case E_XYZ_SERVICE_NODE:
		case E_DATA_SERVICE_NODE:
			{
				if (m_pServiceLayerPropertyWidget)
				{
					m_pServiceLayerPropertyWidget->SetItemCurrent(m_pItemCurrent);
					m_pServiceLayerPropertyWidget->ShowDialogNormal();
				}
				else
				{
					m_pServiceLayerPropertyWidget = new CServiceLayerPropertyWidget(m_pMainWindow);
					m_pServiceLayerPropertyWidget->SetItemCurrent(m_pItemCurrent);
					m_pServiceLayerPropertyWidget->ShowDialogNormal();
				}
			}
			break;

		default:
			break;
		}
	}

	void CServicePluginInterface::SlotDelete()
	{
		if (NULL == m_pItemCurrent || m_bIsShowProperty)
		{
			return;
		}

		EServiceItemType type = (EServiceItemType)m_pItemCurrent->data(0, Qt::UserRole).toInt();
		switch (type)
		{
		case E_OGC_SERVICE_FOLDER:
		case E_TMS_SERVICE_NODE:
		case E_XYZ_SERVICE_NODE:
		case E_DATA_SERVICE_NODE:
			{
				if (m_pServiceManager)
				{
					FeServiceProvider::ServiceProvider *pServiceProvider = m_pItemCurrent->data(1, Qt::UserRole).value<FeServiceProvider::ServiceProvider *>();
					m_pServiceManager->RemoveService(pServiceProvider);

					m_pServiceDockTree->GetTreeWidget()->RemoveTreeItem(m_pItemCurrent);
					m_pServiceDockTree->GetTreeWidget()->clearSelection();
					m_pItemCurrent = NULL;
				}
			}
			break;

		default:
			break;
		}
	}

	void CServicePluginInterface::SlotAddLayer()
	{
		if (NULL == m_pItemCurrent)
		{
			return;
		}
		if (m_pCustomLayerTree == NULL)
		{
			return;
		}

		QString strName = "";
		QString strUrl = "";
		QString strDriver = "";

		EServiceItemType type = (EServiceItemType)m_pItemCurrent->data(0, Qt::UserRole).toInt();
		switch (type)
		{
		case E_LOCAL_SERVICE_IMAGE_NODE:
			{
				strName = m_pItemCurrent->text(0);
				strUrl = m_pItemCurrent->data(1, Qt::UserRole).toString();

				m_pCustomLayerTree->AddImageLayer(strName.toStdString(), strUrl.toStdString(), "");
				
				break;
			}
		case E_LOCAL_SERVICE_MODEL_NODE:
			{
				strName = m_pItemCurrent->text(0);
				strUrl = m_pItemCurrent->data(1, Qt::UserRole).toString();

				 m_pCustomLayerTree->AddFeatureLayer(strName.toStdString(), strUrl.toStdString(), "ogr");
				break;
			}
		case E_WMS_SERVICE_NODE:
			{
				FeServiceProvider::ServiceLayer *pLayer = m_pItemCurrent->data(1, Qt::UserRole).value<FeServiceProvider::ServiceLayer *>();
				if (pLayer != NULL)
				{
				   m_pCustomLayerTree->AddImageLayer(ConvertToCurrentEncoding(pLayer->GetName()).toStdString(), pLayer->GetServiceProvider()->GetServiceUrl(), pLayer->GetServiceProvider()->GetServiceType());
				}

				break;
			}
		case E_WMTS_SERVICE_NODE:
			{
				FeServiceProvider::ServiceLayer *pLayer = m_pItemCurrent->data(1, Qt::UserRole).value<FeServiceProvider::ServiceLayer *>();
				if (pLayer != NULL)
				{
					m_pCustomLayerTree->AddImageLayer(ConvertToCurrentEncoding(pLayer->GetName()).toStdString(), pLayer->GetServiceProvider()->GetServiceUrl(), pLayer->GetServiceProvider()->GetServiceType());
				}

				break;
			}
		case E_WFS_SERVICE_NODE:
			{
				FeServiceProvider::ServiceLayer *pLayer = m_pItemCurrent->data(1, Qt::UserRole).value<FeServiceProvider::ServiceLayer *>();
				if (pLayer != NULL)
				{
					m_pCustomLayerTree->AddFeatureLayer(ConvertToCurrentEncoding(pLayer->GetName()).toStdString(), pLayer->GetServiceProvider()->GetServiceUrl(), pLayer->GetServiceProvider()->GetServiceType());
				}

				break;
			}
		case E_WCS_SERVICE_NODE:
			{
				FeServiceProvider::ServiceLayer *pLayer = m_pItemCurrent->data(1, Qt::UserRole).value<FeServiceProvider::ServiceLayer *>();
				if (pLayer != NULL)
				{
					m_pCustomLayerTree->AddImageLayer(ConvertToCurrentEncoding(pLayer->GetName()).toStdString(), pLayer->GetServiceProvider()->GetServiceUrl(), pLayer->GetServiceProvider()->GetServiceType());
				}

				break;
			}
		case E_TMS_SERVICE_NODE:
			{
				FeServiceProvider::ServiceProvider *pServiceProvider = m_pItemCurrent->data(1, Qt::UserRole).value<FeServiceProvider::ServiceProvider *>();
				std::string type = pServiceProvider->GetServiceType();
				std::string url = pServiceProvider->GetServiceUrl();

				m_pCustomLayerTree->AddImageLayer(pServiceProvider->GetName().c_str(), url, type);

				break;
			}
		case E_XYZ_SERVICE_NODE:
			{
				FeServiceProvider::ServiceProvider *pServiceProvider = m_pItemCurrent->data(1, Qt::UserRole).value<FeServiceProvider::ServiceProvider *>();
				std::string type = pServiceProvider->GetServiceType();
				std::string url = pServiceProvider->GetServiceUrl();

				m_pCustomLayerTree->AddImageLayer(pServiceProvider->GetName().c_str(), url, type);

				break;
			}
		case E_DATA_SERVICE_NODE:
			{
				FeServiceProvider::ServiceProvider *pServiceProvider = m_pItemCurrent->data(1, Qt::UserRole).value<FeServiceProvider::ServiceProvider *>();
				std::string type = pServiceProvider->GetServiceType();
				std::string url = pServiceProvider->GetServiceUrl();
				
				m_pCustomLayerTree->AddImageLayer(pServiceProvider->GetName().c_str(), url, pServiceProvider->GetPluginType());
				

				break;
			}
			
		default:
			break;
		}
	}

	void CServicePluginInterface::SlotRename()
	{
		if (NULL == m_pItemCurrent || m_bIsShowProperty)
		{
			return;
		}

		EServiceItemType type = (EServiceItemType)m_pItemCurrent->data(0, Qt::UserRole).toInt();
		switch (type)
		{
		case E_OGC_SERVICE_FOLDER:
		case E_TMS_SERVICE_NODE:
		case E_XYZ_SERVICE_NODE:
		case E_DATA_SERVICE_NODE:
			{
				m_bIsShowProperty = true;
				m_bItemEditState = true;

				m_strEditItemName = m_pItemCurrent->text(0);
				m_pServiceDockTree->GetTreeWidget()->editItem(m_pItemCurrent, 0);

				QWidget* pWidget = m_pServiceDockTree->GetTreeWidget()->itemWidget(m_pItemCurrent, 0);
				if(pWidget)
				{
					pWidget->setContextMenuPolicy(Qt::NoContextMenu);
					pWidget->setStyleSheet("background-color: rgb(31,57,82);");
				}
			}
			break;
		}
	}

	void CServicePluginInterface::LoadData()
	{
		std::string strPath = FeFileReg->GetFullPath("config/serviceConfig.xml");

		FeServiceProvider::ServiceManager serviceManager;
		serviceManager.DeSerialize(strPath);

		std::vector<FeServiceProvider::ServiceProvider *> providers = serviceManager.GetServices();
		for (std::vector<FeServiceProvider::ServiceProvider *>::iterator iter=providers.begin(); iter!=providers.end(); ++iter)
		{
			if (*iter != NULL)
			{
				SlotAddServiceToTree(*iter);
			}
		}
	}

	void CServicePluginInterface::SaveData()
	{
		std::string serviceConfig = FeFileReg->GetFullPath("config/serviceConfig.xml");

		if (m_pServiceManager)
		{
			m_pServiceManager->Serialize(serviceConfig);
		}
	}

	void CServicePluginInterface::ScanDrivers( QString strPath )
	{
// 		if (strPath.isEmpty())
// 		{
// 			QFileInfoList driversInfoList = QDir::drives();
// 			int i = 0;
// 			while(i < driversInfoList.size())
// 			{
// 				QFileInfo driverInfo = driversInfoList.at(i++);
// 
// 				AddItem(driverInfo);
// 			}
// 		}
// 		else
// 		{
// 			QDir dir(strPath);
// 			if (dir.exists())
// 			{
// 				QFileInfoList fileInfoList = dir.entryInfoList();
// 				int i=0;
// 				while (i < fileInfoList.size())
// 				{
// 					QFileInfo fileInfo = fileInfoList.at(i++);
// 					AddItem(fileInfo);
// 				}
// 			}
// 		}
	}

	void CServicePluginInterface::AddItem( QFileInfo &fileInfo )
	{
// 		if (fileInfo.isDir())   // 路径
// 		{
// 			if ( (fileInfo.fileName() == ".") || (fileInfo.fileName() == "..") )
// 			{
// 				return;
// 			}
// 
// 			QString strDirName = fileInfo.fileName();
// 			QString strDirPath = fileInfo.filePath();
// 			QString strFullPath = fileInfo.absolutePath();
// 
// 			if (!m_hashPathItem.contains(strFullPath))
// 			{
// 				QTreeWidgetItem *pItem = new QTreeWidgetItem(QStringList(strDirPath));
// 				pItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
// 				pItem->setIcon(0, QIcon(":/images/icon/folder.png"));
// 				pItem->setData(0, Qt::UserRole, E_LOCAL_SERVICE_FOLDER);
// 				m_pServiceDockTree->GetTreeWidget()->AddTreeItem(pItem, m_pTreeRootItemLocal);
// 				m_hashPathItem.insert(strFullPath, pItem);
// 			}
// 			else
// 			{
// 				QTreeWidgetItem *pParentItem = m_hashPathItem.value(strFullPath);
// 				QTreeWidgetItem *pChildItem = new QTreeWidgetItem(QStringList(strDirName));
// 				pChildItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
// 				pChildItem->setIcon(0, QIcon(":/images/icon/folder.png"));
// 				pChildItem->setData(0, Qt::UserRole, E_LOCAL_SERVICE_FOLDER);
// 
// 				int i=0;
// 				for (i=0; i<pParentItem->childCount(); i++)
// 				{
// 					QString strChild = pParentItem->child(i)->text(0);
// 					if (strDirName == strChild)
// 					{
// 						m_mapAddItems[pParentItem->child(i)] = 1;
// 						break;
// 					}
// 				}
// 
// 				if(i == pParentItem->childCount())
// 				{
// 					m_pServiceDockTree->GetTreeWidget()->AddTreeItem(pChildItem, pParentItem);
// 					m_hashPathItem.insert(fileInfo.absoluteFilePath(), pChildItem);
// 
// 					if (m_bUpdateFlag)
// 					{
// 						m_mapAddItems.insert(pChildItem, 1);
// 					}
// 					else
// 					{
// 						m_mapAddItems.insert(pChildItem, 0);
// 					}
// 				}
// 			}
// 		}
// 		else if (fileInfo.isFile()) // 文件
// 		{
// 			QString strFileName = fileInfo.fileName();
// 			QString strFilePath = fileInfo.absolutePath();
// 			QString strFullPath = strFilePath + "/" + strFileName;
// 
// 			// 过滤出img、tif、shp、kml文件
// 			QString strSuffix = fileInfo.suffix();
// 
// 			if ( (strSuffix=="img") || (strSuffix=="tif") || (strSuffix=="shp") || (strSuffix=="kml") )
// 			{
// 				if (m_hashPathItem.contains(strFilePath))
// 				{
// 					QTreeWidgetItem *pParentItem = m_hashPathItem.value(strFilePath);
// 					QTreeWidgetItem *pChildItem = new QTreeWidgetItem(QStringList(strFileName));
// 
// 					if (strSuffix == "img" || strSuffix == "tif")
// 					{
// 						pChildItem->setIcon(0, QIcon(":/images/icon/img.png"));
// 						pChildItem->setData(0, Qt::UserRole, E_LOCAL_SERVICE_IMAGE_NODE);
// 					}
// 					else if (strSuffix == "shp" || strSuffix == "kml")
// 					{
// 						pChildItem->setIcon(0, QIcon(":/images/icon/shp.png"));
// 						pChildItem->setData(0, Qt::UserRole, E_LOCAL_SERVICE_MODEL_NODE);
// 					}
// 					pChildItem->setData(1, Qt::UserRole, QVariant::fromValue(strFullPath));
// 
// 					int i=0;
// 					for (i=0; i<pParentItem->childCount(); i++)
// 					{
// 						QString strChild = pParentItem->child(i)->text(0);
// 						if (strFileName == strChild)
// 						{
// 							m_mapAddItems[pParentItem->child(i)] = 1;
// 							break;
// 						}
// 					}
// 
// 					if (i == pParentItem->childCount())
// 					{
// 						m_pServiceDockTree->GetTreeWidget()->AddTreeItem(pChildItem, pParentItem);
// 
// 						if (m_bUpdateFlag)
// 						{
// 							m_mapAddItems.insert(pChildItem, 1);
// 						}
// 						else
// 						{
// 							m_mapAddItems.insert(pChildItem, 0);
// 						}
// 					}
// 				}
// 			}
// 		}
	}

	void CServicePluginInterface::ItemClicked( QTreeWidgetItem *item )
	{
		if(m_bItemEditState)
		{
			m_bItemEditState = false;
			m_bIsShowProperty = false;

		}
		m_pItemCurrent = item;
	}

	void CServicePluginInterface::ItemDoubleClicked( QTreeWidgetItem *item )
	{
// 		EServiceItemType type = (EServiceItemType)(item->data(0, Qt::UserRole).toInt());
// 		switch (type)
// 		{
// 		case E_LOCAL_SERVICE_ROOT:
// 		case E_LOCAL_SERVICE_FOLDER:
// 			{
// // 				if (!m_bUpdateFlag)
// // 				{
// // 					int i=0;
// // 					for (i=0; i<m_vecDoubleItems.count(); i++)
// // 					{
// // 						if (item == m_vecDoubleItems.at(i))
// // 						{
// // 							break;
// // 						}
// // 					}
// // 					if (i == m_vecDoubleItems.count())
// // 					{
// // 						m_vecDoubleItems.push_back(item);
// // 					}
// // 				}
// // 
// // 				QString strFullPath = item->text(0);
// // 
// // 				if ( (NULL != item->parent()) && (item->parent() != m_pTreeRootItemLocal) )
// // 				{
// // 					strFullPath = m_hashPathItem.key(item);
// // 				}
// // 
// // 				ScanDrivers(strFullPath);
// 			}
// 			break;
// 
// 		default:
// 			break;
// 		}
	}

	void CServicePluginInterface::EditRenameItem( QTreeWidgetItem *pItem, bool bState )
	{
		if (NULL == pItem)
		{
			return;
		}

		m_bIsShowProperty = false;

		QString strName = pItem->text(0);
		strName = strName.simplified();	// 去除前后空白符

		if (strName.isEmpty())
		{
			/*if (NULL == m_pWarningWidget)
			{
				m_pWarningWidget = new CFreeWarningWidget(tr("Warning"), tr("You must enter a name !"));
				m_pWarningWidget->ShowDialogNormal();
			}
			else
			{
				m_pWarningWidget->ShowDialogNormal();
			}*/

			strName = m_strEditItemName;
		}
		else
		{
			// 限制最长输入32个字符
			if (strName.count() <= 32)
			{
				FeServiceProvider::ServiceProvider *pServiceProvider = pItem->data(1, Qt::UserRole).value<FeServiceProvider::ServiceProvider *>();
				if (pServiceProvider)
				{
					pServiceProvider->SetName(strName.toStdString());
				}
			}
			else
			{
				m_strEditItemName = strName.left(32);
				strName = m_strEditItemName;
			}
		}

		pItem->setText(0, strName);
	}

	void CServicePluginInterface::UpdateLocalService()
	{
		if(m_pLocalDiskTree)
		{
			m_pLocalDiskTree->RescanDisk();
		}

// 		m_bUpdateFlag = true;
// 
// 		if (!m_vecDoubleItems.isEmpty())
// 		{
// 			for (int i=0; i<m_vecDoubleItems.count(); i++)
// 			{
// 				ItemDoubleClicked(m_vecDoubleItems.at(i));
// 			}
// 
// 			QMap<QTreeWidgetItem*, int>::iterator iter;
// 			for (iter=m_mapAddItems.begin(); iter!=m_mapAddItems.end();)
// 			{
// 				if (iter.value() == 0)
// 				{
// 					QTreeWidgetItem *item = iter.key();
// 
// 					for (int j=0; j<m_vecDoubleItems.count();)
// 					{
// 						if (item == m_vecDoubleItems.at(j))
// 						{
// 							m_vecDoubleItems.remove(j);
// 						}
// 						else
// 						{
// 							j++;
// 						}
// 					}
// 
// 					m_hashPathItem.remove(item->text(0));
// 					iter = m_mapAddItems.erase(iter);
// 					EServiceItemType type = (EServiceItemType)item->data(0, Qt::UserRole).toInt();
// 					if (E_LOCAL_SERVICE_FOLDER == type)
// 					{
// 						QString str = item->text(0);
// 						m_vecDeleteItems.push_back(item);
// 					}
// 					else if (E_LOCAL_SERVICE_IMAGE_NODE == type || E_LOCAL_SERVICE_MODEL_NODE == type)
// 					{
// 						m_pServiceDockTree->GetTreeWidget()->RemoveTreeItem(item);
// 					}
// 				}
// 				else
// 				{
// 					iter.value() = 0;
// 
// 					iter++;
// 				}
// 			}
// 
// 			while (0 < m_vecDeleteItems.count())
// 			{
// 				for (int i=m_vecDeleteItems.count()-1; i>=0; i--)
// 				{
// 					QString str = m_vecDeleteItems.at(i)->text(0);
// 					//if (0 == m_vecDeleteItems.at(i)->childCount())
// 					{
// 						m_pServiceDockTree->GetTreeWidget()->RemoveTreeItem(m_vecDeleteItems.at(i));
// 						m_vecDeleteItems.remove(i);
// 					}
// 				}
// 			}
// 
// 			m_vecDeleteItems.clear();
// 		}
// 
// 		m_bUpdateFlag = false;
	}

	void CServicePluginInterface::UpdateOGCService()
	{
		if (!m_vecService.isEmpty())
		{
			for (int i=0; i<m_vecService.count(); i++)
			{
				// 根节点
				QTreeWidgetItem *pRootItem = m_vecService.at(i);
				QString strRoot = pRootItem->text(0);

				for (int j=0; j<m_vecService.at(i)->childCount(); j++)
				{
					// 子服务节点
					QTreeWidgetItem *pItem = m_vecService.at(i)->child(j);
					QString str = pItem->text(0);
					FeServiceProvider::ServiceProvider *pServiceProvider = pItem->data(1, Qt::UserRole).value<FeServiceProvider::ServiceProvider*>();

					if (pServiceProvider)
					{
						if (pServiceProvider->TestNet())	// 子服务连接成功
						{
							pItem->setIcon(0, QIcon(":/images/icon/serviceLink.png"));

							m_pServiceDockTree->GetTreeWidget()->RemoveTreeItemChilds(pItem);

							// 添加子图层节点
							pServiceProvider->GetAttr();
							int nLayerCount = pServiceProvider->GetLayerCount();
							for (int i = 0; i<nLayerCount; ++i)
							{
								FeServiceProvider::ServiceLayer *pServiceLayer = pServiceProvider->GetLayers().at(i);

								QString strLayerName = ConvertToCurrentEncoding(pServiceLayer->GetName());
								QTreeWidgetItem *pItemLayer = new QTreeWidgetItem(QStringList(strLayerName));

								EServiceItemType type = (EServiceItemType)pRootItem->data(0, Qt::UserRole).toInt();
								switch (type)
								{
								case E_WMS_SERVICE_ROOT:
									{
										pItemLayer->setData(0, Qt::UserRole, E_WMS_SERVICE_NODE);
									}
									break;
								case E_WMTS_SERVICE_ROOT:
									{
										pItemLayer->setData(0, Qt::UserRole, E_WMTS_SERVICE_NODE);
									}
									break;
								case E_WFS_SERVICE_ROOT:
									{
										pItemLayer->setData(0, Qt::UserRole, E_WFS_SERVICE_NODE);
									}
									break;
								case E_WCS_SERVICE_ROOT:
									{
										pItemLayer->setData(0, Qt::UserRole, E_WCS_SERVICE_NODE);
									}
									break;
								}

								pItemLayer->setData(1, Qt::UserRole, QVariant::fromValue(pServiceLayer));
								pItemLayer->setIcon(0, QIcon(":/images/icon/serviceLayer.png"));

								m_pServiceDockTree->GetTreeWidget()->AddTreeItem(pItemLayer, pItem);
							}
						}
						else	// 子服务连接失败
						{
							pItem->setIcon(0, QIcon(":/images/icon/serviceUnlink.png"));

							m_pServiceDockTree->GetTreeWidget()->RemoveTreeItemChilds(pItem);
						}
					}
				}
			}
		}
	}

	void CServicePluginInterface::BtnClicked( EServiceButtonType type )
	{
		switch (type)
		{
		case E_SERVICE_UPDATE_BTN:
			{
				UpdateLocalService();
				UpdateOGCService();
			}
			break;

		case E_SERVICE_EXPAND_BTN:
			{
				m_pServiceDockTree->GetTreeWidget()->expandAll();

				if (m_pTreeRootItemLocal)
				{
					m_pServiceDockTree->GetTreeWidget()->collapseItem(m_pTreeRootItemLocal);
				}
			}
			break;

		case E_SERVICE_COLLAPSE_BTN:
			{
				m_pServiceDockTree->GetTreeWidget()->collapseAll();
			}
			break;

		case E_SERVICE_DELETE_BTN:
			{
				SlotDelete();
			}
			break;

		default:
			break;
		}
	}

	void CServicePluginInterface::SetCurrentItem( QTreeWidgetItem *pItem )
	{
		m_pItemCurrent = pItem;
	}

	QTreeWidgetItem* CServicePluginInterface::GetCurrentItem()
	{
		return m_pItemCurrent;
	}

	void CServicePluginInterface::SlotServiceCurrentItem( QString strText )
	{
		if (strText.isEmpty())
		{
			return;
		}

		// 本地磁盘
		if (strText == m_pTreeRootItemLocal->text(0))
		{
			emit SignalLayerCurrentItem(m_pTreeRootItemLocal);

			return;
		}
		else
		{
			for (int i=0; i<m_pTreeRootItemLocal->childCount(); i++)
			{
				if (strText == m_pTreeRootItemLocal->child(i)->text(0))
				{
					emit SignalLayerCurrentItem(m_pTreeRootItemLocal->child(i));

					return;
				}
			}
		}

		// OGC
		if (strText == m_pTreeRootItemOGC->text(0))
		{
			emit SignalLayerCurrentItem(m_pTreeRootItemOGC);

			return;
		}
		else
		{
			for (int i=0; i<m_pTreeRootItemOGC->childCount(); i++)
			{
				if (strText == m_pTreeRootItemOGC->child(i)->text(0))
				{
					emit SignalLayerCurrentItem(m_pTreeRootItemOGC->child(i));

					return;
				}
			}
		}

		// Tile
		if (strText == m_pTreeRootItemTile->text(0))
		{
			emit SignalLayerCurrentItem(m_pTreeRootItemTile);

			return;
		}
		else
		{
			for (int i=0; i<m_pTreeRootItemTile->childCount(); i++)
			{
				if (strText == m_pTreeRootItemTile->child(i)->text(0))
				{
					emit SignalLayerCurrentItem(m_pTreeRootItemTile->child(i));

					return;
				}
			}
		}

		// 数据集
		if (strText == m_pTreeRootItemData->text(0))
		{
			emit SignalLayerCurrentItem(m_pTreeRootItemData);

			return;
		}
		else
		{
			for (int i=0; i<m_pTreeRootItemData->childCount(); i++)
			{
				if (strText == m_pTreeRootItemData->child(i)->text(0))
				{
					emit SignalLayerCurrentItem(m_pTreeRootItemData->child(i));

					return;
				}
			}
		}
	}

	void CServicePluginInterface::SlotItemDoubleClicked( EServiceItemType type )
	{
		switch (type)
		{
		case E_OGC_SERVICE_ROOT:
		case E_WMS_SERVICE_ROOT:
		case E_WMTS_SERVICE_ROOT:
		case E_WFS_SERVICE_ROOT:
		case E_WCS_SERVICE_ROOT:
			{
				SlotAddOGCService();

				if(m_pOGCServiceWidget)
				{
					m_pOGCServiceWidget->Reset(type);
				}
			}
			break;

		case E_TILE_SERVICE_ROOT:
		case E_TMS_SERVICE_ROOT:
		case E_XYZ_SERVICE_ROOT:
			{
				SlotAddTileService();

				if(m_pTileServiceWidget)
				{
					m_pTileServiceWidget->Reset(type);
				}
			}
			break;

		case E_DATA_SERVICE_ROOT:
			{
				SlotAddDataService();
			}
			break;

		default:
			break;
		}
	}

	void CServicePluginInterface::SlotUpdate()
	{
		if (NULL == m_pItemCurrent)
		{
			return;
		}

		EServiceItemType type = (EServiceItemType)m_pItemCurrent->data(0, Qt::UserRole).toInt();
		switch (type)
		{
		case E_LOCAL_SERVICE_FOLDER:
			{
				UpdateLocalService();
			}
			break;

		case E_WMS_SERVICE_ROOT:
		case E_WMTS_SERVICE_ROOT:
		case E_WFS_SERVICE_ROOT:
		case E_WCS_SERVICE_ROOT:
			{
				UpdateOGCService();
			}
			break;

		default:
			break;
		}
	}

	void CServicePluginInterface::RKeyPressed()
	{
		SlotProperty();
	}

	void CServicePluginInterface::DeleteKeyPressed()
	{
		SlotDelete();
	}

	void CServicePluginInterface::F2KeyPressed()
	{
		SlotRename();
	}

	void CServicePluginInterface::SlotResetCurrentItem()
	{
		SetCurrentItem(NULL);
	}

}