#include <mainWindow/FreeDockWidget.h>

#include <QScrollBar>
#include <iostream>


namespace FreeViewer
{
	CFreeDockWidget::CFreeDockWidget(QString titleText, QWidget *parent)
		: QWidget(parent)
	{
		ui.setupUi(this);

		ui.label_titleText->setText(titleText);
		ui.label_titleText->setMouseTracking(true);
		ui.label_titleText->installEventFilter(parent);

		InitData();
	}

	void CFreeDockWidget::InitData()
	{
		this->setStyleSheet("QWidget{ border: 1px solid rgb(45,79,114);}");

		//关联标题栏隐藏按钮信号和槽
		connect(ui.btn_titleBarHide, SIGNAL(clicked(bool)), this, SLOT(SlotHideBtnClicked(bool)));
	}

	void CFreeDockWidget::SlotHideBtnClicked(bool)
	{
		this->hide();

		QString strTitle = ui.label_titleText->text();
		emit SignalSendTitleText(strTitle);
	}

	void CFreeDockWidget::AddSubWidget( QWidget *pWidget )
	{
		if (pWidget != NULL)
		{
			ui.verticalLayout_Widget->addWidget(pWidget);
		}
	}

}
