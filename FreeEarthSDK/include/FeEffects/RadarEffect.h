#ifndef CRADAR_EFFECT_H
#define CRADAR_EFFECT_H

#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <FeEffects/Export.h>

namespace FeEffect
{
class FEEFFECTS_EXPORT CRadarEffect :
    public osg::Group
{
public:
    CRadarEffect(double radius, osg::Vec4d color, bool grid=true, float radio = 0.001);
    ~CRadarEffect(void);

    void updateRadius(double radius);

	osg::Geometry* genCircle(double radius, osg::Vec4d color = osg::Vec4d(1.0,1.0,1.0,1.0));

    osg::ref_ptr<osg::Sphere> _pSphere;
    double _radius;
};
}
#endif

