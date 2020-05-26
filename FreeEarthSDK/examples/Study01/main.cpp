#include <FeEarth/3DSceneWidget.h>

#include <QApplication>
#include <QMainWindow>
#include <QTextCodec>
#include <FreeSampleFrame/include/FreeMainWindow.h>

#include <FeUtils/PathRegistry.h>
#include <FeUtils/CoordConverter.h>
#include <FeEffects/MutableCylinder.h>
#include <FeEffects/ScanRadarEffect.h>
#include <FeEffects/Scan.h>

#include <FeManager/FreeMarkSys.h>
#include <FeExtNode/ExModelNode.h>


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QTextCodec *codec = NULL;
#ifdef WIN32
	codec = QTextCodec::codecForName("GBK");
#else
	codec = QTextCodec::codecForName("UTF-8");
#endif 
	if (codec)
	{
#ifdef QT4_VERSION
		QTextCodec::setCodecForLocale(codec);
		QTextCodec::setCodecForCStrings(codec);
		QTextCodec::setCodecForTr(codec);
#endif
	}

	CFreeMainWindow* pWidget = new CFreeMainWindow();
	pWidget->SetTitle(QString::fromLocal8Bit("电磁波形模拟效果演示"));

	FeEarth::C3DSceneWidget* p3DScenePanel = new FeEarth::C3DSceneWidget("");
	if(!p3DScenePanel->Initialize())
	{
		return -1;
	}

	p3DScenePanel->setMinimumSize(500, 500);
	pWidget->GetSceneLayout()->addWidget(p3DScenePanel);

	/// 加载场景标记系统
	FeUtil::CRenderContext* pRenderContext = p3DScenePanel->GetSystemManager()->GetRenderContext();
	FeManager::CFreeMarkSys* pFreeMarkSys = new FeManager::CFreeMarkSys();
	pFreeMarkSys->Initialize(pRenderContext);
	p3DScenePanel->GetSystemManager()->GetSystemService()->AddAppModuleSys(pFreeMarkSys);
	//pFreeMarkSys->Start();

	/// 加载模型
	osg::Vec3d pos2(110.0,34.0,1000000.0);
	osg::Vec3d pos1(115.0,34.0,300.0);

	// 加载卫星模型
	osg::ref_ptr<FeExtNode::CExModelNodeOption> rpOP2 = new FeExtNode::CExModelNodeOption();
	rpOP2->name() = "123";
	rpOP2->ModelPath() = FeFileReg->GetFullPath("model/weixing.ive");
	rpOP2->Position() = pos2;
	rpOP2->Scale() = osg::Vec3d(10000.0,10000.0,10000.0);
	osg::ref_ptr<FeExtNode::CExModelNode> rpNodeWinxing = new FeExtNode::CExModelNode(pRenderContext,rpOP2);
	pFreeMarkSys->AddMark(rpNodeWinxing);

	/// 加载雷达效果节点
	osg::ref_ptr<FeEffect::CScanRadarEffect> rpScanRadar = new FeEffect::CScanRadarEffect(pRenderContext,
		osg::Vec3d(0.0,0.0,0.0),200000,-40.0,-140.0,0,60.0,osg::Vec4d(0.0,1.0,1.0,0.8),FeEffect::STATIC_SCANTYPE,5.0);
	osg::ref_ptr<FeExtNode::CExModelNode> rpNodeRadar = new FeExtNode::CExModelNode(pRenderContext,new FeExtNode::CExModelNodeOption());
	rpNodeRadar->SetPosition(pos1);
	rpNodeRadar->GetSceneNode()->addChild(rpScanRadar);
	pFreeMarkSys->AddMark(rpNodeRadar);

	/// 加载卫星与雷达之间的通讯波节点
	osg::Vec3d vecXYZ1;
	osg::Vec3d vecXYZ2;
	FeUtil::DegreeLLH2XYZ(pRenderContext,pos1,vecXYZ1);
	FeUtil::DegreeLLH2XYZ(pRenderContext,pos2,vecXYZ2);
	osg::ref_ptr<FeEffect::CMutableCylinder> rpLink = new FeEffect::CMutableCylinder(vecXYZ2,vecXYZ1,300,100000);
	rpLink->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);
	pRenderContext->GetRoot()->addChild(rpLink);

	/// 加载扫描波节点
	osg::ref_ptr<FeEffect::CScan > rpScan = new FeEffect::CScan(vecXYZ2);
	pRenderContext->GetRoot()->addChild(rpScan);

	/// 加载圆锥扫描波
	osg::ref_ptr<FeEffect::TestNode> tn = new FeEffect::TestNode(1000000,osg::DegreesToRadians(6.0f));
	tn->setMoveSpeed(120000);
	tn->setWaveColor(osg::Vec3d(1.0,0.5,0.0));
	osg::ref_ptr<FeExtNode::CExModelNode> rpNodeCone = new FeExtNode::CExModelNode(pRenderContext,new FeExtNode::CExModelNodeOption());
	rpNodeCone->SetPosition(osg::Vec3d(105.0,34.0,10000));
	rpNodeCone->GetSceneNode()->addChild(tn);
	pFreeMarkSys->AddMark(rpNodeCone);

	/// 加载棱锥扫描波
	osg::ref_ptr<FeEffect::PyramidNode> pn = new FeEffect::PyramidNode(1000000,osg::DegreesToRadians(30.0f),osg::DegreesToRadians(20.0f));
	osg::ref_ptr<FeExtNode::CExModelNode> rpNodePyramid = new FeExtNode::CExModelNode(pRenderContext,new FeExtNode::CExModelNodeOption());
	rpNodePyramid->SetPosition(osg::Vec3d(100.0,34.0,1000000));
	rpNodePyramid->GetSceneNode()->addChild(pn);
	pFreeMarkSys->AddMark(rpNodePyramid);

	/// 加载搜索屏
	const float ssRadius = 800000;
	const float sse = 60;
	float ssa = 0;

	FeEffect::SearchScreenPointList sspl;
	for (int i = 0;i < 100;i++)
	{
		FeEffect::SearchScreenPoint ssp(ssRadius,osg::DegreesToRadians(ssa),DegreesToRadians(sse));
		sspl.push_back(ssp);

		ssa += 1.8;
	}
	osg::ref_ptr<FeEffect::SearchScreen> ss = new FeEffect::SearchScreen(pRenderContext,sspl,osg::Vec3d(95.0,34.0,10000));
	pRenderContext->GetRoot()->addChild(ss);

	ssa = 0;
	FeEffect::SearchScreenPointList sspl1;
	for (int i = 0;i < 50;i++)
	{
		float ie = sse;
		if(i % 2 == 0)
		{
			ie += 5;
		}

		FeEffect::SearchScreenPoint ssp1(ssRadius,osg::DegreesToRadians(ssa),DegreesToRadians(ie));
		sspl1.push_back(ssp1);

		ssa += 3.6;
	}
	osg::ref_ptr<FeEffect::SearchScreen> ss1 = new FeEffect::SearchScreen(pRenderContext,sspl1,osg::Vec3d(90.0,34.0,10000));
	pRenderContext->GetRoot()->addChild(ss1);

	/// 加载误差管道
	osg::Vec3d plPos(85.0,34.0,1000000);
	float plMaxRadius = 100;
	float lonAdd = -0.1;
	float latAdd = -0.04;
	float altAdd = -10000;
	float rAdd = 1000;
	FeEffect::PipeLine::CenterLineList cl;
	for (int i = 0;i < 100;i++)
	{
		osg::Vec4 cp(plPos.x() + i * lonAdd,plPos.y() + i * latAdd,plPos.z() + i * altAdd,plMaxRadius+ i * rAdd);
		cl.push_back(cp);
	}
	osg::ref_ptr<FeEffect::PipeLine> pl = new FeEffect::PipeLine(cl,pRenderContext);
	pRenderContext->GetRoot()->addChild(pl);


	osg::Vec3d plPos1(85.0,34.0,1000000);
	float plMaxRadius1 = 100;
	float lonAdd1 = -0.1;
	float latAdd1 = 0.04;
	float altAdd1 = -10000;
	float rAdd1 = 1000;
	FeEffect::PipeLine::CenterLineList cl1;
	for (int i = 0;i < 100;i++)
	{
		osg::Vec4 cp(plPos1.x() + i * lonAdd1,plPos1.y() + i * latAdd1,plPos1.z() + i * altAdd1,plMaxRadius1+ i * rAdd1);
		cl1.push_back(cp);
	}
	osg::ref_ptr<FeEffect::PipeLine> pl1 = new FeEffect::PipeLine(cl1,pRenderContext);
	pl1->setOutLineVisible(false);
	pl1->setDiffColor(osg::Vec3(1.0,0.0,0.0));
	pl1->setPipeColor(osg::Vec4(1.0,1.0,0.0,0.5));
	pl1->setPolygonMode(osg::PolygonMode::LINE);

	pRenderContext->GetRoot()->addChild(pl1);

	/// 设置初始视角
	p3DScenePanel->GetSystemManager()->GetSystemService()->GetManipulatorManager()->SetHome(
		osgEarth::Viewpoint("", 90, 34, 7110080.968, 0, -89, 0), 0);
	p3DScenePanel->GetSystemManager()->GetSystemService()->GetManipulatorManager()->Home();

	/// 设置界面内容
	{
		CFreeLableWidget* pLabelWidget = new CFreeLableWidget();
		pLabelWidget->SetTitleText(QString::fromLocal8Bit("功能介绍"));
		pLabelWidget->SetContentText(QString::fromLocal8Bit(
			"    本例程序演示了卫星扫描波，通讯波，"
			"卫星载荷，雷达波，雷达搜索屏，导弹误差"
			"管道等显示效果。\n"
			));
		pWidget->AddControlWidget(pLabelWidget);
	}
	pWidget->AddControlItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

	pWidget->ShowDialogNormal();
	pWidget->ShowDialogMaxisize();

	int nFlag = a.exec();

	return nFlag;
}


