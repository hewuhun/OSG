#include <service/DataServiceWidget.h>
#include <mainWindow/FreeUtil.h>

namespace FreeViewer
{
	CDataServiceWidget::CDataServiceWidget(QWidget *parent)
		: CFreeDialog(parent)
		,m_bExampleUrl(true)
	{
		QWidget *pWidget = new QWidget(this);
		ui.setupUi(pWidget);
		AddWidgetToDialogLayout(pWidget);
		pWidget->setFixedSize(pWidget->geometry().size());

		InitWidget();
	}

	CDataServiceWidget::~CDataServiceWidget()
	{
		
	}

	void CDataServiceWidget::InitWidget()
	{
		SetTitleText(tr("Add Data Service"));

		// 模态对话框
		this->setWindowModality(Qt::WindowModal);

		// 限制名称
		ui.lineEdit_name->setMaxLength(32);
		ui.lineEdit_name->setValidator(PutInNoEmpty());

		// 限制网址
		ui.textEdit_url->document()->setMaximumBlockCount(6);

		// 限制驱动
		ui.comboBox_driver->setValidator(PutInNumberAndLetter());

		connect(ui.pushButton_ok, SIGNAL(clicked()), this, SLOT(SlotOkBtnClicked()));
		connect(ui.pushButton_cancle, SIGNAL(clicked()), this, SLOT(SlotCancleBtnClicked()));
		connect(ui.textEdit_url, SIGNAL(textChanged()), this, SLOT(SlotEditTextChanged()));
		connect(ui.comboBox_driver, SIGNAL(currentIndexChanged(QString)), this, SLOT(SlotDriverChanged(QString)));
		connect(ui.textEdit_url, SIGNAL(selectionChanged()), this, SLOT(SlotUrlClicked()));
	}

	void CDataServiceWidget::Reset()
	{
		SlotDriverChanged(ui.comboBox_driver->currentText());

		ui.lineEdit_name->setText(tr("unnamed"));
		ui.comboBox_driver->setCurrentIndex(0);

		ui.lineEdit_name->setStyleSheet("background: rgba(12,22,32,110); border: 1px solid rgb(44,80,114);");
	}

	void CDataServiceWidget::SlotOkBtnClicked()
	{
		if (!CheckInputInfo())
		{
			return;
		}

		close();

		QString strName = ui.lineEdit_name->text();
		QString strUrl = ui.textEdit_url->toPlainText();
		QString strDriver = ui.comboBox_driver->currentText();

		AddData(strName, strUrl, strDriver);

		emit SignalAddServiceFinished();
	}

	void CDataServiceWidget::SlotCancleBtnClicked()
	{
		close();
	}

	void CDataServiceWidget::AddData( QString strName, QString strUrl, QString strDriver )
	{
		FeServiceProvider::LocalDataServiceProvider *pData = new FeServiceProvider::LocalDataServiceProvider(strName.toStdString(), strUrl.toStdString(), strDriver.toStdString());
		SignalAddDataService(pData);
	}

	bool CDataServiceWidget::CheckInputInfo()
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

	void CDataServiceWidget::SlotEditTextChanged()
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

	void CDataServiceWidget::SlotDriverChanged( QString strDriver)
	{
		ui.textEdit_url->blockSignals(true);
		if ("mbtiles" == strDriver)
		{
			ui.textEdit_url->setText(tr("eg: D:/globle.mbtiles"));
			m_bExampleUrl = true;
		}
		else
		{
			ui.textEdit_url->setText("");
		}
		ui.textEdit_url->setStyleSheet("color: gray; background: rgba(12,22,32,110); border: 1px solid rgb(44,80,114);");
		ui.textEdit_url->blockSignals(false);
	}

	void CDataServiceWidget::SlotUrlClicked()
	{
		if (m_bExampleUrl)
		{
			ui.textEdit_url->clear();
		}
	}

}