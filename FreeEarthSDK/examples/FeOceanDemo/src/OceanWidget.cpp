#include <OceanWidget.h>

#include "osg/ComputeBoundsVisitor"

#include <FeKits/sky/FreeSky.h>
#include <FeShell/EnvironmentSys.h>
#include <FeOcean/WakeGeneratorCallback.h>
#include <FeOcean/RotorWashCallback.h>
#include <FeUtils/MiniAnimationCtrl.h>
#include <FeUtils/PathRegistry.h>
#include <FeUtils/EnvironmentVariableReader.h>
#include <FeUtils/TimeUtil.h>

#include "FeExtNode/ExModelNode.h"
#include "FeManager/FreeMarkSys.h"


using namespace FeOceanDemo;

COceanWidget::COceanWidget(FeShell::CSystemManager* pSystem)
	: m_opSystemMgr(pSystem)
{
	ui.setupUi(this);

	InitializeContext();
	InitializeUI();
}

COceanWidget::~COceanWidget()
{
}

bool COceanWidget::InitializeUI()
{ 
	connect(ui.slider_CurrentTime, SIGNAL(valueChanged(int)), this, SLOT(SlotTimeSlider(int)));
	connect(ui.sun_light_Slider, SIGNAL(valueChanged(int)), this, SLOT(SlotAmbientChanged(int)));
	connect(ui.spinBox_Light, SIGNAL(valueChanged(int)), this, SLOT(SlotAmbientChanged(int)));

	connect(ui.buttonGroupCamera, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(SlotCameraChanged(QAbstractButton *)));
	
	return true;
}

bool COceanWidget::InitializeContext()
{
	if(m_opSystemMgr.valid())
	{
		m_opManipulatorMgr = m_opSystemMgr->GetSystemService()->GetManipulatorManager();

		// 开启光照效果
		osg::observer_ptr<FeShell::CEnvironmentSys> pFreeSkySys = m_opSystemMgr->GetSystemService()->GetEnvironmentSys();
		if (pFreeSkySys.valid())
		{
			m_opFreeSky = pFreeSkySys->GetSkyNode();
			if(m_opFreeSky.valid())
			{
				//UpdateCurrentTime(m_opFreeSky->GetDateTime(), false);
				m_opFreeSky->Show();
				SlotAmbientChanged(255);

				UpdateScene(osgEarth::Viewpoint("", 121.6604396, 22.00362263, 59.86647956, 94.80427447, -1.885712019, 0),
					2017, 4, 11, 5, 50);
			}
		}

		// 加载海洋系统
		m_opOceanSys = new FeOcean::COceanSys(FeFileReg->GetFullPath("triton/data"));
		if (m_opOceanSys->Initialize(m_opSystemMgr->GetRenderContext()))
		{
			m_opSystemMgr->GetSystemService()->AddAppModuleSys(m_opOceanSys.get());
		}

		// 加载标记系统
		m_opMarkSys = new FeManager::CFreeMarkSys();
		if(m_opMarkSys->Initialize(m_opSystemMgr->GetRenderContext()))
		{
			m_opSystemMgr->GetSystemService()->AddAppModuleSys(m_opMarkSys.get());
			m_opMarkSys->Start();
		}
		
		InitializeOceanEffect();
	}

	return true;
}

void FeOceanDemo::COceanWidget::InitializeOceanEffect()
{
	// 海洋尾迹效果展示
	{
		// 创建模型
		FeExtNode::CExModelNodeOption* opt = new FeExtNode::CExModelNodeOption;
		opt->ModelPath() = FeFileReg->GetFullPath("model/054.ive");
		osg::ref_ptr<FeExtNode::CExModelNode> pNode = new FeExtNode::CExModelNode(m_opSystemMgr->GetRenderContext(), opt);
		m_opMarkSys->AddMark(pNode);
		m_opSystemMgr->GetRenderContext()->GetOceanReflectionList().push_back(pNode.get());
		m_rpShipWakeNode = pNode->GetModelNode();

		// 添加路径动画
		osg::ref_ptr<FeUtil::CMiniAnimationCtrl> miniAniCtrl = new FeUtil::CMiniAnimationCtrl(m_opSystemMgr->GetRenderContext(), pNode->GetTransMT());
		std::vector<osg::Vec3d> points;
		points.push_back(osg::Vec3d(121.670350029093, 22, 0));
		points.push_back(osg::Vec3d(121.670350029093, 23, 0));
		miniAniCtrl->CreateAnimationPath(points, 6);
		miniAniCtrl->SetAnimationLoopMode(FeUtil::AnimationPath::LOOP);
		miniAniCtrl->Start();

		// 添加海洋尾迹效果
		osg::ComputeBoundsVisitor boundvisitor;
		m_rpShipWakeNode->accept(boundvisitor);
		osg::BoundingBox bbox = boundvisitor.getBoundingBox();
		double dRange = bbox.yMax() - bbox.yMin();

		Triton::WakeGeneratorParameters paras;
		paras.length = 500.0;
		paras.beamWidth = /*100.0;*/ (bbox.xMax() - bbox.xMin())*1.2;
		paras.draft = 25.0;
		paras.bowWave = true;
		paras.sternWaves = true;
		paras.sprayEffects = true;
		paras.propWash = true;
		paras.bowWaveOffset = dRange * 0.5;
		paras.bowSprayOffset = dRange * 0.5;
		paras.sternWaveOffset = -dRange * 0.5;
		paras.propWashOffset = dRange * 0.5 * 0.9;

		m_rpShipWakeNode->addUpdateCallback(new FeOcean::WakeGeneratorCallback(m_opOceanSys->GetOceanNode(), paras));
	}

	// 海洋旋涡效果展示
	{
		// 创建模型
		FeExtNode::CExModelNodeOption* opt = new FeExtNode::CExModelNodeOption;
		opt->ModelPath() = FeFileReg->GetFullPath("model/WZ_9.ive");
		osg::ref_ptr<FeExtNode::CExModelNode> pNode = new FeExtNode::CExModelNode(m_opSystemMgr->GetRenderContext(), opt);
		m_opMarkSys->AddMark(pNode);
		m_opSystemMgr->GetRenderContext()->GetOceanReflectionList().push_back(pNode.get());
		m_rpRotorWashNode = pNode->GetModelNode();

		// 添加路径动画
		osg::ref_ptr<FeUtil::CMiniAnimationCtrl> miniAniCtrl = new FeUtil::CMiniAnimationCtrl(m_opSystemMgr->GetRenderContext(), pNode->GetTransMT());
		std::vector<osg::Vec3d> points;
		points.push_back(osg::Vec3d(121.672350029093, 22, 8));
		points.push_back(osg::Vec3d(121.672350029093, 23, 8));
		miniAniCtrl->CreateAnimationPath(points, 6);
		miniAniCtrl->SetAnimationLoopMode(FeUtil::AnimationPath::LOOP);
		miniAniCtrl->Start();

		// 添加海洋旋涡效果
		m_rpRotorWashNode->addUpdateCallback(new FeOcean::RotorWashCallback(m_opOceanSys->GetOceanNode()));
	}
}


