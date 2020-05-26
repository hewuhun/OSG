#include <mainWindow/FreeServiceButton.h>

namespace FreeViewer
{
	CFreeServiceButton::CFreeServiceButton(QWidget *parent)
		: QWidget(parent)
	{
		ui.setupUi(this);

		InitWidget();
	}

	CFreeServiceButton::~CFreeServiceButton()
	{
		
	}

	void CFreeServiceButton::InitWidget()
	{
		ui.pushButton_update->setToolTip(tr("Update"));
		ui.pushButton_expand->setToolTip(tr("Expand"));
		ui.pushButton_collapse->setToolTip(tr("Collapse"));
		ui.pushButton_delete->setToolTip(tr("Delete"));

		connect(ui.pushButton_update, SIGNAL(clicked()), this, SLOT(SlotUpdateBtnClicked()));
		connect(ui.pushButton_expand, SIGNAL(clicked()), this, SLOT(SlotExpandBtnClicked()));
		connect(ui.pushButton_collapse, SIGNAL(clicked()), this, SLOT(SlotCollapseBtnClicked()));
		connect(ui.pushButton_delete, SIGNAL(clicked()), this, SLOT(SlotDeleteBtnClicked()));
	}

	void CFreeServiceButton::SlotUpdateBtnClicked()
	{
		SignalUpdateBtnClicked(E_SERVICE_UPDATE_BTN);
	}

	void CFreeServiceButton::SlotExpandBtnClicked()
	{
		SignalExpandBtnClicked(E_SERVICE_EXPAND_BTN);
	}

	void CFreeServiceButton::SlotCollapseBtnClicked()
	{
		SignalCollapseBtnClicked(E_SERVICE_COLLAPSE_BTN);
	}

	void CFreeServiceButton::SlotDeleteBtnClicked()
	{
		SignalDeleteBtnClicked(E_SERVICE_DELETE_BTN);
	}

}