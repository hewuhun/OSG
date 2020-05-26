#include <layer/VectorLayerDialog.h>
#include "mainWindow/FreeUtil.h"
#include <FeUtils/ShpAndKMLTypeParser.h>
namespace FreeViewer
{
	using namespace osgEarth;

	CVectorLayerDialog::CVectorLayerDialog(QWidget *parent, FeLayers::CLayerSys * pCustomLayerSys)
		: CFreeDialog(parent)
		, m_pCustomLayerSys(pCustomLayerSys)
		, m_pItem(NULL)
		, m_pLayer(NULL)
	{
		//添加ui界面到属性窗体客户区
		QWidget *widget = new QWidget(this);
		ui.setupUi(widget);
		AddWidgetToDialogLayout(widget);
		widget->setFixedSize(widget->geometry().size());
		//ui.comboBox_driveType->setStyleSheet("down-arrow:none");
		m_pTab_font = ui.tab_font;
		m_pTab_style = ui.tab_style;
		InitData();

		SetStyle();
	}

	void CVectorLayerDialog::InitData()
	{
		ui.checkBox_dodge->hide();

		ui.label_fontFormat->hide();
		ui.lineEdit_fontFormat->hide();

		ui.lineEdit_layerName->setText(tr("unamed vector layer"));
		ui.lineEdit_layerName->setMaxLength(25);	// 限制文本长度

		m_colorLine = QColor(255, 255, 0);
		m_colorFill = QColor(255, 255, 0);
		m_colorFont = QColor(255, 255, 255);
		m_colorFontShadow = QColor(56, 58, 63);

		QString strLineColor;
		strLineColor = "background-color:rgb(" 
			+ QString::number(m_colorLine.red()) + ","
			+ QString::number(m_colorLine.green()) + ","
			+ QString::number(m_colorLine.blue()) + "); border: none;";
		ui.btn_lineColor->setStyleSheet(strLineColor);
		//ui.btn_fillColor->setStyleSheet(strLineColor);

		QString strFontColor;
		strFontColor = "background-color:rgb(" 
			+ QString::number(m_colorFont.red()) + ","
			+ QString::number(m_colorFont.green()) + ","
			+ QString::number(m_colorFont.blue()) + "); border: none;";
		ui.btn_fontColor->setStyleSheet(strFontColor);

		QString strFontShadowColor;
		strFontShadowColor = "background-color:rgb(" 
			+ QString::number(m_colorFontShadow.red()) + ","
			+ QString::number(m_colorFontShadow.green()) + ","
			+ QString::number(m_colorFontShadow.blue()) + "); border: none;";
		ui.btn_shadowColor->setStyleSheet(strFontShadowColor);

		m_vecLaminating.push_back("贴地");
		m_vecLaminating.push_back("相对高度");
		m_vecLaminating.push_back("绝对高度");
		m_vecLaminating.push_back("无");

		m_vecTechnology.push_back("GPU渲染地形");
		m_vecTechnology.push_back("投影纹理");
		m_vecTechnology.push_back("高程map");
		m_vecTechnology.push_back("场景模型");

		connect(ui.btn_find_path, SIGNAL(clicked()), this, SLOT(SlotOpenDataPath()));
		connect(ui.btn_lineColor, SIGNAL(clicked()), this, SLOT(SlotSetLineColor()));
	//	connect(ui.btn_fillColor, SIGNAL(clicked()), this, SLOT(SlotSetLineFillColor()));
		connect(ui.btn_find_icon, SIGNAL(clicked()), this, SLOT(SlotOpenIconPath()));
		connect(ui.btn_fontColor, SIGNAL(clicked()), this, SLOT(SlotSetFontColor()));
		connect(ui.btn_shadowColor, SIGNAL(clicked()), this, SLOT(SlotSetFontShadowColor()));
		connect(ui.btn_ok, SIGNAL(clicked()), this, SLOT(SlotOk()));
		connect(ui.btn_cancel, SIGNAL(clicked()), this, SLOT(SlotCancel()));
	}

