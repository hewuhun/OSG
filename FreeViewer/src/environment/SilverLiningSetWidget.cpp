#include <environment/SilverLiningSetWidget.h>

#include <environment/SettingWidget.h>

#include <FeUtils/PathRegistry.h>
#include <FeUtils/StrUtil.h>

namespace FreeViewer
{
    CSilverLiningSetWidget::CSilverLiningSetWidget(CSettingWidget* pSetttingWidget)
		: QDialog(pSetttingWidget)
		,m_pSettingWidget(pSetttingWidget)
		,m_opSilverLiningSys(NULL)
	{
		ui.setupUi(this);
		
		InitWidget();
	}

    CSilverLiningSetWidget::~CSilverLiningSetWidget()
	{

	}

    void CSilverLiningSetWidget::InitWidget()
	{
		if (m_pSettingWidget)
		{
			m_opSystemService = m_pSettingWidget->GetSystemService();
		}

		if (m_opSystemService.valid())
		{
			m_opSilverLiningSys = dynamic_cast<FeSilverLining::CSilverLiningSys*>(m_opSystemService->GetModuleSys(FeSilverLining::SILVERLINING_CALL_DEFAULT_KEY));
			if (m_opSilverLiningSys.valid())
			{
				BuildContext();
			}
		}
	}

