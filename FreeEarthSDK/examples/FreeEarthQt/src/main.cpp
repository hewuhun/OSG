//
#include <QTranslator>
#include <QLocale>
#include <QApplication>
#include <QCoreApplication>
#include <QDesktopWidget>


#include <FeUtils/logger/LoggerDef.h>
#include <FeUtils/PathRegistry.h>
#include <FeUtils/EnvironmentVariableReader.h>
#include <FeUtils/SystemInfo.h>
#include <FeShell/LanSerializer.h>

#include <FreeMainWindow.h>

#include <QTextCodec>

void SetTextCode(const std::string& strTextCode)
{
	
#ifdef QT4_VERSION
	QTextCodec *codec = QTextCodec::codecForName(strTextCode.c_str());
	QTextCodec::setCodecForLocale(codec);
 	QTextCodec::setCodecForCStrings(codec);
 	QTextCodec::setCodecForTr(codec);
#endif
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	SetTextCode("UTF-8");

	//FeUtil::CSystemInfo::EDisplayCardType type = FeUtil::CSystemInfo::Instance()->GetDisplayCardType();

	CFreeMainWindow mainWindow ;
	if(mainWindow.Initialize())
	{
		mainWindow.setGeometry(QApplication::desktop()->width()/4, QApplication::desktop()->height()/4,QApplication::desktop()->width()/2, QApplication::desktop()->height()/2);
		mainWindow.showMaximized();
	}

	int nFlag = a.exec();
	mainWindow.UnInitialize();

	return nFlag;
}


