#include <environment/SnowSetWidget.h>

#include <environment/SettingWidget.h>

namespace FreeViewer
{
	CSnowSetWidget::CSnowSetWidget( CSettingWidget* pSetttingWidget )
		: QDialog(pSetttingWidget)
		,m_pSettingWidget(pSetttingWidget)
		,m_opSnowNode(NULL)
	{
		ui.setupUi(this);

		InitWidget();
	}

	CSnowSetWidget::~CSnowSetWidget()
	{

	}

	void CSnowSetWidget::InitWidget()
	{
		if (m_pSettingWidget)
		{
			m_opSystemService = m_pSettingWidget->GetSystemService();
		}

		if (m_opSystemService.valid())
		{
			m_opSnowNode = dynamic_cast<FeKit::CWeatherEffectNode*>(m_opSystemService->GetEnvironmentSys()->GetSkyTool("Snow"));
			if (m_opSnowNode.valid())
			{
				BuildContext();
			}
		}
	}

	void CSnowSetWidget::BuildContext()
	{
		if (NULL == m_pSettingWidget)
		{
			return ;
		}

		//设置雪效果的属性设置,雪效果属性与界面的同步
		if (m_opSnowNode.valid())
		{
			ui.checkBox_Snow->setChecked(m_pSettingWidget->GetSetData().m_sEnvSnowSetData.bSnow);

			ui.doubleSpinBox_SnowLongitude->setValue(m_pSettingWidget->GetSetData().m_sEnvSnowSetData.fLongitude);
			ui.doubleSpinBox_SnowLatitude->setValue(m_pSettingWidget->GetSetData().m_sEnvSnowSetData.fLatitude);
			ui.doubleSpinBox_SnowRadius->setValue(m_pSettingWidget->GetSetData().m_sEnvSnowSetData.fRadius);
			ui.doubleSpinBox_SnowHeight->setValue(m_pSettingWidget->GetSetData().m_sEnvSnowSetData.fheigth);

			//设置初始的雪的浓度为0.5并同步到界面
			ui.hSlider_SnowIntensity->setValue(m_pSettingWidget->GetSetData().m_sEnvSnowSetData.dDensity*10.0);
			ui.snow_density_doubleSpinBox->setValue(m_pSettingWidget->GetSetData().m_sEnvSnowSetData.dDensity);

			UpdateWidgetState();
		}

		//初始化雪效果的信号与槽的连接
		{
			connect(ui.checkBox_Snow, SIGNAL(toggled(bool)), this, SLOT(SlotSnowVisible(bool)));

			connect(ui.hSlider_SnowIntensity, SIGNAL(valueChanged(int)),
				this, SLOT(SlotSnowIntensity(int)));

			connect(ui.snow_density_doubleSpinBox, SIGNAL(valueChanged(double)),
				this, SLOT(SlotSnowIntensity(double)));

			connect(ui.doubleSpinBox_SnowHeight, SIGNAL(valueChanged(double)),
				this, SLOT(SlotSnowHeight(double)));

			connect(ui.doubleSpinBox_SnowLongitude, SIGNAL(valueChanged(double)),
				this, SLOT(SlotSnowPosition(double)));

			connect(ui.doubleSpinBox_SnowLatitude, SIGNAL(valueChanged(double)),
				this, SLOT(SlotSnowPosition(double)));

			connect(ui.doubleSpinBox_SnowRadius, SIGNAL(valueChanged(double)),
				this, SLOT(SlotSnowPosition(double)));

			connect(ui.radioBtn_GlobalSnow, SIGNAL(toggled(bool)),
				this, SLOT(SlotGlobalSnow(bool)));

			connect(ui.radioBtn_LocalSnow, SIGNAL(toggled(bool)),
				this, SLOT(SlotLocalSnow(bool)));

			connect(ui.toolBtn_Locate, SIGNAL(clicked()), this, SLOT(SlotLocation()));
		}
	}

