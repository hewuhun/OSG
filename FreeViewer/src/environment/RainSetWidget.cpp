#include <environment/RainSetWidget.h>

#include <environment/SettingWidget.h>

namespace FreeViewer
{
	CRainSetWidget::CRainSetWidget( CSettingWidget* pSetttingWidget )
		:QDialog(pSetttingWidget)
		,m_pSettingWidget(pSetttingWidget)
		,m_opRainNode(NULL)
	{
		ui.setupUi(this);

		InitWidget();
	}

	CRainSetWidget::~CRainSetWidget()
	{

	}

	void CRainSetWidget::InitWidget()
	{
		if (m_pSettingWidget)
		{
			m_opSystemService = m_pSettingWidget->GetSystemService();
		}

		if (m_opSystemService.valid())
		{
			m_opRainNode = dynamic_cast<FeKit::CWeatherEffectNode*>(m_opSystemService->GetEnvironmentSys()->GetSkyTool("Rain"));
			if (m_opRainNode.valid())
			{
				BuildContext();
			}
		}
	}

	void CRainSetWidget::BuildContext()
	{
		if (NULL == m_pSettingWidget)
		{
			return ;
		}

		//设置雨效果的属性设置,雨效果属性与界面的同步
		if (m_opRainNode.valid())
		{
			ui.checkBox_Rain->setChecked(m_pSettingWidget->GetSetData().m_sEnvRainSetData.bRain);

			ui.doubleSpinBox_RainHeight->setValue(m_pSettingWidget->GetSetData().m_sEnvRainSetData.fheigth);
			ui.doubleSpinBox_RainLongitude->setValue(m_pSettingWidget->GetSetData().m_sEnvRainSetData.fLongitude);
			ui.doubleSpinBox_RainLatitude->setValue(m_pSettingWidget->GetSetData().m_sEnvRainSetData.fLatitude);
			ui.doubleSpinBox_RainRadius->setValue(m_pSettingWidget->GetSetData().m_sEnvRainSetData.fRadius);

			//设置初始的雨的浓度为0.5并同步到界面
			ui.hSlider_RainIntensity->setValue(m_pSettingWidget->GetSetData().m_sEnvRainSetData.dDensity*10);
			ui.rain_density_doubleSpinBox->setValue(m_pSettingWidget->GetSetData().m_sEnvRainSetData.dDensity);

			UpdateWidgetState();
		}

		//初始化雨效果的信号与槽的连接
		{
			connect(ui.checkBox_Rain, SIGNAL(toggled(bool)), this, SLOT(SlotRainVisible(bool)));

			connect(ui.hSlider_RainIntensity, SIGNAL(valueChanged(int)),
				this, SLOT(SlotRainIntensity(int)));

			connect(ui.rain_density_doubleSpinBox, SIGNAL(valueChanged(double)),
				this, SLOT(SlotRainIntensity(double)));

			connect(ui.doubleSpinBox_RainHeight, SIGNAL(valueChanged(double)),
				this, SLOT(SlotRainHeight(double)));

			connect(ui.doubleSpinBox_RainLongitude, SIGNAL(valueChanged(double)),
				this, SLOT(SlotRainPosition(double)));

			connect(ui.doubleSpinBox_RainLatitude, SIGNAL(valueChanged(double)),
				this, SLOT(SlotRainPosition(double)));

			connect(ui.doubleSpinBox_RainRadius, SIGNAL(valueChanged(double)),
				this, SLOT(SlotRainPosition(double)));

			connect(ui.raduiBtn_GlobalRain, SIGNAL(toggled(bool)),
				this, SLOT(SlotGlobalRain(bool)));

			connect(ui.raduiBtn_LocalRain, SIGNAL(toggled(bool)),
				this, SLOT(SlotLocalRain(bool)));

			connect(ui.toolBtn_Locate, SIGNAL(clicked()), this, SLOT(SlotLocation()));
		}
	}

