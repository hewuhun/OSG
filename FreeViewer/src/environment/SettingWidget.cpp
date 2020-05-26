#include <environment/SettingWidget.h>

#include <environment/EnvironmentPluginInterface.h>
#include <mainWindow/FreeMainWindow.h>

#include <FeUtils/PathRegistry.h>
#include <FeShell/SmartToolSys.h>
#include <FeOcean/OceanSys.h>

#include <QSplitter>
#include <QListWidgetItem>

namespace FreeViewer
{
    CSettingWidget::CSettingWidget(CEnvironmentWidgetPlugin *pEnvironmentWidget, CFreeMainWindow *pMainWindow)
        : CFreeDialog(pMainWindow)
        , m_pEnvironmentWidget(pEnvironmentWidget)
        , m_pMainWindow(pMainWindow)
        , m_pLightSetWidget(NULL)
        , m_pRainSetWidget(NULL)
        , m_pSnowSetWidget(NULL)
		,m_pFogSetWidget(NULL)
        , m_pOceanSetWidget(NULL)
        , m_pSilverliningSetWidget(NULL)
        , m_pTimeSetWidget(NULL)
    {
        QWidget *widget = new QWidget(this);
        ui.setupUi(widget);
        AddWidgetToDialogLayout(widget);
        widget->setFixedSize(widget->geometry().size());

		ui.listWidget->setStyleSheet("border: none;");

        InitWidget();
    }

    CSettingWidget::~CSettingWidget()
    {

    }

    FeShell::CEnvSetData& CSettingWidget::GetSetData()
    {
		return m_sEvnTempData;
    }

	FeShell::CSystemService* CSettingWidget::GetSystemService()
	{
		return m_opSystemService.get();
	}

    void CSettingWidget::InitWidget()
    {
        SetTitleText(tr("Environment Setting"));

        if(m_pEnvironmentWidget)
        {
            m_opSystemService = m_pEnvironmentWidget->GetSystemService();
			m_sEvnTempData = m_pEnvironmentWidget->GetSetData();
        }

        if(false == m_opSystemService.valid())
        {
            return ;
        }

        if (!m_pLightSetWidget)
        {
            m_pLightSetWidget = new CLightSetWidget(this);
            ui.stackedWidget->addWidget(m_pLightSetWidget);

            QListWidgetItem* pLightItem = new QListWidgetItem(QString(tr("Light")), ui.listWidget);
            pLightItem->setData(1, QVariant::fromValue<void*>(m_pLightSetWidget));
        }

		if (!m_pRainSetWidget)
		{
			m_pRainSetWidget = new CRainSetWidget(this);
			ui.stackedWidget->addWidget(m_pRainSetWidget);

			QListWidgetItem* pRainItem = new QListWidgetItem(QString(tr("Rain")), ui.listWidget);
			pRainItem->setData(1, QVariant::fromValue<void*>(m_pRainSetWidget));
		}

		if (!m_pSnowSetWidget)
		{
			m_pSnowSetWidget = new CSnowSetWidget(this);
			ui.stackedWidget->addWidget(m_pSnowSetWidget);

			QListWidgetItem* pSnowItem = new QListWidgetItem(QString(tr("Snow")), ui.listWidget);
			pSnowItem->setData(1, QVariant::fromValue<void*>(m_pSnowSetWidget));
		}

		if (!m_pFogSetWidget)
		{
			m_pFogSetWidget = new CFogSetWidget(this);
			ui.stackedWidget->addWidget(m_pFogSetWidget);

			QListWidgetItem* pFogItem = new QListWidgetItem(QString(tr("Fog")), ui.listWidget);
			pFogItem->setData(1, QVariant::fromValue<void*>(m_pFogSetWidget));
		}

 		//if (!m_pSilverliningSetWidget)
 		//{
 		//	m_pSilverliningSetWidget = new CSilverLiningSetWidget(this);
 		//	ui.stackedWidget->addWidget(m_pSilverliningSetWidget);
 
 		//	QListWidgetItem* pCloudItem = new QListWidgetItem(QString(tr("Cloud")), ui.listWidget);
 		//	pCloudItem->setData(1, QVariant::fromValue<void*>(m_pSilverliningSetWidget));
 		//}

		if (!m_pOceanSetWidget)
		{
			m_pOceanSetWidget = new COceanSetWidget(this);
			ui.stackedWidget->addWidget(m_pOceanSetWidget);

			QListWidgetItem* pOceanItem = new QListWidgetItem(QString(tr("Ocean")), ui.listWidget);
			pOceanItem->setData(1, QVariant::fromValue<void*>(m_pOceanSetWidget));
		}

		if (!m_pTimeSetWidget)
		{
			m_pTimeSetWidget = new CTimeSetWidget(this);
			ui.stackedWidget->addWidget(m_pTimeSetWidget);

			QListWidgetItem* pTimeSetItem = new QListWidgetItem(tr("Time"), ui.listWidget);
			pTimeSetItem->setData(1, QVariant::fromValue<void*>(m_pTimeSetWidget));
		}

        BuildContext();
    }

