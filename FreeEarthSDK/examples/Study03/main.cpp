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
	pWidget->SetTitle(QString::fromLocal8Bit("F35飞机"));

	FeEarth::C3DSceneWidget* p3DScenePanel = new FeEarth::C3DSceneWidget("");
	if (!p3DScenePanel->Initialize())
	{
		return -1;
	}

	p3DScenePanel->setMinimumSize(500, 500);
	pWidget->GetSceneLayout()->addWidget(p3DScenePanel);

	
	FeUtil::CRenderContext* pRenderContext = p3DScenePanel->GetSystemManager()->GetRenderContext();

	//地球Node
	osgEarth::MapNode* mapNode = pRenderContext->GetMapNode();
	/// 加载场景标记系统
	FeManager::CFreeMarkSys* pFreeMarkSys = new FeManager::CFreeMarkSys();
	pFreeMarkSys->Initialize(pRenderContext);
	p3DScenePanel->GetSystemManager()->GetSystemService()->AddAppModuleSys(pFreeMarkSys);
	//pFreeMarkSys->Start();

	/// 加载模型
	osg::Vec3d pos2(110.0, 34.0, 1000000.0);
	osg::Vec3d pos1(115.0, 34.0, 300.0);

	// 加载飞机模型
	osg::ref_ptr<FeExtNode::CExModelNodeOption> rpOP2 = new FeExtNode::CExModelNodeOption();
	rpOP2->name() = "123";
	//rpOP2->ModelPath() = FeFileReg->GetFullPath("model/weixing.ive");
	rpOP2->ModelPath() = FeFileReg->GetFullPath("D:/OSG/Demo/F35/h.obj");
	rpOP2->Position() = pos2;
	rpOP2->Scale() = osg::Vec3d(10000.0, 10000.0, 10000.0);
	osg::ref_ptr<FeExtNode::CExModelNode> rpNodeWinxing = new FeExtNode::CExModelNode(pRenderContext, rpOP2);
	//rpNodeWinxing->SetModelColor(osg::Vec4(1.f, 0.0f, 0.0f, 1.0f));
	pFreeMarkSys->AddMark(rpNodeWinxing);

	//osgDB::Options  *a = new osgDB::Options(std::string("noTriStripPolygons"));
	//osg::Node * node = osgDB::readNodeFile("D:/OSG/Demo/F35/h.obj", a);
	

	
	/// 设置初始视角
	p3DScenePanel->GetSystemManager()->GetSystemService()->GetManipulatorManager()->SetHome(
		osgEarth::Viewpoint("", 90, 34, 7110080.968, 0, -89, 0), 0);
	p3DScenePanel->GetSystemManager()->GetSystemService()->GetManipulatorManager()->Home();

	
	
	pWidget->SetControlWidgetSize(QSize(1,1));
	pWidget->ShowDialogNormal();
	pWidget->ShowDialogMaxisize();

	int nFlag = a.exec();

	return nFlag;
}


