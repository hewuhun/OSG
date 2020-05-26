#include <viewPoint/ViewPointDialog.h>

namespace FreeViewer
{
	CViewPointDialog::CViewPointDialog(FeShell::CSystemService* pSystemService, QWidget* parent)
		: CFreeDialog(parent)
		, m_opSystemService(pSystemService)
	{
		//添加ui界面到属性窗体客户区
		QWidget *widget = new QWidget(this);
		ui.setupUi(widget);
		AddWidgetToDialogLayout(widget);
		widget->setFixedSize(widget->geometry().size());

		InitWidget();
	}

	CViewPointDialog::~CViewPointDialog()
	{

	}

	void CViewPointDialog::InitWidget()
	{
		connect(ui.btn_currentView, SIGNAL(clicked()), this, SLOT(SlotCurrentViewButtonClicked()));
		connect(ui.btn_ok, SIGNAL(clicked()), this, SLOT(SlotOkButtonClicked()));
		connect(ui.btn_cancel, SIGNAL(clicked()), this, SLOT(SlotCancelButtonClicked()));

		if (m_opSystemService.valid())
		{
			FeKit::CManipulatorManager* pManipulatorManager = m_opSystemService->GetManipulatorManager();
			if (pManipulatorManager)
			{
				osgEarth::Viewpoint viewPoint = pManipulatorManager->GetViewPoint();

				ui.dSpinBox_longitude->setValue(viewPoint.focalPoint().value().x());
				ui.dSpinBox_latitude->setValue(viewPoint.focalPoint().value().y());
				ui.dSpinBox_height->setValue(viewPoint.focalPoint().value().z());
				ui.dSpinBox_heading->setValue(viewPoint.getHeading());
				ui.dSpinBox_pitch->setValue(viewPoint.getPitch());
				ui.dSpinBox_flyTime->setValue(3.0);
			}
		}
	}

	bool CViewPointDialog::CheckInputInfo()
	{
		// 判断输入是否为空
		QString redStyle("QLineEdit{border: 2px solid red;}");
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

		if (redStyleFlag)
		{
			return false;
		}

		return true;
	}

	void CViewPointDialog::SlotCurrentViewButtonClicked()
	{
		if (m_opSystemService.valid())
		{
			FeKit::CManipulatorManager* pManipulatorManager = m_opSystemService->GetManipulatorManager();
			if (pManipulatorManager)
			{
				osgEarth::Viewpoint viewPoint = pManipulatorManager->GetViewPoint();

				ui.dSpinBox_longitude->setValue(viewPoint.focalPoint().value().x());
				ui.dSpinBox_latitude->setValue(viewPoint.focalPoint().value().y());
				ui.dSpinBox_height->setValue(viewPoint.focalPoint().value().z());
				ui.dSpinBox_heading->setValue(viewPoint.getHeading());
				ui.dSpinBox_pitch->setValue(viewPoint.getPitch());
			}
		}
	}

	void CViewPointDialog::SlotOkButtonClicked()
	{
		if (!CheckInputInfo())
		{
			return;
		}

		emit SignalAddViewPoint(
			FeUtil::CFreeViewPoint(
			ui.lineEdit_layerName->text().toStdString(),
			ui.dSpinBox_longitude->value(),
			ui.dSpinBox_latitude->value(),
			ui.dSpinBox_height->value(),
			ui.dSpinBox_heading->value(),
			ui.dSpinBox_pitch->value(),
			0.0,
			ui.dSpinBox_flyTime->value()));

		close();
	}

	void CViewPointDialog::SlotCancelButtonClicked()
	{
		close();
	}
}
