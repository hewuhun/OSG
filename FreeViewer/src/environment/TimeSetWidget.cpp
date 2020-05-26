#include <environment/TimeSetWidget.h>

#include <FeUtils/TimeUtil.h>
#include <environment/SettingWidget.h>

namespace FreeViewer
{
	CTimeSetWidget::CTimeSetWidget( CSettingWidget* pSetttingWidget )
		: QDialog(pSetttingWidget)
		, m_pSettingWidget(pSetttingWidget)
		, m_opSilverLiningNode(NULL)
		, m_opFreeSky(NULL)
		, m_bSimulationTime(false)
	{
		ui.setupUi(this);

		InitWidget();
	}

	CTimeSetWidget::~CTimeSetWidget()
	{

	}

	void CTimeSetWidget::InitWidget()
	{
		if (m_pSettingWidget)
		{
			m_opSystemService = m_pSettingWidget->GetSystemService();
		}

		if (m_opSystemService.valid())
		{
			//获取云层特效
			FeSilverLining::CSilverLiningSys* pSilverLiningSys = dynamic_cast<FeSilverLining::CSilverLiningSys*>(
				m_opSystemService->GetModuleSys(FeSilverLining::SILVERLINING_CALL_DEFAULT_KEY));
			if (pSilverLiningSys)
			{
				m_opSilverLiningNode = pSilverLiningSys->GetSilverLiningNode();
			}

			//获取天空特效
			FeShell::CEnvironmentSys* pEnvironmentSys = m_opSystemService->GetEnvironmentSys();
			if (pEnvironmentSys)
			{
				m_opFreeSky = pEnvironmentSys->GetSkyNode();
			}

			BuildContext();
		}
	}

	void CTimeSetWidget::BuildContext()
	{
		osgEarth::DateTime dateTime = FeUtil::GetLocalTime();
		if (m_opFreeSky.valid())
		{
			m_opFreeSky->SetDateTime(dateTime);

			UpdateCurrentTime(dateTime);
		}

		//设置时间滑动条的最大最小范围
		ui.slider_CurrentTime->setMinimum(0);
		ui.slider_CurrentTime->setMaximum(1440);

		//设置模拟时间的模拟速度(1秒到1小时)
		ui.slider_TimeSpeed->setMinimum(1);
		ui.slider_TimeSpeed->setMaximum(3600);
		ui.slider_TimeSpeed->setValue(1);
		ui.label_speedData->setText(QString::number(1));

		SlotSychTimeChanged(0);

		//连接信号与槽
		{
			connect(ui.slider_CurrentTime, SIGNAL(valueChanged(int)), this, SLOT(SlotTimeSlider(int)));

			connect(ui.slider_TimeSpeed, SIGNAL(valueChanged(int)),this, SLOT(SlotTimeSpeedChanged(int)));
			connect(ui.toolBtn_StartPause, SIGNAL(pressed()), this, SLOT(SlotTimeControl()));
			connect(ui.toolBtn_Reset, SIGNAL(pressed()), this, SLOT(SlotResetTime()));
			connect(&m_timerSimulation, SIGNAL(timeout()), this, SLOT(SlotSimulateTime()));

			connect(ui.checkBox_SyncTime,SIGNAL(stateChanged(int)), this, SLOT(SlotSychTimeChanged(int)));
			connect(&m_timerSyncTime, SIGNAL(timeout()), this, SLOT(SlotSyncTime()));
		}
	}

	void CTimeSetWidget::Reset()
	{
		//时间流逝速度设置为1
		ui.slider_TimeSpeed->setValue(1);

		//设置时间模拟按钮均可用
		m_bSimulationTime = false;
		ui.toolBtn_StartPause->setText(tr("start"));
		ui.toolBtn_Reset->setDisabled(false);
		ui.checkBox_SyncTime->setDisabled(false);

		//设置时间进度条可拖动
		ui.slider_CurrentTime->setDisabled(false);

		//暂停两个定时器
		m_timerSyncTime.stop();
		m_timerSimulation.stop();

		//重置当前的系统时间
		tm currentTime = FeUtil::GetLocalTime();
		UpdateCurrentTime(currentTime);

		connect(ui.slider_CurrentTime, SIGNAL(valueChanged(int)), this, SLOT(SlotTimeSlider(int)));
	}

	void CTimeSetWidget::SlotTimeSpeedChanged(int nValue)
	{
		ui.label_speedData->setText(QString::number(nValue));
		if (m_opFreeSky.valid())
		{
			m_opFreeSky->SetAnimationRate(nValue);
		}
	}

