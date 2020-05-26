#include <layer/PropertyImageLayerDialog.h>
#include "mainWindow/FreeUtil.h"

namespace FreeViewer
{
	using namespace osgEarth;

	PropertyImageLayerDialog::PropertyImageLayerDialog(QWidget *parent, FeLayers::CLayerSys * pCustomLayerSys)
		: CFreeDialog(parent)
		, m_pLayer(NULL)
		, m_pItem(NULL)
	{
		//添加ui界面到属性窗体客户区
		QWidget *widget = new QWidget(this);
		ui.setupUi(widget);
		//AddSubWidget(widget);
		AddWidgetToDialogLayout(widget);
		widget->setFixedSize(widget->geometry().size());
		m_pCustomLayerSys = pCustomLayerSys;

		m_group1 = new QButtonGroup(this);
		m_group2 = new QButtonGroup(this);


		m_group2->addButton(ui.radioBtn_singleData);
		m_group2->addButton(ui.radioBtn_sliceData);

		InitData();
	}

	void PropertyImageLayerDialog::InitData()
	{
		ui.lineEdit_layerName->setText(tr("unamed image layer"));
		ui.hSlider_opacityDegree->setMaximum(100);
		ui.hSlider_opacityDegree->setValue(100);
		ui.label_opacityDegreeData->setText(QString::number(double(ui.hSlider_opacityDegree->value())/ui.hSlider_opacityDegree->maximum()));

		//默认禁用缓存加载功能
		SlotCacheStateChanged(0);

		

		//connect(ui.btn_find_path, SIGNAL(clicked()), this, SLOT(SlotOpenDataOrPath()));
		connect(ui.btn_find_cachePath, SIGNAL(clicked()), this, SLOT(SlotOpenCachePath()));
		connect(ui.btn_ok, SIGNAL(clicked()), this, SLOT(SlotOk()));
		connect(ui.btn_cancel, SIGNAL(clicked()), this, SLOT(SlotCancel()));
		connect(ui.hSlider_opacityDegree, SIGNAL(valueChanged(int)), this, SLOT(SlotOpacityChanged(int)));
		connect(ui.checkBox_cache, SIGNAL(stateChanged(int)), this, SLOT(SlotCacheStateChanged(int)));
		connect(ui.checkBox_opacityLayer, SIGNAL(stateChanged(int)), this, SLOT(SlotOpacityStateChanged(int)));
	}

	void PropertyImageLayerDialog::SlotOpenDataOrPath()
	{

	}

	void PropertyImageLayerDialog::SlotOpenCachePath()
	{
		QString strCaption = tr("Open Cache Path");
		QString prePath = ui.lineEdit_cachePath->text();
		QString strDir = tr(FeFileReg->GetDataPath().c_str());
		if (false == ui.lineEdit_cachePath->text().isEmpty())
		{
			strDir = ui.lineEdit_cachePath->text();
		}

#ifdef WIN32
		QString cachePath = QFileDialog::getExistingDirectory(this, strCaption, strDir);
		ui.lineEdit_cachePath->setText(cachePath);
		if (cachePath.isEmpty())
		{
			 ui.lineEdit_cachePath->setText(prePath);
		}
#else
		QFileDialog fileDialog(0, strCaption, strDir);
		fileDialog.setFileMode( QFileDialog::DirectoryOnly);
		if (fileDialog.exec())
		{
			ui.lineEdit_cachePath->setText(fileDialog.selectedFiles().first());
		}
#endif
	}

	void PropertyImageLayerDialog::SlotOk()
	{
		if (!CheckInputInfo())
		{
			return;
		}
		
	

		float fOpacityValue = QString(ui.label_opacityDegreeData->text()).toFloat();
		

		if (m_pLayer != NULL)
		{
			m_pLayer->SetName(ui.lineEdit_layerName->text().toStdString());
			FeLayers::IFeLayerImage * pImageLayer = m_pLayer->AsImageLayer();
			FeLayers::IFeLayerElevation * pImageLayer2 = m_pLayer->AsElevationLayer();
			if (pImageLayer != NULL)
			{
				pImageLayer->setOpacity(fOpacityValue);
				pImageLayer->SetCacheEnable(ui.checkBox_cache->isChecked());
				pImageLayer->SetCacheName(ui.lineEdit_cacheName->text().toStdString());
				pImageLayer->SetCachePath(ui.lineEdit_cachePath->text().toStdString());
			}
			if (pImageLayer2 != NULL)
			{
				pImageLayer2->SetCacheEnable(ui.checkBox_cache->isChecked());
				pImageLayer2->SetCacheName(ui.lineEdit_cacheName->text().toStdString());
				pImageLayer2->SetCachePath(ui.lineEdit_cachePath->text().toStdString());
			}

			m_pItem->setText(0, ui.lineEdit_layerName->text());
		}

		m_pLayer = NULL;
		m_pItem = NULL;
		close();
	}

