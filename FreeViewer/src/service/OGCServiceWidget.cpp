#include <service/OGCServiceWidget.h>
#include <mainWindow/FreeUtil.h>

#include <QListWidgetItem>
#include <FeServiceProvider/ServiceLayer.h>
namespace FreeViewer
{
	COGCServiceWidget::COGCServiceWidget(QWidget *parent)
		: CFreeDialog(parent)
		,m_bExampleUrl(true)
	{
		QWidget *pWidget = new QWidget(this);
		ui.setupUi(pWidget);
		AddWidgetToDialogLayout(pWidget);
		pWidget->setFixedSize(pWidget->geometry().size());

		InitWidget();
	}

	COGCServiceWidget::~COGCServiceWidget()
	{
		
	}

	void COGCServiceWidget::InitWidget()
	{
		SetTitleText(tr("Add OGC Service"));

		// 模态对话框
		this->setWindowModality(Qt::WindowModal);

		// 限制名称最大长度32
		ui.lineEdit_name->setMaxLength(32);
		ui.lineEdit_name->setValidator(PutInNoEmpty());

		// 限制网址最大长度200
		ui.textEdit_url->document()->setMaximumBlockCount(6);

		connect(ui.comboBox_type, SIGNAL(currentIndexChanged(int)), this, SLOT(SlotTypeChanged(int)));
		connect(ui.pushButton_getLayer, SIGNAL(clicked()), this, SLOT(SlotGetLayerBtnClicked()));
		connect(ui.pushButton_ok, SIGNAL(clicked()), this, SLOT(SlotOkBtnClicked()));
		connect(ui.pushButton_cancle, SIGNAL(clicked()), this, SLOT(SlotCancleBtnClicked()));
		connect(ui.textEdit_url, SIGNAL(textChanged()), this, SLOT(SlotEditTextChanged()));
		connect(ui.textEdit_url, SIGNAL(selectionChanged()), this, SLOT(SlotUrlClicked()));
	}

	void COGCServiceWidget::Reset(QTreeWidgetItem *item)
	{
		if (NULL != item)
		{
			EServiceItemType type = (EServiceItemType)item->data(0, Qt::UserRole).toInt();
			Reset(type);
		}
		else
		{
			Reset(E_WMS_SERVICE_ROOT);
		}
	}

	void COGCServiceWidget::Reset( EServiceItemType itemType )
	{
		ui.listWidget_getLayer->clear();
		ui.lineEdit_userName->clear();
		ui.lineEdit_password->clear();

		ui.lineEdit_name->setText(tr("unnamed"));
		//wms
		SlotTypeChanged(0);

		ui.lineEdit_name->setStyleSheet("background: rgba(12,22,32,110); border: 1px solid rgb(44,80,114);");

		switch (itemType)
		{
		case E_WMS_SERVICE_ROOT:
			{
				ui.comboBox_type->setCurrentIndex(E_WMS);
				ui.comboBox_version->clear();
				ui.comboBox_version->addItem("1.3.0");
				ui.comboBox_version->addItem("1.1.1");
				SlotTypeChanged(0);
			}
			break;

		case E_WMTS_SERVICE_ROOT:
			{
				ui.comboBox_type->setCurrentIndex(E_WMTS);
				ui.comboBox_version->clear();
				ui.comboBox_version->addItem("1.0.0");
				SlotTypeChanged(1);
			}
			break;

		case E_WFS_SERVICE_ROOT:
			{
				ui.comboBox_type->setCurrentIndex(E_WFS);
				ui.comboBox_version->clear();
				ui.comboBox_version->addItem("2.0.0");
				ui.comboBox_version->addItem("1.0.0");
				SlotTypeChanged(2);
			}
			break;

		case E_WCS_SERVICE_ROOT:
			{
				ui.comboBox_type->setCurrentIndex(E_WCS);
				ui.comboBox_version->clear();
				//ui.comboBox_version->addItem("2.0.1");
				ui.comboBox_version->addItem("1.0.0");
				SlotTypeChanged(3);
			}
			break;

		default:
			break;
		}

		ui.comboBox_version->setCurrentIndex(0);
	}

