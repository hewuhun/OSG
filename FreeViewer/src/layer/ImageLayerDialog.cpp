#include <layer/ImageLayerDialog.h>
#include "mainWindow/FreeUtil.h"

namespace FreeViewer
{
	using namespace osgEarth;

	CImageLayerDialog::CImageLayerDialog(QWidget *parent, FeLayers::CLayerSys * pCustomLayerSys)
		: CFreeDialog(parent)
		, m_bIsImage(true)
	{
		//添加ui界面到属性窗体客户区
		QWidget *widget = new QWidget(this);
		ui.setupUi(widget);

		m_group1 = new QButtonGroup(this);
		m_group2 = new QButtonGroup(this);

		m_group1->addButton(ui.radioButton_Ele);
		m_group1->addButton(ui.radioButton_Image);

		m_group2->addButton(ui.radioBtn_singleData);
		m_group2->addButton(ui.radioBtn_sliceData);
		ui.radioBtn_singleData->setChecked(true);

		connect(m_group1, SIGNAL(buttonClicked(QAbstractButton*)),
			this, SLOT(SetTile(QAbstractButton*)));

		connect(m_group2, SIGNAL(buttonClicked(QAbstractButton*)),
			this, SLOT(SetPathState(QAbstractButton*)));

		ui.comboBox_driveType->setDisabled(true);
		//AddSubWidget(widget);
		AddWidgetToDialogLayout(widget);
		widget->setFixedSize(widget->geometry().size());
		m_pCustomLayerSys = pCustomLayerSys;

		InitData();
	}

	void CImageLayerDialog::InitData()
	{
		ui.lineEdit_layerName->setText(tr("unamed image layer"));
		ui.hSlider_opacityDegree->setMaximum(100);
		ui.hSlider_opacityDegree->setValue(100);
		ui.label_opacityDegreeData->setText(QString::number(double(ui.hSlider_opacityDegree->value())/ui.hSlider_opacityDegree->maximum()));

		//默认禁用缓存加载功能
		SlotCacheStateChanged(0);

		// 限制文本长度
		ui.lineEdit_layerName->setMaxLength(25);
		ui.lineEdit_cacheName->setMaxLength(25);

		connect(ui.btn_find_path, SIGNAL(clicked()), this, SLOT(SlotOpenDataOrPath()));
		connect(ui.btn_find_cachePath, SIGNAL(clicked()), this, SLOT(SlotOpenCachePath()));
		connect(ui.btn_ok, SIGNAL(clicked()), this, SLOT(SlotOk()));
		connect(ui.btn_cancel, SIGNAL(clicked()), this, SLOT(SlotCancel()));
		connect(ui.hSlider_opacityDegree, SIGNAL(valueChanged(int)), this, SLOT(SlotOpacityChanged(int)));
		connect(ui.checkBox_cache, SIGNAL(stateChanged(int)), this, SLOT(SlotCacheStateChanged(int)));
		connect(ui.checkBox_opacityLayer, SIGNAL(stateChanged(int)), this, SLOT(SlotOpacityStateChanged(int)));
	}

