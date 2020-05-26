#include <FlightWidget.h>

#include <FeUtils/PathRegistry.h>
#include <FeUtils/EnvironmentVariableReader.h>
#include <FeUtils/TimeUtil.h>

#include <FeKits/sky/FreeSky.h>
#include <FeShell/EnvironmentSys.h>
#include <FeSilverlining/SilverliningSys.h>
#include "FeManager/FreeMarkSys.h"

#include <QString>

CFlightWidget::CFlightWidget(FeShell::CSystemManager* pSystem)
	: m_opSystemMgr(pSystem)
{
	ui.setupUi(this);

	InitializeContext();
	InitializeUI();
}

CFlightWidget::~CFlightWidget()
{
}

bool CFlightWidget::InitializeUI()
{ 
 	connect(ui.spinBox_Speed, SIGNAL(valueChanged(int)), this, SLOT(SlotSpeedChanged(int)));
	connect(ui.buttonGroupCamera, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(SlotCameraChanged(QAbstractButton *)));
	connect(ui.buttonGroupCtrl, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(SlotControlChanged(QAbstractButton *)));
	connect(ui.checkBox_Airway, SIGNAL(clicked(bool)), this, SLOT(SlotAirwayShow(bool)));

	if(m_opManipulatorMgr.valid() && m_rpDemo.valid())
	{
		m_opManipulatorMgr->GetOrCreateLockManipulator()->setNode(m_rpDemo->GetAirModel());
		ui.radioButton_LockCam->setChecked(true);
		ui.spinBox_Speed->setValue(m_rpDemo->GetSpeed());
	}
	

	return true;
}

bool CFlightWidget::InitializeContext()
{
	if(m_opSystemMgr.valid())
	{
		// 获取标记系统
		FeManager::CFreeMarkSys* pMarkSys = dynamic_cast<FeManager::CFreeMarkSys*>(
			m_opSystemMgr->GetSystemService()->GetModuleSys(FeManager::FREE_MARK_SYSTEM_CALL_DEFAULT_KEY));

		if(!pMarkSys)
		{
			pMarkSys = new FeManager::CFreeMarkSys();
			m_opSystemMgr->GetSystemService()->AddAppModuleSys(pMarkSys);
			pMarkSys->Initialize(m_opSystemMgr->GetRenderContext());
			pMarkSys->Start();
		}

		m_rpDemo = new CFlightDemo(m_opSystemMgr->GetRenderContext(), pMarkSys);
		m_rpDemo->Init();

		connect(m_rpDemo.get(), SIGNAL(Sig_StateChanged(const QString&)), this, SLOT(SlotFlightStateInfo(const QString&)));

		// 开启光照效果
		osg::observer_ptr<FeShell::CEnvironmentSys> pFreeSkySys = m_opSystemMgr->GetSystemService()->GetEnvironmentSys();
		if (pFreeSkySys.valid())
		{
			m_opFreeSky = pFreeSkySys->GetSkyNode();
			if(m_opFreeSky.valid())
			{
				m_opFreeSky->Show();
				m_opFreeSky->SetDateTime(osgEarth::DateTime(2017, 3, 30, 6));
				m_opFreeSky->SetAmbient(osg::Vec4d(0.8, 0.8, 0.8, 1));
			}
		}

		//云
		std::string strSilverLiningRSPath = FeFileReg->GetFullPath("silverLining/data");
		osg::ref_ptr<FeSilverLining::CSilverLiningSys> rpSilverLiningSys = new FeSilverLining::CSilverLiningSys(strSilverLiningRSPath);
		if (rpSilverLiningSys->Initialize(m_opSystemMgr->GetSystemService()->GetRenderContext()))
		{
			rpSilverLiningSys->SetSysKey("EnvCloud");
			if (m_opSystemMgr->GetSystemService()->AddAppModuleSys(rpSilverLiningSys.get()))
			{
				FeSilverLining::FeSilverLiningNode * m_rpSilverLiningNode = rpSilverLiningSys->GetSilverLiningNode();
				//m_rpSilverLiningNode->setCamera(p3DScenePanel->GetSystemManager()->GetSystemService()->GetRenderContext()->GetCamera());
				m_rpSilverLiningNode->SetCloudType(4);
				m_rpSilverLiningNode->SetCloudsAlpha(0.35);
			}
		}

		// 视角
		m_opManipulatorMgr = m_opSystemMgr->GetSystemService()->GetManipulatorManager();

		if(m_opManipulatorMgr.valid())
		{
			m_opManipulatorMgr->GetOrCreateLockManipulator()->setThirdPersonHomeParam(120, -50, -80);//(50,0,-75);
			m_opManipulatorMgr->GetOrCreateLockManipulator()->resetCamera();
			//m_opManipulatorMgr->GetOrCreateFirstPersonManipulator()->setFirstPersonCenterOffset(osg::Vec3d(-0.3, 0.6, 0));
		}
		
		double dTmp;
		m_opSystemMgr->GetRenderContext()->GetCamera()->getProjectionMatrixAsPerspective(m_dOriginFov, dTmp, dTmp, dTmp);
	}

	return true;
}

void CFlightWidget::SlotSpeedChanged( int nVal)
{
	if(nVal > 0 && m_rpDemo.valid())
	{
		m_rpDemo->SetSpeed(nVal);
	}
}

void CFlightWidget::SlotCameraChanged(QAbstractButton *pBtn)
{
	if(!m_opManipulatorMgr.valid() || !m_rpDemo.valid()) return;

	double fov, aspect, dnear, dfar;
	m_opSystemMgr->GetRenderContext()->GetCamera()->getProjectionMatrixAsPerspective(fov, aspect, dnear, dfar);

	if(pBtn == ui.radioButton_FreeCam)
	{
		m_opManipulatorMgr->GetOrCreateFEManipulator()->setNode(m_rpDemo->GetAirModel());
		m_opSystemMgr->GetRenderContext()->GetCamera()->setProjectionMatrixAsPerspective(m_dOriginFov, aspect, dnear, dfar);
	}
	else if(pBtn == ui.radioButton_FirstCam)
	{
		m_opManipulatorMgr->GetOrCreateFirstPersonManipulator()->setNode(m_rpDemo->GetAirModel());
		m_opSystemMgr->GetRenderContext()->GetCamera()->setProjectionMatrixAsPerspective(60, aspect, 1, dfar);
	}
	else if(pBtn == ui.radioButton_LockCam)
	{
		m_opManipulatorMgr->GetOrCreateLockManipulator()->setNode(m_rpDemo->GetAirModel());
		m_opSystemMgr->GetRenderContext()->GetCamera()->setProjectionMatrixAsPerspective(m_dOriginFov, aspect, dnear, dfar);
	}
}

void CFlightWidget::SlotControlChanged( QAbstractButton * pBtn)
{
	if(!m_rpDemo.valid()) return;

	if(pBtn == ui.pushButton_Start)
	{
		m_rpDemo->Start();
	}
// 	else if(pBtn == ui.pushButton_Stop)
// 	{
// 		m_rpDemo->Stop();
// 	}
	else if(pBtn == ui.pushButton_Reset)
	{
		m_rpDemo->Reset();
		ui.spinBox_Speed->setValue(m_rpDemo->GetSpeed());
	}
}

void CFlightWidget::SlotFlightStateInfo( const QString& strInfo)
{
	ui.label_StateInfo->setText(strInfo);
}

void CFlightWidget::SlotAirwayShow( bool bShow)
{
	if(m_rpDemo.valid())
	{
		m_rpDemo->ShowAirLine(bShow);
	}
}

