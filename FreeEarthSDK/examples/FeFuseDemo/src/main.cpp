#include <QApplication>
#include <QMainWindow>
#include <QTextCodec>

#include <FeEarth/3DSceneWidget.h>
#include <FeEarth/FreeEarth.h>
#include <FeManager/FreeMarkSys.h>

#include <FreeMainWindow.h>

#include "FreeControlPan.h"

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/StateSetManipulator>
int main(int argc, char *argv[])
{
 
	/*
	osgViewer::Viewer viewer;
	viewer.setSceneData(osgDB::readNodeFile("D:\\OSG\\OSG_3.4.0\\data\\cow.osg"));
	//添加状态事件
	  viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
	//窗口大小变化事件
	  viewer.addEventHandler(new osgViewer::WindowSizeHandler);
	//添加一些常用状态设置
	  viewer.addEventHandler(new osgViewer::StatsHandler);
	viewer.realize();
	viewer.run();
	*/

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
	//申请一个主窗口
	CFreeMainWindow* pWidget = new CFreeMainWindow();
	//主窗口标题为“”
	pWidget->SetTitle(QString::fromLocal8Bit("董康乐-地形融合效果演示"));

	FeEarth::C3DSceneWidget* p3DScenePanel = new FeEarth::C3DSceneWidget("");
	if(!p3DScenePanel->Initialize())
	{
		return -1;
	}

	pWidget->GetSceneLayout()->addWidget(p3DScenePanel);

	FeShell::CSystemManager* pSystemManager = p3DScenePanel->GetSystemManager();
	if (pSystemManager)
	{
		FeManager::CFreeMarkSys* pFreeMarkSys = new FeManager::CFreeMarkSys();
		pFreeMarkSys->Initialize(pSystemManager->GetRenderContext());
		pSystemManager->GetSystemService()->AddAppModuleSys(pFreeMarkSys);
		CFreeControlPan* pControlPan = new CFreeControlPan(pSystemManager, pWidget);
		{
			CFreeLableWidget* pLabelWidget = new CFreeLableWidget(pWidget);
			pLabelWidget->SetTitleText(QString::fromLocal8Bit("功能说明"));
			pLabelWidget->SetContentText(QString::fromLocal8Bit(
				"    地形融合效果主要用于地表模型建筑等"
				"实体，放置在地表时，可将地形平整，表现"
				"出的与地形融为一体的效果。\n"
				"    当前示例程序中，使用两个相同的机场"
				"模型，一个模型使用了地形融合技术，另一个模"
				"型未经处理，用户可对比显示效果。\n"));
			pWidget->AddControlWidget(pLabelWidget);
		}

		{
			CFreeLableWidget* pLabelWidget = new CFreeLableWidget(pWidget);
			pLabelWidget->SetTitleText(QString::fromLocal8Bit("操作面板"));
			pWidget->AddControlWidget(pLabelWidget);
		}

		pWidget->AddControlWidget(pControlPan);
		pWidget->ShowDialogNormal();
		pWidget->ShowDialogMaxisize();

		int nFlag = a.exec();

		return nFlag;
	}

	return 0;
}