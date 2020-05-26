#include <mark/BaseStyleWidget.h>

namespace FreeViewer
{

	CBaseStyleWidget::CBaseStyleWidget( QWidget *parent /*= 0*/ )
		: QWidget(parent)
	{
		this->setWindowFlags(Qt::FramelessWindowHint);
	}

	CBaseStyleWidget::~CBaseStyleWidget()
	{

	}

	bool CBaseStyleWidget::Accept()
	{
		return true;
	}

	bool CBaseStyleWidget::Reject()
	{
		return true;
	}

}