    void CSilverLiningSetWidget::BuildContext()
	{
		if(m_opSilverLiningSys.valid())
		{
			FeSilverLining::FeSilverLiningNode* pSilverLiningNode = m_opSilverLiningSys->GetSilverLiningNode();
			if(pSilverLiningNode)
			{
				ui.Clouds_visible_checkBox->setChecked(pSilverLiningNode->IsCloudsShow());
				CloudEnable(pSilverLiningNode->IsCloudsShow());
				connect(ui.Clouds_visible_checkBox, SIGNAL(toggled(bool)), this, SLOT(SlotCloudsVisibleChanged(bool)));

				//高度
				ui.clouds_altitude_slider->setMinimum(100);
				ui.clouds_altitude_slider->setMaximum(10000);
				ui.clouds_altitude_slider->setValue(pSilverLiningNode->GetCloudsAltitude());
				ui.cloud_alttitude_spinBox->setValue(pSilverLiningNode->GetCloudsAltitude());
				connect(ui.clouds_altitude_slider, SIGNAL(valueChanged(int)),this, SLOT(SlotCloudsAltitudeChanged(int)));
				connect(ui.cloud_alttitude_spinBox, SIGNAL(valueChanged(int)),this, SLOT(SlotCloudsAltitudeChanged(int)));

				//厚度
				ui.clouds_thickness_slider->setMinimum(20);
				ui.clouds_thickness_slider->setMaximum(1000);
				ui.clouds_thickness_slider->setValue(pSilverLiningNode->GetCloudsThickness());
				ui.cloud_thickness_spinBox->setValue(pSilverLiningNode->GetCloudsThickness());
				connect(ui.clouds_thickness_slider, SIGNAL(valueChanged(int)),this, SLOT(SlotCloudsThicknessChanged(int)));
				connect(ui.cloud_thickness_spinBox, SIGNAL(valueChanged(int)),this, SLOT(SlotCloudsThicknessChanged(int)));

				//密度
				ui.clouds_density_slider->setMinimum(0);
				ui.clouds_density_slider->setMaximum(100);
				ui.clouds_density_slider->setValue(pSilverLiningNode->GetCloudsDensity());
				ui.cloud_density_doubleSpinBox->setValue(pSilverLiningNode->GetCloudsDensity());
				connect(ui.clouds_density_slider, SIGNAL(valueChanged(int)),this, SLOT(SlotCloudsDensityChanged(int)));
				connect(ui.cloud_density_doubleSpinBox, SIGNAL(valueChanged(double)),this, SLOT(SlotCloudsDensityChanged(double)));

				//透明度
				ui.clouds_alpha_slider->setMinimum(0);
				ui.clouds_alpha_slider->setMaximum(100);
				ui.clouds_alpha_slider->setValue(pSilverLiningNode->GetCloudsAlpha());			
				ui.clouds_alpha_doubleSpinBox->setValue(pSilverLiningNode->GetCloudsAlpha());
				connect(ui.clouds_alpha_slider, SIGNAL(valueChanged(int)),this, SLOT(SlotCloudsAlphaChanged(int)));
				connect(ui.clouds_alpha_doubleSpinBox, SIGNAL(valueChanged(double)),this, SLOT(SlotCloudsAlphaChanged(double)));

				//风速
				ui.wind_speed_slider->setMinimum(0);
				ui.wind_speed_slider->setMaximum(10000);
				ui.wind_speed_slider->setValue(pSilverLiningNode->GetWindSpeed());
				ui.wind_speed_spinBox->setValue(pSilverLiningNode->GetWindSpeed());
				connect(ui.wind_speed_slider, SIGNAL(valueChanged(int)),this, SLOT(SlotWindSpeedChanged(int)));
				connect(ui.wind_speed_spinBox, SIGNAL(valueChanged(int)),this, SLOT(SlotWindSpeedChanged(int)));

				//风向
				ui.wind_direction_slider->setMinimum(0);
				ui.wind_direction_slider->setMaximum(360);
				ui.wind_direction_slider->setValue(pSilverLiningNode->GetWindDirection());
				ui.wind_direction_spinBox->setValue(pSilverLiningNode->GetWindDirection());
				connect(ui.wind_direction_slider, SIGNAL(valueChanged(int)),this, SLOT(SlotWindDirectionChanged(int)));
				connect(ui.wind_direction_slider, SIGNAL(valueChanged(int)),this, SLOT(SlotWindDirectionChanged(int)));

				ui.CloudType_comboBox->addItem(QString(FeUtil::ToUTF("高层平面卷云CIRROCUMULUS").c_str()));
				ui.CloudType_comboBox->addItem(QString(FeUtil::ToUTF("高层纤维状云CIRRUS_FIBRATUS").c_str()));
				ui.CloudType_comboBox->addItem(QString(FeUtil::ToUTF("积云CUMULUS_CONGESTUS").c_str()));
				ui.CloudType_comboBox->addItem(QString(FeUtil::ToUTF("高清积云CUMULUS_CONGESTUS_HI_RES").c_str()));
				ui.CloudType_comboBox->addItem(QString(FeUtil::ToUTF("高耸积云TOWERING_CUMULUS").c_str()));
				ui.CloudType_comboBox->addItem(QString(FeUtil::ToUTF("沙尘暴SANDSTORM").c_str()));
				ui.CloudType_comboBox->setCurrentIndex(pSilverLiningNode->GetCloudType());
				QObject::connect(ui.CloudType_comboBox,SIGNAL(currentIndexChanged (int)),this,SLOT(SlotCloudTypeType(int)));
				
			}
		}
	}

    void CSilverLiningSetWidget::SlotCloudsVisibleChanged( bool bShow )
	{
		if(m_opSilverLiningSys.valid())
		{
			FeSilverLining::FeSilverLiningNode* pSilverLiningNode = m_opSilverLiningSys->GetSilverLiningNode();
			if(pSilverLiningNode)
			{
				pSilverLiningNode->SetCloudsShow(bShow);
				//pSilverLiningNode->SetSkyShow(false);
				pSilverLiningNode->SetSkyShow(bShow);
				CloudEnable(bShow);
			}
		}
		emit SignalFormCloud(bShow);

		m_pSettingWidget->GetSetData().m_sEnvCloudSetData.bCloud = bShow;
	}

