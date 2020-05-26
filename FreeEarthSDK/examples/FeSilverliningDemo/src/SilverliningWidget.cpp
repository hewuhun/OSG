#include <SilverliningWidget.h>

#include <FeUtils/PathRegistry.h>
#include <FeUtils/EnvironmentVariableReader.h>
#include <FeUtils/CoordConverter.h>
#include <FeUtils/TimeUtil.h>

#include <FeKits/sky/FreeSky.h>
#include <FeShell/EnvironmentSys.h>
#include <QDialog>



CSilverliningWidget::CSilverliningWidget(FeShell::CSystemManager* pSystem)
	: m_opSystemMgr(pSystem)
{
	ui.setupUi(this);

	InitializeContext();
	InitializeUI();
}

CSilverliningWidget::~CSilverliningWidget()
{
}

bool CSilverliningWidget::InitializeUI()
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
			ui.cloud_alttitude_spinBox->setFixedWidth(65);
			ui.clouds_altitude_slider->setValue(pSilverLiningNode->GetCloudsAltitude());
			ui.cloud_alttitude_spinBox->setValue(pSilverLiningNode->GetCloudsAltitude());
			connect(ui.clouds_altitude_slider, SIGNAL(valueChanged(int)),this, SLOT(SlotCloudsAltitudeChanged(int)));
			connect(ui.cloud_alttitude_spinBox, SIGNAL(valueChanged(int)),this, SLOT(SlotCloudsAltitudeChanged(int)));


			//密度
			ui.clouds_density_slider->setMinimum(0);
			ui.clouds_density_slider->setMaximum(100);
			ui.cloud_density_doubleSpinBox->setFixedWidth(65);
			ui.clouds_density_slider->setValue(pSilverLiningNode->GetCloudsDensity());
			ui.cloud_density_doubleSpinBox->setValue(pSilverLiningNode->GetCloudsDensity());
			connect(ui.clouds_density_slider, SIGNAL(valueChanged(int)),this, SLOT(SlotCloudsDensityChanged(int)));
			connect(ui.cloud_density_doubleSpinBox, SIGNAL(valueChanged(double)),this, SLOT(SlotCloudsDensityChanged(double)));

			//透明度
			ui.clouds_alpha_slider->setMinimum(0);
			ui.clouds_alpha_slider->setMaximum(100);
			ui.clouds_alpha_doubleSpinBox->setFixedWidth(65);
			ui.clouds_alpha_slider->setValue(pSilverLiningNode->GetCloudsAlpha());			
			ui.clouds_alpha_doubleSpinBox->setValue(pSilverLiningNode->GetCloudsAlpha());
			connect(ui.clouds_alpha_slider, SIGNAL(valueChanged(int)),this, SLOT(SlotCloudsAlphaChanged(int)));
			connect(ui.clouds_alpha_doubleSpinBox, SIGNAL(valueChanged(double)),this, SLOT(SlotCloudsAlphaChanged(double)));

			//风速
			ui.wind_speed_slider->setMinimum(0);
			ui.wind_speed_slider->setMaximum(10000);
			ui.wind_speed_spinBox->setFixedWidth(65);
			ui.wind_speed_slider->setValue(pSilverLiningNode->GetWindSpeed());
			ui.wind_speed_spinBox->setValue(pSilverLiningNode->GetWindSpeed());
			connect(ui.wind_speed_slider, SIGNAL(valueChanged(int)),this, SLOT(SlotWindSpeedChanged(int)));
			connect(ui.wind_speed_spinBox, SIGNAL(valueChanged(int)),this, SLOT(SlotWindSpeedChanged(int)));

			//风向
			ui.wind_direction_slider->setMinimum(0);
			ui.wind_direction_slider->setMaximum(360);
			ui.wind_direction_spinBox->setFixedWidth(65);
			ui.wind_direction_slider->setValue(pSilverLiningNode->GetWindDirection());
			ui.wind_direction_spinBox->setValue(pSilverLiningNode->GetWindDirection());
			connect(ui.wind_direction_slider, SIGNAL(valueChanged(int)),this, SLOT(SlotWindDirectionChanged(int)));
			connect(ui.wind_direction_spinBox, SIGNAL(valueChanged(int)),this, SLOT(SlotWindDirectionChanged(int)));

		
			// 对应的效果值 0,1,4,5,8,9
			ui.CloudType_comboBox->addItem(QString::fromLocal8Bit("高层平面卷云"), QVariant(0));  
			ui.CloudType_comboBox->addItem(QString::fromLocal8Bit("高层纤维状云"), QVariant(1));
			ui.CloudType_comboBox->addItem(QString::fromLocal8Bit("积云"), QVariant(4));
			ui.CloudType_comboBox->addItem(QString::fromLocal8Bit("高清积云"), QVariant(5));
			ui.CloudType_comboBox->addItem(QString::fromLocal8Bit("高耸积云"), QVariant(8));
			ui.CloudType_comboBox->addItem(QString::fromLocal8Bit("沙尘暴"), QVariant(9));
			connect(ui.CloudType_comboBox,SIGNAL(currentIndexChanged (int)),this,SLOT(SlotCloudTypeType(int)));
			ui.CloudType_comboBox->setCurrentIndex(3);
			
			connect(ui.slider_CurrentTime, SIGNAL(valueChanged(int)), this, SLOT(SlotTimeSlider(int)));
		}
	}

	return true;
}

