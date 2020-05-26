#include <service/OGCLayerPropertyWidget.h>
#include <mainWindow/FreeServiceDockTree.h>
#include <mainWindow/FreeUtil.h>
#include <FeServiceProvider/ServiceLayer.h>
namespace FreeViewer
{
	COGCLayerPropertyWidget::COGCLayerPropertyWidget(QWidget *parent)
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

	COGCLayerPropertyWidget::~COGCLayerPropertyWidget()
	{
		
	}

	void COGCLayerPropertyWidget::InitWidget()
	{
		SetTitleText(tr("OGC Layer Property"));

		// 模态对话框
		this->setWindowModality(Qt::WindowModal);

		connect(ui.pushButton_ok, SIGNAL(clicked()), this, SLOT(SlotOkBtnClicked()));
		connect(ui.pushButton_cancle, SIGNAL(clicked()), this, SLOT(SlotCancleBtnClicked()));
	}

	void COGCLayerPropertyWidget::SetItemCurrent( QTreeWidgetItem *pItem )
	{
		Reset();

		m_pItemCurrent = pItem;

		if (m_pItemCurrent)
		{
			EServiceItemType type = (EServiceItemType)m_pItemCurrent->data(0, Qt::UserRole).toInt();
			switch (type)
			{
			case E_WMS_SERVICE_NODE:
			case E_WMTS_SERVICE_NODE:
			case E_WFS_SERVICE_NODE:
			case E_WCS_SERVICE_NODE:
				{
					// OGC标准服务图层
					m_pServiceLayer = m_pItemCurrent->data(1, Qt::UserRole).value<FeServiceProvider::ServiceLayer*>();
					if (m_pServiceLayer)
					{
						QString strName = ConvertToCurrentEncoding(m_pServiceLayer->GetName());
						QString strType = m_pServiceLayer->GetServiceProvider()->GetServiceType().c_str();
						QString strUrl = ConvertToCurrentEncoding(m_pServiceLayer->GetServiceProvider()->GetServiceUrl());
						QString strEPSG = tr("EPSG:") + m_pServiceLayer->GetEPSGCode().c_str() + "\n" + "\n";
						QString strCRS = tr("CRS:") + m_pServiceLayer->GetCRSCode().c_str() + "\n" + "\n";
						QString strLowerCorner = tr("LowerCorner:") + m_pServiceLayer->GetLowerCorner().c_str() + "\n" + "\n";
						QString strUpperCorner = tr("UpperCorner:") + m_pServiceLayer->GetUpperCorner().c_str();

						ui.lineEdit_name->setText(strName);
						ui.lineEdit_type->setText(strType);
						ui.lineEdit_url->setText(strUrl);
						QString strInfo = strEPSG + strCRS + strLowerCorner + strUpperCorner;
						ui.textEdit_info->setText(strInfo);
					}
				}
				break;
			case E_TMS_SERVICE_NODE:
			case E_XYZ_SERVICE_NODE:
				{
					ui.label_info->hide();
					ui.textEdit_info->hide();

					// Tile服务图层
					m_pServiceProvider = m_pItemCurrent->data(1, Qt::UserRole).value<FeServiceProvider::ServiceProvider*>();
					if (m_pServiceProvider)
					{
						QString strName = m_pServiceProvider->GetName().c_str();
						QString strType = m_pServiceProvider->GetServiceType().c_str();
						QString strUrl = m_pServiceProvider->GetUrl().c_str();

						ui.lineEdit_name->setText(strName);
						ui.lineEdit_type->setText(strType);
						ui.lineEdit_url->setText(strUrl);
					}
				}
				break;
			case E_DATA_SERVICE_NODE:
				{
					ui.label_info->hide();
					ui.textEdit_info->hide();

					// 数据集
					m_pServiceProvider = m_pItemCurrent->data(1, Qt::UserRole).value<FeServiceProvider::ServiceProvider*>();
					if (m_pServiceProvider)
					{
						QString strName = m_pServiceProvider->GetName().c_str();
						QString strPluginType = m_pServiceProvider->GetPluginType().c_str();
						QString strUrl = m_pServiceProvider->GetUrl().c_str();

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

	void COGCLayerPropertyWidget::Reset()
	{
		ui.label_info->show();
		ui.textEdit_info->show();

		ui.lineEdit_name->clear();
		ui.lineEdit_type->clear();
		ui.lineEdit_url->clear();
		ui.textEdit_info->clear();
	}

	void COGCLayerPropertyWidget::SlotOkBtnClicked()
	{
		close();
	}

	void COGCLayerPropertyWidget::SlotCancleBtnClicked()
	{
		close();
	}

}