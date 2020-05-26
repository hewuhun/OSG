#include <layer/CustomLayerTree.h>

#include <mainWindow/FreeMainWindow.h>
#include <mainWindow/FreeToolBar.h>
#include <mainWindow/FreeUtil.h>
#include <layer/PropertyImageLayerDialog.h>
#include <FeUtils/ShpAndKMLTypeParser.h>
namespace FreeViewer
{
	CCustomLayerTree::CCustomLayerTree(CFreeMainWindow* pMainWindow)
		: CUIObserver(pMainWindow)
		, m_rpCustomLayerSys(NULL)
		, m_p3DTreeWidget(NULL)
		, m_pCustomLayerItemRoot(NULL)
		, m_pActiveItem(NULL)
		, m_pImageDialog(NULL)
		, m_pImageDialog2(NULL)
		, m_pElevationDialog(NULL)
		, m_pElevationDialog2(NULL)
		, m_pFeatureDialog(NULL)
		, m_pFeatureDialog2(NULL)
		, m_pFeatureDialog3(NULL)
		, m_pRGBColorFilterDialog(NULL)
		, m_pAddImageLayerAction(NULL)
		, m_pPropertyAction(NULL)
		, m_pAddElevationLayerAction(NULL)
		, m_pAddFeatureLayerAction(NULL)
		, m_pDeleteLayerAction(NULL)
		, m_pClearFolderAction(NULL)
		, m_pColorFilterAction(NULL)
		, m_pFolderWidget(NULL)
		, m_pAddTDTRasterLayerAction(NULL)
		, m_bMouseRightMenu(false)
		, m_pPropertyImageLayerDialog(NULL)
	{
		m_strMenuTitle = tr("Layer");

		

		InitWidget();
	}
						 
	void CCustomLayerTree::InitWidget()
	{
		if (NULL == m_pMainWindow)
		{
			return ;
		}

		m_p3DTreeWidget = m_pMainWindow->Get3DTreeWidget();
		if (NULL == m_p3DTreeWidget)
		{
			return ;
		}
		m_p3DTreeWidget->PushBackSystem(E_CUSTOM_LAYER, this);

		//标题栏右键菜单，添加、删除和清空图层文件夹
		m_pAddLayerFolderAction = new QAction(tr("Add Folder"), this);
		m_pAddLayerFolderAction->setToolTip(tr("Add Folder"));
		m_pAddLayerFolderAction->setDisabled(false);
		connect(m_pAddLayerFolderAction, SIGNAL(triggered()), this, SLOT(SlotAddLayerFolder()));

		m_pDelelteFolderAction = new QAction(tr("Delete"), this);
		m_pDelelteFolderAction->setToolTip(tr("Delete"));
		m_pDelelteFolderAction->setDisabled(false);
		connect(m_pDelelteFolderAction, SIGNAL(triggered()), this, SLOT(SlotDeleteLayerFolder()));

		m_pClearFolderAction = new QAction(tr("Clear"), this);
		m_pClearFolderAction->setToolTip(tr("Clear"));
		m_pClearFolderAction->setDisabled(false);
		connect(m_pClearFolderAction, SIGNAL(triggered()), this, SLOT(SlotClearLayerFolder()));

		//树节点右键菜单，删除图层
		m_pDeleteLayerAction = new QAction(tr("Delete"), this);
		m_pDeleteLayerAction->setToolTip(tr("Delete"));
		m_pDeleteLayerAction->setDisabled(false);
		connect(m_pDeleteLayerAction, SIGNAL(triggered()), this, SLOT(SlotDeleteLayer()));

		//树节点右键菜单，添加图层
		m_pAddImageLayerAction = CreateMenuAndToolAction(
			tr("Add Imagery"),
			QString(":/images/icon/load_image.png"), 
			QString(":/images/icon/load_image.png"), 
			QString(":/images/icon/load_image_press.png"),
			QString(":/images/icon/load_image.png"),
			false);
		connect(m_pAddImageLayerAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddImageLayer(bool)));


		m_pPropertyAction = new QAction(
			tr("Property"), this);
		connect(m_pPropertyAction, SIGNAL(triggered(bool)), this, SLOT(SlotLayerProperty(bool)));


		m_pAddElevationLayerAction = CreateMenuAndToolAction(
			tr("Add Elevation"),
			QString(":/images/icon/load_altitude.png"), 
			QString(":/images/icon/load_altitude.png"), 
			QString(":/images/icon/load_altitude_press.png"),
			QString(":/images/icon/load_altitude.png"),
			false);
		connect(m_pAddElevationLayerAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddImageLayer(bool)));

		m_pAddFeatureLayerAction = CreateMenuAndToolAction(
			tr("Add Feature"),
			QString(":/images/icon/load_vector.png"), 
			QString(":/images/icon/load_vector.png"), 
			QString(":/images/icon/load_vector_press.png"),
			QString(":/images/icon/load_vector.png"),
			false);
		connect(m_pAddFeatureLayerAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddFeatureLayer(bool)));

