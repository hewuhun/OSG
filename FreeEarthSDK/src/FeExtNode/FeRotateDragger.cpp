#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/PolygonMode>
#include <osg/CullFace>
#include <osg/Quat>
#include <osgViewer/View>
#include <FeRotateDragger.h>

using namespace osgManipulator;


namespace FeNodeEditor
{
	CExRotateCylinderDragger::CExRotateCylinderDragger()
	{
		_pickCylinderHeight = 0.3f;
	}

	osg::Geometry* createCircleGeometry(float radius, unsigned int numSegments)
	{
		const float angleDelta = 2.0f*osg::PI/(float)numSegments;
		const float r = radius;
		float angle = 0.0f;
		osg::Vec3Array* vertexArray = new osg::Vec3Array(numSegments);
		osg::Vec3Array* normalArray = new osg::Vec3Array(numSegments);
		for(unsigned int i = 0; i < numSegments; ++i,angle+=angleDelta)
		{
			float c = cosf(angle);
			float s = sinf(angle);
			(*vertexArray)[i].set(c*r,s*r,0.0f);
			(*normalArray)[i].set(c,s,0.0f);
		}
		osg::Geometry* geometry = new osg::Geometry();
		geometry->setVertexArray(vertexArray);
		geometry->setNormalArray(normalArray, osg::Array::BIND_PER_VERTEX);
		geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP,0,vertexArray->size()));
		return geometry;
	}

	void CExRotateCylinderDragger::setupDefaultGeometry()
	{
		_geode = new osg::Geode;
		{
			osg::TessellationHints* hints = new osg::TessellationHints;
			hints->setCreateTop(false);
			hints->setCreateBottom(false);
			hints->setCreateBackFace(false);
			
			_cylinder = new osg::Cylinder;
			_cylinder->setHeight(_pickCylinderHeight);
			osg::ShapeDrawable* cylinderDrawable = new osg::ShapeDrawable(_cylinder.get(), hints);
			_geode->addDrawable(cylinderDrawable);
			setDrawableToAlwaysCull(*cylinderDrawable);
			_geode->addDrawable(createCircleGeometry(1.0f, 100));
		}

		// Draw in line mode.
		{
			osg::PolygonMode* polymode = new osg::PolygonMode;
			polymode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
			_geode->getOrCreateStateSet()->setAttributeAndModes(polymode,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
			_geode->getOrCreateStateSet()->setAttributeAndModes(m_rpLineWidth.get(), osg::StateAttribute::ON);

#if !defined(OSG_GLES2_AVAILABLE)
			_geode->getOrCreateStateSet()->setMode(GL_NORMALIZE, osg::StateAttribute::ON);
#endif
		}

		// Add line to dragger.
		addChild(_geode.get());
	}

	bool CExRotateCylinderDragger::handle( const osgManipulator::PointerInfo& pi, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
	{
		if(RotateCylinderDragger::handle(pi, ea, us))
		{
			CFeDraggerAttribute::HandleAttributes(ea, us);
			return true;
		}
		
		return false;
	}
}


namespace FeNodeEditor
{
	CFeRotateDragger::CFeRotateDragger(void)
	{
		_xDragger = new CExRotateCylinderDragger();
		addChild(_xDragger.get());
		addDragger(_xDragger.get());

		_yDragger = new CExRotateCylinderDragger();
		addChild(_yDragger.get());
		addDragger(_yDragger.get());

		_zDragger = new CExRotateCylinderDragger();
		addChild(_zDragger.get());
		addDragger(_zDragger.get());

		setParentDragger(getParentDragger());
	}

	CFeRotateDragger::~CFeRotateDragger(void)
	{
	}

	void CFeRotateDragger::setupDefaultGeometry()
	{
		_xDragger->setupDefaultGeometry();
		_yDragger->setupDefaultGeometry();
		_zDragger->setupDefaultGeometry();

		// Rotate X-axis dragger appropriately.
		{
			osg::Quat rotation; rotation.makeRotate(osg::Vec3(0.0f, 0.0f, 1.0f), osg::Vec3(1.0f, 0.0f, 0.0f));
			_xDragger->setMatrix(osg::Matrix(rotation));
		}

		// Rotate Y-axis dragger appropriately.
		{
			osg::Quat rotation; rotation.makeRotate(osg::Vec3(0.0f, 0.0f, 1.0f), osg::Vec3(0.0f, 1.0f, 0.0f));
			_yDragger->setMatrix(osg::Matrix(rotation));
		}

		// Send different colors for each dragger.
		_xDragger->setColor(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
		_yDragger->setColor(osg::Vec4(0.0f,1.0f,0.0f,1.0f));
		_zDragger->setColor(osg::Vec4(0.0f,0.0f,1.0f,1.0f));
	}

}