	void CVectorLayerDialog::SetStyle()
	{
		ui.tab_style->setStyleSheet("border: none;");
		ui.tab_font->setStyleSheet("border: none;");
		ui.spinBox_minDistance->setStyleSheet("border: 1px solid rgb(44,80,114);");
		ui.spinBox_maxDistance->setStyleSheet("border: 1px solid rgb(44,80,114);");
		ui.dSpinBox_lineWidth->setStyleSheet("border: 1px solid rgb(44,80,114);");
		ui.dspinBox_lineOpacity->setStyleSheet("border: 1px solid rgb(44,80,114);");
	//	ui.comboBox_attribute->setStyleSheet("border: 1px solid rgb(44,80,114);");
	//	ui.comboBox_technology->setStyleSheet("border: 1px solid rgb(44,80,114);");
		ui.spinBox_fontSize->setStyleSheet("border: 1px solid rgb(44,80,114);");
		ui.lineEdit_fontFormat->setStyleSheet("border: 1px solid rgb(44,80,114);");
		ui.lineEdit_icon->setStyleSheet("border: 1px solid rgb(44,80,114);");
		ui.btn_find_icon->setStyleSheet("border: 1px solid rgb(44,80,114);");
	}

	void CVectorLayerDialog::SlotOpenDataPath()
	{
		QString strCaption = tr("Add Feature Layer");
		QString strDir = tr(FeFileReg->GetDataPath().c_str());
		if (false == ui.lineEdit_path->text().isEmpty())
		{
			strDir = ui.lineEdit_path->text();
		}
		QString strFilter = tr("Feature(*.shp)");

		//打开用于创建矢量的原始文件
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


		if (filePaths.length() <= 0)
		{
			return;
		}
	/*	for(QStringList::iterator itr = filePaths.begin(); itr != filePaths.end(); ++itr)
		{
			QString filePath = *itr;
			if(!filePath.isNull())
			{*/
				ui.lineEdit_path->setText(filePaths.at(0));
		/*	}
		}*/

		//std::vector<FeUtil::GeometryType> types = FeUtil::ShpAndKMLTypeParser::Parse(filePaths.at(0).toStdString());

		//for (std::vector<FeUtil::GeometryType>::iterator iter = types.begin(); iter != types.end(); ++iter)
		//{
		//	if (*iter == FeUtil::POINT)
		//	{

		//		//ui.labelShpType->setText(tr("PointVector"));
		//		OnlyShowFont();

		//	}
		//	else
		//	{
		//		//ui.labelShpType->setText(tr("LineVector"));
		//		OnlyShowStyle();
		//	}
		//}

		ui.tabWidget->setVisible(true);

	}

