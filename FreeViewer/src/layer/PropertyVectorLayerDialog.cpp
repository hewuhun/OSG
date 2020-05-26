#include <layer/PropertyVectorLayerDialog.h>
#include "mainWindow/FreeUtil.h"

namespace FreeViewer
{
	using namespace osgEarth;

	PropertyVectorLayerDialog::PropertyVectorLayerDialog(QWidget *parent, FeLayers::CLayerSys * pCustomLayerSys)
		: CFreeDialog(parent)
		, m_pCustomLayerSys(pCustomLayerSys)
		, m_pLayer(NULL)
		, m_pItem(NULL)
	{
		//添加ui界面到属性窗体客户区
		QWidget *widget = new QWidget(this);
		ui.setupUi(widget);
		//AddSubWidget(widget);
		AddWidgetToDialogLayout(widget);
		widget->setFixedSize(widget->geometry().size());

		
	}

	void PropertyVectorLayerDialog::SlotOk()
	{
		if (!CheckInputInfo())
		{
			return;
		}
		

		close();
	}

	
	bool PropertyVectorLayerDialog::CheckInputInfo()
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

	void PropertyVectorLayerDialog::SlotCancel()
	{
		close();
	}

	PropertyVectorLayerDialog::~PropertyVectorLayerDialog()
	{

	}

	void PropertyVectorLayerDialog::SetLayerAndItem(FeLayers::IFeLayer * pLayer, QTreeWidgetItem * pItem)
	{

	}

}