    void CSettingWidget::BuildContext()
    {
        connect(ui.listWidget,SIGNAL(itemClicked(QListWidgetItem*)),this, SLOT(SlotSelectedListItem(QListWidgetItem*)));
        connect(ui.pBtnOK, SIGNAL(clicked()), this, SLOT(SlotOKClicked()));
        connect(ui.pBtnCancel, SIGNAL(clicked()), this, SLOT(SlotCancelClicked()));
        connect(ui.pBtnReset, SIGNAL(clicked()), this, SLOT(SlotResetClicked()));

        //向下传递信号，当菜单栏中的状态值发生变化时，通知到设置窗口中
        connect(m_pEnvironmentWidget, SIGNAL(SignalToSetLigth(bool)), m_pLightSetWidget, SLOT(SlotSetLight(bool)));
        connect(m_pEnvironmentWidget, SIGNAL(SignalToSetAtmosphere(bool)), m_pLightSetWidget, SLOT(SlotSetAtmosphere(bool)));
		connect(m_pEnvironmentWidget, SIGNAL(SignalToSetRain(bool)), m_pRainSetWidget, SLOT(SlotSetRain(bool)));
		connect(m_pEnvironmentWidget, SIGNAL(SignalToSetSnow(bool)), m_pSnowSetWidget, SLOT(SlotSetSnow(bool)));
        //connect(m_pEnvironmentWidget, SIGNAL(SignalToSetCloud(bool)), m_pSilverliningSetWidget, SLOT(SlotSetCloud(bool)));
        connect(m_pEnvironmentWidget, SIGNAL(SignalToSetOcean(bool)), m_pOceanSetWidget, SLOT(SlotSetOcean(bool)));
		connect(m_pEnvironmentWidget, SIGNAL(SignalToSetFog(bool)), m_pFogSetWidget, SLOT(SlotSetFog(bool)));

        //向上传递信号，当设置窗口中的状态值发生变化时，通知到菜单栏中
        connect(m_pLightSetWidget, SIGNAL(SignalFromSetSun(bool)), m_pEnvironmentWidget, SLOT(SlotFromSetLight(bool)));
        connect(m_pLightSetWidget, SIGNAL(SignalFromSetAtmosphere(bool)), m_pEnvironmentWidget, SLOT(SlotFromSetAtmosphere(bool)));
		connect(m_pRainSetWidget, SIGNAL(SignalFromSetRain(bool)), m_pEnvironmentWidget, SLOT(SlotFromSetRain(bool)));
		connect(m_pSnowSetWidget, SIGNAL(SignalFromSetSnow(bool)), m_pEnvironmentWidget, SLOT(SlotFromSetSnow(bool)));
        //connect(m_pSilverliningSetWidget, SIGNAL(SignalFormCloud(bool)), m_pEnvironmentWidget, SLOT(SlotFromSetCloud(bool)));
        connect(m_pOceanSetWidget, SIGNAL(SignalFromOcean(bool)), m_pEnvironmentWidget, SLOT(SlotFromSetOcean(bool)));
		connect(m_pFogSetWidget, SIGNAL(SignalFromSetFog(bool)), m_pEnvironmentWidget, SLOT(SlotFormSetFog(bool)));

	}