	void PropertyImageLayerDialog::SlotCancel()
	{
		close();
	}

	void PropertyImageLayerDialog::SlotOpacityChanged(int value)
	{
		ui.label_opacityDegreeData->setText(QString::number(double(ui.hSlider_opacityDegree->value())/double(ui.hSlider_opacityDegree->maximum())));
	}

	bool PropertyImageLayerDialog::CheckInputInfo()
	{
		// 判断输入是否为空
		QString redStyle("QLineEdit{ border: 1px solid red;}");
		QString normalStyle("QLineEdit { background: transparent; border: 1px solid rgb(44,80,114);}");

		bool redStyleFlag = false;
		if (ui.lineEdit_layerName->text().isEmpty())
		{
			ui.lineEdit_layerName->setStyleSheet(redStyle);
			redStyleFlag = true;
		}
		else
		{
			ui.lineEdit_layerName->setStyleSheet(normalStyle);
		}

		if (ui.lineEdit_path->text().isEmpty())
		{
			ui.lineEdit_path->setStyleSheet(redStyle);
			redStyleFlag = true;
		}
		else
		{
			ui.lineEdit_path->setStyleSheet(normalStyle);
		}

		if (ui.checkBox_cache->isChecked() && ui.lineEdit_cacheName->text().isEmpty())
		{
			ui.lineEdit_cacheName->setStyleSheet(redStyle);
			redStyleFlag = true;
		}
		else
		{
			ui.lineEdit_cacheName->setStyleSheet(normalStyle);
		}

		if (ui.checkBox_cache->isChecked() && ui.lineEdit_cachePath->text().isEmpty())
		{
			ui.lineEdit_cachePath->setStyleSheet(redStyle);
			redStyleFlag = true;
		}
		else
		{
			ui.lineEdit_cachePath->setStyleSheet(normalStyle);
		}

		if (redStyleFlag)
		{
			return false;
		}

		return true;
	}

	void PropertyImageLayerDialog::CreateImageLayer()
	{
		
	}

	void PropertyImageLayerDialog::SlotCacheStateChanged(int state)
	{
		if (state == Qt::Checked)
		{
			ui.lineEdit_cacheName->setEnabled(true);
			ui.lineEdit_cachePath->setEnabled(true);
			ui.btn_find_cachePath->setEnabled(true);
			ui.label_cacheName->setEnabled(true);
			ui.label_cachePath->setEnabled(true);
		}
		else
		{
			ui.lineEdit_cacheName->setEnabled(false);
			ui.lineEdit_cachePath->setEnabled(false);
			ui.btn_find_cachePath->setEnabled(false);
			ui.label_cacheName->setEnabled(false);
			ui.label_cachePath->setEnabled(false);
		}
	}

	void PropertyImageLayerDialog::SlotOpacityStateChanged(int state)
	{
		if (state == Qt::Checked)
		{
			ui.hSlider_opacityDegree->setEnabled(true);
			ui.label_opacityDegree->setEnabled(true);
			ui.label_opacityDegreeData->setEnabled(true);
		}
		else
		{
			ui.hSlider_opacityDegree->setEnabled(false);
			ui.label_opacityDegree->setEnabled(false);
			ui.label_opacityDegreeData->setEnabled(false);
		}
	}

	

	PropertyImageLayerDialog::~PropertyImageLayerDialog()
	{

	}

	void PropertyImageLayerDialog::SetLayerAndItem(FeLayers::IFeLayer * pLayer, QTreeWidgetItem * pItem)
	{
		 m_pLayer = pLayer;
		 m_pItem = pItem;
	}

	

}