	void CSnowSetWidget::UpdateWidgetState()
	{
		if(m_pSettingWidget->GetSetData().m_sEnvSnowSetData.bSnow)
		{
			ui.hSlider_SnowIntensity->setEnabled(true);
			ui.snow_density_doubleSpinBox->setEnabled(true);
			ui.doubleSpinBox_SnowHeight->setEnabled(true);
		}
		else
		{
			ui.hSlider_SnowIntensity->setEnabled(false);
			ui.snow_density_doubleSpinBox->setEnabled(false);
			ui.doubleSpinBox_SnowHeight->setEnabled(false);
		}

		if (m_pSettingWidget->GetSetData().m_sEnvSnowSetData.bGlobal)
		{
			ui.radioBtn_GlobalSnow->setChecked(true);
			ui.radioBtn_LocalSnow->setChecked(false);
			ui.label_Longitude->setEnabled(false);
			ui.label_Latitude->setEnabled(false);
			ui.label_Radius->setEnabled(false);
			ui.doubleSpinBox_SnowLatitude->setEnabled(false);
			ui.doubleSpinBox_SnowLongitude->setEnabled(false);
			ui.doubleSpinBox_SnowRadius->setEnabled(false);
			ui.toolBtn_Locate->setEnabled(false);
		}
		else
		{
			ui.radioBtn_GlobalSnow->setChecked(false);
			ui.radioBtn_LocalSnow->setChecked(true);
			ui.label_Longitude->setEnabled(true);
			ui.label_Latitude->setEnabled(true);
			ui.label_Radius->setEnabled(true);
			ui.doubleSpinBox_SnowLatitude->setEnabled(true);
			ui.doubleSpinBox_SnowLongitude->setEnabled(true);
			ui.doubleSpinBox_SnowRadius->setEnabled(true);
			ui.toolBtn_Locate->setEnabled(true);
		}
	}

	void CSnowSetWidget::SlotSetSnow(bool bShow)
	{
		ui.checkBox_Snow->setChecked(bShow);
	}

	void CSnowSetWidget::SlotSnowVisible(bool bSnow)
	{
		if (m_opSnowNode.valid())
		{
			if (bSnow)
			{
				m_opSnowNode->Show();
			}
			else
			{
				m_opSnowNode->Hide();
			}

			m_pSettingWidget->GetSetData().m_sEnvSnowSetData.bSnow = bSnow;
			UpdateWidgetState();
		}

		emit SignalFromSetSnow(bSnow);
	}

	void CSnowSetWidget::SlotSnowIntensity( int nValue )
	{
		double dSnowIntensity =  (nValue * 1.0)/10.0;
		ui.snow_density_doubleSpinBox->setValue(dSnowIntensity);

		if (m_opSnowNode.valid())
		{
			m_opSnowNode->snow(dSnowIntensity);
		}

		if (m_pSettingWidget)
		{
			m_pSettingWidget->GetSetData().m_sEnvSnowSetData.dDensity = nValue;
		}
	}

	void CSnowSetWidget::SlotSnowIntensity( double dValue )
	{
		if (m_opSnowNode.valid())
		{
			m_opSnowNode->snow(dValue);
		}

		ui.hSlider_SnowIntensity->setValue(dValue*10.0);

		if (m_pSettingWidget)
		{
			m_pSettingWidget->GetSetData().m_sEnvSnowSetData.dDensity = dValue;
		}
	}

	void CSnowSetWidget::SlotSnowHeight( double dHei )
	{
		if (m_opSnowNode.valid())
		{
			m_opSnowNode->SetWeatherHeight(dHei);
		}

		if (m_pSettingWidget)
		{
			m_pSettingWidget->GetSetData().m_sEnvSnowSetData.fheigth = dHei;
		}
	}

	void CSnowSetWidget::SlotGlobalSnow( bool bGlobal )
	{
		if (m_opSnowNode.valid())
		{
			m_opSnowNode->SetWorld(true);
		}

		ui.label_Longitude->setEnabled(false);
		ui.label_Latitude->setEnabled(false);
		ui.label_Radius->setEnabled(false);
		ui.doubleSpinBox_SnowLatitude->setEnabled(false);
		ui.doubleSpinBox_SnowLongitude->setEnabled(false);
		ui.doubleSpinBox_SnowRadius->setEnabled(false);
		ui.toolBtn_Locate->setEnabled(false);

		if (m_pSettingWidget)
		{
			m_pSettingWidget->GetSetData().m_sEnvSnowSetData.bGlobal = true;
		}
	}

