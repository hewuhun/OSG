#include <environment/FogSetWidget.h>

#include <environment/SettingWidget.h>
#include "FeExtNode/ExCircleNode.h"
#include "FeExtNode/ExLodModelNode.h"
#include "FeExtNode/ExModelNode.h"
#include "osgEarthUtil/Fog"
#include "FeUtils/UtilityGeom.h"
#include <osg/ShapeDrawable>
#include <environment/EnvironmentPluginInterface.h>
#include "FeUtils/CoordConverter.h"


namespace FreeViewer
{

	CFogSetWidget::CFogSetWidget( CSettingWidget* pSettingWidget)
		:QDialog(pSettingWidget)
		,m_pSettingWidget(pSettingWidget)
		,fog(NULL)
		,m_pFog(NULL)
		,m_rpCallBack(NULL)
	{
		ui.setupUi(this);
		InitWidget();
		SlotFogVisible(m_pSettingWidget->GetSetData().m_sEnvFogSetData.bFog);
	}

	CFogSetWidget::~CFogSetWidget()
	{

	}

	void CFogSetWidget::Reset( FeShell::SEnvFogSetData data )
	{
		//显隐
		ui.checkBox_Fog->setChecked(data.bFog);

		///透明度
		ui.hSlider_FogIntensity->setValue(data.dDensity*100.0);
		ui.Fog_density_doubleSpinBox->setValue(data.dDensity);

	}

	void CFogSetWidget::InitWidget()
	{
		if (m_pSettingWidget)
		{
			m_opSystemService = m_pSettingWidget->GetSystemService();
			fog = m_pSettingWidget->GetEnvironmentPlugin()->GetFog();
			m_pFog = new osgEarth::Util::FogEffect;
			m_dHight = m_pSettingWidget->GetSetData().m_sEnvFogSetData.dHight;
			m_dIntensity = m_pSettingWidget->GetSetData().m_sEnvFogSetData.dDensity;
		}

		if (m_opSystemService.valid())
		{
			BuildContext();
		}
	}

	void CFogSetWidget::BuildContext()
	{
		if (NULL == m_pSettingWidget)
		{
			return;
		}
		ui.checkBox_Fog->setChecked(m_pSettingWidget->GetSetData().m_sEnvFogSetData.bFog);

		//设置初始的wu的浓度并同步到界面
		ui.Fog_density_doubleSpinBox->setValue(m_pSettingWidget->GetSetData().m_sEnvFogSetData.dDensity);
		ui.hSlider_FogIntensity->setValue(m_pSettingWidget->GetSetData().m_sEnvFogSetData.dDensity*100.0);
		//初始化雾效果的信号与槽的连接
		{
			connect(ui.checkBox_Fog, SIGNAL(toggled(bool)), this, SLOT(SlotFogVisible(bool)));

			connect(ui.hSlider_FogIntensity, SIGNAL(valueChanged(int)),
				this, SLOT(SlotFogIntensity(int)));

			connect(ui.Fog_density_doubleSpinBox, SIGNAL(valueChanged(double)),
				this, SLOT(SlotFogIntensity(double)));
		}
	}

	void CFogSetWidget::SlotSetFog( bool bShow)
	{
		ui.checkBox_Fog->setChecked(bShow);
	}


	void CFogSetWidget::SlotFogIntensity( int nValue)
	{
		double dFogIntensity =  nValue * 0.01;
		ui.Fog_density_doubleSpinBox->setValue(dFogIntensity);
		m_dIntensity = dFogIntensity;
		if (m_pSettingWidget)
		{
			m_pSettingWidget->GetSetData().m_sEnvFogSetData.dDensity = dFogIntensity;
		}
	}

	void CFogSetWidget::SlotFogIntensity( double dValue)
	{
		double dFogIntensity =  dValue;
		ui.hSlider_FogIntensity->setValue(dFogIntensity * 100);
		m_dIntensity = dValue;
		if (m_pSettingWidget)
		{
			m_pSettingWidget->GetSetData().m_sEnvFogSetData.dDensity = dValue;
		}
	}

