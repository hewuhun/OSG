#include <QApplication>
#include <QMainWindow>
#include <QTextCodec>

#include <FeEarth/3DSceneWidget.h>
#include <FeUtils/PathRegistry.h>
#include <FeUtils/EnvironmentVariableReader.h>
#include <FeOcean/OceanSys.h>

#include <FreeMainWindow.h>

#include "OceanWidget.h"

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
	pWidget->SetTitle(QString::fromLocal8Bit("海洋模拟效果演示"));

	FeUtil::CEnvironmentVariableReader reader;
	FeUtil::CFileRegInitAgent fileReg;
	fileReg.InitDataPath(reader.DoRead("FREE_EARTH_FILE_PATH"));

	FeEarth::C3DSceneWidget* p3DScenePanel = new FeEarth::C3DSceneWidget("");
	p3DScenePanel->SetEarthPath(FeFileReg->GetFullPath("earth/FreeEarth_flat_Ocean.earth"));
	if(!p3DScenePanel->Initialize())
	{
		return -1;
	}

	p3DScenePanel->setMinimumSize(500, 500);
	pWidget->GetSceneLayout()->addWidget(p3DScenePanel);

	/// 设置界面内容
	{
		CFreeLableWidget* pLabelWidget = new CFreeLableWidget();
		pLabelWidget->SetTitleText(QString::fromLocal8Bit("功能介绍"));
		pLabelWidget->SetContentText(QString::fromLocal8Bit(
			"    海洋效果模拟了多种真实的效果，包括："
			"轮船尾迹效果，直升机旋涡效果，海面光"
			"线效果，以及海底效果等。\n"));
		pWidget->AddControlWidget(pLabelWidget);
	}
	{
		CFreeLableWidget* pLabelWidget = new CFreeLableWidget();
		pLabelWidget->SetTitleText(QString::fromLocal8Bit("控制面板"));
		pWidget->AddControlWidget(pLabelWidget);
	}

	pWidget->AddControlWidget(new FeOceanDemo::COceanWidget(p3DScenePanel->GetSystemManager()));
	pWidget->SetControlWidgetSize(QSize(300, 600));

	pWidget->ShowDialogNormal();
	pWidget->ShowDialogMaxisize();

	int nFlag = a.exec();

	return nFlag;
}