bool CSilverliningWidget::InitializeContext()
{
	if(m_opSystemMgr.valid())
	{
		FeUtil::CRenderContext* pRenderContext = m_opSystemMgr->GetRenderContext();
		if(!pRenderContext) return false;

		m_opSilverLiningSys = new FeSilverLining::CSilverLiningSys(FeFileReg->GetFullPath("silverLining/data"));
		if(m_opSilverLiningSys->Initialize(pRenderContext))
		{
			m_opSystemMgr->GetSystemService()->AddAppModuleSys(m_opSilverLiningSys.get());
		}

		// 开启光照效果
		osg::observer_ptr<FeShell::CEnvironmentSys> pFreeSkySys = m_opSystemMgr->GetSystemService()->GetEnvironmentSys();
		if (pFreeSkySys.valid())
		{
			m_opFreeSky = pFreeSkySys->GetSkyNode();
			if(m_opFreeSky.valid())
			{
				UpdateCurrentTime(m_opFreeSky->GetDateTime(), false);
				m_opFreeSky->Show();
			}
		}

		FeSilverLining::FeSilverLiningNode* pSilverLiningNode = m_opSilverLiningSys->GetSilverLiningNode();
		if(pSilverLiningNode)
		{
			pSilverLiningNode->setCamera(pRenderContext->GetCamera());
			pSilverLiningNode->SetCloudType(0);
			pSilverLiningNode->SetCloudsAltitude(600);
			pSilverLiningNode->SetCloudsThickness(40);
			pSilverLiningNode->SetCloudsDensity(30);
			pSilverLiningNode->SetCloudsAlpha(60);
			pSilverLiningNode->SetWindSpeed(50);



			/// 设置初始视角
			m_opSystemMgr->GetSystemService()->GetManipulatorManager()->Locate(
				osgEarth::Viewpoint(120.30088, 23.501005, -90, 0, pSilverLiningNode->GetCloudsAltitude()), 1);

// 			m_opSystemMgr->GetSystemService()->GetManipulatorManager()->SetHome(
// 				osgEarth::Viewpoint(120.30088, 23.501005, -179.93441,-42.789456, pSilverLiningNode->GetCloudsAltitude()), 1);
// 			m_opSystemMgr->GetSystemService()->GetManipulatorManager()->Home();

		}
	}

	return true;
}


void CSilverliningWidget::SlotCloudsVisibleChanged( bool bShow )
{
	if(m_opSilverLiningSys.valid())
	{
		FeSilverLining::FeSilverLiningNode* pSilverLiningNode = m_opSilverLiningSys->GetSilverLiningNode();
		if(pSilverLiningNode)
		{
			pSilverLiningNode->SetCloudsShow(bShow);
			pSilverLiningNode->SetSkyShow(bShow);
			CloudEnable(bShow);
		}
	}
}





void CSilverliningWidget::SlotCloudsAltitudeChanged(  int nValue )
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
	}
}

void CSilverliningWidget::SlotCloudsDensityChanged( int nValue )
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
	}
}

void CSilverliningWidget::SlotCloudsDensityChanged( double dValue )
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
	}
}

void CSilverliningWidget::SlotCloudsAlphaChanged( int nValue)
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
	}
}

void CSilverliningWidget::SlotCloudsAlphaChanged( double dValue)
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
	}
}

void CSilverliningWidget::SlotWindDirectionChanged( int nValue )
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
	}
}

void CSilverliningWidget::SlotWindSpeedChanged( int nValue )
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
	}
}