	void CFogSetWidget::SlotFogVisible( bool bfog)
	{
		if (bfog)
		{
			m_rpCallBack	 = new CFogUpdateCallBack(fog , m_opSystemService->GetRenderContext(), this);
			osg::StateSet* ss = m_opSystemService->GetRenderContext()->GetMapNode()->getOrCreateStateSet();	
			m_pFog->attach(ss); 
			osg::Vec4 fogColor(0.66f, 0.7f, 0.81f, 1.0f); 
			fog->setColor( fogColor );   
			m_opSystemService->GetRenderContext()->GetMapNode()->getOrCreateStateSet()->setAttributeAndModes( fog, osg::StateAttribute::ON );   
			m_opSystemService->GetRenderContext()->GetMapNode()->addUpdateCallback(m_rpCallBack);
		}
		else
		{
			if (NULL == m_rpCallBack)
			{
				return;
			}
			osg::StateSet* ss = m_opSystemService->GetRenderContext()->GetMapNode()->getOrCreateStateSet();	
			m_pFog->detach(ss); 
			m_rpCallBack->ResetOceanAndSky();
			m_opSystemService->GetRenderContext()->GetMapNode()->removeUpdateCallback(m_rpCallBack);
			m_opSystemService->GetEnvironmentSys()->GetSkyNode()->GetUniformFogEnabled()->set(false);
		}

		if (m_pSettingWidget)
		{
			m_pSettingWidget->GetSetData().m_sEnvFogSetData.bFog = bfog;
		}
		emit SignalFromSetFog(bfog);
	}

	double CFogSetWidget::GetHight()
	{
		return m_dHight;
	}

	double CFogSetWidget::GetIntensity()
	{
		return m_dIntensity;
	}


	CFogUpdateCallBack::CFogUpdateCallBack(osg::Fog * pFog,FeUtil::CRenderContext* pRender,CFogSetWidget* pFogWidget)
		:m_pFog(pFog)
		,m_pRender(pRender)
		,m_pFogWidget(pFogWidget)
	{
		m_pSettingWidget = m_pFogWidget->GetSettingWidget();
		FeShell::CSystemService* pSystemService = m_pSettingWidget->GetSystemService();
		m_opOceanNode = dynamic_cast<FeOcean::COceanSys*>(pSystemService->GetModuleSys(FeOcean::OCEAN_SYSTEM_CALL_DEFAULT_KEY))->GetOceanNode();
		m_pSkyNode = pSystemService->GetEnvironmentSys()->GetSkyNode();
		m_ipUniformSkyFogColor = m_pSkyNode->GetUniformFogColor();
		m_ipUniformSkyFogEnabled = m_pSkyNode->GetUniformFogEnabled();
	}

	CFogUpdateCallBack::~CFogUpdateCallBack()
	{

	}

	void CFogUpdateCallBack::operator()( osg::Node* node, osg::NodeVisitor* nv )
	{
		traverse(node, nv);
		osg::Vec3d eye, center, up;
		//计算浓度的参数 如果不用radio 拉远之后 整个地球呈现白色
		m_pRender->GetCamera()->getViewMatrixAsLookAt(eye,center, up);

		GeoPoint map;
		map.fromWorld( m_pRender->GetMapSRS(), eye );           

		//就算当前视点位置高程
		osg::Vec3d vecLLH;
		XYZ2DegreeLLH(m_pRender, eye, vecLLH);
		DegreeLL2LLH(m_pRender, vecLLH);

		//计算雾浓度
		float fMaxDensity     = 0.000125;
		float fFogStartHeight = 1e4;
		float fRatio = (fFogStartHeight - map.z()) / fFogStartHeight;
		fRatio = osg::clampBetween(fRatio, 0.0f, 1.0f);
		float fDensity = fRatio * fMaxDensity * m_pFogWidget->GetIntensity() * 3; //浓度*3倍显示效果好
		m_pFog->setMode(osg::Fog::EXP2);
		m_pFog->setDensity( fDensity );       
		
		//雾浓度为0 关闭大气层雾效果
		if (0.0 ==  m_pFogWidget->GetIntensity())
		{
			m_ipUniformSkyFogEnabled->set(false);
			return;
		}

		//为了提升雾显示效果 当视口不在海洋上时 显示的雾的时候关闭海洋
		if (m_pSettingWidget->GetSetData().m_sEnvOceanSetData.bOcean)
		{
			if (vecLLH.z() > 0)
			{
				m_opOceanNode->Hide();
			}
			else
			{
				m_opOceanNode->Show();
			}
		}

		//为了提升雾显示效果 显示雾的时候保持大气层开启
		m_pSkyNode->SetAtmosphereVisible(true);

		if(map.z() < 0 || map.z() > fFogStartHeight) // 水面以下关闭雾效果 远处总是关闭雾
		{
			m_ipUniformSkyFogEnabled->set(false);
			ResetOceanAndSky();
		}
		else
		{
			m_ipUniformSkyFogEnabled->set(true);
		}
	}

	void CFogUpdateCallBack::ResetOceanAndSky()
	{
		if (m_pSettingWidget->GetSetData().m_sEnvOceanSetData.bOcean)
		{
			m_opOceanNode->Show();
		}
		else
		{
			m_opOceanNode->Hide();
		}
		m_pSkyNode->SetAtmosphereVisible(m_pSettingWidget->GetSetData().m_sEnvLightSetData.bAtmosphere);
	}

}
