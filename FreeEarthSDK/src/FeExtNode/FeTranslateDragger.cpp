#include <osg/LineWidth>
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <osg/AutoTransform>
#include <osg/PolygonMode>
#include <osgViewer/View>
#include <FeTranslateDragger.h>

using namespace osgManipulator;

namespace FeNodeEditor
{
	CExTranslate1DDragger::CExTranslate1DDragger( const osg::Vec3d& s, const osg::Vec3d& e )
		:Translate1DDragger(s, e)
	{
		_pickCylinderRadius = 0.015f;
		_coneHeight = 0.2f;
	}

	CExTranslate1DDragger::~CExTranslate1DDragger()
	{
	}

	void CExTranslate1DDragger::setupDefaultGeometry()
	{
		// Create a line.
		osg::ref_ptr<osg::Geode> _lineGeode = new osg::Geode;
		{
			osg::Geometry* geometry = new osg::Geometry();

			osg::Vec3Array* vertices = new osg::Vec3Array(2);
			(*vertices)[0] = osg::Vec3(0.0f,0.0f,0.0f);
			(*vertices)[1] = osg::Vec3(0.0f,0.0f,1.0f);

			geometry->setVertexArray(vertices);
			geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,2));

			_lineGeode->addDrawable(geometry);
		}

		// Turn of lighting for line and set line width.
		{
			_lineGeode->getOrCreateStateSet()->setAttributeAndModes(m_rpLineWidth.get(), osg::StateAttribute::ON);
			_lineGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		}

		// Add line to dragger.
		addChild(_lineGeode.get());
		
		osg::Geode* geode = new osg::Geode;

		// Create a cone.
		{
			_cone = new osg::Cone (osg::Vec3(0.0f, 0.0f, 1.0f), _coneHeight * 0.25f, _coneHeight);
			osg::ShapeDrawable* coneDrawable = new osg::ShapeDrawable(_cone.get());
			// coneDrawable->setColor(osg::Vec4(0.0f,0.0f,1.0f,1.0f));
			geode->addDrawable(coneDrawable);

			// This ensures correct lighting for scaled draggers.
#if !defined(OSG_GLES2_AVAILABLE)
			geode->getOrCreateStateSet()->setMode(GL_NORMALIZE, osg::StateAttribute::ON);
#endif
		}

		// Create an invisible cylinder for picking the line.
		{
			osg::ref_ptr<osg::Cylinder> _cylinder = new osg::Cylinder (osg::Vec3(0.0f,0.0f,0.5f), _pickCylinderRadius, 1.0f);
			osg::Drawable* geometry = new osg::ShapeDrawable(_cylinder.get());
			setDrawableToAlwaysCull(*geometry);
			geode->addDrawable(geometry);
		}

		// Add geode to dragger.
		addChild(geode);
		
	}

	bool CExTranslate1DDragger::handle( const osgManipulator::PointerInfo& pi, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
	{
		if(!Translate1DDragger::handle(pi, ea, us))
		{
			return false;
		}

		CFeDraggerAttribute::HandleAttributes(ea, us);

		return true;
	}
}

namespace FeNodeEditor
{
	void CExTranslatePlaneDragger::setupDefaultGeometry()
	{
		/// outline
		{
			osg::ref_ptr<osg::Geode> m_rpLineGeode = new osg::Geode;
			osg::Geometry* geom = new osg::Geometry;
			m_rpLineGeode->addDrawable(geom);
			addChild(m_rpLineGeode);

			osg::ref_ptr<osg::Vec3Array> vertexes = new osg::Vec3Array;
			vertexes->push_back(osg::Vec3(-0.5,0.0,0.5));
			vertexes->push_back(osg::Vec3(0.5,0.0,0.5));
			vertexes->push_back(osg::Vec3(0.5,0.0,-0.5));
			vertexes->push_back(osg::Vec3(-0.5,0.0,-0.5));
			
			geom->setVertexArray(vertexes);
			geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, vertexes->size()));
			
			//osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
			m_rpColorsArray = new osg::Vec4Array;
			m_rpColorsArray->push_back(osg::Vec4(0, 0, 0.2, 1));
			geom->setColorArray(m_rpColorsArray, osg::Array::BIND_OVERALL);