    void CSilverLiningSetWidget::SlotCloudsThicknessChanged(int nValue)
	{

		if(m_opSilverLiningSys.valid())
		{
			FeSilverLining::FeSilverLiningNode* pSilverLiningNode = m_opSilverLiningSys->GetSilverLiningNode();
			if(pSilverLiningNode)
			{
				pSilverLiningNode->SetCloudsThickness(nValue);
			}
			ui.cloud_thickness_spinBox->setValue(nValue);
			ui.clouds_thickness_slider->setValue(nValue);

			m_pSettingWidget->GetSetData().m_sEnvCloudSetData.nThickness = nValue;
		}
	}



    void CSilverLiningSetWidget::SlotCloudsAltitudeChanged(  int nValue )
	{
		if(m_opSilverLiningSys.valid())
		{
			FeSilverLining::FeSilverLiningNode* pSilverLiningNode = m_opSilverLiningSys->GetSilverLiningNode();
			if(pSilverLiningNode)
			{
				pSilverLiningNode->SetCloudsAltitude(nValue);
			}
			ui.clouds_altitude_slider->setValue(nValue);
			ui.cloud_alttitude_spinBox->setValue(nValue);

			m_pSettingWidget->GetSetData().m_sEnvCloudSetData.nAltitude = nValue;
		}
	}

    void CSilverLiningSetWidget::SlotCloudsDensityChanged( int nValue )
	{
		if(m_opSilverLiningSys.valid())
		{
			FeSilverLining::FeSilverLiningNode* pSilverLiningNode = m_opSilverLiningSys->GetSilverLiningNode();
			if(pSilverLiningNode)
			{
				double dValue = (nValue * 1.0)/100;
				pSilverLiningNode->SetCloudsDensity(dValue);
				ui.cloud_density_doubleSpinBox->setValue(dValue);
			}

			m_pSettingWidget->GetSetData().m_sEnvCloudSetData.nDensity = nValue;
		}
	}

    void CSilverLiningSetWidget::SlotCloudsDensityChanged( double dValue )
	{
		if(m_opSilverLiningSys.valid())
		{
			FeSilverLining::FeSilverLiningNode* pSilverLiningNode = m_opSilverLiningSys->GetSilverLiningNode();
			if(pSilverLiningNode)
			{
				int nValue = dValue * 100;
				pSilverLiningNode->SetCloudsDensity(dValue);
				ui.clouds_density_slider->setValue(nValue);
			}

			m_pSettingWidget->GetSetData().m_sEnvCloudSetData.dDensity = dValue;
		}
	}

    void CSilverLiningSetWidget::SlotCloudsAlphaChanged( int nValue)
	{
		if(m_opSilverLiningSys.valid())
		{
			FeSilverLining::FeSilverLiningNode* pSilverLiningNode = m_opSilverLiningSys->GetSilverLiningNode();
			if(pSilverLiningNode)
			{
				double dValue = (nValue * 1.0)/100;
				pSilverLiningNode->SetCloudsAlpha(dValue);
				ui.clouds_alpha_doubleSpinBox->setValue(dValue);
			}

			m_pSettingWidget->GetSetData().m_sEnvCloudSetData.nAlpha = nValue;
		}
	}

    void CSilverLiningSetWidget::SlotCloudsAlphaChanged( double dValue)
	{
		if(m_opSilverLiningSys.valid())
		{
			FeSilverLining::FeSilverLiningNode* pSilverLiningNode = m_opSilverLiningSys->GetSilverLiningNode();
			if(pSilverLiningNode)
			{
				int nValue = dValue * 100;
				pSilverLiningNode->SetCloudsAlpha(dValue);
				ui.clouds_alpha_slider->setValue(nValue);
			}

			m_pSettingWidget->GetSetData().m_sEnvCloudSetData.dAlpha = dValue;
		}
	}

    void CSilverLiningSetWidget::SlotWindDirectionChanged( int nValue )
	{
		if(m_opSilverLiningSys.valid())
		{
			FeSilverLining::FeSilverLiningNode* pSilverLiningNode = m_opSilverLiningSys->GetSilverLiningNode();
			if(pSilverLiningNode)
			{
				pSilverLiningNode->SetWindDirection(nValue);
				ui.wind_direction_slider->setValue(nValue);
				ui.wind_direction_spinBox->setValue(nValue);
			}

			m_pSettingWidget->GetSetData().m_sEnvCloudSetData.nDirection = nValue;
		}
	}

