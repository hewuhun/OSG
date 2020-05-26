#include <layer/ImageLayerAttributeDialog.h>

#include <mainWindow/FreeUtil.h>

namespace FreeViewer
{
	using namespace osgEarth;

	CImageLayerAttributeDialog::CImageLayerAttributeDialog(FeLayers::CLayerSys* pSystem, osgEarth::ImageLayer* pCurrentLayer, QWidget *parent)
		: CFreeDialog(parent)
		, m_opCurrentImageLayer(pCurrentLayer)
		, m_opLayerSystem(pSystem)
	{
		//添加ui界面到属性窗体客户区
		QWidget *widget = new QWidget(this);
		ui.setupUi(widget);
		AddWidgetToDialogLayout(widget);
		//AddSubWidget(widget);
		widget->setFixedSize(widget->geometry().size());

		InitData();
	}

	void CImageLayerAttributeDialog::InitData()
	{
		if(m_opLayerSystem.valid())
		{
			InitContext();
		}
	}

	void CImageLayerAttributeDialog::InitContext()
	{
		if (m_opCurrentImageLayer.valid())
		{
			double dOpacity = m_opCurrentImageLayer->getOpacity();

			ui.hSlider_opacityDegree->setValue(dOpacity*100.0);
			ui.label_opacityDegreeData->setText(QString::number(dOpacity));
		}

		connect(ui.hSlider_opacityDegree, SIGNAL(valueChanged(int)), this, SLOT(SlotOpacityChanged(int)));
		connect(ui.btn_ok, SIGNAL(pressed()), this, SLOT(SlotOk()));
		connect(ui.btn_apply, SIGNAL(pressed()), this, SLOT(SlotApply()));
		connect(ui.btn_cancel, SIGNAL(pressed()), this, SLOT(SlotCancel()));
	}

	void CImageLayerAttributeDialog::ReGetImageLayer()
	{
		//清除listWidget所有节点
		int nSize = ui.listWidget->count();

		for (int i = 0; i < nSize; i++)
		{
			QListWidgetItem *item = ui.listWidget->takeItem(0);

			delete item;
		}

		m_itemLayerMap.clear();
		m_layerOpacityMap.clear();

		//添加所有影像图层
		osgEarth::ImageLayerVector layerVector;
		m_opLayerSystem->GetImageLayers(layerVector);
		for (int i = 0; i < layerVector.size(); i++)
		{
			osgEarth::ImageLayer* pLayer = layerVector.at(i);
			if (pLayer)
			{
				QString strLayerName = ConvertToCurrentEncoding(pLayer->getName());

				//屏蔽光照图层
				if (strLayerName == QString("光照图层"))
				{
					continue;
				}

				QListWidgetItem* pItem = new QListWidgetItem(strLayerName);
				m_itemLayerMap.insert(std::make_pair(pItem, pLayer));

				double dOpacity = pLayer->getOpacity(); 
				m_layerOpacityMap.insert(std::make_pair(pItem, dOpacity));

				ui.listWidget->addItem(pItem);

				if(m_opCurrentImageLayer.get() == pLayer)
				{
					ui.listWidget->setCurrentItem(pItem);
				}
			}
		}

		connect(ui.listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(SlotItemChanged()));
	}

	void CImageLayerAttributeDialog::SlotOpacityChanged( int nValue )
	{
		ui.label_opacityDegreeData->setText(QString::number(nValue/100.0));

		if (m_opCurrentImageLayer.valid())
		{
			m_opCurrentImageLayer->setOpacity(nValue/100.0);
		}
	}

	void CImageLayerAttributeDialog::SlotItemChanged()
	{
		if (0 != m_itemLayerMap.count(ui.listWidget->currentItem()))
		{
			m_opCurrentImageLayer = m_itemLayerMap[ui.listWidget->currentItem()];
		}

		double dOpacity = m_opCurrentImageLayer->getOpacity();
		ui.hSlider_opacityDegree->setValue(dOpacity*100);
		ui.label_opacityDegreeData->setText(QString::number(dOpacity));
	}

	void CImageLayerAttributeDialog::SlotOk()
	{
		SlotApply();

		close();
		disconnect(ui.listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(SlotItemChanged()));
	}

	void CImageLayerAttributeDialog::SlotApply()
	{
		osg::ref_ptr<osgEarth::Map> map = new osgEarth::Map;
		map->beginUpdate();

		int nSize = ui.listWidget->count();

		for (int i = 0; i < nSize; i++)
		{
			osgEarth::ImageLayer* pLayer = m_itemLayerMap[ui.listWidget->item(i)].get();

			if (pLayer)
			{
				//m_opLayerSystem->DeleteImageLayer(pLayer);
			}  
		}

		for (int i = 0; i < nSize; i++)
		{
			osgEarth::ImageLayer* pLayer = m_itemLayerMap[ui.listWidget->item(i)].get();

			if (pLayer)
			{
				//m_opLayerSystem->AddImageLayer(pLayer);
			}  
		}

		map->endUpdate();
	}

	void CImageLayerAttributeDialog::SlotCancel()
	{
		int nSize = ui.listWidget->count();

		for (int i=0; i<nSize; i++)
		{
			osgEarth::ImageLayer* pLayer = m_itemLayerMap[ui.listWidget->item(i)].get();
			double dOpacity = m_layerOpacityMap[ui.listWidget->item(i)];
			pLayer->setOpacity(dOpacity);
		}

		SlotItemChanged();

		close();
		disconnect(ui.listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(SlotItemChanged()));
	}

	CImageLayerAttributeDialog::~CImageLayerAttributeDialog()
	{

	}
}