			m_rpLineGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, 
				osg::StateAttribute::OFF|osg::StateAttribute::PROTECTED|osg::StateAttribute::OVERRIDE);

			m_rpLineGeode->getOrCreateStateSet()->setAttributeAndModes(m_rpLineWidth.get(), osg::StateAttribute::ON);
		}

		/// face
		{
			osg::Geode* geode = new osg::Geode;
			osg::Geometry* geometry = new osg::Geometry();
			geode->addDrawable(geometry);
			addChild(geode);

			osg::ref_ptr<osg::Vec3Array> vertexes = new osg::Vec3Array;
			vertexes->push_back(osg::Vec3(-0.5,0.0,0.5));
			vertexes->push_back(osg::Vec3(-0.5,0.0,-0.5));
			vertexes->push_back(osg::Vec3(0.5,0.0,-0.5));
			vertexes->push_back(osg::Vec3(0.5,0.0,0.5));

			geometry->setVertexArray(vertexes);
			geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,vertexes->size()));

			osg::Vec3Array* normals = new osg::Vec3Array;
			normals->push_back(osg::Vec3(0.0,1.0,0.0));
			geometry->setNormalArray(normals, osg::Array::BIND_OVERALL);

			osg::PolygonMode* polymode = new osg::PolygonMode;
			polymode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::FILL);
			geode->getOrCreateStateSet()->setAttributeAndModes(polymode,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
			geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		}
	}

	void CExTranslatePlaneDragger::setLineColor( const osg::Vec4& color )
	{
		if(m_rpColorsArray.valid())
		{
			m_rpColorsArray->at(0) = color;
		}
	}

	bool CExTranslatePlaneDragger::handle( const osgManipulator::PointerInfo& pi, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
	{
		if(Translate2DDragger::handle(pi, ea, us))
		{
			CFeDraggerAttribute::HandleAttributes(ea, us);

			if(ea.getEventType() == osgGA::GUIEventAdapter::PUSH)
			{
				ActiveAssociateDraggers(true);
			}
			else if(ea.getEventType() == osgGA::GUIEventAdapter::RELEASE)
			{
				ActiveAssociateDraggers(false);
			}

			return true;
		}

		return false;
	}

	void CExTranslatePlaneDragger::AddAssociateDragger( CExTranslate1DDragger* dragger )
	{
		if(dragger)
		{
			m_associatedDraggerList.push_back(dragger);
		}
	}

	void CExTranslatePlaneDragger::ActiveAssociateDraggers( bool bActive )
	{
		for(ASSOCIATE_DRAGGER_LIST::iterator iter = m_associatedDraggerList.begin();
			iter != m_associatedDraggerList.end(); iter++)
		{
			if((*iter).valid())
			{
				setMaterialColor(bActive?(*iter)->getPickColor():(*iter)->getColor(), *(*iter));
			}
		}
	}

}


namespace FeNodeEditor
{

	CFeTranslateDragger::CFeTranslateDragger(void)
	{
		_xDragger = new CExTranslate1DDragger(osg::Vec3(0.0,0.0,0.0), osg::Vec3(0.0,0.0,1.0));
		addChild(_xDragger.get());
		addDragger(_xDragger.get());

		_yDragger = new CExTranslate1DDragger(osg::Vec3(0.0,0.0,0.0), osg::Vec3(0.0,0.0,1.0));
		addChild(_yDragger.get());
		addDragger(_yDragger.get());

		_zDragger = new CExTranslate1DDragger(osg::Vec3(0.0,0.0,0.0), osg::Vec3(0.0,0.0,1.0));
		addChild(_zDragger.get());
		addDragger(_zDragger.get());

		_xyDragger = new CExTranslatePlaneDragger;
		addChild(_xyDragger);
		addDragger(_xyDragger);

		_xzDragger = new CExTranslatePlaneDragger;
		addChild(_xzDragger);
		addDragger(_xzDragger);

		_yzDragger = new CExTranslatePlaneDragger;
		addChild(_yzDragger);
		addDragger(_yzDragger);

		setParentDragger(getParentDragger());
	}

	CFeTranslateDragger::~CFeTranslateDragger(void)
	{
	}

	void CFeTranslateDragger::setupDefaultGeometry()
	{
		// Create Axis Dragger
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
		}