	void CTimeSetWidget::SlotResetTime()
	{
		tm currentTime = FeUtil::GetLocalTime();

		UpdateCurrentTime(currentTime);
	}

	void CTimeSetWidget::SlotSyncTime()
	{
		//获取本地时间，并同步时间显示轴和时间Label
		tm currentTime = FeUtil::GetLocalTime();

		UpdateCurrentTime(currentTime);
	}

	void CTimeSetWidget::SlotTimeControl()
	{
		//如果当前没有模拟时间流逝，则将开始暂停按钮设置为pause
		if (false == m_bSimulationTime)
		{
			ui.toolBtn_StartPause->setText(tr("pause"));
			ui.label_CurrentTime->setEnabled(false);
			ui.slider_CurrentTime->setEnabled(false);
			ui.checkBox_SyncTime->setEnabled(false);
			ui.toolBtn_Reset->setEnabled(false);

			m_timerSimulation.start(50);
			m_bSimulationTime = true;

			disconnect(ui.slider_CurrentTime, SIGNAL(valueChanged(int)), this, SLOT(SlotTimeSlider(int)));
		}
		else
		{ 
			ui.toolBtn_StartPause->setText(tr("start"));
			ui.label_CurrentTime->setEnabled(true);
			ui.slider_CurrentTime->setEnabled(true);
			ui.checkBox_SyncTime->setEnabled(true);
			ui.toolBtn_Reset->setEnabled(true);

			m_timerSimulation.stop();
			m_bSimulationTime = false;

			connect(ui.slider_CurrentTime, SIGNAL(valueChanged(int)), this, SLOT(SlotTimeSlider(int)));
		}
	}

	void CTimeSetWidget::SlotTimeSlider(int nIndex)
	{
		std::cout<<nIndex<<std::endl;
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

	void CTimeSetWidget::SlotSychTimeChanged(int nState)
	{
		if(m_opFreeSky.valid())
		{
			//如果当前设置为同步系统时间，则界面上所有按钮禁用，并开启定时器进行更新时间
			//否则将界面上所有按钮启用，并停止定时器
			if(nState == Qt::Checked)
			{
				ui.label_CurrentTime->setEnabled(false);
				ui.slider_CurrentTime->setEnabled(false);
				ui.label_speed->setEnabled(false);
				ui.slider_TimeSpeed->setEnabled(false);
				ui.label_speedData->setEnabled(false);
				ui.toolBtn_StartPause->setEnabled(false);
				ui.toolBtn_Reset->setEnabled(false);

				m_timerSyncTime.start(50);
			}
			else
			{
				ui.label_CurrentTime->setEnabled(true);
				ui.slider_CurrentTime->setEnabled(true);
				ui.label_speed->setEnabled(true);
				ui.slider_TimeSpeed->setEnabled(true);
				ui.label_speedData->setEnabled(true);
				ui.toolBtn_StartPause->setEnabled(true);
				ui.toolBtn_Reset->setEnabled(true);

				m_timerSyncTime.stop();
			}
		}
	}

	void CTimeSetWidget::UpdateCurrentTime( osgEarth::DateTime time, bool bSlider )
	{
		time_t timep = time.asTimeStamp();

		int nYear, nMonth, nDay, nHour, nMin, nSencond;
		FeUtil::Convert2YMDHMS(timep, nYear, nMonth, nDay, nHour, nMin, nSencond);

		//更新时间轴显示的时间
		if (false == bSlider)
		{
			ui.slider_CurrentTime->setValue(nHour * 60 + nMin);
		}
		ui.label_CurrentTime->setText(ctime(&timep));

		if (m_opFreeSky.valid())
		{
			m_opFreeSky->SetDateTime(time);
		}

		if (m_opSilverLiningNode.valid())
		{
			m_opSilverLiningNode->SetLocalDateTime(time);
		}
	}

	void CTimeSetWidget::SlotSimulateTime()
	{
		if (m_opFreeSky.valid())
		{
			osgEarth::DateTime currentTime = m_opFreeSky->GetDateTime();
			time_t time = currentTime.asTimeStamp();
			//由于定时器使用50ms一更新，即每秒更新20次，为了保证1s更新时间为1s，所有乘以1/20
			time += double(ui.slider_TimeSpeed->value())*0.05;

			UpdateCurrentTime(time);
		}
	}

}