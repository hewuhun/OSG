#include <QApplication>
#include <QMainWindow>
#include <QTextCodec>
#include <FreeSampleFrame/include/FreeMainWindow.h>
#include <HdrLightingWidget.h>

#include <FeEarth/3DSceneWidget.h>

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
	pWidget->SetTitle(QString::fromLocal8Bit("HDR环境灯光效果演示"));
	FeEarth::C3DSceneWidget* p3DScenePanel = new FeEarth::C3DSceneWidget("");
	//p3DScenePanel->Initialize();

	if(!p3DScenePanel->Initialize())//初始化成功，返回值为false
	{
		return -1;
	}

	pWidget->ShowDialogNormal();
	pWidget->ShowDialogMaxisize();
	p3DScenePanel->setMinimumSize(500, 500);
	pWidget->GetSceneLayout()->addWidget(p3DScenePanel);


	/// 设置界面内容
	{
		CFreeLableWidget* pLabelWidget = new CFreeLableWidget();
		pLabelWidget->SetTitleText(QString::fromLocal8Bit("功能介绍"));
		pLabelWidget->SetContentText(QString::fromLocal8Bit(
			"    环境灯光是一种可以影响到其周围环境的灯光效果。\n"
			"    本例程序提供了多种类型环境灯光的效果演示。\n")
			);
		pWidget->AddControlWidget(pLabelWidget);
	}
	{
		CFreeLableWidget* pLabelWidget = new CFreeLableWidget();
		pLabelWidget->SetTitleText(QString::fromLocal8Bit("控制面板"));
		pWidget->AddControlWidget(pLabelWidget);
	}
	
	pWidget->AddControlWidget(new CHdrLightingWidget(p3DScenePanel->GetSystemManager()));

	pWidget->ShowDialogNormal();
	pWidget->ShowDialogMaxisize();
	int nFlag = a.exec();

	return nFlag;
}