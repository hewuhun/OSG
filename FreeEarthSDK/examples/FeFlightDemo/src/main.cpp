#include <QApplication>
#include <QMainWindow>
#include <QTextCodec>

#include <FeUtils/PathRegistry.h>
#include <FeEarth/3DSceneWidget.h>

#include <FeEarth/3DSceneWidget.h>

#include <FreeMainWindow.h>
#include <FlightWidget.h>



int main(int argc, char *argv[])
{
	//This file is part of the QtGui module of the Qt Toolkit.
	QApplication a(argc, argv);

	//此文件是Qt工具包的QtCore模块的一部分
	/*
	QtCore模块是所有其它Qt模块的基础，包含以下核心功能：
	Qt Data Types：数据类型
	Qt Object Model：对象模型（包括元对象模型、属性系统、信号与槽机制、对象树）
	Input/Output Classes：I/O读写
	Threading and Concurrent Programming：多线程和并发编程
	QCoreApplication：应用程序类
	Event System ：事件系统
	Container Classes：容器类
	Animation Framework：动画框架
	Qt Plugins：插件类
	State Machine Framework：状态机
	JSON Support：JSON读写
	Qt Resource System： 资源系统
	Implicit Sharing：隐性共享
	*/
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

	//FreeEarth中Demo使用的主框架，其中可以嵌入三维渲染窗口，以及控制面板
	CFreeMainWindow* pWidget = new CFreeMainWindow();
	//设置标题
	pWidget->SetTitle(QString::fromLocal8Bit("董康乐--飞行仿真效果演示"));


	//三维渲染窗口
	FeEarth::C3DSceneWidget* p3DScenePanel = new FeEarth::C3DSceneWidget("");
	//初始化
	if(!p3DScenePanel->Initialize())
	{
		return -1;
	}
	//设置大小
	p3DScenePanel->setMinimumSize(500, 500);

	pWidget->GetSceneLayout()->addWidget(p3DScenePanel);


	/// 设置界面内容
	{
		CFreeLableWidget* pLabelWidget = new CFreeLableWidget();
		pLabelWidget->SetTitleText(QString::fromLocal8Bit("董康乐-功能介绍"));
		
		pLabelWidget->SetContentText(QString::fromLocal8Bit(
			" 董康乐---本例中展示了飞机飞行的整个过程，包括:"
			"起飞、收放起落架，穿云层和降落等效果。并"
			"提供了驾驶舱视角，锁定视角和自由视角供"
			"用户自由切换。\n"));
		pWidget->AddControlWidget(pLabelWidget);
	}

	////按键操作
	{
		CFreeLableWidget* pLabelWidget = new CFreeLableWidget();
		pLabelWidget->SetTitleText(QString::fromLocal8Bit("董康乐-控制面板"));
		pWidget->AddControlWidget(pLabelWidget);
	}


	pWidget->AddControlWidget(new CFlightWidget(p3DScenePanel->GetSystemManager()));
	pWidget->SetControlWidgetSize(QSize(300, 650));//QSize(int w, int h)
	pWidget->ShowDialogNormal();
	pWidget->ShowDialogMaxisize();
	//调用exec()来显示模式对话框
	int nFlag = a.exec();

	return nFlag;

	return 0;
}