	void COGCServiceWidget::SlotGetLayerBtnClicked()
	{
		ui.pushButton_getLayer->setDisabled(true);

		while (ui.listWidget_getLayer->count() > 0)
		{
			QListWidgetItem *pItem = ui.listWidget_getLayer->item(0);
			delete pItem;
			pItem = NULL;
		}
		ui.listWidget_getLayer->clear();

		// 判断服务类型添加服务
		QString strService = ui.comboBox_type->currentText();

		QString strName = ui.lineEdit_name->text();
		QString strUrl = ui.textEdit_url->toPlainText();
		QString strVersion = ui.comboBox_version->currentText();
		QString strUserName = ui.lineEdit_userName->text();
		QString strPassword = ui.lineEdit_password->text();

		if (strService == "WMS")
		{
			FeServiceProvider::WMSServiceProvider *pWMS = new FeServiceProvider::WMSServiceProvider(strName.toStdString(), strUrl.toStdString(), strUserName.toStdString(), strPassword.toStdString());
			pWMS->SetVersion(strVersion.toStdString());

			// 服务连接成功，显示图层列表
			if (pWMS->TestNet())
			{
				ShowLayerList(pWMS);
			}
			else
			{
				ShowLinkError();
			}
		}
		else if (strService == "WMTS")
		{
			FeServiceProvider::WMTSServiceProvider *pWMTS = new FeServiceProvider::WMTSServiceProvider(strName.toStdString(), strUrl.toStdString(), strUserName.toStdString(), strPassword.toStdString());
			pWMTS->SetVersion(strVersion.toStdString());

			// 服务连接成功，显示图层列表
			if (pWMTS->TestNet())
			{
				ShowLayerList(pWMTS);
			}
			else
			{
				ShowLinkError();
			}
		}
		else if (strService == "WFS")
		{
			FeServiceProvider::WFSServiceProvider *pWFS = new FeServiceProvider::WFSServiceProvider(strName.toStdString(), strUrl.toStdString(), strUserName.toStdString(), strPassword.toStdString());
			pWFS->SetVersion(strVersion.toStdString());

			// 服务连接成功，显示图层列表
			if (pWFS->TestNet())
			{
				ShowLayerList(pWFS);
			}
			else
			{
				ShowLinkError();
			}
		}
		else if (strService == "WCS")
		{
			FeServiceProvider::WCSServiceProvider *pWCS = new FeServiceProvider::WCSServiceProvider(strName.toStdString(), strUrl.toStdString(), strUserName.toStdString(), strPassword.toStdString());
			pWCS->SetVersion(strVersion.toStdString());

			// 服务连接成功，显示图层列表
			if (pWCS->TestNet())
			{
				ShowLayerList(pWCS);
			}
			else
			{
				ShowLinkError();
			}
		}
	}

	void COGCServiceWidget::ShowLayerList(FeServiceProvider::ServiceProvider *pServiceProvider)
	{
		// 显示图层列表
		pServiceProvider->GetAttr();

		for (int i = 0; i<pServiceProvider->GetLayerCount(); ++i)
		{
			FeServiceProvider::ServiceLayer *pServiceLayer = pServiceProvider->GetLayers().at(i);

			QString strLayerName = ConvertToCurrentEncoding(pServiceLayer->GetName());
			QListWidgetItem *pItemLayer = new QListWidgetItem();
			pItemLayer->setText(strLayerName);
			pItemLayer->setIcon(QIcon(":/images/icon/serviceLayer.png"));

			ui.listWidget_getLayer->addItem(pItemLayer);
		}

		ui.pushButton_getLayer->setEnabled(true);
	}

	void COGCServiceWidget::ShowLinkError()
	{
		QListWidgetItem *pItem = new QListWidgetItem();
		pItem->setText(tr("Service Connection Failed!"));

		ui.listWidget_getLayer->addItem(pItem);

		ui.pushButton_getLayer->setEnabled(true);
	}

	void COGCServiceWidget::SlotOkBtnClicked()
	{
		if (!CheckInputInfo())
		{
			return;
		}

		close();
		SignalResetCurrentItem();

		QString strType = ui.comboBox_type->currentText();
		QString strName = ui.lineEdit_name->text();
		QString strUrl = ui.textEdit_url->toPlainText();
		QString strVersion = ui.comboBox_version->currentText();
		QString strUserName = ui.lineEdit_userName->text();
		QString strPassword = ui.lineEdit_password->text();

		if (strType == "WMS")
		{
			AddWMS(strName, strUrl, strVersion, strUserName, strPassword);
		}
		else if (strType == "WMTS")
		{
			AddWMTS(strName, strUrl, strVersion, strUserName, strPassword);
		}
		else if (strType == "WFS")
		{
			AddWFS(strName, strUrl, strVersion, strUserName, strPassword);
		}
		else if (strType == "WCS")
		{
			AddWCS(strName, strUrl, strVersion, strUserName, strPassword);
		}

		emit SignalAddServiceFinished();
	}

	void COGCServiceWidget::SlotCancleBtnClicked()
	{
		close();
		SignalResetCurrentItem();
	}

	void COGCServiceWidget::AddWMS(QString strName, QString strUrl, QString strVersion, QString usrName, QString passWord)
	{
		FeServiceProvider::WMSServiceProvider *pWMS = new FeServiceProvider::WMSServiceProvider(strName.toStdString(), strUrl.toStdString(), usrName.toStdString(), passWord.toStdString());
		pWMS->SetVersion(strVersion.toStdString());
		SignalAddOGCService(pWMS);
	}