    void CSilverLiningSetWidget::SlotWindSpeedChanged( int nValue )
	{
		if(m_opSilverLiningSys.valid())
		{
			FeSilverLining::FeSilverLiningNode* pSilverLiningNode = m_opSilverLiningSys->GetSilverLiningNode();
			if(pSilverLiningNode)
			{
				pSilverLiningNode->SetWindSpeed(nValue);
				ui.wind_speed_slider->setValue(nValue);
				ui.wind_speed_spinBox->setValue(nValue);
			}

			m_pSettingWidget->GetSetData().m_sEnvCloudSetData.nSpeed = nValue;
		}
	}

    void CSilverLiningSetWidget::SlotCloudTypeType( int nIndex )
	{
		if(m_opSilverLiningSys.valid())
		{
			FeSilverLining::FeSilverLiningNode* pSilverLiningNode = m_opSilverLiningSys->GetSilverLiningNode();
			if(pSilverLiningNode )
			{
				pSilverLiningNode->SetCloudType(nIndex);
			}

			m_pSettingWidget->GetSetData().m_sEnvCloudSetData.nCloudType = nIndex;
		}
	}

    void CSilverLiningSetWidget::SlotSetCloud( bool bShow)
	{
		ui.Clouds_visible_checkBox->setChecked(bShow);
	}

    void CSilverLiningSetWidget::CloudEnable(bool bShow)
	{
		ui.groupBox->setEnabled(bShow);
		ui.groupBox_2->setEnabled(bShow);
		ui.groupBox_3->setEnabled(bShow);
		ui.groupBox_4->setEnabled(bShow);
		ui.groupBox_5->setEnabled(bShow);
		ui.groupBox_6->setEnabled(bShow);
		ui.groupBox_7->setEnabled(bShow);
		ui.CloudType_comboBox->setEnabled(bShow);
	}

	void CSilverLiningSetWidget::Reset( FeShell::SEnvCloudSetData data )
	{
		///云显隐
		ui.Clouds_visible_checkBox->setChecked(data.bCloud);
		FeSilverLining::FeSilverLiningNode* pSilverLiningNode = m_opSilverLiningSys->GetSilverLiningNode();
		if(pSilverLiningNode)
		{
			pSilverLiningNode->SetCloudsShow(data.bCloud);
			pSilverLiningNode->SetSkyShow(data.bCloud);
		}
		///云类型
		if(ui.CloudType_comboBox->currentIndex() != data.nCloudType)
		{
			pSilverLiningNode->SetCloudType(data.nCloudType);
			ui.CloudType_comboBox->setCurrentIndex(data.nCloudType);
		}		
		///高度
		ui.clouds_altitude_slider->setValue(data.nAltitude);
		pSilverLiningNode->SetCloudsAltitude(data.nAltitude);
		///厚度
		ui.clouds_thickness_slider->setValue(data.nThickness);
		pSilverLiningNode->SetCloudsThickness(data.nAltitude);
		///密度
		ui.clouds_density_slider->setValue(data.nDensity);
		ui.cloud_density_doubleSpinBox->setValue(data.dDensity);
		pSilverLiningNode->SetCloudsDensity(data.dDensity);
		///透明度
		ui.clouds_alpha_slider->setValue(data.nAlpha);
		ui.clouds_alpha_doubleSpinBox->setValue(data.dAlpha);
		pSilverLiningNode->SetCloudsAlpha(data.dAlpha);
		///风速
		ui.wind_speed_slider->setValue(data.nSpeed);
		pSilverLiningNode->SetWindSpeed(data.nSpeed);
		///风向
		ui.wind_direction_slider->setValue(data.nDirection);
		pSilverLiningNode->SetWindDirection(data.nDirection);
	}

}