void COceanWidget::SlotTimeSlider(int nIndex)
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

void COceanWidget::UpdateCurrentTime( osgEarth::DateTime time, bool bSlider )
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
	strTime << setw(2) << nYear << QString::fromLocal8Bit("年").toStdString() << nMonth 
		<< QString::fromLocal8Bit("月").toStdString() << nDay << QString::fromLocal8Bit("日 ").toStdString() 
		<< nHour << ":" << nMin << ":" << nSencond;
	ui.label_CurrentTime->setText(strTime.str().c_str());

	if (m_opFreeSky.valid())
	{
		m_opFreeSky->SetDateTime(time);
	}

}

void COceanWidget::SlotAmbientChanged( int currentValue)
{
	if(m_opFreeSky.valid())
	{
		float fValue = (currentValue)/255.0;

		ui.sun_light_Slider->setValue(currentValue);
		ui.spinBox_Light->setValue(currentValue);
		m_opFreeSky->SetAmbient(osg::Vec4(fValue,fValue,fValue,1.0));
	}

}

void COceanWidget::SlotCameraChanged(QAbstractButton *pBtn)
{
	if(!m_opManipulatorMgr.valid()) return;

	if(pBtn == ui.radioButton_FreeCam)
	{
		m_opManipulatorMgr->GetOrCreateFEManipulator();
	}
	else if(pBtn == ui.radioButton_FirstCam)
	{
		m_opManipulatorMgr->GetOrCreateFirstPersonManipulator()->setNode(m_rpShipWakeNode.get());
	}
	else if(pBtn == ui.radioButton_RotorEffect)
	{
		m_opManipulatorMgr->GetOrCreateLockManipulator()->setThirdPersonHomeParam(130, 130, -50);
		m_opManipulatorMgr->GetOrCreateLockManipulator()->resetCamera();
		m_opManipulatorMgr->GetOrCreateLockManipulator()->setNode(m_rpRotorWashNode.get());
	}
	else if(pBtn == ui.radioButton_ShipWake)
	{
		m_opManipulatorMgr->GetOrCreateLockManipulator()->setThirdPersonHomeParam(800, -70, -60);
		m_opManipulatorMgr->GetOrCreateLockManipulator()->resetCamera();
		m_opManipulatorMgr->GetOrCreateLockManipulator()->setNode(m_rpShipWakeNode.get());
	}
	else if(pBtn == ui.radioButton_UnderOcean)
	{
		m_opManipulatorMgr->GetOrCreateFEManipulator();
		UpdateScene(osgEarth::Viewpoint("", 121.4611849, 22.04716996, -81.30974025, 111.1098539, 9.418642526, 0),
			2017, 11, 4, 9, 0);
	}
	else if(pBtn == ui.radioButton_SunRise)
	{
		m_opManipulatorMgr->GetOrCreateFEManipulator();
		UpdateScene(osgEarth::Viewpoint("", 121.4118451, 22.16654853, 69.42166036, 111.5631039, -4.962713242, 0),
			2017, 11, 4, 6, 20);
	}

}

void FeOceanDemo::COceanWidget::UpdateScene(const osgEarth::Viewpoint& vp, int year, int month, int day, int hours, int min)
{
	tm time;
	time.tm_year = year-1900;
	time.tm_mon = month-1;
	time.tm_mday = day;
	time.tm_hour = hours;
	time.tm_min = min;
	time.tm_sec = 0;
	osgEarth::DateTime newTime(time);
	UpdateCurrentTime(newTime, false);

	m_opManipulatorMgr->SetHome(vp,0);
	m_opManipulatorMgr->Home();
}