	void COGCServiceWidget::AddWMTS(QString strName, QString strUrl, QString strVersion, QString usrName, QString passWord)
	{
		FeServiceProvider::WMTSServiceProvider *pWMTS = new FeServiceProvider::WMTSServiceProvider(strName.toStdString(), strUrl.toStdString(), usrName.toStdString(), passWord.toStdString());
		pWMTS->SetVersion(strVersion.toStdString());
		SignalAddOGCService(pWMTS);
	}

	void COGCServiceWidget::AddWFS(QString strName, QString strUrl, QString strVersion, QString usrName, QString passWord)
	{
		FeServiceProvider::WFSServiceProvider *pWFS = new FeServiceProvider::WFSServiceProvider(strName.toStdString(), strUrl.toStdString(), usrName.toStdString(), passWord.toStdString());
		pWFS->SetVersion(strVersion.toStdString());
		SignalAddOGCService(pWFS);
	}

	void COGCServiceWidget::AddWCS(QString strName, QString strUrl, QString strVersion, QString usrName, QString passWord)
	{
		FeServiceProvider::WCSServiceProvider *pWCS = new FeServiceProvider::WCSServiceProvider(strName.toStdString(), strUrl.toStdString(), usrName.toStdString(), passWord.toStdString());
		pWCS->SetVersion(strVersion.toStdString());
		SignalAddOGCService(pWCS);
	}

	void COGCServiceWidget::SlotTypeChanged( int nIndex )
	{
		ui.textEdit_url->blockSignals(true);
		ui.comboBox_version->clear();

		switch (nIndex)
		{
		case E_WMS:
			{
				ui.comboBox_version->addItem("1.3.0");
				ui.comboBox_version->addItem("1.1.1");
				ui.textEdit_url->setText(tr("eg: http://39.104.75.125:8080/freeserver/ows?"));
				m_bExampleUrl = true;
			}
			break;

		case E_WMTS:
			{
				ui.comboBox_version->addItem("1.0.0");
				ui.textEdit_url->setText(tr("eg: http://39.104.75.125:8080/freeserver/gwc/service/wmts?"));
				m_bExampleUrl = true;
			}
			break;

		case E_WFS:
			{
				ui.comboBox_version->addItem("2.0.0");
				ui.comboBox_version->addItem("1.0.0");
				ui.textEdit_url->setText(tr("eg: http://39.104.75.125:8080/freeserver/ows?"));
				m_bExampleUrl = true;
			}
			break;

		case E_WCS:
			{
				//ui.comboBox_version->addItem("2.0.1");
				ui.comboBox_version->addItem("1.0.0");
				ui.textEdit_url->setText(tr("eg: http://39.104.75.125:8080/freeserver/ows?"));
				m_bExampleUrl = true;
			}
			break;

		default:
			{
				ui.textEdit_url->setText("'");
			}
			break;
		}
		ui.textEdit_url->setStyleSheet("color: gray; background: rgba(12,22,32,110); border: 1px solid rgb(44,80,114);");
		ui.textEdit_url->blockSignals(false);
	}

	bool COGCServiceWidget::CheckInputInfo()
	{
		// 判断输入是否为空
		QString emptyStyle("color: gray; background: rgba(12,22,32,110); border: 1px solid red;");
		QString normalStyle("background: rgba(12,22,32,110); border: 1px solid rgb(44,80,114);");

		bool bIsEmpty = true;
		if (ui.lineEdit_name->text().isEmpty())
		{
			ui.lineEdit_name->setStyleSheet(emptyStyle);
			bIsEmpty = false;
		}
		else
		{
			ui.lineEdit_name->setStyleSheet(normalStyle);
		}

		if (ui.textEdit_url->toPlainText().isEmpty() || m_bExampleUrl)
		{
			ui.textEdit_url->setStyleSheet(emptyStyle);
			bIsEmpty = false;
		}
		else
		{
			ui.textEdit_url->setStyleSheet(normalStyle);
		}

		return bIsEmpty;
	}

	void COGCServiceWidget::SlotCloseBtnClicked()
	{
		close();
		SignalResetCurrentItem();
	}

	void COGCServiceWidget::SlotEditTextChanged()
	{
		ui.textEdit_url->blockSignals(true);
		ui.textEdit_url->setStyleSheet("color: white");
		QString str = ui.textEdit_url->toPlainText();
		int nPos = ui.textEdit_url->textCursor().position();
		int nLength = str.length(); 
		str = str.simplified();

		if (m_bExampleUrl)
		{
			m_bExampleUrl = false;
			ui.textEdit_url->clear();
			ui.textEdit_url->setText(str.right(1));
		}
		else
		{
			ui.textEdit_url->setText(str);
		}

		QTextCursor textCursor = ui.textEdit_url->textCursor();
		textCursor.setPosition(nPos - (nLength - str.length() ));
		ui.textEdit_url->setTextCursor(textCursor);
		ui.textEdit_url->blockSignals(false);
	}

	void COGCServiceWidget::SlotUrlClicked()
	{
		if (m_bExampleUrl)
		{
			ui.textEdit_url->clear();
		}
	}
}