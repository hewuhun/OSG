#include <environment/OceanSetWidget.h>
#include <environment/SettingWidget.h>

#include <FeUtils/PathRegistry.h>

namespace FreeViewer
{
	COceanSetWidget::COceanSetWidget( CSettingWidget* pSetttingWidget )
		: QDialog(pSetttingWidget)
		, m_pSettingWidget(pSetttingWidget)
		, m_opOcean(NULL)
		, m_dSpeed(100.0)
		, m_dDirection(0)
		, m_dLength(10.0)
	{
		ui.setupUi(this);

		InitWidget();
	}

	COceanSetWidget::~COceanSetWidget()
	{

	}

	void COceanSetWidget::InitWidget()
	{
		if (m_pSettingWidget)
		{
			m_opSystemService = m_pSettingWidget->GetSystemService();
		}

		if (m_opSystemService.valid())
		{
			m_opOcean = dynamic_cast<FeOcean::COceanSys*>(m_opSystemService->GetModuleSys(FeOcean::OCEAN_SYSTEM_CALL_DEFAULT_KEY));
		}

		if (m_opOcean.valid())
		{
			BuildContext();
		}
	}

	void COceanSetWidget::BuildContext()
	{
		if(m_opOcean.valid())
		{
			FeOcean::TritonNode* pTritonNode = m_opOcean->GetOceanNode();
			if(pTritonNode)
			{
				//初始化海洋显隐
				bool m = m_opOcean->GetOceanNode()->IsHide();
				ui.ocean_visible_checkBox->setChecked(!m_opOcean->GetOceanNode()->IsHide());
				ControlWidgetEnable(!m_opOcean->GetOceanNode()->IsHide());
				connect(ui.ocean_visible_checkBox, SIGNAL(toggled(bool)), this, SLOT(SlotOceanVisibleChanged(bool)));

				//初始化海平面高度
				ui.ocean_level_slider->setValue(pTritonNode->GetSeaLevel());
				ui.ocean_level_spinBox->setValue(pTritonNode->GetSeaLevel());
				connect(ui.ocean_level_slider, SIGNAL(valueChanged(int)),this, SLOT(SlotOceanLevelChanged(int)));
				connect(ui.ocean_level_spinBox, SIGNAL(valueChanged(int)),this, SLOT(SlotOceanLevelChanged(int)));

				////初始化海洋细节高度  暂时不使用
				//ui.view_point_level_slider->setValue(pTritonNode->GetOceanLODHeight());
				//ui.view_point_level_spinBox->setValue(pTritonNode->GetOceanLODHeight());
				//connect(ui.view_point_level_slider, SIGNAL(valueChanged(int)),this, SLOT(SlotViewPointChanged(int)));
				//connect(ui.view_point_level_spinBox, SIGNAL(valueChanged(int)),this, SLOT(SlotViewPointChanged(int)));

				//初始化浪花 由风速、风向、风波长控制			 
				m_dSpeed = pTritonNode->GetWindSpeed();
				m_dDirection = osg::RadiansToDegrees(pTritonNode->GetWindDirection());
				m_dLength = pTritonNode->GetWindLength();

				//初始化风速
				ui.wind_speed_slider->setValue(m_dSpeed);
				ui.wind_speed_spinBox->setValue(m_dSpeed);
				connect(ui.wind_speed_slider, SIGNAL(valueChanged(int)),this, SLOT(SlotWindSpeedChanged(int)));
				connect(ui.wind_speed_spinBox, SIGNAL(valueChanged(int)),this, SLOT(SlotWindSpeedChanged(int)));

				//初始化风向
				ui.wind_direction_slider->setValue(m_dDirection);
				ui.wind_direction_spinBox->setValue(m_dDirection);
				connect(ui.wind_direction_slider, SIGNAL(valueChanged(int)), this, SLOT(SlotWindDirectionChanged(int)));
				connect(ui.wind_direction_spinBox, SIGNAL(valueChanged(int)), this, SLOT(SlotWindDirectionChanged(int)));

				//初始化风波长
				ui.groupBox_7->setVisible(false);
				ui.wind_Lenght_slider->setValue(m_dLength);
				ui.wind_Lenght_doubleSpinBox->setValue(m_dLength);
				connect(ui.wind_Lenght_slider, SIGNAL(valueChanged(int)), this, SLOT(SlotWindLengthChange(int)));
				connect(ui.wind_Lenght_doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(SlotWindLengthDoubleChange(double)));

				//设置固定大小
				ui.ocean_level_spinBox->setFixedWidth(50);
				ui.wind_speed_spinBox->setFixedWidth(50);
				ui.wind_direction_spinBox->setFixedWidth(50);
				ui.wind_Lenght_doubleSpinBox->setFixedWidth(50);
			}
		}
	}

	void COceanSetWidget::SlotOceanVisibleChanged( bool bShow )
	{
		if(m_opOcean.valid())
		{
			FeOcean::TritonNode* pTritonNode = m_opOcean->GetOceanNode();
			if(pTritonNode)
			{
				bShow? pTritonNode->Show() : pTritonNode->Hide();
			}
			ControlWidgetEnable(bShow);

			if (m_pSettingWidget)
			{
				m_pSettingWidget->GetSetData().m_sEnvOceanSetData.bOcean = bShow;
			}
		}
		emit SignalFromOcean(bShow);
	}


