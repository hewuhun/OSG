#include <layer/SystemLayerTree.h>

#include <mainWindow/FreeMainWindow.h>
#include <mainWindow/FreeUtil.h>

namespace FreeViewer
{
	CSystemLayerTree::CSystemLayerTree(CFreeMainWindow* pMainWindow)
		: CUIObserver(pMainWindow)
		,m_rpSystemLayerSys(NULL)
		,m_p3DTreeWidget(NULL)
		,m_pSystemLayerItemRoot(NULL)
		,m_pActiveItem(NULL)
		,m_pImageLayerAttributeDialog(NULL)
	{
		InitWidget();
	}

	CSystemLayerTree::~CSystemLayerTree(void)
	{

	}

	void CSystemLayerTree::InitWidget()
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
		m_p3DTreeWidget->PushBackSystem(E_SYSTEM_LAYER, this);

		if(m_opSystemService.valid())
		{
			FeLayers::CLayerSysOptions opt;
			opt.featurePath() = m_opSystemService->GetSysConfig().GetFeatureLayerConfig();
			opt.customLayerPath() = m_opSystemService->GetSysConfig().GetCustomLayerConfig();
			m_rpSystemLayerSys = new FeLayers::CLayerSys(opt);
			m_rpSystemLayerSys->Initialize(m_opSystemService->GetRenderContext());
			m_rpSystemLayerSys->RegistryEffectLayer();
			m_rpSystemLayerSys->RegistryFeatureLayer();
			m_opSystemService->AddAppModuleSys(m_rpSystemLayerSys);

			if (m_rpSystemLayerSys)
			{
				BuildContext();
			}
		}
	}

	void CSystemLayerTree::BuildContext()
	{
		//系统图层树根节点
		m_pSystemLayerItemRoot = new QTreeWidgetItem(m_p3DTreeWidget->GetTreeWidget(), QStringList(tr("System Layer")), 0);
		m_pSystemLayerItemRoot->setCheckState(0, Qt::Unchecked);
		m_pSystemLayerItemRoot->setData(0, Qt::UserRole, E_SYSTEM_LAYER_ROOT);
		m_p3DTreeWidget->GetTreeWidget()->AddTopTreeItem(m_pSystemLayerItemRoot);

		//影像图层
		CreateImageLayer();

		//高程图层
		CreateElevationLayer();

		//矢量图层
		CreateVectorLayer();

		//效果图层
		CreateEffectLayer();

		m_pSystemLayerItemRoot->setExpanded(true);
	}

	void CSystemLayerTree::CreateImageLayer()
	{
		ImageLayerVector imageLayer;
		m_rpSystemLayerSys->GetImageLayers(imageLayer);

		if(imageLayer.size() > 0)
		{
			QString strTitle(tr("Image Layers"));
			QTreeWidgetItem* pImageLayerItem = new QTreeWidgetItem(m_pSystemLayerItemRoot, QStringList(strTitle), 0);
			pImageLayerItem->setData(0, Qt::UserRole, QVariant(E_SYSTEM_LAYER_IMAGE_FOLDER));
			for(ImageLayerVector::iterator itr = imageLayer.begin(); itr != imageLayer.end(); ++itr)
			{
				//如果是光照图层，则在图层管理的列表中不显示该图层
				if ((*itr)->getInitialOptions().driver()->getDriver() == "hgbump")
				{
					continue;
				}
#ifdef QT4_VERSION
				//Qt4
				AddLayerItems(pImageLayerItem, (*itr), ConvertToCurrentEncoding((*itr)->getName().c_str()), (*itr)->getVisible());
#else
				//Qt5
				AddLayerItems(pImageLayerItem, (*itr), QString((*itr)->getName().c_str()), (*itr)->getVisible());
#endif
			}

			pImageLayerItem->setExpanded(true);
		}
	}

	void CSystemLayerTree::CreateElevationLayer()
	{
		ElevationLayerVector elevationLayer;
		m_rpSystemLayerSys->GetElevationLayers(elevationLayer);

		if(elevationLayer.size() > 0)
		{
			QString strTitle(tr("Elevation Layers"));
			QTreeWidgetItem* pElevationLayerItem = new QTreeWidgetItem(m_pSystemLayerItemRoot, QStringList(strTitle));
			for(ElevationLayerVector::iterator itr = elevationLayer.begin(); itr != elevationLayer.end(); ++itr)
			{
				if(itr->valid())
				{
#ifdef QT4_VERSION
					//Qt4
					AddLayerItems(pElevationLayerItem, (*itr), ConvertToCurrentEncoding((*itr)->getName().c_str()), (*itr)->getVisible());
#else
					//Qt5
					AddLayerItems(pElevationLayerItem, (*itr), QString((*itr)->getName().c_str()), (*itr)->getVisible());
#endif
				}
			}

			pElevationLayerItem->setExpanded(true);
		}
	}

	void CSystemLayerTree::CreateVectorLayer()
	{
		ModelLayerVector vectorLayer;
		m_rpSystemLayerSys->GetModelLayers(vectorLayer);

		if(vectorLayer.size() > 0)
		{
			QString strTitle(tr("Vector Layers"));
			QTreeWidgetItem* pVectorLayerItem = new QTreeWidgetItem(m_pSystemLayerItemRoot, QStringList(strTitle));
			for(ModelLayerVector::iterator itr = vectorLayer.begin(); itr != vectorLayer.end(); ++itr)
			{
				if(itr->valid())
				{
#ifdef QT4_VERSION
					//Qt4
					AddLayerItems(pVectorLayerItem, (*itr), ConvertToCurrentEncoding((*itr)->getName().c_str()), (*itr)->getVisible());
#else
					//Qt5
					AddLayerItems(pVectorLayerItem, (*itr), QString((*itr)->getName().c_str()), (*itr)->getVisible());
#endif
				}
			}

			pVectorLayerItem->setExpanded(true);
		}
	}

	void CSystemLayerTree::CreateEffectLayer()
	{
		
		FeLayers::CEffectLayersSet	 effectLayer;
		m_rpSystemLayerSys->GetEffectLayers(effectLayer);

		if(effectLayer.size() > 0)
		{
			QString strTitle(tr("Effect Layers"));
			QTreeWidgetItem* pEffectLayerItem = new QTreeWidgetItem(m_pSystemLayerItemRoot, QStringList(strTitle));
			for(FeLayers::CEffectLayersSet::iterator itr = effectLayer.begin(); itr != effectLayer.end(); ++itr)
			{
				if(itr->valid())
				{
#ifdef QT4_VERSION
					//Qt4
					AddLayerItems(pEffectLayerItem, (*itr).get(), ConvertToCurrentEncoding((*itr)->GetName().c_str()), (*itr)->GetVisible());
#else
					//Qt5
					AddLayerItems(pEffectLayerItem, (*itr), QString((*itr)->GetName().c_str()), (*itr)->GetVisible());
#endif
				}
			}

			pEffectLayerItem->setExpanded(true);
		}
	}

	void CSystemLayerTree::AddLayerItems(QTreeWidgetItem* pParentItem, osgEarth::Layer* pLayer, const QString& strTitle, bool bVisible)
	{
		QTreeWidgetItem * pItem = new QTreeWidgetItem(pParentItem, QStringList(strTitle), 0);
		pItem->setData(0, Qt::UserRole, QVariant(E_SYSTEM_LAYER_NODE));
		pItem->setData(1, Qt::UserRole, QVariant::fromValue(pLayer));
		m_p3DTreeWidget->GetTreeWidget()->AddTreeItem(pItem, pParentItem);

		if (bVisible)
		{
			pItem->setCheckState(0, Qt::Checked);
		}
		else
		{
			pItem->setCheckState(0, Qt::Unchecked);
		}
	}

	void CSystemLayerTree::AddLayerItems(QTreeWidgetItem* pParentItem, FeLayers::IFeLayer* pLayer, const QString& strTitle, bool bVisible)
	{
		QTreeWidgetItem * pItem = new QTreeWidgetItem(pParentItem, QStringList(strTitle), 0);
		pItem->setData(0, Qt::UserRole, QVariant(E_SYSTEM_LAYER_NODE));
		pItem->setData(1, Qt::UserRole, QVariant::fromValue(pLayer));
		m_p3DTreeWidget->GetTreeWidget()->AddTreeItem(pItem, pParentItem);

		if (bVisible)
		{
			pItem->setCheckState(0, Qt::Checked);
		}
		else
		{
			pItem->setCheckState(0, Qt::Unchecked);
		}
	}

	void CSystemLayerTree::SlotItemsVisibleChange(QTreeWidgetItem* pItem, bool bShow)
	{
		if (NULL == pItem)
		{
			return ;
		}
		m_pActiveItem = pItem;

		//获取到Item中保存的图层数据
		Layer* pLayer = pItem->data(1, Qt::UserRole).value<osgEarth::Layer*>();

		ImageLayer* pImageLayer = dynamic_cast<ImageLayer*>(pLayer);
		if (pImageLayer)
		{
			pImageLayer->setVisible(bShow);
			return ;
		}

		ElevationLayer* pElevationLayer = dynamic_cast<ElevationLayer*>(pLayer);
		if (pElevationLayer)
		{
			pElevationLayer->setVisible(bShow);
			return;
		}

		ModelLayer* pModelLayer = dynamic_cast<ModelLayer*>(pLayer);
		if (pModelLayer)
		{
			pModelLayer->setVisible(bShow);
			return;
		}

		FeLayers::IFeLayer * pLayer2 =  pItem->data(1, Qt::UserRole).value<FeLayers::IFeLayer*>();
	
		if(pLayer2)
		{
			pLayer2->SetVisible(bShow);
			return;
		}
	}

	QMenu* CSystemLayerTree::CreatePopMenu(osgEarth::Layer* pLayer)
	{
		osgEarth::ImageLayer* pImageLayer = dynamic_cast<osgEarth::ImageLayer*>(pLayer);
		if(pImageLayer)
		{
			QMenu* pPopMenu = new QMenu(m_p3DTreeWidget->GetTreeWidget());
			QAction* pPropertyAction = new QAction(tr("Property"), pPopMenu);
			pPopMenu->addAction(pPropertyAction);

			connect(pPropertyAction, SIGNAL(triggered(bool)), this, SLOT(SlotImageLayerAttribute(bool)));
			return pPopMenu;
		}	

		return NULL;
	}

	void CSystemLayerTree::SlotContextMenuItem(QTreeWidgetItem* pItem, QPoint pos)
	{
		if (NULL == pItem)
		{
			return ;
		}

		if (pItem->data(0, Qt::UserRole).toInt() == E_SYSTEM_LAYER_IMAGE_FOLDER)
		{
			if(pItem->childCount()>0)
			{
				m_p3DTreeWidget->GetTreeWidget()->setCurrentItem(pItem);
				pItem = pItem->child(0);
			}
			m_pActiveItem = pItem;

			if (pItem->data(0, Qt::UserRole).toInt() == E_SYSTEM_LAYER_NODE)
			{
				QMenu* pPopMenu = CreatePopMenu(pItem->data(1, Qt::UserRole).value<osgEarth::Layer*>());
				if (pPopMenu)
				{
					pPopMenu->exec(pos);
				}
			}
		}
	}

	void CSystemLayerTree::SlotImageLayerAttribute(bool)
	{
		if(m_pActiveItem)
		{
			osgEarth::ImageLayer* pImageLayer = dynamic_cast<osgEarth::ImageLayer*>(
				m_pActiveItem->data(1, Qt::UserRole).value<osgEarth::Layer*>());
			if (pImageLayer)
			{
				if (NULL == m_pImageLayerAttributeDialog)
				{
					m_pImageLayerAttributeDialog = new CImageLayerAttributeDialog(m_rpSystemLayerSys, pImageLayer, m_pMainWindow);
					m_pImageLayerAttributeDialog->SetTitleText(tr("Image Attribute"));
					m_pImageLayerAttributeDialog->ReGetImageLayer();
				}
				else
				{
					m_pImageLayerAttributeDialog->ReGetImageLayer();
				}
				m_pImageLayerAttributeDialog->ShowDialogNormal();
			}
		}
	}
};
