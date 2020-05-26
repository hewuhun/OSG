#include <service/TileServiceWidget.h>
#include <mainWindow/FreeUtil.h>

namespace FreeViewer
{
	CTileServiceWidget::CTileServiceWidget(QWidget *parent)
		: CFreeDialog(parent)
	{
		QWidget *pWidget = new QWidget(this);
		ui.setupUi(pWidget);
		AddWidgetToDialogLayout(pWidget);
		pWidget->setFixedSize(pWidget->geometry().size());

		InitWidget();
	}

	CTileServiceWidget::~CTileServiceWidget()
	{
		
	}

	void CTileServiceWidget::InitWidget()
	{
		SetTitleText(tr("Add Tile Service"));

		// 模态对话框
		this->setWindowModality(Qt::WindowModal);

		// 限制名称
		ui.lineEdit_name->setMaxLength(32);
		ui.lineEdit_name->setValidator(PutInNoEmpty());

		// 限制网址
		ui.textEdit_url->document()->setMaximumBlockCount(6);

		connect(ui.pushButton_ok, SIGNAL(clicked()), this, SLOT(SlotOkBtnClicked()));
		connect(ui.pushButton_cancle, SIGNAL(clicked()), this, SLOT(SlotCancleBtnClicked()));
		connect(ui.textEdit_url, SIGNAL(textChanged()), this, SLOT(SlotEditTextChanged()));
		connect(ui.comboBox_type, SIGNAL(currentIndexChanged(QString)), this, SLOT(SlotTypeChanged(QString)));
		connect(ui.textEdit_url, SIGNAL(selectionChanged()), this, SLOT(SlotUrlClicked()));
	}

	void CTileServiceWidget::Reset(QTreeWidgetItem *item)
	{
		if (item != NULL)
		{
			EServiceItemType type = (EServiceItemType)item->data(0, Qt::UserRole).toInt();
			Reset(type);
		}
		else
		{
			Reset(E_TMS_SERVICE_ROOT);
		}
	}

	void CTileServiceWidget::Reset( EServiceItemType itemType )
	{
		SlotTypeChanged(ui.comboBox_type->currentText());

		switch (itemType)
		{
		case E_TMS_SERVICE_ROOT:
			{
				ui.comboBox_type->setCurrentIndex(E_TMS);
			}
			break;

		case E_XYZ_SERVICE_ROOT:
			{
				ui.comboBox_type->setCurrentIndex(E_XYZ);
			}
			break;

		default:
			break;
		}
		
		ui.lineEdit_name->setText(tr("unnamed"));
		ui.lineEdit_name->setStyleSheet("background: rgba(12,22,32,110); border: 1px solid rgb(44,80,114);");
	}

	void CTileServiceWidget::SlotOkBtnClicked()
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

		if (strType == "TMS")
		{
			AddTMS(strName, strUrl);
		}
		else if (strType == "XYZ")
		{
			AddXYZ(strName, strUrl);
		}

		emit SignalAddServiceFinished();
	}

	void CTileServiceWidget::SlotCancleBtnClicked()
	{
		close();
		SignalResetCurrentItem();
	}

	void CTileServiceWidget::AddTMS( QString strName, QString strUrl )
	{
		FeServiceProvider::TMSServiceProvider *pTMS = new FeServiceProvider::TMSServiceProvider(strName.toStdString(), strUrl.toStdString());
		SignalAddTileService(pTMS);
	}

	void CTileServiceWidget::AddXYZ( QString strName, QString strUrl )
	{
		FeServiceProvider::XYZServiceProvider *pXYZ = new FeServiceProvider::XYZServiceProvider(strName.toStdString(), strUrl.toStdString());
		SignalAddTileService(pXYZ);
	}

	bool CTileServiceWidget::CheckInputInfo()
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

	void CTileServiceWidget::SlotCloseBtnClicked()
	{
		close();
		SignalResetCurrentItem();
	}

	void CTileServiceWidget::SlotEditTextChanged()
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

	void CTileServiceWidget::SlotTypeChanged( QString strType)
	{
		ui.textEdit_url->blockSignals(true);
		if ("TMS" == strType)
		{
			ui.textEdit_url->setText(tr("eg: http://39.104.75.125:8080/freeserver/gwc/service/tms/1.0.0/satellite@EPSG:4326@png/"));
			m_bExampleUrl = true;
		}
		else if("XYZ" == strType)
		{
			ui.textEdit_url->setText(tr("eg: http://example.com/{z}/{x}/{y}.png"));
			m_bExampleUrl = true;
		}
		else
		{
			ui.textEdit_url->setText("");
		}
		ui.textEdit_url->setStyleSheet("color: gray; background: rgba(12,22,32,110); border: 1px solid rgb(44,80,114);");
		ui.textEdit_url->blockSignals(false);
	}

	void CTileServiceWidget::SlotUrlClicked()
	{
		if (m_bExampleUrl)
		{
			ui.textEdit_url->clear();
		}
	}

}