	void COceanSetWidget::ControlWidgetEnable( bool bShow)
	{
		ui.groupBox->setEnabled(bShow);
		ui.groupBox_2->setEnabled(bShow);
		ui.groupBox_3->setEnabled(bShow);
		ui.groupBox_7->setEnabled(bShow);
	}

	void COceanSetWidget::SlotViewPointChanged(int nValue)
	{
		//if(m_opOcean.valid())
		//{
		//	FeOcean::TritonNode* pTritonNode = m_opOcean->GetOceanNode();
		//	if(pTritonNode)
		//	{
		//		//pTritonNode->SetOceanLODHeight(nValue);
		//		ui.view_point_level_slider->setValue(nValue);
		//		ui.view_point_level_spinBox->setValue(nValue);
		//	}
		//}
	}

	void COceanSetWidget::SlotOceanLevelChanged(  int nValue )
	{
		if(m_opOcean.valid())
		{
			FeOcean::TritonNode* pTritonNode = m_opOcean->GetOceanNode();
			if(pTritonNode)
			{
				pTritonNode->SetSeaLevel(nValue);
				ui.ocean_level_slider->setValue(nValue);
				ui.ocean_level_spinBox->setValue(nValue);
			}

			if (m_pSettingWidget)
			{
				m_pSettingWidget->GetSetData().m_sEnvOceanSetData.fOceanLevel = nValue;
			}
		}		
	}

	void COceanSetWidget::SlotAboveChanged( int nValue )
	{

	}

	void COceanSetWidget::SlotAboveColorChanged( )
	{

	}

	void COceanSetWidget::SlotBelowChanged( int nValue )
	{

	}


	void COceanSetWidget::SlotBelowColorChanged( )
	{

	}

	void COceanSetWidget::SlotWindSpeedChanged( int nValue )
	{
		if(m_opOcean.valid())
		{
			
			FeOcean::TritonNode* pTritonNode = m_opOcean->GetOceanNode();
			if(pTritonNode)
			{
				pTritonNode->SetWindSpeed(nValue);

				ui.wind_speed_spinBox->setValue(nValue);
				ui.wind_speed_slider->setValue(nValue);
			}	

			if (m_pSettingWidget)
			{
				m_pSettingWidget->GetSetData().m_sEnvOceanSetData.fWindSpeed = nValue;
			}	
		}
	}


	void COceanSetWidget::SlotWindDirectionChanged( int nValue )
	{
		if(m_opOcean.valid())
		{
			FeOcean::TritonNode* pTritonNode = m_opOcean->GetOceanNode();
			if(pTritonNode)
			{
				pTritonNode->SetWindDirection(osg::DegreesToRadians(double(nValue)));

				ui.wind_direction_spinBox->setValue(nValue);
				ui.wind_direction_slider->setValue(nValue);
			}

			if (m_pSettingWidget)
			{
				m_pSettingWidget->GetSetData().m_sEnvOceanSetData.fWindDirection = nValue;
			}	
		}
	}

	void COceanSetWidget::SlotWindLengthDoubleChange( double dValue )
	{
		if(m_opOcean.valid())
		{	
			FeOcean::TritonNode* pTritonNode = m_opOcean->GetOceanNode();
			if(pTritonNode)
			{	
				pTritonNode->SetWindLength(dValue);

				int nValue = 0;
				dValue = dValue * 100;
				nValue = int(dValue);
				ui.wind_Lenght_slider->setValue(nValue);			
			}
		}
	}

	void COceanSetWidget::SlotWindLengthChange( int nValue )
	{
		if(m_opOcean.valid())
		{	
			FeOcean::TritonNode* pTritonNode = m_opOcean->GetOceanNode();
			if(pTritonNode)
			{	
				pTritonNode->SetWindLength(nValue);

				double dValue = double(nValue);
				dValue/=100;
				ui.wind_Lenght_doubleSpinBox->setValue(dValue);
			}
		}
	}

	void COceanSetWidget::SlotSetOcean( bool bShow)
	{
		ui.ocean_visible_checkBox->setChecked(bShow);
	}

	void COceanSetWidget::Reset( FeShell::SEnvOceanSetData data )
	{
		///海洋显隐
		ui.ocean_visible_checkBox->setChecked(data.bOcean);
		FeOcean::TritonNode* pTritonNode = m_opOcean->GetOceanNode();
		if(data.bOcean)
		{
			pTritonNode->Show();
		}
		else
		{
			pTritonNode->Hide();
		}

		///海洋高度
		ui.ocean_level_slider->setValue(data.fOceanLevel);

		///风速
		ui.wind_speed_slider->setValue(data.fWindSpeed);

		///风向
		ui.wind_direction_slider->setValue(data.fWindDirection);

		///风波长
		ui.wind_Lenght_slider->setValue(data.fWindLength);

		pTritonNode->SetSeaLevel(data.fOceanLevel);
		pTritonNode->SetWindDirection(data.fWindDirection);
		pTritonNode->SetWindLength(data.fWindLength);
		pTritonNode->SetWindSpeed(data.fWindSpeed);
	}
}

