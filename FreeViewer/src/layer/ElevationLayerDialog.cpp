#include <layer/ElevationLayerDialog.h>
#include "mainWindow/FreeUtil.h"

namespace FreeViewer
{
	using namespace osgEarth;

	CElevationLayerDialog::CElevationLayerDialog(QWidget *parent, FeLayers::CLayerSys * pCustomLayerSys)
		: CFreeDialog(parent)
		, m_pCustomLayerSys(pCustomLayerSys)
	{
		//添加ui界面到属性窗体客户区
		QWidget *widget = new QWidget(this);
		ui.setupUi(widget);
		//AddSubWidget(widget);
		AddWidgetToDialogLayout(widget);
		widget->setFixedSize(widget->geometry().size());

		InitData();
	}

	void CElevationLayerDialog::InitData()
	{
		ui.lineEdit_layerName->setText(tr("unamed elevation layer"));

		//默认禁用缓存加载功能
		SlotStateChanged(0);

		// 限制文本长度
		ui.lineEdit_layerName->setMaxLength(25);
		ui.lineEdit_cacheName->setMaxLength(25);

		connect(ui.btn_find_path, SIGNAL(clicked()), this, SLOT(SlotOpenDataOrPath()));
		connect(ui.btn_find_cachePath, SIGNAL(clicked()), this, SLOT(SlotOpenCachePath()));
		connect(ui.btn_ok, SIGNAL(clicked()), this, SLOT(SlotOk()));
		connect(ui.btn_cancel, SIGNAL(clicked()), this, SLOT(SlotCancel()));
		connect(ui.checkBox_cache, SIGNAL(stateChanged(int)), this, SLOT(SlotStateChanged(int)));
	}

	void CElevationLayerDialog::SlotOpenDataOrPath()
	{
		QString strCaption = tr("Add Elevation Layer");
		QString strDir = tr(FeFileReg->GetDataPath().c_str());
		if (false == ui.lineEdit_path->text().isEmpty())
		{
			strDir = ui.lineEdit_path->text();
		}
		QString strFilter = tr("Elevations(*.tif *.png *.jpeg);;All files(*.*)");

		//如果创建影像的是原始数据
		if (ui.radioBtn_singleData->isChecked())
		{
			QStringList filePaths;
#ifdef WIN32
			QFileDialog* pFileDialog = new QFileDialog();
			pFileDialog->setAttribute(Qt::WA_DeleteOnClose, true);
			filePaths = pFileDialog->getOpenFileNames(0, strCaption, strDir, strFilter);
#else
			QFileDialog fileDialog(0, strCaption, strDir, strFilter);
			if (fileDialog.exec())
			{
				filePaths = fileDialog.selectedFiles();
			}
#endif
			for(QStringList::iterator itr = filePaths.begin(); itr != filePaths.end(); ++itr)
			{
				QString filePath = *itr;
				if(!filePath.isNull())
				{
					ui.lineEdit_path->setText(filePath);
				}
			}
		}
		//否则，如果创建影像使用的是切片数据
		else if (ui.radioBtn_sliceData->isChecked())
		{
#ifdef WIN32
			QFileDialog* pFileDialog = new QFileDialog();
			pFileDialog->setAttribute(Qt::WA_DeleteOnClose, true);
			QString filePaths = pFileDialog->getExistingDirectory(this, strCaption, strDir);
			ui.lineEdit_path->setText(filePaths);
#else
			QFileDialog fileDialog(0, strCaption, strDir, strFilter);
			fileDialog.setFileMode( QFileDialog::DirectoryOnly);
			if (fileDialog.exec())
			{
				ui.lineEdit_path->setText(fileDialog.selectedFiles().first());
			}
#endif
		}
	}

	void CElevationLayerDialog::SlotOpenCachePath()
	{
		QString strCaption = tr("Open Cache Path");
		QString strDir = tr(FeFileReg->GetDataPath().c_str());
		if (false == ui.lineEdit_cachePath->text().isEmpty())
		{
			strDir = ui.lineEdit_cachePath->text();
		}
#ifdef WIN32
		QString cachePath = QFileDialog::getExistingDirectory(this, strCaption, strDir);
		ui.lineEdit_cachePath->setText(cachePath);
#else
		QFileDialog fileDialog(0, strCaption, strDir);
		fileDialog.setFileMode( QFileDialog::DirectoryOnly);
		if (fileDialog.exec())
		{
			ui.lineEdit_cachePath->setText(fileDialog.selectedFiles().first());
		}
#endif
	}

	void CElevationLayerDialog::SlotOk()
	{
		if (!CheckInputInfo())
		{
			return;
		}

		CreateElevationLayer();

		close();
	}

	void CElevationLayerDialog::SlotCancel()
	{
		close();
	}

	bool CElevationLayerDialog::CheckInputInfo()
	{
		// 判断输入是否为空
		QString redStyle("QLineEdit{border: 1px solid red;}");
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

	void CElevationLayerDialog::CreateElevationLayer()
	{
		//获取到对话框中的设置值
		std::string strDriver = ui.comboBox_driveType->currentText().toStdString();
		std::string strUrl = ui.lineEdit_path->text().toStdString();
		std::string strName = ConvertToSDKEncoding(ui.lineEdit_layerName->text());
		std::string strCachePath = ui.lineEdit_cachePath->text().toStdString();
		std::string strCacheId = ui.lineEdit_cacheName->text().toStdString();

		FeLayers::CFeLayerElevationConfig  cfg;
		cfg.name() = strName;
		cfg.sourceConfig().driver() = strDriver;
		cfg.sourceConfig().url() = strDriver;
		cfg.visible() = true;


		if (ui.checkBox_cache->isChecked())
		{
		
			cfg.cacheConfig().cacheEnable() = true;
			cfg.cacheConfig().cachePath() = strCachePath;
			
		}
		else
		{
			cfg.cacheConfig().cacheEnable() = false;
		}

		if (m_pCustomLayerSys != NULL)
		{
			FeLayers::IFeLayer * pLayer =  m_pCustomLayerSys->AddLayer(cfg);
			emit SignaElevationLayerFinish(pLayer, cfg);
		}
		
	}

	void CElevationLayerDialog::SlotStateChanged(int state)
	{
		if (state == Qt::Checked)
		{
			ui.label_cacheName->setEnabled(true);
			ui.label_cachePath->setEnabled(true);
			ui.lineEdit_cacheName->setEnabled(true);
			ui.lineEdit_cachePath->setEnabled(true);
			ui.btn_find_cachePath->setEnabled(true);
		}
		else
		{
			ui.label_cacheName->setEnabled(false);
			ui.label_cachePath->setEnabled(false);
			ui.lineEdit_cacheName->setEnabled(false);
			ui.lineEdit_cachePath->setEnabled(false);
			ui.btn_find_cachePath->setEnabled(false);
		}
	}

	CElevationLayerDialog::~CElevationLayerDialog()
	{

	}
}
