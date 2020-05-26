#include <layer/PointVectorLayerDialog.h>
#include "mainWindow/FreeUtil.h"

namespace FreeViewer
{
	using namespace osgEarth;

	CPointVectorLayerDialog::CPointVectorLayerDialog(QWidget *parent, FeLayers::CLayerSys * pCustomLayerSys)
		: CFreeDialog(parent)
		, m_pCustomLayerSys(pCustomLayerSys)
		, m_bIsProperty(false)
	{
		//添加ui界面到属性窗体客户区
		QWidget *widget = new QWidget(this);
		ui.setupUi(widget);
		//AddSubWidget(widget);
		AddWidgetToDialogLayout(widget);
		widget->setFixedSize(widget->geometry().size());

		InitData();

		SetStyle();
	}

	void CPointVectorLayerDialog::InitData()
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
		/*ui.btn_lineColor->setStyleSheet(strLineColor);
		ui.btn_fillColor->setStyleSheet(strLineColor);*/

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
		connect(ui.btn_fillColor, SIGNAL(clicked()), this, SLOT(SlotSetLineFillColor()));
		connect(ui.btn_find_icon, SIGNAL(clicked()), this, SLOT(SlotOpenIconPath()));
		connect(ui.btn_fontColor, SIGNAL(clicked()), this, SLOT(SlotSetFontColor()));
		connect(ui.btn_shadowColor, SIGNAL(clicked()), this, SLOT(SlotSetFontShadowColor()));
		connect(ui.btn_ok, SIGNAL(clicked()), this, SLOT(SlotOk()));
		connect(ui.btn_cancel, SIGNAL(clicked()), this, SLOT(SlotCancel()));
	}

	void CPointVectorLayerDialog::SetStyle()
	{
		ui.tab_style->setStyleSheet("border: none;");
		ui.tab_font->setStyleSheet("border: none;");
		ui.spinBox_minDistance->setStyleSheet("border: 1px solid rgb(44,80,114);");
		ui.spinBox_maxDistance->setStyleSheet("border: 1px solid rgb(44,80,114);");
		ui.dSpinBox_lineWidth->setStyleSheet("border: 1px solid rgb(44,80,114);");
		ui.dspinBox_lineOpacity->setStyleSheet("border: 1px solid rgb(44,80,114);");
		ui.comboBox_attribute->setStyleSheet("border: 1px solid rgb(44,80,114);");
		ui.comboBox_technology->setStyleSheet("border: 1px solid rgb(44,80,114);");
		ui.spinBox_fontSize->setStyleSheet("border: 1px solid rgb(44,80,114);");
		ui.lineEdit_fontFormat->setStyleSheet("border: 1px solid rgb(44,80,114);");
		ui.lineEdit_icon->setStyleSheet("border: 1px solid rgb(44,80,114);");
		ui.btn_find_icon->setStyleSheet("border: 1px solid rgb(44,80,114);");
	}

	void CPointVectorLayerDialog::SlotOpenDataPath()
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
		for(QStringList::iterator itr = filePaths.begin(); itr != filePaths.end(); ++itr)
		{
			QString filePath = *itr;
			if(!filePath.isNull())
			{
				ui.lineEdit_path->setText(filePath);
			}
		}
	}

	void CPointVectorLayerDialog::SlotOpenIconPath()
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

	void CPointVectorLayerDialog::SlotSetLineColor()
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

	void CPointVectorLayerDialog::SlotSetLineFillColor()
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
			ui.btn_fillColor->setStyleSheet(strLineFillColor);

			m_colorFill = pColorDlg->currentColor();
		}
	}

	void CPointVectorLayerDialog::SlotSetFontColor()
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

	void CPointVectorLayerDialog::SlotSetFontShadowColor()
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

	void CPointVectorLayerDialog::SlotOk()
	{
		if (!CheckInputInfo())
		{
			return;
		}
		if (m_bIsProperty == false)
		{
			CreateVectorLayer();
		}
		m_bIsProperty = false;

		close();
	}

	void CPointVectorLayerDialog::SlotCancel()
	{
		close();
	}

	bool CPointVectorLayerDialog::CheckInputInfo()
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

	void CPointVectorLayerDialog::CreateVectorLayer()
	{
		//构造features配置项
		FeLayers::CFeLayerModelConfig cfg;
		
		cfg.sourceConfig().driver() = "ogr";
		cfg.sourceConfig().url() = 	 ui.lineEdit_path->text().toStdString();
		cfg.visibleConfig().maxVisibleRange() = ui.spinBox_maxDistance->value();
		cfg.visibleConfig().minVisibleRange() = ui.spinBox_minDistance->value();

		if (ui.radioBtn_lineVector->isChecked())
		{
			/*QString strLineR = QString("%1").arg(m_colorLine.red(), 2, 16, QLatin1Char('0'));
			QString strLineG = QString("%1").arg(m_colorLine.green(), 2, 16, QLatin1Char('0'));
			QString strLineB = QString("%1").arg(m_colorLine.blue(), 2, 16, QLatin1Char('0'));
			QString strLineColor = strLineR + strLineG + strLineB;
			QString strClamp = GetLaminating(ui.comboBox_attribute->currentText());
			QString strTechnique = GetTechnology(ui.comboBox_technology->currentText());
			strStyle =
				QString("lo{")
				+ "stroke-width:" + QString::number(ui.dSpinBox_lineWidth->value()) + ";"
				+ "stroke-opacity:" + QString::number(ui.dspinBox_lineOpacity->value()) + ";"
				+ "stroke:" + strLineColor + ";"
				+ "altitude-clamping:" + strClamp + ";"
				+ "altitude-technique:" + strTechnique + ";"
				+ "}";*/

			cfg.featureSymbol().lineColor() = 	osgEarth::Symbology::Color(m_colorLine.red()/255.0, m_colorLine.green()/255.0, m_colorLine.blue()/255.0);

		}
		else
		{
			/*QString strFontR = QString("%1").arg(m_colorFont.red(), 2, 16, QLatin1Char('0'));
			QString strFontG = QString("%1").arg(m_colorFont.green(), 2, 16, QLatin1Char('0'));
			QString strFontB = QString("%1").arg(m_colorFont.blue(), 2, 16, QLatin1Char('0'));
			QString strFontColor = strFontR + strFontG + strFontB;
			QString strFontShadowR = QString("%1").arg(m_colorFontShadow.red(), 2, 16, QLatin1Char('0'));
			QString strFontShadowG = QString("%1").arg(m_colorFontShadow.green(), 2, 16, QLatin1Char('0'));
			QString strFontShadowB = QString("%1").arg(m_colorFontShadow.blue(), 2, 16, QLatin1Char('0'));
			QString strFontShadowColor = strFontShadowR + strFontShadowG + strFontShadowB;
			strStyle = 
				QString("lo{")
				+ "text-content: [NAME];"
				+ "text-size:" + QString::number(ui.spinBox_fontSize->value()) + ";"
				+ "text-fill:#" + strFontColor + ";"
				+ "text-halo:#" + strFontShadowColor + ";"
				+ "text-font:fonts/simhei.ttf;"
				+ "text-encoding:utf-8;"
				+ "icon:" + "\"" + ui.lineEdit_icon->text() + "\"" + ";"
				+ "icon-placement: vertex;"
				+ "icon-scale: 1.0;"
				+ "icon-declutter:false;"
				+"}";*/
			cfg.featureSymbol().fontName() = "fonts/simhei.ttf";
			cfg.featureSymbol().AddFeatureType(FeLayers::CFeFeatureSymbol::FT_POINT);
			cfg.featureSymbol().fillColor() =  osgEarth::Symbology::Color(m_colorFont.red()/255.0, m_colorFont.green()/255.0, m_colorFont.blue()/255.0);
			
		}

		
		/*	osgEarth::Config modelConfig("model");
		modelConfig.add("driver", ui.comboBox_driveType->currentText().toStdString());
		std::string strName = ConvertToSDKEncoding(ui.lineEdit_layerName->text());
		modelConfig.add("name", strName);
		modelConfig.add("visible", true);
		modelConfig.add(featuresConfig);
		modelConfig.add(layoutConfig);
		modelConfig.add(stylesConfig);*/

		cfg.sourceConfig().driver() = ui.comboBox_driveType->currentText().toStdString();
		std::string strName = ConvertToSDKEncoding(ui.lineEdit_layerName->text());
		cfg.name() = strName;
		cfg.sourceConfig().layerName() = strName;
		cfg.visible() = true;

		if (m_pCustomLayerSys != NULL)
		{
			FeLayers::IFeLayer * pLayer = m_pCustomLayerSys->AddLayer(cfg);
			if (pLayer != NULL)
			{
			   emit SignalVectorLayerFinish(pLayer, cfg);
			}
		}
	}

	QString CPointVectorLayerDialog::GetLaminating( QString str )
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

	QString CPointVectorLayerDialog::GetTechnology( QString str )
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

	CPointVectorLayerDialog::~CVectorLayerDialog()
	{

	}

}
