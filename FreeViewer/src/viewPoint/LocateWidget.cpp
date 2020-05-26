#include <viewPoint/LocateWidget.h>

namespace FreeViewer
{
	CLocateWidget::CLocateWidget(QWidget* parent)
		: CFreeDialog(parent)
	{
		//添加ui界面到属性窗体客户区
		QWidget *widget = new QWidget(this);
		ui.setupUi(widget);
		AddWidgetToDialogLayout(widget);
		widget->setFixedSize(widget->geometry().size());

		InitWidget();
	}

	CLocateWidget::~CLocateWidget()
	{

	}

	void CLocateWidget::SetSystemService(FeShell::CSystemService* pSystemService)
	{
		m_opSystemService = pSystemService;
	}

	void CLocateWidget::InitWidget()
	{
		SetTitleText(tr("Locate Widget"));

		//设置初始值
		ui.dSpinBox_longitude->setValue(109.0);
		ui.dSpinBox_latitude->setValue(34.5);
		ui.dSpinBox_height->setValue(416000);
		ui.dSpinBox_flyTime->setValue(5);

 		connect(ui.btn_search, SIGNAL(clicked()), this, SLOT(SlotLocateClicked()));
	}
		
	void CLocateWidget::SlotLocateClicked()
	{
		if (m_opSystemService.valid())
		{
			//获取参数
			double dLon = ui.dSpinBox_longitude->value();
			double dLat = ui.dSpinBox_latitude->value();
			double dHei = ui.dSpinBox_height->value();
			double dFlyTime = ui.dSpinBox_flyTime->value();

			//设置位置
			osgEarth::Viewpoint pViewPoint("", dLon, dLat, dHei, 0.0, -90.0, 0.0);
			m_opSystemService->GetManipulatorManager()->Locate(pViewPoint, dFlyTime);
		}
	}
}