	void CRainSetWidget::UpdateWidgetState()
	{
		if(m_pSettingWidget->GetSetData().m_sEnvRainSetData.bRain)
		{
			ui.doubleSpinBox_RainHeight->setEnabled(true);
			ui.hSlider_RainIntensity->setEnabled(true);
			ui.rain_density_doubleSpinBox->setEnabled(true);
		}
		else
		{
			ui.doubleSpinBox_RainHeight->setEnabled(false);
			ui.hSlider_RainIntensity->setEnabled(false);
			ui.rain_density_doubleSpinBox->setEnabled(false);
		}

		if (m_pSettingWidget->GetSetData().m_sEnvRainSetData.bGlobal)
		{
			ui.raduiBtn_GlobalRain->setChecked(true);
			ui.raduiBtn_LocalRain->setChecked(false);
			ui.label_Longitude->setEnabled(false);
			ui.label_Latitude->setEnabled(false);
			ui.label_Radius->setEnabled(false);
			ui.doubleSpinBox_RainLatitude->setEnabled(false);
			ui.doubleSpinBox_RainLongitude->setEnabled(false);
			ui.doubleSpinBox_RainRadius->setEnabled(false);
			ui.toolBtn_Locate->setEnabled(false);
		}
		else
		{
			ui.raduiBtn_GlobalRain->setChecked(false);
			ui.raduiBtn_LocalRain->setChecked(true);
			ui.label_Longitude->setEnabled(true);
			ui.label_Latitude->setEnabled(true);
			ui.label_Radius->setEnabled(true);
			ui.doubleSpinBox_RainLatitude->setEnabled(true);
			ui.doubleSpinBox_RainLongitude->setEnabled(true);
			ui.doubleSpinBox_RainRadius->setEnabled(true);
			ui.toolBtn_Locate->setEnabled(true);
		}
	}

	void CRainSetWidget::SlotSetRain(bool bShow)
	{
		ui.checkBox_Rain->setChecked(bShow);
	}

	void CRainSetWidget::SlotRainVisible(bool bRain)
	{
		if (m_opRainNode.valid())
		{
			if (bRain)
			{
				m_opRainNode->Show();
			}
			else
			{
				m_opRainNode->Hide();
			}

			m_pSettingWidget->GetSetData().m_sEnvRainSetData.bRain = bRain;
			UpdateWidgetState();
		}

		emit SignalFromSetRain(bRain);
	}

	void CRainSetWidget::SlotRainIntensity( int nValue )
	{
		double dRainDensity = (nValue * 1.0)/10;
		ui.rain_density_doubleSpinBox->setValue(dRainDensity);

		if (m_opRainNode.valid())
		{
			m_opRainNode->rain(dRainDensity);
		}

		if (m_pSettingWidget)
		{
			m_pSettingWidget->GetSetData().m_sEnvRainSetData.dDensity = dRainDensity;
		}
	}

	void CRainSetWidget::SlotRainIntensity( double dValue )
	{
		if (m_opRainNode.valid())
		{
			m_opRainNode->rain(dValue);
		}

		ui.hSlider_RainIntensity->setValue(dValue * 10);

		if (m_pSettingWidget)
		{
			m_pSettingWidget->GetSetData().m_sEnvRainSetData.dDensity = dValue;
		}
	}

	void CRainSetWidget::SlotRainHeight( double dHei )
	{
		if (m_opRainNode.valid())
		{
			m_opRainNode->SetWeatherHeight(dHei);
		}

		if (m_pSettingWidget)
		{
			m_pSettingWidget->GetSetData().m_sEnvRainSetData.fheigth = dHei;
		}
	}

	void CRainSetWidget::SlotGlobalRain( bool bGlobal )
	{
		if (m_opRainNode.valid())
		{
			m_opRainNode->SetWorld(true);
		}

		ui.label_Longitude->setEnabled(false);
		ui.label_Latitude->setEnabled(false);
		ui.label_Radius->setEnabled(false);
		ui.doubleSpinBox_RainLatitude->setEnabled(false);
		ui.doubleSpinBox_RainLongitude->setEnabled(false);
		ui.doubleSpinBox_RainRadius->setEnabled(false);
		ui.toolBtn_Locate->setEnabled(false);

		if (m_pSettingWidget)
		{
			m_pSettingWidget->GetSetData().m_sEnvRainSetData.bGlobal = true;
		}
	}