	void CSnowSetWidget::SlotLocalSnow( bool bPart )
	{
		ui.label_Longitude->setEnabled(true);
		ui.label_Latitude->setEnabled(true);
		ui.label_Radius->setEnabled(true);
		ui.doubleSpinBox_SnowLatitude->setEnabled(true);
		ui.doubleSpinBox_SnowLongitude->setEnabled(true);
		ui.doubleSpinBox_SnowRadius->setEnabled(true);
		ui.toolBtn_Locate->setEnabled(true);
		
		osg::Vec3 vecPosition(
			ui.doubleSpinBox_SnowLongitude->value(),
			ui.doubleSpinBox_SnowLatitude->value(),
			ui.doubleSpinBox_SnowRadius->value());

		if (m_opSnowNode.valid())
		{
			m_opSnowNode->SetWeatherPosition(vecPosition);
		}

		if (m_pSettingWidget)
		{
			m_pSettingWidget->GetSetData().m_sEnvSnowSetData.bGlobal = false;
		}
	}

	void CSnowSetWidget::SlotSnowPosition( double )
	{
		osg::Vec3 vecPosition(
			ui.doubleSpinBox_SnowLongitude->value(),
			ui.doubleSpinBox_SnowLatitude->value(),
			ui.doubleSpinBox_SnowRadius->value());

		if (m_opSnowNode.valid())
		{
			m_opSnowNode->SetWeatherPosition(vecPosition);
		}

		if (m_pSettingWidget)
		{
			m_pSettingWidget->GetSetData().m_sEnvSnowSetData.fLongitude = vecPosition.x();
			m_pSettingWidget->GetSetData().m_sEnvSnowSetData.fLatitude = vecPosition.y();
			m_pSettingWidget->GetSetData().m_sEnvSnowSetData.fRadius = vecPosition.z();
		}
	}

	void CSnowSetWidget::SlotLocation()
	{
		if (m_opSystemService.valid())
		{
			//将视点高度设置为雪高度的四分之一
			osgEarth::Viewpoint vp(
				"",
				ui.doubleSpinBox_SnowLongitude->value(),
				ui.doubleSpinBox_SnowLatitude->value(),
				ui.doubleSpinBox_SnowHeight->value()/4.0,
				0.0,
				0.0,
				ui.doubleSpinBox_SnowHeight->value()/4.0);

			m_opSystemService->GetManipulatorManager()->Locate(vp, 3.0);
		}
	}

	void CSnowSetWidget::Reset( FeShell::SEnvSnowSetData data )
	{
		if (m_opSnowNode.valid())
		{
			//显隐
			ui.checkBox_Snow->setChecked(data.bSnow);

			///透明度
			ui.hSlider_SnowIntensity->setValue(data.dDensity*10.0);
			ui.snow_density_doubleSpinBox->setValue(data.dDensity);
			m_opSnowNode->snow(data.dDensity);

			//高度
			ui.doubleSpinBox_SnowHeight->setValue(data.fheigth);
			m_opSnowNode->SetWeatherHeight(data.fheigth);

			///位置
			ui.doubleSpinBox_SnowLatitude->setValue(data.fLatitude);
			ui.doubleSpinBox_SnowLongitude->setValue(data.fLongitude);
			ui.doubleSpinBox_SnowRadius->setValue(data.fRadius);
			m_opSnowNode->SetWeatherPosition(osg::Vec3d(data.fLatitude, data.fLongitude, data.fRadius), !data.bGlobal);

			///全球，本地
			ui.radioBtn_GlobalSnow->setChecked(data.bGlobal);
			m_opSnowNode->SetWorld(data.bGlobal);
			ui.radioBtn_LocalSnow->setChecked(!data.bGlobal);
		}
	}
}

