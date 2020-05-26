#include <HdrLightingWidget.h>

#include <FeUtils/PathRegistry.h>
#include <FeUtils/EnvironmentVariableReader.h>
#include <FeUtils/CoordConverter.h>

#include <FeKits/sky/FreeSky.h>
#include <FeShell/EnvironmentSys.h>


class MyNodeCallback : public osg::NodeCallback
{
	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		if(node && nv)
		{
			FeEffect::FeHDRLightingNode* ln = dynamic_cast<FeEffect::FeHDRLightingNode*>(node);
			if(ln)
			{
				static float timePeriod = 1.0;
				float ct = fmodf(nv->getFrameStamp()->getSimulationTime(),timePeriod) / timePeriod; 

				FeEffect::HDRLightParam p = ln->getLight(7);
				p.lightPow = ct;
				ln->setLight(7,p);

				p = ln->getLight(0);
				p.lightPow = ct;
				ln->setLight(0,p);
			}
		}
	}
};

CHdrLightingWidget::CHdrLightingWidget(FeShell::CSystemManager* pSystem)
	: m_opSystemMgr(pSystem)
{
	ui.setupUi(this);

	InitializeContext();
	InitializeUI();
}

CHdrLightingWidget::~CHdrLightingWidget()
{
}

bool CHdrLightingWidget::InitializeUI()
{
    ui.comboBox_LightStyle->setContextMenuPolicy(Qt::NoContextMenu);

	for(int n = 0; n < FeEffect::NUM_GLARELIBTYPES; n++)
	{
		std::stringstream streamStr;
        streamStr << QString::fromLocal8Bit("效果").toStdString() << n+1 ;
		ui.comboBox_LightStyle->addItem(streamStr.str().c_str());
	}

	connect(ui.comboBox_LightStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(SlotLightTypeChanged(int)));
	connect(ui.sun_light_Slider, SIGNAL(valueChanged(int)), this, SLOT(SlotLightPowChanged(int)));
	connect(ui.slider_Radius, SIGNAL(valueChanged(int)), this, SLOT(SlotLightSizeChanged(int)));

	ui.sun_light_Slider->setValue(ui.sun_light_Slider->maximum()/2);
	ui.slider_Radius->setValue(ui.slider_Radius->maximum()/2);
    ui.comboBox_LightStyle->setCurrentIndex(1);

	return true;
}

bool CHdrLightingWidget::InitializeContext()
{
	if(m_opSystemMgr.valid())
	{
		FeUtil::CRenderContext* pRenderContext = m_opSystemMgr->GetRenderContext();
		if(!pRenderContext) return false;
		
		//pRenderContext->GetView()->setUpViewOnSingleScreen(0);

		// 		pRenderContext->GetCamera()->setViewport(new osg::Viewport(0, 0, 
		// 			1608, 1019));
		// 		pRenderContext->GetCamera()->setProjectionMatrixAsPerspective(30.0f, 
		// 			1608 / 1019, 1.0f, 10000.0f);

		
		osg::MatrixTransform* mt = new osg::MatrixTransform();
		osg::Matrix transMat;
		FeUtil::DegreeLLH2Matrix(pRenderContext,osg::Vec3d(120.726,22.03,100),transMat);
		mt->setMatrix(transMat);
		mt->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::ON);
		pRenderContext->GetRoot()->addChild(mt);

		m_rpLightRootNode = new osg::Group;
		mt->addChild(m_rpLightRootNode);
		
		//RebuildLighting(FeHDRLighting::GLT_FILTER_CROSSSCREEN);


		/// 设置初始视角
		// 		p3DScenePanel->GetSystemManager()->GetSystemService()->GetManipulatorManager()->SetHome(
		// 			osgEarth::Viewpoint(120.30088, 23.501005, -179.93441,-42.789456, 1055.3453),1);
		// 		p3DScenePanel->GetSystemManager()->GetSystemService()->GetManipulatorManager()->Home();

		osgViewer::View* v = pRenderContext->GetView();
		if(v)
		{
			FeKit::CManipulatorManager* mm = dynamic_cast<FeKit::CManipulatorManager*>(v->getCameraManipulator());
			if(mm)
			{
				FeKit::FreeEarthManipulator* fe = mm->GetOrCreateFEManipulator();
				if(fe)
				{
					fe->setHomeViewpoint(osgEarth::Viewpoint("",120.726,22.03,10000,0,-89,0,pRenderContext->GetMapSRS()), 4);
					fe->home(0);
				}
			}
		}
		

		//开启光照效果
		osg::observer_ptr<FeShell::CEnvironmentSys> pFreeSkySys = m_opSystemMgr->GetSystemService()->GetEnvironmentSys();
		if (pFreeSkySys.valid())
		{
			FeKit::CFreeSky* pFreeSky = pFreeSkySys->GetSkyNode();
			if(pFreeSky)
			{
				pFreeSky->Show();
				pFreeSky->SetAmbient(osg::Vec4d(0.1, 0.1, 0.1, 0.1));
				pFreeSky->SetDateTime(osgEarth::DateTime(2016, 1, 1, 16.0));
			}
		}
	}

	return true;
}

