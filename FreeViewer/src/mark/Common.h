#ifndef SCENE_WIDGET_COMMON_H
#define SCENE_WIDGET_COMMON_H

#include <string>
#include <sstream>

#include <QString>
#include <QStringList>
#include <osg/Vec3d>
#include <osg/Vec3>
#include <osgEarth/StringUtils>

namespace FreeViewer
{
	QString AsQString(double dvalue);

	QString AsQString(const osg::Vec3d& vec);

	double AsDouble(const QString& str);

	osg::Vec3 AsVec3(const QString& qstrVec);

	bool AsVec3(const QString& qstrVec,osg::Vec3&  vec);

	osg::Vec3 AsVec3(const QString& strLon, const QString& strLat, const QString& strHeight);

	bool AsVec3(const QString& strLon, const QString& strLat, const QString& strHeight,osg::Vec3& vec);
}


#endif // SCENE_WIDGET_COMMON_H

