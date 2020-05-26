/**************************************************************************************************
* @file SettingWidget.h
* @note 环境设置总管理界面，其中包含了光照、雨、雪、云、海洋等环境的参数设置
* @author c00005
* @data 2017-2-13
**************************************************************************************************/
#ifndef SETING_WIDGET_H
#define SETING_WIDGET_H

#include <mainWindow/FreeDialog.h>

#include <FeShell/EnvironmentSys.h>
#include <FeShell/EnvironmentSerializer.h>
#include <FeKits/weather/WeatherEffectNode.h>
#include <FeSilverlining/SilverliningSys.h>

#include <environment/OceanSetWidget.h>
#include <environment/LightSetWidget.h>
#include <environment/RainSetWidget.h>
#include <environment/SnowSetWidget.h>
#include <environment/FogSetWidget.h>
#include <environment/SilverLiningSetWidget.h>
#include <environment/TimeSetWidget.h>

#include "ui_SettingWidget.h"

namespace FreeViewer
{
    class CEnvironmentWidgetPlugin;
    class CFreeMainWindow;

    class CSettingWidget : public CFreeDialog
	{
		Q_OBJECT

	public:
        CSettingWidget(CEnvironmentWidgetPlugin*pEnvironmentWidget, CFreeMainWindow* pMainWindow);

        ~CSettingWidget();

    public:
        FeShell::CEnvSetData& GetSetData();

		FeShell::CSystemService* GetSystemService();

		FreeViewer::CEnvironmentWidgetPlugin* GetEnvironmentPlugin();

	protected:
        void InitWidget();

        void BuildContext();

        ///本类的事件响应
    protected slots:
        void SlotSelectedListItem(QListWidgetItem*);

        void SlotOKClicked();

        void SlotCancelClicked();

        void SlotResetClicked();
		
    protected:
        Ui::SettingWidget		ui;

		osg::observer_ptr<FeShell::CSystemService>    m_opSystemService;

        CEnvironmentWidgetPlugin*     m_pEnvironmentWidget;

        CFreeMainWindow*        m_pMainWindow;
        CRainSetWidget*         m_pRainSetWidget;
        CSnowSetWidget*         m_pSnowSetWidget;
		CFogSetWidget*            m_pFogSetWidget;
        COceanSetWidget*        m_pOceanSetWidget;
        CSilverLiningSetWidget* m_pSilverliningSetWidget;
        CLightSetWidget*        m_pLightSetWidget;
        CTimeSetWidget*         m_pTimeSetWidget;

		FeShell::CEnvSetData				m_sEvnTempData;

    };
}
#endif // SETING_WIDGET_H