void CHdrLightingWidget::RebuildLighting( FeEffect::EGLARELIBTYPE lightType )
{
	FeUtil::CRenderContext* pRenderContext = m_opSystemMgr->GetRenderContext();
	if(!pRenderContext) return;

	if(m_rpLightRootNode.valid())
	{
		m_rpLightRootNode->removeChildren(0, 100);
	}

	std::string modelName = FeFileReg->GetFullPath("model/airport.ive");
	m_rpLightModelNode = osgDB::readNodeFile(modelName);
	m_rpLightRootNode->addChild(m_rpLightModelNode);

	float minRadius = 8;
	float maxRadius = 18;
	float lightAlt = 90;

	//设置灯光参数
	FeEffect::HDRLightParam p;
	p.lightPow = 1.0;
	p.lightPos = osg::Vec3(0,0,lightAlt);
	p.lightRadius = minRadius;
	p.lightColor = osg::Vec3(1.0,1.0,1.0);

	//创建灯光节点
	m_rpHdrLightingNode = new FeEffect::FeHDRLightingNode(pRenderContext, m_rpLightModelNode, lightType);
	m_rpLightRootNode->addChild(m_rpHdrLightingNode);

	//设置灯光闪烁回调
	MyNodeCallback* cb = new MyNodeCallback();
	m_rpHdrLightingNode->setUpdateCallback(cb);

	//设置漫反射强度
	m_rpHdrLightingNode->setDiffuseCoefficient(1);
	//设置镜面光强度
	m_rpHdrLightingNode->setPhongCoefficient(1);
	//设置漫反射半径
	m_rpHdrLightingNode->setLightDiffRadius(100);

	//默认最大灯光数为10

	//设置灯光0
	m_rpHdrLightingNode->setLight(0,p);

	//设置灯光1
	p.lightRadius = maxRadius;
	p.lightPos = osg::Vec3(0,-400,lightAlt);
	m_rpHdrLightingNode->setLight(1,p);

	//设置灯光2
	p.lightColor = osg::Vec3(1.0,0.0,0.0);
	p.lightPos = osg::Vec3(200,0,lightAlt);
	p.lightRadius = minRadius;
	m_rpHdrLightingNode->setLight(2,p);

	//设置灯光3
	p.lightRadius = maxRadius;
	p.lightPos = osg::Vec3(200,-400,lightAlt);
	m_rpHdrLightingNode->setLight(3,p);

	//设置灯光4
	p.lightColor = osg::Vec3(0.0,1.0,0.0);
	p.lightPos = osg::Vec3(400,0,lightAlt);
	p.lightRadius = minRadius;
	m_rpHdrLightingNode->setLight(4,p);

	//设置灯光5
	p.lightRadius = maxRadius;
	p.lightPos = osg::Vec3(400,-400,lightAlt);
	m_rpHdrLightingNode->setLight(5,p);

	//设置灯光6
	p.lightColor = osg::Vec3(1.0,1.0,0.0);
	p.lightPos = osg::Vec3(600,0,lightAlt);
	p.lightRadius = minRadius;
	m_rpHdrLightingNode->setLight(6,p);

	//设置灯光7
	p.lightRadius = maxRadius;
	p.lightPos = osg::Vec3(600,-400,lightAlt);
	m_rpHdrLightingNode->setLight(7,p);
}

void CHdrLightingWidget::SlotLightTypeChanged( int nIndex)
{
	//OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
	if(nIndex >= 0)
	{
#ifdef WIN32
		RebuildLighting((FeEffect::EGLARELIBTYPE)nIndex);

		SlotLightPowChanged(ui.sun_light_Slider->value());
		SlotLightSizeChanged(ui.slider_Radius->value());
#else
        if(nIndex == 0 || nIndex == 2)
        {
            return ;
        }

        RebuildLighting((FeEffect::EGLARELIBTYPE)nIndex);

        SlotLightPowChanged(ui.sun_light_Slider->value());
        SlotLightSizeChanged(ui.slider_Radius->value());
#endif
	}
}

double CHdrLightingWidget::GetLightPow( int nSpliderNum )
{
	return 0.001 + nSpliderNum / 1000.0;
}

double CHdrLightingWidget::GetLightSize( int nSpliderNum )
{
	return 2.0 + 20.0 * (nSpliderNum / 255.0);
}

void CHdrLightingWidget::SlotLightPowChanged( int nIndex)
{
	//OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);

	if(!m_rpHdrLightingNode.valid()) return;

	float fPow = GetLightPow(nIndex);
	FeEffect::HDRLightParam p;
	
	for(int n = 1; n < 7; n++)
	{
		p = m_rpHdrLightingNode->getLight(n);
		p.lightPow = fPow;
		m_rpHdrLightingNode->setLight(n,p);
	}
}

void CHdrLightingWidget::SlotLightSizeChanged( int nIndex)
{
	//OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);

	if(!m_rpHdrLightingNode.valid()) return;

	float fPow = GetLightSize(nIndex);
	FeEffect::HDRLightParam p;

	for(int n = 1; n < 7; n++)
	{
		p = m_rpHdrLightingNode->getLight(n);
		p.lightRadius = fPow;
		m_rpHdrLightingNode->setLight(n,p);
	}
}