	void CImageLayerDialog::SlotOpenDataOrPath()
	{
		QString strCaption = tr("Add Image Layer");
		QString strDir = tr(FeFileReg->GetDataPath().c_str());
		if (false == ui.lineEdit_path->text().isEmpty())
		{
			strDir = ui.lineEdit_path->text();
		}
		QString strFilter = tr("Images(*.tif *.png *.jpeg);;All files(*.*)");

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

	void CImageLayerDialog::SlotOpenCachePath()
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

	void CImageLayerDialog::SlotOk()
	{
		if (!CheckInputInfo())
		{
			return;
		}
		
		
		CreateImageLayer();
		

		float fOpacityValue = QString(ui.label_opacityDegreeData->text()).toFloat();

		emit SignalOpacityValue(fOpacityValue);

		close();
	}

	void CImageLayerDialog::SlotCancel()
	{
		close();
	}

	void CImageLayerDialog::SlotOpacityChanged(int value)
	{
		ui.label_opacityDegreeData->setText(QString::number(double(ui.hSlider_opacityDegree->value())/double(ui.hSlider_opacityDegree->maximum())));
	}

	bool CImageLayerDialog::CheckInputInfo()
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

	void CImageLayerDialog::CreateImageLayer()
	{
		//获取到对话框中的设置值
		std::string strDriver = ui.comboBox_driveType->currentText().toStdString();
		std::string strUrl = ui.lineEdit_path->text().toStdString();
		std::string strName = ui.lineEdit_layerName->text().toStdString();
		std::string strCachePath = ui.lineEdit_cachePath->text().toStdString();
		std::string strCacheId = ui.lineEdit_cacheName->text().toStdString();

		//FeLayers::CFeConfig terrainConfig;
		////初始化terrainConfig
		//terrainConfig.key() = "image";

		//terrainConfig.add("name", strName);
		//terrainConfig.add("driver", strDriver);
		//terrainConfig.add("url", strUrl);
		//terrainConfig.add("opactiy", ui.label_opacityDegreeData->text().toDouble());
		//terrainConfig.add("hastransparency", ui.checkBox_opacityLayer->isChecked());
		//terrainConfig.add("visible", true);
		//terrainConfig.add("cachepath", strCachePath);
		//terrainConfig.add("cacheid", strCacheId);

		
		if (ui.radioButton_Image->isChecked())
		{
			FeLayers::CFeLayerImageConfig cfg;
			cfg.name() = strName;
			cfg.visibleConfig().opacity() = ui.label_opacityDegreeData->text().toDouble();
			cfg.visibleConfig().opacityEnable() = ui.checkBox_opacityLayer->isChecked();
			cfg.sourceConfig().url() = 	strUrl;
			cfg.sourceConfig().driver() = strDriver;
			cfg.sourceConfig().layerName() = m_strLayerRealName;
			cfg.visible() = true;
			cfg.cacheConfig().cacheEnable() = ui.checkBox_cache->isChecked();
			cfg.cacheConfig().cachePath() =  strCachePath;
			cfg.cacheConfig().cacheName() = strCacheId;
			 this->setWindowTitle(tr("添加影像"));
			FeLayers::IFeLayer * pImageLayer = 	m_pCustomLayerSys->AddLayer(cfg);

			emit SignalImageLayerFinish(pImageLayer, cfg);
		}
		else
		{
			FeLayers::CFeLayerElevationConfig cfg;
			cfg.name() = strName;
			cfg.sourceConfig().url() = 	strUrl;
			cfg.sourceConfig().driver() = strDriver;
			cfg.sourceConfig().layerName() = m_strLayerRealName;
			cfg.visible() = true;
			cfg.cacheConfig().cacheEnable() = ui.checkBox_cache->isChecked();
			cfg.cacheConfig().cachePath() =  strCachePath;
			cfg.cacheConfig().cacheName() = strCacheId;
			this->setWindowTitle(tr("添加高程"));
			FeLayers::IFeLayer * pImageLayer = 	m_pCustomLayerSys->AddLayer(cfg);

			emit SignalImageLayerFinish(pImageLayer, cfg);
		}


		
	}

	void CImageLayerDialog::SlotCacheStateChanged(int state)
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

	void CImageLayerDialog::SlotOpacityStateChanged(int state)
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

	void CImageLayerDialog::SetTile(QAbstractButton* btn)
	{
		if (btn == ui.radioButton_Image)
		{
			this->SetTitleText(tr("影像图层"));
			ui.label_display->setVisible(true);
			ui.checkBox_opacityLayer->setVisible(true);
			ui.label_opacityDegree->setVisible(true);
			ui.hSlider_opacityDegree->setVisible(true);
			ui.label_opacityDegreeData->setVisible(true);
		}
		else
		{
			this->SetTitleText(tr("高程图层"));

			ui.label_display->setVisible(false);
			ui.checkBox_opacityLayer->setVisible(false);
			ui.label_opacityDegree->setVisible(false);
			ui.hSlider_opacityDegree->setVisible(false);
			ui.label_opacityDegreeData->setVisible(false);
		}
	 
	/*	if (m_bIsImage)
		{
			this->setWindowTitle(tr("添加影像"));
				

			ui.label_display->setVisible(true);
			ui.checkBox_opacityLayer->setVisible(true);
			ui.label_opacityDegree->setVisible(true);
			ui.hSlider_opacityDegree->setVisible(true);
			ui.label_opacityDegreeData->setVisible(true);
		}
		else
		{
			this->setWindowTitle(tr("添加高程"));
				
			ui.label_display->setVisible(false);
			ui.checkBox_opacityLayer->setVisible(false);
			ui.label_opacityDegree->setVisible(false);
			ui.hSlider_opacityDegree->setVisible(false);
			ui.label_opacityDegreeData->setVisible(false);
		}*/

		
	   
	}

	void CImageLayerDialog::SetPathState(QAbstractButton* btn)
	{
		  if (btn == ui.radioBtn_singleData)
		  {
			  ui.comboBox_driveType->clear();
			  QStringList strList;
			  strList << QString("gdal");
			  ui.comboBox_driveType->addItems(strList);
			  ui.comboBox_driveType->setDisabled(true);
		  }
		  else
		  {
			  ui.comboBox_driveType->setDisabled(false);


			  ui.comboBox_driveType->clear();
			  QStringList strList;
			  strList << QString("flatgis");
			  strList << QString("tms");
			  ui.comboBox_driveType->addItems(strList);
		  }
	}

	CImageLayerDialog::~CImageLayerDialog()
	{

	}
}
