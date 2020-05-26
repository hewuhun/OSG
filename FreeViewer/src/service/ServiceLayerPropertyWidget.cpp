#include <service/ServiceLayerPropertyWidget.h>
#include <mainWindow/FreeServiceDockTree.h>
#include <mainWindow/FreeUtil.h>

namespace FreeViewer
{
	CServiceLayerPropertyWidget::CServiceLayerPropertyWidget(QWidget *parent)
		: CFreeDialog(parent)
		, m_pItemCurrent(NULL)
		, m_pServiceProvider(NULL)
		, m_pServiceLayer(NULL)
	{
		QWidget *pWidget = new QWidget(this);
		ui.setupUi(pWidget);
		AddWidgetToDialogLayout(pWidget);
		pWidget->setFixedSize(pWidget->geometry().size());

		InitWidget();
	}

	CServiceLayerPropertyWidget::~CServiceLayerPropertyWidget()
	{
		
	}

	void CServiceLayerPropertyWidget::InitWidget()
	{
		// 模态对话框
		this->setWindowModality(Qt::WindowModal);

		// 限制名称
		ui.lineEdit_name->setMaxLength(32);
		ui.lineEdit_name->setValidator(PutInNoEmpty());

		connect(ui.pushButton_ok, SIGNAL(clicked()), this, SLOT(SlotOkBtnClicked()));
		connect(ui.pushButton_cancle, SIGNAL(clicked()), this, SLOT(SlotCancleBtnClicked()));
	}

	void CServiceLayerPropertyWidget::SetItemCurrent( QTreeWidgetItem *pItem )
	{
		Reset();

		m_pItemCurrent = pItem;

		if (m_pItemCurrent)
		{
			EServiceItemType type = (EServiceItemType)m_pItemCurrent->data(0, Qt::UserRole).toInt();
			switch (type)
			{
			case E_TMS_SERVICE_NODE:
			case E_XYZ_SERVICE_NODE:
				{
					// Tile服务图层
					m_pServiceProvider = m_pItemCurrent->data(1, Qt::UserRole).value<FeServiceProvider::ServiceProvider*>();
					if (m_pServiceProvider)
					{
						SetTitleText(tr("Tile Layer Property"));

						QString strName = m_pServiceProvider->GetName().c_str();
						QString strType = m_pServiceProvider->GetServiceType().c_str();
						QString strUrl = m_pServiceProvider->GetUrl().c_str();

						ui.label_type->setText(tr("Sercive Type:"));
						ui.lineEdit_name->setText(strName);
						ui.lineEdit_type->setText(strType);
						ui.lineEdit_url->setText(strUrl);
					}
				}
				break;
			case E_DATA_SERVICE_NODE:
				{
					// 数据集
					m_pServiceProvider = m_pItemCurrent->data(1, Qt::UserRole).value<FeServiceProvider::ServiceProvider*>();
					if (m_pServiceProvider)
					{
						SetTitleText(tr("Data Layer Property"));

						QString strName = m_pServiceProvider->GetName().c_str();
						QString strPluginType = m_pServiceProvider->GetPluginType().c_str();
						QString strUrl = m_pServiceProvider->GetUrl().c_str();

						ui.label_type->setText(tr("DataService Driver:"));
						ui.lineEdit_name->setText(strName);
						ui.lineEdit_type->setText(strPluginType);
						ui.lineEdit_url->setText(strUrl);
					}
				}
				break;
			default:
				break;
			}
		}
	}

	void CServiceLayerPropertyWidget::Reset()
	{
		ui.lineEdit_name->clear();
		ui.lineEdit_type->clear();
		ui.lineEdit_url->clear();

		ui.lineEdit_name->setStyleSheet("background: rgba(12,22,32,110); border: 1px solid rgb(44,80,114);");
	}

	void CServiceLayerPropertyWidget::SlotOkBtnClicked()
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

	void CServiceLayerPropertyWidget::SlotCancleBtnClicked()
	{
		close();
	}

	bool CServiceLayerPropertyWidget::CheckInputInfo()
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