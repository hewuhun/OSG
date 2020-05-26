#include <help/AboutWidget.h>

namespace FreeViewer
{
	CAboutWidget::CAboutWidget( QWidget*parent /*= 0*/ )
		:CFreeDialog(parent)
	{
		QWidget *pWidget= new QWidget(this);
		ui.setupUi(pWidget);
		AddWidgetToDialogLayout(pWidget);	
		SetTitleText(tr("About FreeViewer Desktop"));

		ui.widget_about->setStyleSheet("border-image: url(:/images/help/about.png);");
		ui.widget_logo->setStyleSheet("background:transparent; border-image: url(:/images/help/logo.png);");
		ui.label_text->setText("2.3.0");
	}

	CAboutWidget::~CAboutWidget()
	{

	}
}

