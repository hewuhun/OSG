#include <environment/LightSetWidget.h>

#include <environment/SettingWidget.h>

namespace FreeViewer
{
	CLightSetWidget::CLightSetWidget( CSettingWidget* pSetttingWidget )
		:QDialog(pSetttingWidget)
		,m_opFreeSky(NULL)
		,m_pSettingWidget(pSetttingWidget)
	{
		ui.setupUi(this);

		InitWidget();
	}

	CLightSetWidget::~CLightSetWidget()
	{

	}

	void CLightSetWidget::InitWidget()
	{
		if (m_pSettingWidget)
		{
			m_opSystemService = m_pSettingWidget->GetSystemService();
		}

		if (m_opSystemService.valid())
		{
			m_opFreeSky = m_opSystemService->GetEnvironmentSys()->GetSkyNode();
			if(m_opFreeSky.valid())
			{
				BuildContext();
			}
		}
	}

	void CLightSetWidget::BuildContext()
	{
		if (NULL == m_pSettingWidget)
		{
			return ;
		}

		//初始化光照
		ui.sun_checkBox->setChecked(m_pSettingWidget->GetSetData().m_sEnvLightSetData.bSun);
		ui.sun_light_Slider->setMinimum(0);
		ui.sun_light_Slider->setMaximum(255);
		ui.sun_light_Slider->setEnabled(m_pSettingWidget->GetSetData().m_sEnvLightSetData.bSun);
		ui.spinBox_Light->setEnabled(m_pSettingWidget->GetSetData().m_sEnvLightSetData.bSun);
		ui.sun_light_Slider->setValue(m_pSettingWidget->GetSetData().m_sEnvLightSetData.fDensity);
		ui.spinBox_Light->setValue(m_pSettingWidget->GetSetData().m_sEnvLightSetData.fDensity);

		//初始化大气层		
		ui.Atmosphere_checkBox->setChecked(m_pSettingWidget->GetSetData().m_sEnvLightSetData.bAtmosphere);
		//初始化月球
		ui.moon_checkBox->setChecked(m_pSettingWidget->GetSetData().m_sEnvLightSetData.bMoon);
		//初始化星辰
		ui.star_checkBox->setChecked(m_pSettingWidget->GetSetData().m_sEnvLightSetData.bStar);
		//初始化星云背景
		ui.nebula_checkBox->setChecked(m_pSettingWidget->GetSetData().m_sEnvLightSetData.bNebula);

		//建立信号和槽的连接
		connect(ui.sun_checkBox, SIGNAL(toggled(bool)), this, SLOT(SlotSunVisible(bool)));
		connect(ui.sun_light_Slider, SIGNAL(valueChanged(int)),this, SLOT(SlotAmbientChanged(int)));
		connect(ui.spinBox_Light, SIGNAL(valueChanged(int)), this, SLOT(SlotAmbientChanged(int)));

		connect(ui.moon_checkBox, SIGNAL(toggled(bool)), this, SLOT(SlotMoonVisible(bool)));
		connect(ui.Atmosphere_checkBox, SIGNAL(toggled(bool)), this ,SLOT(SlotAtmosphereVisible(bool)));

		connect(ui.star_checkBox, SIGNAL(toggled(bool)), this, SLOT(SlotStarVisible(bool)));
		connect(ui.nebula_checkBox, SIGNAL(toggled(bool)), this, SLOT(SlotNebulaVisible(bool)));
	}

	void CLightSetWidget::Reset( FeShell::SEnvLightSetData data )
	{
		if (false == m_opFreeSky.valid())
		{
			return ;
		}

		///光照设置
		ui.sun_light_Slider->setValue(data.fDensity);
		m_opFreeSky->SetAmbient(osg::Vec4(data.fDensity/255.0,data.fDensity/255.0,data.fDensity/255.0, 1));

		ui.sun_checkBox->setChecked(data.bSun);
		m_opFreeSky->SetSunVisible(data.bSun);

		ui.moon_checkBox->setChecked(data.bMoon);
		m_opFreeSky->SetMoonVisible(data.bMoon);

		ui.Atmosphere_checkBox->setChecked(data.bAtmosphere);
		m_opFreeSky->SetAtmosphereVisible(data.bAtmosphere);

		ui.star_checkBox->setChecked(data.bStar);
		m_opFreeSky->SetStarsVisible(data.bStar);

		ui.nebula_checkBox->setChecked(data.bStar);
		m_opFreeSky->SetNebulaVisible(data.bNebula);
	}

	void CLightSetWidget::SlotAmbientChanged( int currentValue)
	{
		if(m_opFreeSky.valid())
		{
			float fValue = (currentValue)/255.0;

			ui.sun_light_Slider->setValue(currentValue);
			ui.spinBox_Light->setValue(currentValue);
			m_opFreeSky->SetAmbient(osg::Vec4(fValue,fValue,fValue,1.0));
		}

		if (m_pSettingWidget)
		{
			m_pSettingWidget->GetSetData().m_sEnvLightSetData.fDensity = currentValue;
		}
	}

	void CLightSetWidget::SlotSunVisible( bool bShow)
	{
		if(m_opFreeSky.valid())
		{
			m_opFreeSky->SetSunVisible(bShow);
			ui.sun_light_Slider->setEnabled(bShow);
			ui.spinBox_Light->setEnabled(bShow);
			ui.sun_checkBox->setChecked(bShow);
		}

		if (m_pSettingWidget)
		{
			m_pSettingWidget->GetSetData().m_sEnvLightSetData.bSun = bShow;
		}

		emit SignalFromSetSun(bShow);
	}

	void CLightSetWidget::SlotAtmosphereVisible( bool bShow)
	{
		if (m_opFreeSky.valid())
		{
			m_opFreeSky->SetAtmosphereVisible(bShow);
		}

		if (m_pSettingWidget)
		{
			m_pSettingWidget->GetSetData().m_sEnvLightSetData.bAtmosphere = bShow;
		}

		emit SignalFromSetAtmosphere(bShow);
	}

	void CLightSetWidget::SlotMoonVisible( bool bShow )
	{
		if(m_opFreeSky.valid())
		{
			m_opFreeSky->SetMoonVisible(bShow);
		}

		if (m_pSettingWidget)
		{
			m_pSettingWidget->GetSetData().m_sEnvLightSetData.bMoon = bShow;
		}
	}

	void CLightSetWidget::SlotStarVisible( bool bState )
	{
		if (m_opFreeSky.valid())
		{
			m_opFreeSky->SetStarsVisible(bState);
		}

		if (m_pSettingWidget)
		{
			m_pSettingWidget->GetSetData().m_sEnvLightSetData.bStar = bState;
		}
	}

	void CLightSetWidget::SlotNebulaVisible( bool bState )
	{
		if (m_opFreeSky.valid())
		{
			m_opFreeSky->SetNebulaVisible(bState);
		}

		if (m_pSettingWidget)
		{
			m_pSettingWidget->GetSetData().m_sEnvLightSetData.bNebula = bState;
		}
	}

	void CLightSetWidget::SlotSetLight(bool bShow)
	{	
		ui.sun_checkBox->setChecked(bShow);
		ui.sun_light_Slider->setEnabled(bShow);
		ui.spinBox_Light->setEnabled(bShow);
	}

	void CLightSetWidget::SlotSetAtmosphere( bool bShow)
	{
		ui.Atmosphere_checkBox->setChecked(bShow);
	}

}