		// Create Plane Dragger
		{
			_xyDragger->setupDefaultGeometry();
			_xzDragger->setupDefaultGeometry();
			_yzDragger->setupDefaultGeometry();

			const float fScale = 0.25;
			const float fSize_2 = 1 * fScale / 2;

			_xzDragger->setMatrix( osg::Matrix::scale(fScale, fScale, fScale)
				* osg::Matrix::translate(fSize_2, 0, fSize_2));

			_xyDragger->setMatrix( osg::Matrix::scale(fScale, fScale, fScale)
									* osg::Matrix::translate(fSize_2, 0, fSize_2) 
									* osg::Matrix::rotate(-osg::PI_2, osg::X_AXIS));

			_yzDragger->setMatrix( osg::Matrix::scale(fScale, fScale, fScale)
				* osg::Matrix::translate(fSize_2, 0, fSize_2) 
				* osg::Matrix::rotate(osg::PI_2, osg::Z_AXIS));

			// Associate axis draggers
			_xzDragger->AddAssociateDragger(_xDragger);
			_xzDragger->AddAssociateDragger(_zDragger);

			_xyDragger->AddAssociateDragger(_xDragger);
			_xyDragger->AddAssociateDragger(_yDragger);

			_yzDragger->AddAssociateDragger(_yDragger);
			_yzDragger->AddAssociateDragger(_zDragger);
		}

		// Send different colors for each dragger.
		{
			_xDragger->setColor(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
			_yDragger->setColor(osg::Vec4(0.0f,1.0f,0.0f,1.0f));
			_zDragger->setColor(osg::Vec4(0.0f,0.0f,1.0f,1.0f));

			_xzDragger->setColor(osg::Vec4(1.0f,0.65f,0.6f,0.6f));
			_xyDragger->setColor(osg::Vec4(0.6f,1.0f,0.65f,0.6f));
			_yzDragger->setColor(osg::Vec4(0.6f,0.65f,1.0f,0.6f));
			_xzDragger->setLineColor(osg::Vec4(1, 0, 0, 1));
			_xyDragger->setLineColor(osg::Vec4(0, 1, 0, 1));
			_yzDragger->setLineColor(osg::Vec4(0, 0, 1, 1));
		}
	}

	void CFeTranslateDragger::ShowAxisX( bool bShow )
	{
		if(!_xDragger.valid()) return;

		// axis dragger
		_xDragger->setNodeMask(bShow?NODE_VISIBLE_MASK:NODE_UNVISBLE_MASK);

		// plane dragger
		if(_yDragger.valid() && (NODE_UNVISBLE_MASK != _yDragger->getNodeMask()) && _xyDragger.valid())
		{
			_xyDragger->setNodeMask(bShow?NODE_VISIBLE_MASK:NODE_UNVISBLE_MASK);
		}
		if(_zDragger.valid() && (NODE_UNVISBLE_MASK != _zDragger->getNodeMask()) && _xzDragger.valid())
		{
			_xzDragger->setNodeMask(bShow?NODE_VISIBLE_MASK:NODE_UNVISBLE_MASK);
		}
	}

	void CFeTranslateDragger::ShowAxisY( bool bShow )
	{
		if(!_yDragger.valid()) return;

		// axis dragger
		_yDragger->setNodeMask(bShow?NODE_VISIBLE_MASK:NODE_UNVISBLE_MASK);

		// plane dragger
		if(_xDragger.valid() && (NODE_UNVISBLE_MASK != _xDragger->getNodeMask()) && _xyDragger.valid())
		{
			_xyDragger->setNodeMask(bShow?NODE_VISIBLE_MASK:NODE_UNVISBLE_MASK);
		}
		if(_zDragger.valid() && (NODE_UNVISBLE_MASK != _zDragger->getNodeMask()) && _yzDragger.valid())
		{
			_yzDragger->setNodeMask(bShow?NODE_VISIBLE_MASK:NODE_UNVISBLE_MASK);
		}
	}

	void CFeTranslateDragger::ShowAxisZ( bool bShow )
	{
		if(!_zDragger.valid()) return;

		// axis dragger
		_zDragger->setNodeMask(bShow?NODE_VISIBLE_MASK:NODE_UNVISBLE_MASK);

		// plane dragger
		if(_xDragger.valid() && (NODE_UNVISBLE_MASK != _xDragger->getNodeMask()) && _xzDragger.valid())
		{
			_xzDragger->setNodeMask(bShow?NODE_VISIBLE_MASK:NODE_UNVISBLE_MASK);
		}
		if(_yDragger.valid() && (NODE_UNVISBLE_MASK != _yDragger->getNodeMask()) && _yzDragger.valid())
		{
			_yzDragger->setNodeMask(bShow?NODE_VISIBLE_MASK:NODE_UNVISBLE_MASK);
		}
	}

}

