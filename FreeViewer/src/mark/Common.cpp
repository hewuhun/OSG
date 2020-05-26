#include <mark/Common.h>

#include <mainWindow/FreeUtil.h>

namespace FreeViewer
{
	QString AsQString(double dvalue)
	{
		/*std::stringstream ss;
		ss <<dvalue;
		return QString(ss.str().c_str());*/
		QString strDnum;
		return strDnum.setNum(dvalue,'f',6);
	}

	QString AsQString(const osg::Vec3d& vec)
	{
		std::stringstream ss;
		ss << "(" <<vec.x()<<","<<vec.y()<<","<<vec.z()<<")";
		return QString(ss.str().c_str());
	}

	double AsDouble(const QString& str)
	{
		return osgEarth::as<double>(str.toStdString(), 0.0);
	}

	osg::Vec3 AsVec3(const QString& strLon, const QString& strLat, const QString& strHeight)
	{
		return osg::Vec3(AsDouble(strLon), AsDouble(strLat),AsDouble(strHeight));
	}

	osg::Vec3 AsVec3(const QString& qstrVec)
	{
		osg::Vec3 vecCen;
		QStringList tempList;
		QStringList qstrList = qstrVec.split(",",QString::SkipEmptyParts);

		if (qstrList.size() > 2)
		{
			tempList = qstrList[0].split("(",QString::SkipEmptyParts);
			vecCen[0] = AsDouble(tempList[0]);

			vecCen[1] = AsDouble(qstrList[1]);

			tempList = qstrList[2].split(")",QString::SkipEmptyParts);
			vecCen[2] = AsDouble(tempList[0]);
		}

		return vecCen;
	}

	bool AsVec3( const QString& qstrVec,osg::Vec3& vec )
	{
		QString temStr = qstrVec.trimmed();

		if (temStr[0] != '(' || temStr[temStr.size()-1] != ')')return false;

		osg::Vec3 vecCen;
		QStringList tempList;

		QStringList qstrList = qstrVec.split(",",QString::SkipEmptyParts);

		if (qstrList.size() != 3)return false;
		bool ok1,ok2,ok3;
		if (qstrList.size() > 2)
		{
			tempList = qstrList[0].split("(",QString::SkipEmptyParts);

			vecCen[0] = tempList[0].toDouble(&ok1);

			vecCen[1] = qstrList[1].toDouble(&ok2);

			tempList.clear();
			tempList = qstrList[2].split(")",QString::SkipEmptyParts);
			vecCen[2] = tempList[0].toDouble(&ok3);
		}

		vec = vecCen;

		return ok1 && ok2 && ok3;

	}

	bool AsVec3( const QString& strLon, const QString& strLat, const QString& strHeight,osg::Vec3& vec )
	{
		bool okLon,okLat,okH;
		vec = osg::Vec3(strLon.toDouble(&okLon),strLat.toDouble(&okLat),strHeight.toDouble(&okH));
		return okH & okLat & okLon;
	}

}