    void CSettingWidget::SlotSelectedListItem(QListWidgetItem *pItem)
    {
        if(pItem)
        {
			int nCount = ui.stackedWidget->count();
			int nIndex = ui.listWidget->currentRow();

			// 当需要显示的页面索引大于等于总页面时，切换至首页
			if (nIndex >= nCount)
			{
				nIndex = 0;
			}

			QWidget* pWidget = (QWidget*)(pItem->data(1).value<void*>());

            CLightSetWidget* pLightWidget = dynamic_cast<CLightSetWidget*>(pWidget);
            if(pLightWidget)
            {
                pLightWidget->show();
            }

			CRainSetWidget* pRainSetWidget = dynamic_cast<CRainSetWidget*>(pWidget);
            if(pRainSetWidget)
            {
                pRainSetWidget->show();
            }

			CFogSetWidget* pFogSetWidget = dynamic_cast<CFogSetWidget*>(pWidget);
			if(pFogSetWidget)
			{
				pFogSetWidget->show();
			}

			CSnowSetWidget* pSnowSetWidget = dynamic_cast<CSnowSetWidget*>(pWidget);
			if(pSnowSetWidget)
			{
				pSnowSetWidget->show();
			}

			COceanSetWidget* pOceanSetWidget = dynamic_cast<COceanSetWidget*>(pWidget);
			if(pOceanSetWidget)
			{
				pOceanSetWidget->show();
			}

			CSilverLiningSetWidget* pSilverLiningSetWidget = dynamic_cast<CSilverLiningSetWidget*>(pWidget);
            if(pSilverLiningSetWidget)
            {
                pSilverLiningSetWidget->show();
            }

			CTimeSetWidget* pTimeSetWidget = dynamic_cast<CTimeSetWidget*>(pWidget);
			if(pTimeSetWidget)
			{
				pTimeSetWidget->show();
			}

			ui.stackedWidget->setCurrentIndex(nIndex);
        }
    }

    void CSettingWidget::SlotOKClicked()
	{
        //保存环境配置数据到配置文件
        if (m_pEnvironmentWidget)
        {
			m_pEnvironmentWidget->GetSetData() = m_sEvnTempData;
			m_pEnvironmentWidget->SaveData();
        }

		this->close();
    }

    void CSettingWidget::SlotCancelClicked()
	{
		if (NULL == m_pEnvironmentWidget)
		{
			return ;
		}

		if(m_pLightSetWidget)
		{
			m_pLightSetWidget->Reset(m_pEnvironmentWidget->GetSetData().m_sEnvLightSetData);
		}

		if(m_pRainSetWidget)
		{
			m_pRainSetWidget->Reset(m_pEnvironmentWidget->GetSetData().m_sEnvRainSetData);
		}

		if(m_pSnowSetWidget)
		{
			m_pSnowSetWidget->Reset(m_pEnvironmentWidget->GetSetData().m_sEnvSnowSetData);
		}

// 		if(m_pSilverliningSetWidget)
// 		{
// 			m_pSilverliningSetWidget->Reset(m_pEnvironmentWidget->GetSetData().m_sEnvCloudSetData);
// 		}
		if(m_pFogSetWidget)
		{
			m_pFogSetWidget->Reset(m_pEnvironmentWidget->GetSetData().m_sEnvFogSetData);
		}

		if(m_pOceanSetWidget)
		{
			m_pOceanSetWidget->Reset(m_pEnvironmentWidget->GetSetData().m_sEnvOceanSetData);
		}

		if(m_pTimeSetWidget)
		{
			m_pTimeSetWidget->Reset();
		}

        this->close();
	}


    void CSettingWidget::SlotResetClicked()
	{
		if (NULL == m_pEnvironmentWidget)
		{
			return ;
		}

		m_pEnvironmentWidget->LoadResetData();
		if(m_pLightSetWidget)
		{
			m_pLightSetWidget->Reset(m_pEnvironmentWidget->GetSetData().m_sEnvLightSetData);
		}

		if(m_pRainSetWidget)
		{
			m_pRainSetWidget->Reset(m_pEnvironmentWidget->GetSetData().m_sEnvRainSetData);
		}

		if(m_pSnowSetWidget)
		{
			m_pSnowSetWidget->Reset(m_pEnvironmentWidget->GetSetData().m_sEnvSnowSetData);
		}

// 		if(m_pSilverliningSetWidget)
// 		{
// 			m_pSilverliningSetWidget->Reset(m_pEnvironmentWidget->GetSetData().m_sEnvCloudSetData);
// 		}
		if(m_pFogSetWidget)
		{
			m_pFogSetWidget->Reset(m_pEnvironmentWidget->GetSetData().m_sEnvFogSetData);
		}

		if(m_pOceanSetWidget)
		{
			m_pOceanSetWidget->Reset(m_pEnvironmentWidget->GetSetData().m_sEnvOceanSetData);
		}

		if(m_pTimeSetWidget)
		{
			m_pTimeSetWidget->Reset();
		}
	}

	FreeViewer::CEnvironmentWidgetPlugin* CSettingWidget::GetEnvironmentPlugin()
	{
		if (m_pEnvironmentWidget != NULL)
		{
			return m_pEnvironmentWidget;
		}
	}

}

