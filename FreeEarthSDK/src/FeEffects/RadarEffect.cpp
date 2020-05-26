#include <FeEffects/RadarEffect.h>

#include <osg/PolygonMode>
#include <osg/BlendColor>
#include <osg/BlendFunc>
#include <osg/MatrixTransform>
#include <osg/AnimationPath>
#include <osg/LineWidth>
#include <osg/Image>

#include <osgDB/ReadFile>

#include <iostream>
#include <osgEarthAnnotation/ImageOverlay>

namespace FeEffect
{

CRadarEffect::CRadarEffect(double radius,  osg::Vec4d color, bool grid, float radio)
{
	
    _radius = radius;

    osg::Geode* gnode = new osg::Geode;

    osg::ref_ptr<osg::TessellationHints> hints = new osg::TessellationHints;
    hints->setDetailRatio(0.6f);

	if(grid == true)
	{
		_pSphere = new osg::Sphere(osg::Vec3d(0.0,0.0,0.0), radius);
	}
	else
	{
		_pSphere = new osg::Sphere(osg::Vec3d(0.0,0.0,0.0), 5000);
	}

    osg::ShapeDrawable* sd = new osg::ShapeDrawable(_pSphere, hints.get());
    sd->setColor(color);
    sd->dirtyBound();
    sd->dirtyDisplayList();
    gnode->addDrawable(sd);

    this->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    this->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
    this->getOrCreateStateSet()->setAttribute(new osg::BlendColor(), osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
    this->getOrCreateStateSet()->setAttribute(new osg::BlendFunc(), osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

    if(true == grid)
    {
        osg::ref_ptr<osg::PolygonMode> polymode = new osg::PolygonMode;
        polymode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
        gnode->getOrCreateStateSet()->setAttributeAndModes(polymode,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    }

    osg::AnimationPath* animationPath = new osg::AnimationPath;
    animationPath->setLoopMode(osg::AnimationPath::LOOP);

    double time = 0.0;
    for(int i = 0; i<90; i++)
    {
        osg::Quat rotation(osg::Quat(osg::DegreesToRadians(i*2.0),osg::Vec3(1.0,0.0,1.0)));
        osg::Vec3d scale = osg::Vec3d(3.0*sin(osg::DegreesToRadians(i*1.0)), 3.0*sin(osg::DegreesToRadians(i*1.0)), 3.0*sin(osg::DegreesToRadians(i*1.0)));

        animationPath->insert(time,osg::AnimationPath::ControlPoint(osg::Vec3(0.0, 0.0, 0.0),rotation,scale));
        time = time  + 0.01 + i*radio;
    }

    osg::ref_ptr<osg::MatrixTransform> animation0 = new osg::MatrixTransform;
    animation0->addChild(gnode);
    animation0->setUpdateCallback(new osg::AnimationPathCallback(animationPath, 0.0, 1.0));
    addChild(animation0.get());

    osg::ref_ptr<osg::MatrixTransform> animation1 = new osg::MatrixTransform;
    animation1->addChild(gnode);
    animation1->setUpdateCallback(new osg::AnimationPathCallback(animationPath, 1.0, 1.0));
    addChild(animation1.get());

    osg::ref_ptr<osg::MatrixTransform> animation2 = new osg::MatrixTransform;
    animation2->addChild(gnode);
    animation2->setUpdateCallback(new osg::AnimationPathCallback(animationPath, 2.0, 1.0));
    addChild(animation2.get());

    osg::ref_ptr<osg::MatrixTransform> animation3 = new osg::MatrixTransform;
    animation3->addChild(gnode);
    animation3->setUpdateCallback(new osg::AnimationPathCallback(animationPath, 3.0, 1.0));
    addChild(animation3.get());
}


CRadarEffect::~CRadarEffect(void)
{
}

void CRadarEffect::updateRadius( double radius )
{
    _radius = radius;
    _pSphere->setRadius(radius);
}

osg::Geometry* CRadarEffect::genCircle( double radius, osg::Vec4d color /*= osg::Vec4d(1.0,1.0,1.0,1.0)*/ )
{
	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
	osg::Vec3dArray* vertexes = new osg::Vec3dArray;
	for (int i=0; i<36; i++)
	{
		vertexes->push_back(osg::Vec3d(cos(osg::DegreesToRadians(i*10.0)) * radius, radius*sin(osg::DegreesToRadians(i*10.0)), 0));
	}
	geom->setVertexArray(vertexes);
	osg::Vec4dArray* colorArray = new osg::Vec4dArray;
	colorArray->push_back(color);
	geom->setColorArray(colorArray);
	geom->setColorBinding(osg::Geometry::BIND_OVERALL);
	geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, vertexes->size()));

	return  geom.release();
}

}