#ifdef WIN32
//		m_pAddTDTRasterLayerAction = CreateMenuAction(
//			tr("Add Raster Vector"),
//			false,
//			false);
//		connect(m_pAddTDTRasterLayerAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddRastorLayer(bool)));
#endif

		m_pColorFilterAction = CreateMenuAndToolAction(
			tr("Color Filter"),
			QString(":/images/icon/layer_effect.png"),
			QString(":/images/icon/layer_effect.png"),
			QString(":/images/icon/layer_effect_press.png"),
			QString(":/images/icon/layer_effect.png"),
			false);
		connect(m_pColorFilterAction, SIGNAL(triggered(bool)), this, SLOT(SlotColorFilterDialog(bool)));

		//添加工具条分割线
		CFreeToolBar* pToolBar = GetToolBar();
		pToolBar->AddSperator();

		BuildContext();
	}

	void CCustomLayerTree::BuildContext()
	{
		if(m_opSystemService.valid())
		{
// 			FeLayers::CLayerSysOptions opt;
// 			opt.customLayerPath() = m_opSystemService->GetSysConfig().GetCustomLayerConfig();
// 
// 			m_rpCustomLayerSys = new FeLayers::CLayerSys(opt);
// 			m_rpCustomLayerSys->Initialize(m_opSystemService->GetRenderContext());
// 			m_opSystemService->AddAppModuleSys(m_rpCustomLayerSys);

			m_rpCustomLayerSys = dynamic_cast<FeLayers::CLayerSys*>(m_opSystemService->GetModuleSys(FeLayers::LAYER_SYSTEM_CALL_DEFAULT_KEY));

			if(!m_rpCustomLayerSys.valid())
			{
				FeLayers::CLayerSysOptions opt;
				opt.featurePath() = m_opSystemService->GetSysConfig().GetFeatureLayerConfig();
				opt.customLayerPath() = m_opSystemService->GetSysConfig().GetCustomLayerConfig();

				m_rpCustomLayerSys = new FeLayers::CLayerSys(opt);
				m_rpCustomLayerSys->Initialize(m_opSystemService->GetRenderContext());
				m_rpCustomLayerSys->RegistryEffectLayer();
				m_rpCustomLayerSys->RegistryFeatureLayer();
				m_opSystemService->AddAppModuleSys(m_rpCustomLayerSys);
			}


			if (NULL != m_rpCustomLayerSys)
			{
				//创建用户图层的根节点
				m_pCustomLayerItemRoot = new QTreeWidgetItem(m_p3DTreeWidget->GetTreeWidget(), QStringList(tr("Custom Layer")), 0);
				m_pCustomLayerItemRoot->setCheckState(0, Qt::Unchecked);
				m_pCustomLayerItemRoot->setData(0, Qt::UserRole, E_CUSTOM_LAYER_ROOT);
				m_p3DTreeWidget->GetTreeWidget()->AddTopTreeItem(m_pCustomLayerItemRoot);

				m_pImageLayerItemRoot = new QTreeWidgetItem(m_pCustomLayerItemRoot, QStringList(tr("Custom Image Layer")));
				m_pImageLayerItemRoot->setData(0, Qt::UserRole, E_CUSTOM_IMAGE_LAYER_ROOT);
				m_pImageLayerItemRoot->setCheckState(0, Qt::Unchecked);

				m_pElevationLayerItemRoot = new QTreeWidgetItem(m_pCustomLayerItemRoot, QStringList(tr("Custom Elevation Layer")));
				m_pElevationLayerItemRoot->setData(0, Qt::UserRole, E_CUSTOM_ELEVATION_LAYER_ROOT);

				m_pVectorLayerItemRoot = new QTreeWidgetItem(m_pCustomLayerItemRoot, QStringList(tr("Custom Vector Layer")));
				m_pVectorLayerItemRoot->setData(0,Qt::UserRole, E_CUSTOM_VECTOR_LAYER_ROOT);
				
				m_pElevationLayerItemRoot->setCheckState(0, Qt::Unchecked);
				m_pVectorLayerItemRoot->setCheckState(0, Qt::Unchecked);
				LoadData(m_rpCustomLayerSys->GetOptions().customLayerPath());

				m_pCustomLayerItemRoot->setExpanded(true);
			}
		}
	}

	void CCustomLayerTree::LoadData(std::string strPath)
	{
		FeLayers::CCustomLayerReaderWriter m_customRW;
		osgEarth::Config rootConfig = m_customRW.Load(strPath);

		osgEarth::Config layersConfig = rootConfig.child("layers");
		osgEarth::ConfigSet topLevelSet = layersConfig.children();
		osgEarth::ConfigSet::iterator levelItem = topLevelSet.begin();
		for (; levelItem != topLevelSet.end(); levelItem++)
		{
		
		/*	QTreeWidgetItem* pItem;
			QString strKey = ConvertToCurrentEncoding(levelItem->key());
			pItem = new QTreeWidgetItem(m_pCustomLayerItemRoot, QStringList(strKey));
			pItem->setData(0, Qt::UserRole, E_CUSTOM_LAYER_FOLDER);
			pItem->setCheckState(0, Qt::Unchecked);

			m_pActiveItem = pItem;*/
			
			
			ParseLayer(*levelItem);
		}
	}

	void CCustomLayerTree::SaveData()
	{
		if (NULL == m_pCustomLayerItemRoot)
		{
			return ;
		}

		int nFolderCount = m_pCustomLayerItemRoot->childCount();
		FeLayers::CCustomLayerReaderWriter customRW;
		//for (int i = 0; i < nFolderCount; i++)
		//{
		//	QTreeWidgetItem* pItem = m_pCustomLayerItemRoot->child(i);
		//	if (pItem)
		//	{
		//		EItemType type = (EItemType)pItem->data(0, Qt::UserRole).toInt();
		//		if (type == E_CUSTOM_LAYER_FOLDER)
		//		{
		//			std::string strKey = ConvertToSDKEncoding(pItem->text(0));
		//			osgEarth::Config folderConfig(strKey);
		//			UnParseLayer(pItem, folderConfig);
		//			customRW.AppendConfig(folderConfig);
		//		}
		//		/*else if (type == E_CUSTOM_LAYER_NODE && !m_curLayerConfig.Empty())
		//		{
		//			UnParseLayer(pItem, m_curLayerConfig);
		//			customRW.AppendConfig(m_curLayerConfig);
		//		}*/
		//	}
		//}

		//int imageCount = m_pImageLayerItemRoot->childCount();
		//int eleCount = m_pElevationLayerItemRoot->childCount();
		//int vecCount = m_pVectorLayerItemRoot->childCount();
	
		int count = m_pCustomLayerItemRoot->childCount();

		for (int i = 0; i < count; ++i)
		{
			QTreeWidgetItem* pItem = m_pCustomLayerItemRoot->child(i);
			if (pItem)
			{
				int itemCount = pItem->childCount();
				for (int j = 0; j < itemCount; ++j)
				{
					QTreeWidgetItem * temp = pItem->child(j);
					/*	osgEarth::Config conf = m_curLayerConfig.getConfig();
					conf.key() =  "layer";*/
					UnParseLayer(temp, customRW);
				}
			}
		}

		
		customRW.Save(m_rpCustomLayerSys->GetOptions().customLayerPath());
	}

	void CCustomLayerTree::ParseLayer(osgEarth::Config& layersConfig)
	{
		/*if (layersConfig.key() == "layer" && m_pCustomLayerItemRoot)
		{
			
			FeLayers::IFeLayer * pLayer =  m_rpCustomLayerSys->AddLayer(layersConfig);
			if (pLayer != NULL)
			{
				QIcon icon = m_pAddImageLayerAction->icon();
				AddItem(m_pCustomLayerItemRoot, pLayer->AsImageLayer(), pLayer->GetLayerConfig());
			}
			return;
		}
		*/

		//初始化影像图层
		/*	osgEarth::ConfigSet imageSet = layersConfig.children("layer");
		osgEarth::ConfigSet::iterator itImage = imageSet.begin();
		while(itImage != imageSet.end())
		{
		osgEarth::Config imageConfig = *itImage;*/

			FeLayers::IFeLayer * pLayer = m_rpCustomLayerSys->AddLayer(layersConfig);
			if (pLayer != NULL)
			{
				QTreeWidgetItem * item;
				if (pLayer->IsImageLayer())
				{
					item = m_pImageLayerItemRoot;
				}
				else if(pLayer->IsElevationLayer())
				{
					item = m_pElevationLayerItemRoot;
				}
				else if (pLayer->IsModelLayer())
				{
					item = 	m_pVectorLayerItemRoot;
				}
				AddItem(item, pLayer, pLayer->GetLayerConfig());
			}
				 
				
	/*		}
			itImage++;
		}*/

		
	}

	void CCustomLayerTree::UnParseLayer( QTreeWidgetItem* pItem, FeLayers::CCustomLayerReaderWriter & cw )
	{
		if (NULL == pItem)
		{
			return ;
		}


		QTreeWidgetItem* pChildItem = pItem;
		if (pChildItem)
		{
			int nType = (EItemType)pChildItem->data(0, Qt::UserRole).toInt();
			if (nType == E_CUSTOM_LAYER_NODE)
			{
				FeLayers::IFeLayer* pLayer = pChildItem->data(1, Qt::UserRole).value<FeLayers::IFeLayer*>();
				osgEarth::Config layerConfig = pLayer->GetLayerConfig().getConfig();
				layerConfig.key() = "layer";
				cw.AppendConfig(layerConfig);

			}
		}
		
		
	}

	void CCustomLayerTree::AddItem(QTreeWidgetItem* pParentItem, FeLayers::IFeLayer* pLayer,  FeLayers::CFeLayerConfig config)
	{
		if(pParentItem && pLayer)
		{
			bool bVisible = false;
		
			if (config.visible().value() == true)
			{
				bVisible = true;
			}

			m_curLayerConfig = config;
			QTreeWidgetItem * pParent = NULL;

			if (pLayer->IsImageLayer())
			{
				pParent = m_pImageLayerItemRoot;
			}
			else if (pLayer->IsElevationLayer())
			{
				pParent = m_pElevationLayerItemRoot;
			}
			else if (pLayer->IsModelLayer())
			{
			    pParent = m_pVectorLayerItemRoot;
			}

			QString strName = QString(config.name().value().c_str());
			QTreeWidgetItem* pItem = new QTreeWidgetItem(pParent, QStringList(strName));
			pItem->setData(0, Qt::UserRole, E_CUSTOM_LAYER_NODE);
			pItem->setData(1, Qt::UserRole, QVariant::fromValue(pLayer));
			//m_p3DTreeWidget->GetTreeWidget()->AddTreeItem(pItem, pParent);

			if (bVisible)
			{
				pItem->setCheckState(0, Qt::Checked);
			}
			else
			{
				pItem->setCheckState(0, Qt::Unchecked);
			}

			m_p3DTreeWidget->GetTreeWidget()->setCurrentItem(pItem);

			pParentItem->setExpanded(true);
		}
	}

	void CCustomLayerTree::DeleteItem( QTreeWidgetItem* pItem )
	{
		if (NULL == pItem)
		{
			return;
		}

		m_bMouseRightMenu = false;

		EItemType type = (EItemType)(pItem->data(0, Qt::UserRole).toInt());
		switch(type)
		{
		case E_CUSTOM_LAYER_FOLDER:
			{
				ClearItem(pItem);
				m_p3DTreeWidget->GetTreeWidget()->RemoveTreeItem(pItem);
			}
			break;

		case E_CUSTOM_LAYER_NODE:
			{
				if (m_rpCustomLayerSys.valid())
				{
					m_rpCustomLayerSys->DeleteLayer(pItem->data(1, Qt::UserRole).value<FeLayers::IFeLayer*>());
					m_p3DTreeWidget->GetTreeWidget()->RemoveTreeItem(pItem);
				}
			}
			break;

		default:
			break;
		}
	}

	void CCustomLayerTree::ClearItem( QTreeWidgetItem* pItem )
	{
		if (NULL == pItem)
		{
			return;
		}

		m_bMouseRightMenu = false;
		
		int nCount = pItem->childCount();
		for (int i = 0; i < nCount; i++)
		{
			//删除一个孩子后，下次进来孩子数量就少一个
			QTreeWidgetItem* pChild = pItem->child(0);
			if (pChild)
			{
				EItemType type = (EItemType)(pChild->data(0, Qt::UserRole).toInt());
				switch(type)
				{
				case E_CUSTOM_LAYER_FOLDER:
					{
						ClearItem(pChild);
						m_p3DTreeWidget->GetTreeWidget()->RemoveTreeItem(pChild);
					}
					break;

				case E_CUSTOM_LAYER_NODE:
					{
						if (m_rpCustomLayerSys.valid())
						{
							m_rpCustomLayerSys->DeleteLayer(pChild->data(1, Qt::UserRole).value<FeLayers::IFeLayer*>());
							m_p3DTreeWidget->GetTreeWidget()->RemoveTreeItem(pChild);
						}
					}
					break;

				default:
					break;
				}
			}
		}
	}

	void CCustomLayerTree::SlotItemsVisibleChange(QTreeWidgetItem* pItem, bool bShow)
	{
		if (NULL == pItem)
		{
			return ;
		}

		//更改图层的显示或者隐藏
		FeLayers::IFeLayer* pLayer = pItem->data(1, Qt::UserRole).value<FeLayers::IFeLayer*>();

		if (pLayer)
		{
			pLayer->SetVisible(bShow);
			SaveData();
			return;
		}
	}

	void CCustomLayerTree::SlotAddLayerFolder()
	{
		if (NULL == m_pFolderWidget)
		{
			m_pFolderWidget = new CFolderWidget(m_pMainWindow);
			m_pFolderWidget->SetTitleText("3D Layer Folder");
			connect(m_pFolderWidget, SIGNAL(SignalFolderName(QString, bool)), this, SLOT(SlotAddLayerFolder(QString, bool)));
		}

		m_pFolderWidget->ShowDialogNormal();
	}

	void CCustomLayerTree::SlotAddLayerFolder(QString strName, bool bState)
	{
		if (bState && !strName.isEmpty())
		{
			QTreeWidgetItem* pItem = new QTreeWidgetItem(m_pCustomLayerItemRoot, QStringList(strName));
			pItem->setData(0, Qt::UserRole, QVariant(E_CUSTOM_LAYER_FOLDER));
			pItem->setCheckState(0, Qt::Checked);
			m_p3DTreeWidget->GetTreeWidget()->AddTreeItem(pItem, m_pCustomLayerItemRoot);
			m_pActiveItem = pItem;
		}

		SaveData();
	}

	void CCustomLayerTree::SlotDeleteLayerFolder()
	{
		if (NULL == m_pActiveItem)
		{
			return;
		}

		DeleteItem(m_pActiveItem);
		SaveData();
	}

	void CCustomLayerTree::SlotClearLayerFolder()
	{
		if (m_pActiveItem == m_pCustomLayerItemRoot)
		{
			  int imageCount = m_pImageLayerItemRoot->childCount();
			  int eleCount = m_pElevationLayerItemRoot->childCount();
			  int vecCount = m_pVectorLayerItemRoot->childCount();


			 while (imageCount > 0)
			 {
				 DeleteItem(m_pImageLayerItemRoot->child(imageCount-1));
				 imageCount--;
			 }

			 while (eleCount > 0)
			 {
				 DeleteItem(m_pElevationLayerItemRoot->child(eleCount-1));
				 eleCount--;
			 }

			 while (vecCount > 0)
			 {
				 DeleteItem(m_pVectorLayerItemRoot->child(vecCount-1));
				 vecCount--;
			 }

		} 
		else
		{
			if (NULL == m_pActiveItem)
			{
				return;
			}

			ClearItem(m_pActiveItem);
		}
		
		SaveData();
	}

	void CCustomLayerTree::SlotContextMenuItem(QTreeWidgetItem* pItem, QPoint pos)
	{
		if (NULL == pItem)
		{
			return ;
		}
		m_pActiveItem = pItem;
		//鼠标右键菜单时m_bMouseRightMenu=true
		m_bMouseRightMenu = true;

		EItemType type = (EItemType)pItem->data(0, Qt::UserRole).toInt();
		//如果Item的类型是文件夹或者图层
		if (E_CUSTOM_LAYER_FOLDER == type || E_CUSTOM_LAYER_NODE == type)
		{
			QMenu* pPopMenu = NULL;
			//如果是图层Item，则只需创建删除菜单
			if(pItem->data(1, Qt::UserRole).value<FeLayers::IFeLayer*>())
			{
				pPopMenu = new QMenu(m_p3DTreeWidget->GetTreeWidget());
				pPopMenu->addAction(m_pDeleteLayerAction);
				pPopMenu->addAction(m_pPropertyAction);
			}

			//如果是文件夹Item，则创建添加影像、高程、矢量、栅格菜单
			else
			{
				pPopMenu = new QMenu(m_p3DTreeWidget->GetTreeWidget());

				QMenu* pAddLayerMenu = new QMenu(tr("Add Layer"), m_p3DTreeWidget->GetTreeWidget());
				pAddLayerMenu->addAction(m_pAddImageLayerAction);
				pAddLayerMenu->addAction(m_pAddElevationLayerAction);
			
				pAddLayerMenu->addAction(m_pAddFeatureLayerAction);
				pAddLayerMenu->addAction(m_pAddTDTRasterLayerAction);
				
				pPopMenu->addMenu(pAddLayerMenu);

				pPopMenu->addAction(m_pClearFolderAction);
				pPopMenu->addAction(m_pDelelteFolderAction);
			}

			pPopMenu->exec(pos);
		}
		//如果Item的类型是图层根节点，创建添加文件夹菜单
		else if (E_CUSTOM_LAYER_ROOT == type)
		{
			QMenu pPopMenu(m_p3DTreeWidget->GetTreeWidget());
			//pPopMenu->addAction(m_pAddLayerFolderAction);
			pPopMenu.addAction(m_pClearFolderAction);

			pPopMenu.exec(pos);
		}

		if (pItem == m_pImageLayerItemRoot)
		{
			QMenu pPopMenu(m_p3DTreeWidget->GetTreeWidget());
			pPopMenu.addAction(m_pAddImageLayerAction);
			pPopMenu.exec(pos);	
		}
		else if (pItem == m_pElevationLayerItemRoot)
		{
			QMenu pPopMenu(m_p3DTreeWidget->GetTreeWidget());
			pPopMenu.addAction(m_pAddElevationLayerAction);
			pPopMenu.exec(pos);	
		}
		else if (pItem == m_pVectorLayerItemRoot)
		{
			QMenu pPopMenu(m_p3DTreeWidget->GetTreeWidget());
			pPopMenu.addAction(m_pAddFeatureLayerAction);
			pPopMenu.exec(pos);	
		}
	}

	void CCustomLayerTree::SlotAddLayer(FeLayers::IFeLayer* pLayer,  FeLayers::CFeLayerConfig config)
	{
		if (!m_bMouseRightMenu || (0 == m_pCustomLayerItemRoot->childCount()))
		{
			m_pActiveItem = m_pCustomLayerItemRoot;
		}
		EItemType type = (EItemType)m_pActiveItem->data(0, Qt::UserRole).toInt();
		if (type == E_CUSTOM_LAYER_NODE)
		{
			m_pActiveItem = m_pActiveItem->parent();
		}

		m_bMouseRightMenu = false;

		if(m_rpCustomLayerSys)
		{
			if (pLayer != NULL)
			{
				//m_rpCustomLayerSys->AddLayer(pLayer);
				AddItem(m_pActiveItem, pLayer, config);
				SaveData();
				return;
			}
		}
	}

	void CCustomLayerTree::SlotLayerProperty(bool)
	{
		if (m_pActiveItem != NULL)
		{
			FeLayers::IFeLayer * layer = m_pActiveItem->data(1, Qt::UserRole).value<FeLayers::IFeLayer*>();
			
		
			if (layer != NULL)
			{
				 FeLayers::CFeLayerConfig cfg = layer->GetLayerConfig();
				 
	
				 if (layer->IsImageLayer())
				 {
					 if (m_pPropertyImageLayerDialog == NULL)
					 {
						 m_pPropertyImageLayerDialog = new PropertyImageLayerDialog(m_pMainWindow, m_rpCustomLayerSys.get());

					 }
					 m_pPropertyImageLayerDialog->ui.checkBox_cache->setChecked(false);
					 m_pPropertyImageLayerDialog->ui.lineEdit_cacheName->setText("");
					 m_pPropertyImageLayerDialog->ui.lineEdit_cachePath->setText("");
					 m_pPropertyImageLayerDialog->SetLayerAndItem(layer, m_pActiveItem);

					 m_pPropertyImageLayerDialog->ui.lineEdit_layerName->setText(layer->GetName().c_str());
					 m_pPropertyImageLayerDialog->ui.labelDriver->setText(cfg.sourceConfig().driver().value().c_str());
					 m_pPropertyImageLayerDialog->ui.lineEdit_path->setText(QString(layer->GetLayerConfig().sourceConfig().url().value().c_str()));
					 m_pPropertyImageLayerDialog->ui.lineEdit_path->setEnabled(false);
				

					 m_pPropertyImageLayerDialog->ui.radioBtn_singleData->setVisible(false);
					 m_pPropertyImageLayerDialog->ui.radioBtn_sliceData->setVisible(false);
					 m_pPropertyImageLayerDialog->ui.label_display->setVisible(true);
					 m_pPropertyImageLayerDialog->ui.checkBox_opacityLayer->setVisible(true);
					 m_pPropertyImageLayerDialog->ui.label_opacityDegree->setVisible(true);
					 m_pPropertyImageLayerDialog->ui.hSlider_opacityDegree->setVisible(true);
					 m_pPropertyImageLayerDialog->ui.label_opacityDegreeData->setVisible(true);

					 FeLayers::CFeLayerImageConfig config(cfg.getConfig());
					  m_pPropertyImageLayerDialog->ui.checkBox_cache->setChecked(config.cacheConfig().cacheEnable().value());
					 m_pPropertyImageLayerDialog->ui.lineEdit_cacheName->setText(config.cacheConfig().cacheName().value().c_str());
					 m_pPropertyImageLayerDialog->ui.lineEdit_cachePath->setText(config.cacheConfig().cachePath().value().c_str());
					 m_pPropertyImageLayerDialog->SetTitleText(tr("Image Layer"));
					 if (cfg.sourceConfig().driver().value() == "hgvector")
					 {
						 m_pPropertyImageLayerDialog->ui.lineEdit_path->setVisible(false);
						 m_pPropertyImageLayerDialog->ui.label_path->setVisible(false);
						 m_pPropertyImageLayerDialog->ui.label_file->setVisible(false);
						  m_pPropertyImageLayerDialog->ui.lineEdit_path->setText("hg");
					 }
					 else
					 {
						 m_pPropertyImageLayerDialog->ui.lineEdit_path->setVisible(true);
						 m_pPropertyImageLayerDialog->ui.label_path->setVisible(true);
						 m_pPropertyImageLayerDialog->ui.label_file->setVisible(true);
					 }
					 m_pPropertyImageLayerDialog->ShowDialogNormal();

					
				 }
				 else if (layer->IsElevationLayer())
				 {
					 if (m_pPropertyImageLayerDialog == NULL)
					 {
						 m_pPropertyImageLayerDialog = new PropertyImageLayerDialog(m_pMainWindow, m_rpCustomLayerSys.get());

					 }
					 m_pPropertyImageLayerDialog->ui.checkBox_cache->setChecked(false);
					 m_pPropertyImageLayerDialog->ui.lineEdit_cacheName->setText("");
					 m_pPropertyImageLayerDialog->ui.lineEdit_cachePath->setText("");

					 m_pPropertyImageLayerDialog->SetLayerAndItem(layer, m_pActiveItem);

					  m_pPropertyImageLayerDialog->setWindowTitle(tr("Ele Layer"));

					 m_pPropertyImageLayerDialog->ui.lineEdit_layerName->setText(layer->GetName().c_str());
					 m_pPropertyImageLayerDialog->ui.labelDriver->setText(cfg.sourceConfig().driver().value().c_str());
					 m_pPropertyImageLayerDialog->ui.lineEdit_path->setText(QString(layer->GetLayerConfig().sourceConfig().url().value().c_str()));
					 m_pPropertyImageLayerDialog->ui.lineEdit_path->setEnabled(false);
					
					
					 m_pPropertyImageLayerDialog->ui.radioBtn_singleData->setVisible(false);
					 m_pPropertyImageLayerDialog->ui.radioBtn_sliceData->setVisible(false);
					 m_pPropertyImageLayerDialog->ui.label_display->setVisible(false);
					 m_pPropertyImageLayerDialog->ui.checkBox_opacityLayer->setVisible(false);
					 m_pPropertyImageLayerDialog->ui.label_opacityDegree->setVisible(false);
					 m_pPropertyImageLayerDialog->ui.hSlider_opacityDegree->setVisible(false);
					 m_pPropertyImageLayerDialog->ui.label_opacityDegreeData->setVisible(false);
					 FeLayers::CFeLayerElevationConfig config(cfg.getConfig());
					 m_pPropertyImageLayerDialog->ui.checkBox_cache->setChecked(config.cacheConfig().cacheEnable().value());
					 m_pPropertyImageLayerDialog->ui.lineEdit_cacheName->setText(config.cacheConfig().cacheName().value().c_str());
					 m_pPropertyImageLayerDialog->ui.lineEdit_cachePath->setText(config.cacheConfig().cachePath().value().c_str());
					  m_pPropertyImageLayerDialog->SetTitleText(tr("Ele Layer"));
					 m_pPropertyImageLayerDialog->ShowDialogNormal();
				 }
				 else if (layer->IsEffectLayer())
				 {
					
				 }
				 else if (layer->IsModelLayer())
				 {
					 
					 if (m_pFeatureDialog3 == NULL)
					 {
						 m_pFeatureDialog3 = new CVectorLayerDialog(m_pMainWindow, m_rpCustomLayerSys.get());
					 }

					 m_pFeatureDialog3->SetLayerAndItem(layer, m_pActiveItem);

					 FeLayers::IFeLayerModel * modelLayer = layer->AsModelLayer();
					 
					 FeLayers::CFeLayerModelConfig conf(modelLayer->GetLayerConfig().getConfig());

					 m_pFeatureDialog3->SetTitleText(tr("Feature Layer"));
					 m_pFeatureDialog3->ui.comboBox_driveType->clear();
					
					 QStringList strList = QStringList() << cfg.sourceConfig().driver().value().c_str(); 
					 m_pFeatureDialog3->ui.comboBox_driveType->addItems(strList);
					 m_pFeatureDialog3->ui.lineEdit_path->setText(QString(conf.sourceConfig().url().value().c_str())); 
					 m_pFeatureDialog3->ui.btn_find_path->setDisabled(true);
					 m_pFeatureDialog3->ui.lineEdit_path->setDisabled(true);
					 m_pFeatureDialog3->ui.lineEdit_layerName->setText(modelLayer->GetName().c_str());
					 if (modelLayer->HasPointFeature())
					 {
						 m_pFeatureDialog3->OnlyShowFont();

						 float fontSize = modelLayer->GetFontSize();
						 osg::Vec4f fontColor = modelLayer->GetFontColor();
						 osg::Vec4f haloColor = modelLayer->GetFontHaloColor();
						 std::string path = modelLayer->GetPointIconPath();
						 bool vis = modelLayer->GetPointIconVisible();

						 m_pFeatureDialog3->ui.spinBox_fontSize->setValue(fontSize);

						 QString strFontColor = "background-color:rgb(" 
							 + QString::number(fontColor.r() * 255) + "," 
							 + QString::number(fontColor.g() * 255) + ","
							 + QString::number(fontColor.b() * 255) + "); border: none;";
						 m_pFeatureDialog3->ui.btn_fontColor->setStyleSheet(strFontColor);

						 QString strHaloColor = "background-color:rgb(" 
							 + QString::number(haloColor.r() * 255) + "," 
							 + QString::number(haloColor.g() * 255) + ","
							 + QString::number(haloColor.b() * 255) + "); border: none;";
						 m_pFeatureDialog3->ui.btn_shadowColor->setStyleSheet(strHaloColor);

						 m_pFeatureDialog3->ui.lineEdit_icon->setText(QString(path.c_str()));
						 m_pFeatureDialog3->ui.checkBox_dodge->setChecked(vis);

						 m_pFeatureDialog3->m_colorFont = QColor(fontColor.r() * 255, fontColor.g() * 255, fontColor.b() * 255, fontColor.a() * 255);
						 m_pFeatureDialog3->m_colorFontShadow = QColor(haloColor.r() * 255, haloColor.g() * 255, haloColor.b() * 255, haloColor.a() * 255);

					 }
					 else
					 {
						 m_pFeatureDialog3->OnlyShowStyle();
						 osg::Vec4f lineColor = modelLayer->GetLineColor();
					

						 QString strLineColor = "background-color:rgb(" 
							 + QString::number(lineColor.r() * 255) + "," 
							 + QString::number(lineColor.g() * 255) + ","
							 + QString::number(lineColor.b() * 255) + "); border: none;";
						 m_pFeatureDialog3->ui.btn_lineColor->setStyleSheet(strLineColor);

						 //m_pFeatureDialog3->ui.labelShpType->setText(tr("LineVector"));


						 float lineWidth = modelLayer->GetLineWidth();
						 m_pFeatureDialog3->ui.dSpinBox_lineWidth->setValue(lineWidth);

						 osg::Vec4f fillColor = modelLayer->GetFillColor();
						 QString strFillColor = "background-color:rgb(" 
							 + QString::number(fillColor.r() * 255) + "," 
							 + QString::number(fillColor.g() * 255) + ","
							 + QString::number(fillColor.b() * 255) + "); border: none;";
						// m_pFeatureDialog3->ui.btn_fillColor->setStyleSheet(strFillColor);

						 m_pFeatureDialog3->m_colorLine = QColor(lineColor.r() * 255, lineColor.g() * 255, lineColor.b() * 255, lineColor.a() * 255);
						 m_pFeatureDialog3->m_colorFill = QColor(fillColor.r() * 255, fillColor.g() * 255, fillColor.b() * 255, fillColor.a() * 255);

						 float minRng = 0.0f;
						 float maxRng = 0.0f;

						 modelLayer->GetVisibleRange(minRng, maxRng);
						 m_pFeatureDialog3->ui.spinBox_minDistance->setValue(minRng);
						 m_pFeatureDialog3->ui.spinBox_maxDistance->setValue(maxRng);

						/* FeLayers::AltitudeClamp attr = modelLayer->GetAltitudeClamp();
						 switch(attr)
						 {
						 case FeLayers::AltitudeClamp::CLAMP_TO_TERRAIN:
							 m_pFeatureDialog3->ui.comboBox_attribute->setCurrentIndex(0);
							 break;
						 case FeLayers::AltitudeClamp::CLAMP_RELATIVE_TO_TERRAIN:
							 m_pFeatureDialog3->ui.comboBox_attribute->setCurrentIndex(1);
							 break;
						 case FeLayers::AltitudeClamp::CLAMP_ABSOLUTE:
							 m_pFeatureDialog3->ui.comboBox_attribute->setCurrentIndex(2);
							 break;
						 case FeLayers::AltitudeClamp::CLAMP_NONE:
							 m_pFeatureDialog3->ui.comboBox_attribute->setCurrentIndex(3);
							 break;
						 default:
							 m_pFeatureDialog3->ui.comboBox_attribute->setCurrentIndex(0);
							 break;
						 }

						 FeLayers::AltitudeTechnique te = modelLayer->GetAltitudeTechnique();
						 switch (te)
						 {
						 case FeLayers::AltitudeTechnique::TECHNIQUE_GPU:
							 m_pFeatureDialog3->ui.comboBox_technology->setCurrentIndex(0);
							 break;
						 case FeLayers::AltitudeTechnique::TECHNIQUE_DRAPE:
							  m_pFeatureDialog3->ui.comboBox_technology->setCurrentIndex(1);
							 break;
						 case FeLayers::AltitudeTechnique::TECHNIQUE_MAP:
							  m_pFeatureDialog3->ui.comboBox_technology->setCurrentIndex(2);
							 break;
						 case FeLayers::AltitudeTechnique::TECHNIQUE_SCENE:
							  m_pFeatureDialog3->ui.comboBox_technology->setCurrentIndex(3);
							 break;
						 default:
							  m_pFeatureDialog3->ui.comboBox_technology->setCurrentIndex(0);
							 break;
						 }*/

						 
						 
					 }

					 m_pFeatureDialog3->ShowDialogNormal();

				 }
			}
		}
		
	}

	void CCustomLayerTree::SlotDeleteLayer()
	{

		if (NULL == m_pActiveItem)
		{
			return;
		}

		DeleteItem(m_pActiveItem);
		SaveData();
	}

	void CCustomLayerTree::SlotAddImageLayer(bool bFlag)
	{	

		if (!m_pImageDialog)
		{
			m_pImageDialog = new CImageLayerDialog(m_pMainWindow, m_rpCustomLayerSys.get());
			m_pImageDialog->SetTitleText(tr("Image Layer"));

			connect(m_pImageDialog, SIGNAL(SignalImageLayerFinish(FeLayers::IFeLayer*, FeLayers::CFeLayerConfig)), this, SLOT(SlotAddLayer(FeLayers::IFeLayer*, FeLayers::CFeLayerConfig)));
			connect(m_pImageDialog, SIGNAL(SignalOpacityValue(float)), this, SLOT(SlotSetImageLayerOpacity(float)));
		}


		m_pImageDialog->ui.radioBtn_singleData->setChecked(true);
		m_pImageDialog->ui.groupBox->setVisible(false);
		m_pImageDialog->ui.lineEdit_layerName->setText("");
		m_pImageDialog->ui.checkBox_cache->setChecked(false);
		m_pImageDialog->ui.lineEdit_cachePath->setText("");
		m_pImageDialog->ui.lineEdit_cacheName->setText("");
		if (m_pAddImageLayerAction == QObject::sender())
		{
			m_pImageDialog->SetTitleText(tr("Image Layer"));
		
			m_pImageDialog->m_bIsImage = true;
			m_pImageDialog->ui.radioButton_Image->setVisible(false);
			m_pImageDialog->ui.radioButton_Image->setChecked(true);
			m_pImageDialog->ui.radioButton_Ele->setVisible(false);
			

			m_pImageDialog->ui.label_display->setVisible(true);
			m_pImageDialog->ui.checkBox_opacityLayer->setVisible(true);
			m_pImageDialog->ui.label_opacityDegree->setVisible(true);
			m_pImageDialog->ui.hSlider_opacityDegree->setVisible(true);
			m_pImageDialog->ui.label_opacityDegreeData->setVisible(true);

		}
		else if (m_pAddElevationLayerAction == QObject::sender())
		{	
			m_pImageDialog->SetTitleText(tr("Ele Layer"));
			m_pImageDialog->m_bIsImage = false;
			m_pImageDialog->ui.radioButton_Image->setVisible(false);
			m_pImageDialog->ui.radioButton_Image->setChecked(false);
			m_pImageDialog->ui.radioButton_Ele->setVisible(false);
		   m_pImageDialog->ui.radioButton_Ele->setChecked(true);
			m_pImageDialog->ui.label_display->setVisible(false);
			m_pImageDialog->ui.checkBox_opacityLayer->setVisible(false);
			m_pImageDialog->ui.label_opacityDegree->setVisible(false);
			m_pImageDialog->ui.hSlider_opacityDegree->setVisible(false);
			m_pImageDialog->ui.label_opacityDegreeData->setVisible(false);
			
		}
		m_pImageDialog->ShowDialogNormal();
	
	}

	void CCustomLayerTree::SlotAddElevationLayer(bool bFlag)
	{
		if (!m_pElevationDialog)
		{
			m_pElevationDialog = new CElevationLayerDialog(m_pMainWindow, m_rpCustomLayerSys.get());
			m_pElevationDialog->SetTitleText(tr("Elevation Layer"));

			connect(m_pElevationDialog, SIGNAL(SignaElevationLayerFinish(FeLayers::IFeLayer*, FeLayers::CFeLayerConfig)), this, SLOT(SlotAddLayer(FeLayers::IFeLayer*, FeLayers::CFeLayerConfig)));

			m_pElevationDialog->ShowDialogNormal();
		}
		else
		{
			m_pElevationDialog->ShowDialogNormal();
		}
	}

	void CCustomLayerTree::SlotAddFeatureLayer(bool bFlag)
	{
		if (!m_pFeatureDialog)
		{
			m_pFeatureDialog = new CVectorLayerDialog(m_pMainWindow, m_rpCustomLayerSys.get());
			m_pFeatureDialog->SetTitleText(tr("Feature Layer"));
			m_pFeatureDialog->Reset();
			
			connect(m_pFeatureDialog, SIGNAL(SignalVectorLayerFinish(FeLayers::IFeLayer*, FeLayers::CFeLayerConfig)), this, SLOT(SlotAddLayer(FeLayers::IFeLayer*, FeLayers::CFeLayerConfig)));
			m_pFeatureDialog->ui.tabWidget->setVisible(false);
		//	m_pFeatureDialog->ui.labelShpType->setText("");
			m_pFeatureDialog->ShowDialogNormal();
		}
		else
		{
			//m_pFeatureDialog->ui.labelShpType->setText("");
			m_pFeatureDialog->ui.tabWidget->setVisible(false);
			m_pFeatureDialog->Reset();
			m_pFeatureDialog->ShowDialogNormal();
		}
	}

	void CCustomLayerTree::SlotSetImageLayerOpacity(float fValue)
	{
		QTreeWidgetItem* pItem = m_p3DTreeWidget->GetTreeWidget()->currentItem();
		if (pItem)
		{
			/*osgEarth::ImageLayer* pImageLayer = dynamic_cast<osgEarth::ImageLayer*>(
			pItem->data(1, Qt::UserRole).value<osgEarth::Layer*>());*/

			FeLayers::IFeLayer* pImageLayer = pItem->data(1,Qt::UserRole).value<FeLayers::IFeLayer*>();
			if (pImageLayer)
			{
				FeLayers::IFeLayerImage * pLayer =  pImageLayer->AsImageLayer();
				if (pLayer)
				{
					pLayer->setOpacity(fValue);
				}
			}
		}
	}


	void CCustomLayerTree::AddImageLayer(const std::string & name, const std::string & path, const std::string & driver)
	{
		if (m_pImageDialog2 == NULL)
		{
			m_pImageDialog2 = new CImageLayerDialog(m_pMainWindow, m_rpCustomLayerSys.get());
			connect(m_pImageDialog2, SIGNAL(SignalImageLayerFinish(FeLayers::IFeLayer*, FeLayers::CFeLayerConfig)), this, SLOT(SlotAddLayer(FeLayers::IFeLayer*, FeLayers::CFeLayerConfig)));
			connect(m_pImageDialog2, SIGNAL(SignalOpacityValue(float)), this, SLOT(SlotSetImageLayerOpacity(float)));
		}	

		m_pImageDialog2->ui.stackedWidget->setCurrentIndex(0);
		QString str(driver.c_str());
		if (driver == "")
		{
			 
			  str = "gdal";
		}
	
		QString strName = name.c_str();
		strName.replace(":","_");
		strName.replace("?","_");
		strName.replace("/","_");
		strName.replace("<","_");
		strName.replace(">","_");
		strName.replace("|","_");

		m_pImageDialog2->ui.radioButton_Image->setChecked(true);
		m_pImageDialog2->SetTitleText(tr("Image Layer"));
		m_pImageDialog2->ui.lineEdit_layerName->setText(strName/*QString(name.c_str())*/);
		m_pImageDialog2->ui.lineEdit_path->setText(QString(path.c_str())); 
		m_pImageDialog2->ui.btn_find_path->setVisible(false);
		m_pImageDialog2->ui.lineEdit_path->setDisabled(true);
		m_pImageDialog2->ui.checkBox_cache->setChecked(false);
		m_pImageDialog2->ui.lineEdit_cacheName->setText("");
		m_pImageDialog2->ui.lineEdit_cachePath->setText("");
		m_pImageDialog2->m_strLayerRealName = name;

		m_pImageDialog2->ui.comboBox_driveType->clear();
		QStringList strList;
		
		strList << str.toLower();
		
		
		m_pImageDialog2->ui.comboBox_driveType->addItems(strList);

			
		m_pImageDialog2->ShowDialogNormal();
		
	}

	void CCustomLayerTree::AddFeatureLayer(const std::string & name, const std::string & path, const std::string & driver)
	{
		if (m_pFeatureDialog2 == NULL)
		{
			 m_pFeatureDialog2 = new CVectorLayerDialog(m_pMainWindow, m_rpCustomLayerSys.get());
			 connect(m_pFeatureDialog2, SIGNAL(SignalVectorLayerFinish(FeLayers::IFeLayer*, FeLayers::CFeLayerConfig)), this, SLOT(SlotAddLayer(FeLayers::IFeLayer*, FeLayers::CFeLayerConfig)));
		}
		if (m_pFeatureDialog2)
		{
			
			m_pFeatureDialog2->SetTitleText(tr("Feature Layer"));
		
			m_pFeatureDialog2->ui.lineEdit_path->setText(QString(path.c_str()));
			m_pFeatureDialog2->ui.lineEdit_layerName->setText(QString(name.c_str()));
			m_pFeatureDialog2->ui.btn_find_path->setDisabled(true);
			m_pFeatureDialog2->ui.lineEdit_path->setDisabled(true);
			if (driver != "")
			{
				QString dr = QString(driver.c_str());
				
			
				m_pFeatureDialog2->ui.comboBox_driveType->clear();
				QStringList strList;
				strList << dr;
				m_pFeatureDialog2->ui.comboBox_driveType->addItems(strList);
				m_pFeatureDialog2->m_strLayerRealName = name;
				std::vector<FeUtil::GeometryType> types = ShpAndKMLTypeParser::Parse(path);
			
				if (driver == "WFS" || driver == "wfs")
				{
				   m_pFeatureDialog2->ui.tabWidget->setVisible(false);
				}
				else
				{
					  m_pFeatureDialog2->ui.tabWidget->setVisible(true);
				}
				for (std::vector<FeUtil::GeometryType>::iterator iter = types.begin(); iter != types.end(); ++iter)
				{
					if (*iter == FeUtil::POINT)
					{
						
					//	m_pFeatureDialog2->ui.labelShpType->setText(tr("PointVector"));
						m_pFeatureDialog2->OnlyShowFont();
						
					}
					else
					{
					//	m_pFeatureDialog2->ui.labelShpType->setText(tr("LineVector"));
						m_pFeatureDialog2->OnlyShowStyle();
					}
				}
				m_pFeatureDialog2->ui.comboBox_driveType->setDisabled(true);
			}
			

			m_pFeatureDialog2->ShowDialogNormal();
		}
	
	}

	void CCustomLayerTree::AddElevationLayer(const std::string & name, const std::string & path, const std::string & driver)
	{
		if (!m_pElevationDialog2)
		{
			m_pElevationDialog2 = new CElevationLayerDialog(m_pMainWindow, m_rpCustomLayerSys.get());
			m_pElevationDialog2->SetTitleText(tr("Elevation Layer"));
			m_pElevationDialog2->ui.lineEdit_path->setText(QString(path.c_str())); 
			m_pElevationDialog2->ui.btn_find_path->setDisabled(true);
			m_pElevationDialog2->ui.lineEdit_path->setDisabled(true);
			connect(m_pElevationDialog2, SIGNAL(SignaElevationLayerFinish(FeLayers::IFeLayer*, FeLayers::CFeLayerConfig)), this, SLOT(SlotAddLayer(FeLayers::IFeLayer*, FeLayers::CFeLayerConfig)));

			m_pElevationDialog2->ShowDialogNormal();
		}
		else
		{
			m_pElevationDialog2->ShowDialogNormal();
		}
	}

	CCustomLayerTree::~CCustomLayerTree(void)
	{
		SaveData();
		if (m_pCustomLayerItemRoot)
		{
			delete m_pCustomLayerItemRoot;
			m_pCustomLayerItemRoot = NULL;
		}

		if (m_pImageDialog)
		{
			delete m_pImageDialog;
			m_pImageDialog = NULL;
		}

		if (m_pElevationDialog)
		{
			delete m_pElevationDialog;
			m_pElevationDialog = NULL;
		}

		if (m_pFeatureDialog)
		{
			delete m_pFeatureDialog;
			m_pFeatureDialog = NULL;
		}

		if (m_pImageDialog2)
		{
			delete m_pImageDialog2;
			m_pImageDialog2 = NULL;
		}

		if (m_pElevationDialog2)
		{
			delete m_pElevationDialog2;
			m_pElevationDialog2 = NULL;
		}

		if (m_pFeatureDialog2)
		{
			delete m_pFeatureDialog2;
			m_pFeatureDialog2 = NULL;
		}
	}

	void CCustomLayerTree::SlotColorFilterDialog(bool)
	{
		if (NULL == m_pRGBColorFilterDialog)
		{
			m_pRGBColorFilterDialog = new CRGBColorFilterDialog(m_rpCustomLayerSys.get(), m_pMainWindow);
		}

		m_pRGBColorFilterDialog->ShowDialogNormal();
	}

	void CCustomLayerTree::SlotAddRastorLayer( bool bFlag )
	{
		if(false == m_rpCustomLayerSys.valid())
		{
			return ;
		}

		std::string strDriver = "hgVector";
		QString strRastorName = tr("RastorVector");
		std::string strRastorLayerName = strRastorName.toStdString();

		//首先判断图层中是否已经添加了该图层
		FeLayers::CImageLayersSet imgVector;
		m_rpCustomLayerSys->GetImageLayers(imgVector);
		
		FeLayers::CImageLayersSet::iterator it = imgVector.begin();
		while(it != imgVector.end())
		{
			FeLayers::IFeLayerImage* pImageLayer = *it;
			if (pImageLayer && 0 == strRastorLayerName.compare(pImageLayer->GetName()))
			{
				return;
			}
			it++;
		}
		
		////初始化terrainConfig
		//osgEarth::Config terrainConfig;
		//terrainConfig.key() = "image";

		//terrainConfig.add("name", strRastorLayerName);
		//terrainConfig.add("driver", strDriver);
		//terrainConfig.add("hastransparency", true);
		//terrainConfig.add("visible", true);
		//terrainConfig.add("cachepath", FeFileReg->GetFullPath("earth/cache_flat"));
		//terrainConfig.add("cacheid", "RastorVector");

		////初始化TileSourceOptions
		//TileSourceOptions tileSourceOpt;
		//tileSourceOpt.merge(terrainConfig);
		//tileSourceOpt.setDriver(strDriver);
		//osg::ref_ptr<osgEarth::TileSource> pTileSource = TileSourceFactory::create(tileSourceOpt);

		///*ImageLayerOptions layerOpt( terrainConfig );
		//osg::ref_ptr<osgEarth::ImageLayer> pImageLayer = new osgEarth::ImageLayer(layerOpt, pTileSource);*/

		//FeLayers::CFeLayerImageConfig opt(terrainConfig);

		FeLayers::CFeLayerImageConfig cfg;
		cfg.name() = strRastorLayerName;
		cfg.visibleConfig().opacity() =   1.0;
		cfg.visibleConfig().opacityEnable() = true;
		
		cfg.sourceConfig().driver() = strDriver;
		cfg.sourceConfig().layerName() = strRastorLayerName;
		cfg.cacheConfig().cacheName() =  "RastorVector";
		cfg.cacheConfig().cacheEnable() = true;
		cfg.cacheConfig().cachePath() = FeFileReg->GetFullPath("earth/cache_flat");
		cfg.visible() = true;

		FeLayers::IFeLayer * pImageLayer = 	m_rpCustomLayerSys->AddLayer(cfg);

		

		SlotAddLayer(pImageLayer, cfg);
	}

}
