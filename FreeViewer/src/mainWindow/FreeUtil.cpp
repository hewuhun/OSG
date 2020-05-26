#include <mainWindow/FreeUtil.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include "qtextcodec.h"

namespace FreeViewer
{

	extern int GetPrimaryScreenIndex()
	{
		return QApplication::desktop()->primaryScreen();
	}

	QRect GetAvailableGeometry(int nIndex)
	{
		QDesktopWidget* pDesktopWidget = QApplication::desktop();
		return pDesktopWidget->availableGeometry(nIndex);
	}

	QRect GetScreenGeometry(int nIndex)
	{
		QDesktopWidget* pDesktopWidget = QApplication::desktop();
		return pDesktopWidget->screenGeometry(nIndex);
	}

	void SetStyleSheet( QWidget* pWidget, QString strQSS )
	{
		if (NULL == pWidget)
		{
			return ;
		}

		QFile qss(strQSS);
		qss.open(QFile::ReadOnly);
		pWidget->setStyleSheet(qss.readAll());
		qss.close();
	}

	extern QString GB2312ToUTF8( std::string strGB2312 )
	{
		QTextCodec* pUTF8Codec= QTextCodec::codecForName("utf-8");
		QTextCodec* pGB2312Codec = QTextCodec::codecForName("gb2312");

		if (pUTF8Codec && pGB2312Codec)
		{
			QString strUnicode= pGB2312Codec->toUnicode(strGB2312.c_str());
			QByteArray byteUTF8= pUTF8Codec->fromUnicode(strUnicode);

			return QString(byteUTF8.data());
		}

		return QString(strGB2312.c_str());
	}

	extern QString UTF8ToGB2312( std::string strUTF8 )
	{
		QTextCodec* pUTF8Codec= QTextCodec::codecForName("utf-8");
		QTextCodec* pGB2312Codec = QTextCodec::codecForName("gb2312");

		if (pUTF8Codec && pGB2312Codec)
		{
			QString strUnicode= pUTF8Codec->toUnicode(strUTF8.c_str());
			QByteArray byteGB2312= pGB2312Codec->fromUnicode(strUnicode);

			return QString(byteGB2312.data());
		}

		return QString(strUTF8.c_str());
    }

    QString ConvertToCurrentEncoding(std::string str)
    {
#ifdef WIN32
#ifdef __QT4_USE_UNICODE
        return UTF8ToGB2312(str);
#else
		return QString(str.c_str());
#endif
#else
        return QString(str.c_str());
#endif
    }

	extern std::string ConvertToSDKEncoding( QString str )
	{
		/// 目前SDK中使用的字体编码格式为UTF8
		return str.toUtf8().data();
	}

	extern void FormatPathString( QString& str )
	{
		str.replace("\\", "/");
		str.replace("//", "/");
	}

	extern QString GetMeasureFormatString( double dVal, double multiNum, const std::string& unit1, const std::string& unit2, double baseValid )
	{
		const double dValFormat = dVal / multiNum;
		std::string strF = "%1 ";
		strF += (dValFormat > baseValid)?unit2:unit1;
		return QString(strF.c_str()).arg(((dValFormat > baseValid)?dValFormat:dVal), 0, 'f');
	}

	extern QValidator* PutInNumberAndLetterAndCN()
	{
		QRegExp regx("^[\\w]+$");
		QValidator *pValidator = new QRegExpValidator(regx);

		return pValidator;
	}

	extern QValidator* PutInNoEmpty()
	{
		QRegExp regx("^[^\\s]+$");
		QValidator *pValidator = new QRegExpValidator(regx);

		return pValidator;
	}

	extern QString GetStringNoEmpty(QString str)
	{
		QRegExp regx("^[^\\s]+$");
 		QValidator *pValidator = new QRegExpValidator(regx);
		int mode = 0;
        QString strRight = str.right(1);
        if (0 == pValidator->validate(strRight, mode))
        {
            str = str.left(str.length()-1);
        }
		return str;
	}

	extern QValidator* PutInNumberAndLetter()
	{
		QRegExp regx("^[a-zA-Z0-9]+$");
		QValidator *pValidator = new QRegExpValidator(regx);

		return pValidator;
	}

	extern QValidator* PutInZeroToOneHundred()
	{
		QRegExp regx("^(\\d|[1-9]\\d|100)$");
		QValidator *pValidator = new QRegExpValidator(regx);

		return pValidator;
	}

	extern QValidator* PutInScale()
	{
		QRegExp regx("^[0-9:]+$");
		QValidator *pValidator = new QRegExpValidator(regx);

		return pValidator;
	}

}