	void CRainSetWidget::SlotLocalRain( bool bLocal )
	{
		ui.label_Longitude->setEnabled(true);
		ui.label_Latitude->setEnabled(true);
		ui.label_Radius->setEnabled(true);
		ui.doubleSpinBox_RainLatitude->setEnabled(true);
		ui.doubleSpinBox_RainLongitude->setEnabled(true);
		ui.doubleSpinBox_RainRadius->setEnabled(true);
		ui.toolBtn_Locate->setEnabled(true);

		osg::Vec3d vecPos = osg::Vec3d(
			ui.doubleSpinBox_RainLongitude->value(),
			ui.doubleSpinBox_RainLatitude->value(),
			ui.doubleSpinBox_RainRadius->value());

		if (m_opRainNode.valid())
		{
			m_opRainNode->SetWeatherPosition(vecPos, true);
		}

		if (m_pSettingWidget)
		{
			m_pSettingWidget->GetSetData().m_sEnvRainSetData.bGlobal = false;
		}
	}

	void CRainSetWidget::SlotRainPosition( double )
	{
		osg::Vec3d vecPos = osg::Vec3d(
			ui.doubleSpinBox_RainLongitude->value(),
			ui.doubleSpinBox_RainLatitude->value(),
			ui.doubleSpinBox_RainRadius->value());

		if (m_opRainNode.valid())
		{
			m_opRainNode->SetWeatherPosition(vecPos, true);
		}

		if (m_pSettingWidget)
		{
			m_pSettingWidget->GetSetData().m_sEnvRainSetData.fLongitude = vecPos.x();
			m_pSettingWidget->GetSetData().m_sEnvRainSetData.fLatitude = vecPos.y();
			m_pSettingWidget->GetSetData().m_sEnvRainSetData.fRadius = vecPos.z();
		}
	}

	void CRainSetWidget::SlotLocation()
	{
		if (m_opSystemService.valid())
		{
			//将视点高度设置为雨高度的四分之一
			osgEarth::Viewpoint vp(
				"",
				ui.doubleSpinBox_RainLongitude->value(),
				ui.doubleSpinBox_RainLatitude->value(),
				ui.doubleSpinBox_RainHeight->value()/4.0,
				0.0,
				0.0,
				ui.doubleSpinBox_RainHeight->value()/4.0);

			m_opSystemService->GetManipulatorManager()->Locate(vp, 3.0);
		}
	}

	void CRainSetWidget::Reset( FeShell::SEnvRainSetData data )
	{
		if (m_opRainNode.valid())
		{
			//显隐
			ui.checkBox_Rain->setChecked(data.bRain);

			///透明度
			ui.hSlider_RainIntensity->setValue(data.dDensity*10.0);
			ui.rain_density_doubleSpinBox->setValue(data.dDensity);
			m_opRainNode->rain(data.dDensity);

			//高度
			ui.doubleSpinBox_RainHeight->setValue(data.fheigth);
			m_opRainNode->SetWeatherHeight(data.fheigth);

			///位置
			ui.doubleSpinBox_RainLatitude->setValue(data.fLatitude);
			ui.doubleSpinBox_RainLongitude->setValue(data.fLongitude);
			ui.doubleSpinBox_RainRadius->setValue(data.fRadius);
			m_opRainNode->SetWeatherPosition(osg::Vec3d(data.fLatitude, data.fLongitude, data.fRadius), !data.bGlobal);

			///全球，本地
			ui.raduiBtn_GlobalRain->setChecked(data.bGlobal);
			m_opRainNode->SetWorld(data.bGlobal);
			ui.raduiBtn_LocalRain->setChecked(!data.bGlobal);
		}
	}
}
