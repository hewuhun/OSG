#include <service/OGCServicePropertyWidget.h>
#include <mainWindow/FreeServiceDockTree.h>
#include <mainWindow/FreeUtil.h>
#include <FeServiceProvider/ServiceLayer.h>
namespace FreeViewer
{
	COGCServicePropertyWidget::COGCServicePropertyWidget(QWidget *parent)
		: CFreeDialog(parent)
		, m_pItemCurrent(NULL)
		, m_pServiceProvider(NULL)
	{
		QWidget *pWidget = new QWidget(this);
		ui.setupUi(pWidget);
		AddWidgetToDialogLayout(pWidget);
		pWidget->setFixedSize(pWidget->geometry().size());

		InitWidget();
	}

	COGCServicePropertyWidget::~COGCServicePropertyWidget()
	{
		
	}

	void COGCServicePropertyWidget::InitWidget()
	{
		SetTitleText(tr("OGC Service Property"));

		// 模态对话框
		this->setWindowModality(Qt::WindowModal);

		// 限制名称
		ui.lineEdit_name->setMaxLength(32);
		ui.lineEdit_name->setValidator(PutInNoEmpty());

		connect(ui.listWidget_layer, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(SlotLayerListClicked(QListWidgetItem*)));
		connect(ui.pushButton_ok, SIGNAL(clicked()), this, SLOT(SlotOkBtnClicked()));
		connect(ui.pushButton_cancle, SIGNAL(clicked()), this, SLOT(SlotCancleBtnClicked()));
	}

	void COGCServicePropertyWidget::SlotOkBtnClicked()
	{
		if (!CheckInputInfo())
		{
			return;
		}

		close();

		QString strName = ui.lineEdit_name->text();
		if (!strName.isEmpty() && m_pServiceProvider)
		{
			m_pServiceProvider->SetName(strName.toStdString());
			m_pItemCurrent->setText(0, strName);
		}
	}

	void COGCServicePropertyWidget::SlotCancleBtnClicked()
	{
		close();
	}

	void COGCServicePropertyWidget::SetItemCurrent( QTreeWidgetItem *pItem )
	{
		Reset();

		m_pItemCurrent = pItem;

		if (m_pItemCurrent)
		{
			m_pServiceProvider = m_pItemCurrent->data(1, Qt::UserRole).value<FeServiceProvider::ServiceProvider *>();

			if (m_pServiceProvider)
			{
				ui.lineEdit_name->setText(m_pServiceProvider->GetName().c_str());
				ui.lineEdit_type->setText(m_pServiceProvider->GetServiceType().c_str());
				ui.lineEdit_url->setText(m_pServiceProvider->GetUrl().c_str());
				ui.lineEdit_version->setText(m_pServiceProvider->GetCurrentVersion().c_str());
				
				// 显示图层列表
				m_pServiceProvider->GetAttr();
				for (int i = 0; i<m_pServiceProvider->GetLayerCount(); ++i)
				{
					FeServiceProvider::ServiceLayer *pServiceLayer = m_pServiceProvider->GetLayers().at(i);

					QString strLayerName = ConvertToCurrentEncoding(pServiceLayer->GetName());
					QListWidgetItem *pItemLayer = new QListWidgetItem();
					pItemLayer->setText(strLayerName);
					pItemLayer->setIcon(QIcon(":/images/icon/serviceLayer.png"));
					pItemLayer->setData(Qt::UserRole, QVariant::fromValue(pServiceLayer));

					ui.listWidget_layer->addItem(pItemLayer);
				}
			}
		}
	}

	void COGCServicePropertyWidget::Reset()
	{
		ui.lineEdit_name->clear();
		ui.lineEdit_type->clear();
		ui.lineEdit_url->clear();
		ui.lineEdit_version->clear();
		ui.listWidget_layer->clear();
		ui.textEdit_layer->clear();

		ui.lineEdit_name->setStyleSheet("background: rgba(12,22,32,110); border: 1px solid rgb(44,80,114);");
	}

	void COGCServicePropertyWidget::SlotLayerListClicked(QListWidgetItem *item)
	{
		FeServiceProvider::ServiceLayer *pServiceLayer = item->data(Qt::UserRole).value<FeServiceProvider::ServiceLayer*>();
		if (pServiceLayer)
		{
			QString strName = tr("LayerName:") + "\n"+ ConvertToCurrentEncoding(pServiceLayer->GetName()) + "\n" + "\n";
			QString strType = tr("LayerType:") + "\n" + pServiceLayer->GetServiceProvider()->GetServiceType().c_str() + "\n" + "\n";
			QString strUrl = tr("LayerUrl:") + "\n" + ConvertToCurrentEncoding(pServiceLayer->GetServiceProvider()->GetServiceUrl()) + "\n" + "\n";
			QString strVersion = tr("Version:") + "\n" + pServiceLayer->GetServiceProvider()->GetCurrentVersion().c_str() + "\n" + "\n";
			QString strEPSG = tr("EPSG:") + "\n" + pServiceLayer->GetEPSGCode().c_str() + "\n" + "\n";
			QString strCRS = tr("CRS:") + "\n" + pServiceLayer->GetCRSCode().c_str() + "\n" + "\n";
			QString strLowerCorner = tr("LowerCorner:") + "\n" + pServiceLayer->GetLowerCorner().c_str() + "\n" + "\n";
			QString strUpperCorner = tr("UpperCorner:") + "\n" + pServiceLayer->GetUpperCorner().c_str();

			QString strInfo = strName + strType + strUrl + strVersion + strEPSG + strCRS + strLowerCorner + strUpperCorner;
			ui.textEdit_layer->setText(strInfo);
		}
	}

	bool COGCServicePropertyWidget::CheckInputInfo()
	{
		// 判断输入是否为空
		QString emptyStyle("background: rgba(12,22,32,110); border: 1px solid red;");
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

		return bIsEmpty;
	}

}