void CSilverliningWidget::SlotCloudTypeType( int nIndex )
{
	if(m_opSilverLiningSys.valid())
	{
		FeSilverLining::FeSilverLiningNode* pSilverLiningNode = m_opSilverLiningSys->GetSilverLiningNode();
		if(pSilverLiningNode )
		{
			pSilverLiningNode->SetCloudType(ui.CloudType_comboBox->itemData(nIndex).value<int>());
			ApplyCurAttri();
		}
	}
}

void CSilverliningWidget::SlotSetCloud( bool bShow)
{
	ui.Clouds_visible_checkBox->setChecked(bShow);
}

void CSilverliningWidget::SlotTimeSlider( int nIndex)
{
	if(m_opFreeSky.valid())
	{
		osgEarth::DateTime currentTime = m_opFreeSky->GetDateTime();

		int nYear, nMonth, nDay, nHour, nMin, nSencond;
		FeUtil::Convert2YMDHMS(currentTime.asTimeStamp(), nYear, nMonth, nDay, nHour, nMin, nSencond);

		nHour = nIndex / 60.0;
		nMin = nIndex % 60;

		//方法封装到FeUtil中
		tm time;
		time.tm_year = nYear-1900;
		time.tm_mon = nMonth-1;
		time.tm_mday = nDay;
		time.tm_hour = nHour;
		time.tm_min = nMin;
		time.tm_sec = nSencond;
		osgEarth::DateTime dateTime(time);

		UpdateCurrentTime(dateTime, true);
	}
}


void CSilverliningWidget::UpdateCurrentTime( osgEarth::DateTime time, bool bSlider )
{
	time_t timep = time.asTimeStamp();

	int nYear, nMonth, nDay, nHour, nMin, nSencond;
	FeUtil::Convert2YMDHMS(timep, nYear, nMonth, nDay, nHour, nMin, nSencond);

	//更新时间轴显示的时间
	if (false == bSlider)
	{
		ui.slider_CurrentTime->setValue(nHour * 60 + nMin);
	}
	std::stringstream strTime;
	strTime << setw(2) << nYear << QString::fromLocal8Bit("年").toStdString() << nMonth << QString::fromLocal8Bit("月").toStdString() << nDay << QString::fromLocal8Bit("日 ").toStdString() 
		<< nHour << ":" << nMin << ":" << nSencond;
	ui.label_CurrentTime->setText(strTime.str().c_str());

	if (m_opFreeSky.valid())
	{
		m_opFreeSky->SetDateTime(time);
	}

	FeSilverLining::FeSilverLiningNode* pSilverLiningNode = m_opSilverLiningSys->GetSilverLiningNode();
	if(pSilverLiningNode )
	{
		pSilverLiningNode->SetLocalDateTime(time);
	}
}

void CSilverliningWidget::CloudEnable(bool bShow)
{
	ui.CloudType_comboBox->setEnabled(bShow);
	ui.cloud_alttitude_spinBox->setEnabled(bShow);
	ui.cloud_density_doubleSpinBox->setEnabled(bShow);

	ui.clouds_alpha_doubleSpinBox->setEnabled(bShow);
	ui.clouds_alpha_slider->setEnabled(bShow);
	ui.clouds_altitude_slider->setEnabled(bShow);
	ui.clouds_density_slider->setEnabled(bShow);

	ui.wind_direction_slider->setEnabled(bShow);
	ui.wind_direction_spinBox->setEnabled(bShow);
	ui.wind_speed_slider->setEnabled(bShow);
	ui.wind_speed_spinBox->setEnabled(bShow);
}

void CSilverliningWidget::ApplyCurAttri()
{
	FeSilverLining::FeSilverLiningNode* pSilverLiningNode = m_opSilverLiningSys->GetSilverLiningNode();
	if(pSilverLiningNode )
	{
		///高度
		pSilverLiningNode->SetCloudsAltitude(ui.cloud_alttitude_spinBox->value());
		
		
		
		///密度
		pSilverLiningNode->SetCloudsDensity(ui.cloud_density_doubleSpinBox->value());
		
		///透明度
		pSilverLiningNode->SetCloudsAlpha(ui.clouds_alpha_doubleSpinBox->value());
		
		///风速
		pSilverLiningNode->SetWindSpeed(ui.wind_speed_spinBox->value());
		
		///风向
		pSilverLiningNode->SetWindDirection(ui.wind_direction_spinBox->value());

		///时间
		osgEarth::DateTime currentTime = m_opFreeSky->GetDateTime();
		pSilverLiningNode->SetLocalDateTime(currentTime);
	}
}