	void CVectorLayerDialog::SlotOpenIconPath()
	{
		QString strCaption = tr("Add Icon");
		QString strDir = tr(FeFileReg->GetDataPath().c_str());
		if (false == ui.lineEdit_icon->text().isEmpty())
		{
			strDir = ui.lineEdit_icon->text();
		}
		QString strFilter = tr("Icon(*.png *.jpg *.jpeg);;All files(*.*)");

		//打开用于创建矢量的原始文件
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
				ui.lineEdit_icon->setText(filePath);
			}
		}
	}

	void CVectorLayerDialog::SlotSetLineColor()
	{
		QColorDialog* pColorDlg = new QColorDialog(QColor());
		pColorDlg->setWindowFlags(Qt::WindowStaysOnTopHint);

		if (pColorDlg->exec() == QDialog::Accepted)
		{
			QString strLineColor;
			strLineColor = "background-color:rgb(" 
				+ QString::number(pColorDlg->currentColor().red()) + "," 
				+ QString::number(pColorDlg->currentColor().green()) + ","
				+ QString::number(pColorDlg->currentColor().blue()) + "); border: none;";
			ui.btn_lineColor->setStyleSheet(strLineColor);

			m_colorLine = pColorDlg->currentColor();
		}
	}

	void CVectorLayerDialog::SlotSetLineFillColor()
	{
		QColorDialog* pColorDlg = new QColorDialog(QColor());
		pColorDlg->setWindowFlags(Qt::WindowStaysOnTopHint);

		if (pColorDlg->exec() == QDialog::Accepted)
		{
			QString strLineFillColor;
			strLineFillColor = "background-color:rgb(" 
				+ QString::number(pColorDlg->currentColor().red()) + "," 
				+ QString::number(pColorDlg->currentColor().green()) + ","
				+ QString::number(pColorDlg->currentColor().blue()) + "); border: none;";
		//	ui.btn_fillColor->setStyleSheet(strLineFillColor);

			m_colorFill = pColorDlg->currentColor();
		}
	}

	void CVectorLayerDialog::SlotSetFontColor()
	{
		QColorDialog* pColorDlg = new QColorDialog(QColor());
		pColorDlg->setWindowFlags(Qt::WindowStaysOnTopHint);

		if (pColorDlg->exec() == QDialog::Accepted)
		{
			QString strFontColor;
			strFontColor = "background-color:rgb(" 
				+ QString::number(pColorDlg->currentColor().red()) + "," 
				+ QString::number(pColorDlg->currentColor().green()) + ","
				+ QString::number(pColorDlg->currentColor().blue()) + "); border: none;";
			ui.btn_fontColor->setStyleSheet(strFontColor);

			m_colorFont = pColorDlg->currentColor();
		}
	}

	void CVectorLayerDialog::SlotSetFontShadowColor()
	{
		QColorDialog* pColorDlg = new QColorDialog(QColor());
		pColorDlg->setWindowFlags(Qt::WindowStaysOnTopHint);

		if (pColorDlg->exec() == QDialog::Accepted)
		{
			QString strFontShadowColor;
			strFontShadowColor = "background-color:rgb(" 
				+ QString::number(pColorDlg->currentColor().red()) + "," 
				+ QString::number(pColorDlg->currentColor().green()) + ","
				+ QString::number(pColorDlg->currentColor().blue()) + "); border: none;";
			ui.btn_shadowColor->setStyleSheet(strFontShadowColor);

			m_colorFontShadow = pColorDlg->currentColor();
		}
	}

	void CVectorLayerDialog::SlotOk()
	{
		if (!CheckInputInfo())
		{
			return;
		}
		if (m_pLayer == NULL)
		{
			CreateVectorLayer();
		}
		
		if (m_pLayer != NULL)
		{
			FeLayers::IFeLayerModel * modelLayer = m_pLayer->AsModelLayer();
			
		
			ui.tabWidget->setVisible(true);


			if (modelLayer->HasPointFeature())
			{
				OnlyShowFont();
				 ChangeFont();
			}
			else
			{
				OnlyShowStyle();
				ChangeLine();
			}
			
		}

		
		m_pLayer = NULL;
		close();

		
	}

	void CVectorLayerDialog::SlotCancel()
	{
		close();
	}

	bool CVectorLayerDialog::CheckInputInfo()
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

		if (redStyleFlag)
		{
			return false;
		}

		return true;
	}

	void CVectorLayerDialog::CreateVectorLayer()
	{
		

		//构造features配置项
		FeLayers::CFeLayerModelConfig cfg;
		
		cfg.sourceConfig().url() = 	 ui.lineEdit_path->text().toStdString();
		cfg.visibleConfig().maxVisibleRange() = ui.spinBox_maxDistance->value();
		cfg.visibleConfig().minVisibleRange() = ui.spinBox_minDistance->value();
		cfg.sourceConfig().driver() = ui.comboBox_driveType->currentText().toStdString();

		std::string strName = ui.lineEdit_layerName->text().toStdString();
		cfg.name() = strName;
		cfg.sourceConfig().layerName() = strName;
		cfg.visible() = true;

		if (m_pCustomLayerSys != NULL)
		{
			FeLayers::IFeLayer * pLayer = m_pCustomLayerSys->AddLayer(cfg);

			if (pLayer != NULL)
			{
				if (cfg.sourceConfig().driver() == "WFS" || cfg.sourceConfig().driver() == "wfs")
				{
					ui.tabWidget->setVisible(true);
					
					FeLayers::IFeLayerModel * temp = pLayer->AsModelLayer();
					if (temp->HasPointFeature())
					{
						OnlyShowFont();
					}
					else
					{
						OnlyShowStyle();
					}
				}
				
				m_pLayer = pLayer;
			   emit SignalVectorLayerFinish(pLayer, cfg);
			}
		}
	}

	QString CVectorLayerDialog::GetLaminating( QString str )
	{
		int i=0;
		for (i=0; i<m_vecLaminating.size(); i++)
		{
			if (str == m_vecLaminating.value(i))
			{
				break;
			}
		}

		QString strValue;
		switch (i)
		{
		case 0:
			strValue = "terrain";
			break;
		case 1:
			strValue = "relative";
			break;
		case 2:
			strValue = "absolute";
			break;
		case 3:
			strValue = "none";
			break;
		default:
			break;
		}

		return strValue;
	}

	QString CVectorLayerDialog::GetTechnology( QString str )
	{
		int i=0;
		for (i=0; i<m_vecTechnology.size(); i++)
		{
			if (str == m_vecTechnology.value(i))
			{
				break;
			}
		}

		QString strValue;
		switch (i)
		{
		case 0:
			strValue = "gpu";
			break;
		case 1:
			strValue = "drape";
			break;
		case 2:
			strValue = "map";
			break;
		case 3:
			strValue = "scene";
			break;
		default:
			break;
		}

		return strValue;
	}

	void CVectorLayerDialog::OnlyShowFont()
	{
		ui.tabWidget->clear();
		ui.tabWidget->addTab(m_pTab_font, QString(tr("Font")));
	}

	void CVectorLayerDialog::OnlyShowStyle()
	{
		ui.tabWidget->clear();
		ui.tabWidget->addTab(m_pTab_style, QString(tr("Style")));
	}

	void CVectorLayerDialog::ShowAll()
	{
		ui.tabWidget->clear();
		ui.tabWidget->addTab(m_pTab_style, QString(tr("Style")));
		ui.tabWidget->addTab(m_pTab_font, QString(tr("Style")));
	}


	void CVectorLayerDialog::Reset()
	{
		ui.checkBox_dodge->hide();

		ui.label_fontFormat->hide();
		ui.lineEdit_fontFormat->hide();

		ui.lineEdit_layerName->setText(tr("unamed vector layer"));
		ui.lineEdit_layerName->setMaxLength(25);	// 限制文本长度

		m_colorLine = QColor(255, 255, 0);
		m_colorFill = QColor(255, 255, 0);
		m_colorFont = QColor(255, 255, 255);
		m_colorFontShadow = QColor(56, 58, 63);

		QString strLineColor;
		strLineColor = "background-color:rgb(" 
			+ QString::number(m_colorLine.red()) + ","
			+ QString::number(m_colorLine.green()) + ","
			+ QString::number(m_colorLine.blue()) + "); border: none;";
		ui.btn_lineColor->setStyleSheet(strLineColor);
		//ui.btn_fillColor->setStyleSheet(strLineColor);

		QString strFontColor;
		strFontColor = "background-color:rgb(" 
			+ QString::number(m_colorFont.red()) + ","
			+ QString::number(m_colorFont.green()) + ","
			+ QString::number(m_colorFont.blue()) + "); border: none;";
		ui.btn_fontColor->setStyleSheet(strFontColor);

		QString strFontShadowColor;
		strFontShadowColor = "background-color:rgb(" 
			+ QString::number(m_colorFontShadow.red()) + ","
			+ QString::number(m_colorFontShadow.green()) + ","
			+ QString::number(m_colorFontShadow.blue()) + "); border: none;";
		ui.btn_shadowColor->setStyleSheet(strFontShadowColor);
	    ui.lineEdit_path->setText("");
		ui.lineEdit_layerName->setText("");
		SetStyle();
	}

	void CVectorLayerDialog::ChangeLine()
	{
		if (m_pLayer == NULL)
		{
			return;
		}
		FeLayers::IFeLayerModel * modelLayer = m_pLayer->AsModelLayer();
		if (modelLayer == NULL)
		{
			return;
		}
		modelLayer->SetLineColor(osg::Vec4f(m_colorLine.red()/255.0, m_colorLine.green()/255.0, m_colorLine.blue()/255.0, m_colorLine.alpha()/255.0));

		modelLayer->SetLineWidth(ui.dSpinBox_lineWidth->value());

		//int attr = ui.comboBox_attribute->currentIndex();
		//switch (attr)
		//{
		//case 0:
		//	modelLayer->SetAltitudeClamp(FeLayers::AltitudeClamp::CLAMP_TO_TERRAIN);
		//	break;
		//case 1:
		//	modelLayer->SetAltitudeClamp(FeLayers::AltitudeClamp::CLAMP_RELATIVE_TO_TERRAIN);
		//	break;
		//case 2:
		//	modelLayer->SetAltitudeClamp(FeLayers::AltitudeClamp::CLAMP_ABSOLUTE);
		//	break;
		//case 3:
		//	modelLayer->SetAltitudeClamp(FeLayers::AltitudeClamp::CLAMP_NONE);
		//	break;
		//default:
		//	modelLayer->SetAltitudeClamp(FeLayers::AltitudeClamp::CLAMP_TO_TERRAIN);
		//	break;
		//}

		//int te = ui.comboBox_technology->currentIndex();
		//switch (te)
		//{
		//case 0:
		//	modelLayer->SetAltitudeTechnique(FeLayers::AltitudeTechnique::TECHNIQUE_GPU);
		//	break;
		//case 1:
		//	modelLayer->SetAltitudeTechnique(FeLayers::AltitudeTechnique::TECHNIQUE_DRAPE);
		//	break;
		//case 2:
		//	modelLayer->SetAltitudeTechnique(FeLayers::AltitudeTechnique::TECHNIQUE_MAP);
		//	break;
		//case 3:
		//	modelLayer->SetAltitudeTechnique(FeLayers::AltitudeTechnique::TECHNIQUE_SCENE);
		//	break;
		//default:
		//	modelLayer->SetAltitudeTechnique(FeLayers::AltitudeTechnique::TECHNIQUE_GPU);
		//	break;
		//}

		modelLayer->SetVisibleRange(ui.spinBox_minDistance->value(), ui.spinBox_maxDistance->value());
		modelLayer->SetFillColor(osg::Vec4f(m_colorFill.red()/255.0, m_colorFill.green()/255.0, m_colorFill.blue()/255.0, m_colorFill.alpha()/255.0));
		modelLayer->setOpacity(ui.dspinBox_lineOpacity->value());
	}

	void CVectorLayerDialog::ChangeFont()
	{
		if (m_pLayer == NULL)
		{
			return;
		}
		FeLayers::IFeLayerModel * modelLayer = m_pLayer->AsModelLayer();
		if (modelLayer == NULL)
		{
			return;
		}

		modelLayer->SetFontSize(ui.spinBox_fontSize->value());
		modelLayer->SetFontColor(osg::Vec4f(m_colorFont.red()/255.0, m_colorFont.green()/255.0, m_colorFont.blue()/255.0, m_colorFont.alpha()/255.0));
		modelLayer->SetFontHaloColor(osg::Vec4f(m_colorFontShadow.red()/255.0, m_colorFontShadow.green()/255.0, m_colorFontShadow.blue()/255.0, m_colorFontShadow.alpha()/255.0));
		modelLayer->SetPointIconPath(ui.lineEdit_icon->text().toStdString());
		modelLayer->SetPointIconVisible(true);


	}

	void CVectorLayerDialog::SetLayerAndItem(FeLayers::IFeLayer * pLayer, QTreeWidgetItem * pItem)
	{
		m_pLayer = pLayer;
		m_pItem = pItem;
		if (m_pLayer != NULL)
		{
			FeLayers::IFeLayerModel * modelLayer = m_pLayer->AsModelLayer();


			ui.tabWidget->setVisible(true);


			if (modelLayer->HasPointFeature())
			{
				OnlyShowFont();
			
				//ChangeFont();
			}
			else
			{
				OnlyShowStyle();
			
				//ChangeLine();
			}

		}

	}


	CVectorLayerDialog::~CVectorLayerDialog()
	{

	}

}
