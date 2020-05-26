//
#include <QTranslator>
#include <QLocale>
#include <QApplication>
#include <QCoreApplication>
#include <QDesktopWidget>
#include <QLayout>
#include <QTextCodec>

#include <FeUtils/logger/LoggerDef.h>
#include <FeUtils/PathRegistry.h>
#include <FeUtils/EnvironmentVariableReader.h>
#include <FeShell/LanSerializer.h>

#include <mainWindow/FreeMainWindow.h>

void SetTextCode(const std::string& strTextCode)
{
#ifdef __QT4_USE_UNICODE
	QTextCodec *codec = QTextCodec::codecForName(strTextCode.c_str());
	QTextCodec::setCodecForLocale(codec);
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForTr(codec);
#endif // __QT4_USE_UNICODE
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	
#ifdef WIN32
	SetTextCode("GB2312");
#else
	SetTextCode("UTF-8");
#endif // WIN32

	//读取环境变量或者本地数据的路径
	FeUtil::CEnvironmentVariableReader reader;
	FeUtil::CFileRegInitAgent agent;
#if 1
	agent.InitDataPath(reader.DoRead("FREE_EARTH_FILE_PATH"));
#else
	QString qStrDataPath = QCoreApplication::applicationDirPath();
	std::string strDatapath = qStrDataPath.toStdString();
	agent.InitDataPath(strDatapath + "/Data");
#endif

	//加载语言列表
	QTranslator *translator = new QTranslator;
	QTranslator *translatorSys = new QTranslator;
	FeShell::CLanConfigReader load;

	FeShell::CLanConfig lanConfig = load.Execute(FeFileReg->GetFullPath("apps/language/lanConfig.xml"));
	FeShell::CLanConfig::LanMap lanMap = lanConfig.GetLanguageList();

	//获取默认的语言的加载文件
	FeShell::CLanConfig::LanMap::iterator it = lanMap.find(lanConfig.GetDefLanguage());
	if (it != lanMap.end())
	{
		std::string strValue = it->second;
		QString strLanFile = QString("apps/language/%1.qm").arg(strValue.c_str());

		//加载语言翻译文件
		translator->load(FeUtil::CFileReg::Instance()->GetFullPath(strLanFile.toStdString()).c_str());
		translatorSys->load(FeUtil::CFileReg::Instance()->GetFullPath("apps/language/qt_zh_CN.qm").c_str());
		QApplication::installTranslator(translator);
		QApplication::installTranslator(translatorSys);
	}

	FreeViewer::CFreeMainWindow mainWindow;
	mainWindow.setGeometry(
		QApplication::desktop()->width()/4, 
		QApplication::desktop()->height()/4,
		QApplication::desktop()->width()/2, 
		QApplication::desktop()->height()/2);

	mainWindow.ShowDialogMaxisize();

	return a.exec();
}


