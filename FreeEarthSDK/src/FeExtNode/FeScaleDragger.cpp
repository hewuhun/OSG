#include <osg/LineWidth>
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <osgViewer/View>
#include <FeScaleDragger.h>

using namespace osgManipulator;

namespace FeNodeEditor
{
	CExScale1DDragger::CExScale1DDragger()
	{
		/// 设置缩放的最小临界值
		setMinScale(0.01);
	}

	void CExScale1DDragger::setupDefaultGeometry()
	{
		// Create a line.
		{
			osg::ref_ptr<osg::Geode> _lineGeode = new osg::Geode;
			osg::Geometry* geometry = new osg::Geometry();

			osg::Vec3Array* vertices = new osg::Vec3Array(2);
			(*vertices)[0] = osg::Vec3(0.0f,0.0f,0.0f);
			(*vertices)[1] = osg::Vec3(1.0f,0.0f,0.0f);

			geometry->setVertexArray(vertices);
			geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,2));

			_lineGeode->addDrawable(geometry);

			// Turn of lighting for line and set line width.
			_lineGeode->getOrCreateStateSet()->setAttributeAndModes(m_rpLineWidth.get(), osg::StateAttribute::ON);
			_lineGeode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
		
			// Add line to all the individual 1D draggers.
			addChild(_lineGeode.get());
		}

		// Create a box.
		{
			osg::Geode* geode = new osg::Geode;
			osg::ref_ptr<osg::Box> _box = new osg::Box(osg::Vec3(1.0f,0.0f,0.0f), 0.1);
			geode->addDrawable(new osg::ShapeDrawable(_box.get()));

			// This ensures correct lighting for scaled draggers.
#if !defined(OSG_GLES2_AVAILABLE)
			geode->getOrCreateStateSet()->setMode(GL_NORMALIZE, osg::StateAttribute::ON);
#endif
			addChild(geode);
		}
	}

	bool CExScale1DDragger::handle( const osgManipulator::PointerInfo& pi, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
	{
		if(Scale1DDragger::handle(pi, ea, us))
		{
			CFeDraggerAttribute::HandleAttributes(ea, us);
			return true;
		}

		return false;
	}

}


namespace FeNodeEditor
{
	CScaleUniformDragger::CScaleUniformDragger(void)
	{
	}

	CScaleUniformDragger::~CScaleUniformDragger(void)
	{
	}

	double computeScale(const osg::Vec3d& startProjectedPoint,
		const osg::Vec3d& projectedPoint, double scaleCenter)
	{
		double scale = (projectedPoint[0] - startProjectedPoint[0]) + 1;

		/// 设置一次缩放的临界值
		const float fMaxScale = 3.0;
		const float fMinScale = 0.1;
		if(scale < fMinScale)
		{
			scale = fMinScale;
		}

		return scale;
	}

	bool CScaleUniformDragger::handle( const PointerInfo& pointer, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
	{
		if(HandleInternalEvent(pointer, ea, aa))
		{
			CFeDraggerAttribute::HandleAttributes(ea, aa);
			return true;
		}

		return false;
	}

	bool CScaleUniformDragger::HandleInternalEvent( const osgManipulator::PointerInfo& pointer, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
	{
		// Check if the dragger node is in the nodepath.
		if (!pointer.contains(this)) return false;

		switch (ea.getEventType())
		{
			// Pick start.
		case (osgGA::GUIEventAdapter::PUSH):
			{
				// Get the LocalToWorld matrix for this node and set it for the projector.
				osg::NodePath nodePathToRoot;
				computeNodePathToRoot(*this,nodePathToRoot);
				osg::Matrix localToWorld = osg::computeLocalToWorld(nodePathToRoot);
				_projector->setLocalToWorld(localToWorld);

				if (_projector->project(pointer, _startProjectedPoint))
				{
					_scaleCenter = 0.0;
					if (_scaleMode == SCALE_WITH_OPPOSITE_HANDLE_AS_PIVOT)
					{
						if ( pointer.contains(_leftHandleNode.get()) )
							_scaleCenter = _projector->getLineEnd()[0];
						else if ( pointer.contains( _rightHandleNode.get()) )
							_scaleCenter = _projector->getLineStart()[0];
					}

					// Generate the motion command.
					osg::ref_ptr<ScaleUniformCommand> cmd = new ScaleUniformCommand();
					cmd->setStage(MotionCommand::START);
					cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());

					// Dispatch command.
					dispatch(*cmd);

					// Set color to pick color.
					setMaterialColor(_pickColor,*this);

					aa.requestRedraw();

					return true;
				}
			}
			break;

			// Pick move.
		case (osgGA::GUIEventAdapter::DRAG):
			{
				osg::Vec3d projectedPoint;
				if (_projector->project(pointer, projectedPoint))
				{
					// Generate the motion command.
					osg::ref_ptr<ScaleUniformCommand> cmd = new ScaleUniformCommand();

					// Compute scale.
					double scale = computeScale(_startProjectedPoint,projectedPoint,_scaleCenter);
					if (scale < getMinScale()) scale = getMinScale();

					// Snap the referencePoint to the line start or line end depending on which is closer.
					double referencePoint = _startProjectedPoint[0];
					if (fabs(_projector->getLineStart()[0] - referencePoint) <
						fabs(_projector->getLineEnd()[0]   - referencePoint))
						referencePoint = _projector->getLineStart()[0];
					else
						referencePoint = _projector->getLineEnd()[0];

					cmd->setStage(MotionCommand::MOVE);
					cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());
					cmd->setScale(scale);

					// Dispatch command.
					dispatch(*cmd);

					aa.requestRedraw();

					return true;
				}
			}
			break;

			// Pick finish.
		case (osgGA::GUIEventAdapter::RELEASE):
			{
				osg::ref_ptr<ScaleUniformCommand> cmd = new ScaleUniformCommand();

				cmd->setStage(MotionCommand::FINISH);
				cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());

				// Dispatch command.
				dispatch(*cmd);

				// Reset color.
				setMaterialColor(_color,*this);

				aa.requestRedraw();

				return true;
			}
			break;
		default:
			return false;
		}

		return false;
	}

	void CScaleUniformDragger::setupDefaultGeometry()
	{
		// Create Scale Handle node.
		{
			osg::Group* scaleHandleNode = new osg::Group;
			osg::Geode* geodeOutline = new osg::Geode;
			osg::Geode* geodeFace = new osg::Geode;
			scaleHandleNode->addChild(geodeOutline);
			scaleHandleNode->addChild(geodeFace);
			addChild(scaleHandleNode);
			setRightHandleNode(*scaleHandleNode);

			/// outline
			{
				osg::Geometry* geom = new osg::Geometry;
				geodeOutline->addDrawable(geom);

				osg::ref_ptr<osg::Vec3Array> vertexes = new osg::Vec3Array;
				vertexes->push_back(osg::Vec3(0.6, 0, 0));
				vertexes->push_back(osg::Vec3(0, 0.6, 0));
				vertexes->push_back(osg::Vec3(0, 0, 0.6));
				vertexes->push_back(osg::Vec3(0.6, 0, 0));
				geom->setVertexArray(vertexes);
				geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, vertexes->size()));

				osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
				colors->push_back(osg::Vec4(0, 0, 0.2, 1));
				geom->setColorArray(colors, osg::Array::BIND_OVERALL);
				
				geom->getOrCreateStateSet()->setMode(GL_LIGHTING, 
					osg::StateAttribute::OFF|osg::StateAttribute::PROTECTED|osg::StateAttribute::OVERRIDE);

				geom->getOrCreateStateSet()->setAttributeAndModes(m_rpLineWidth.get(), osg::StateAttribute::ON);
			}
			
			/// face
			{
				osg::Geometry* geom = new osg::Geometry;
				geodeFace->addDrawable(geom);

				osg::ref_ptr<osg::Vec3Array> vertexes = new osg::Vec3Array;
				vertexes->push_back(osg::Vec3(0, 0, 0.6));
				vertexes->push_back(osg::Vec3(0, 0, 0));
				vertexes->push_back(osg::Vec3(0.6, 0, 0));
				vertexes->push_back(osg::Vec3(0, 0.6, 0));
				vertexes->push_back(osg::Vec3(0, 0, 0.6));
				geom->setVertexArray(vertexes);
				geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, 0, vertexes->size()));
			}
			
			scaleHandleNode->getOrCreateStateSet()->setMode(GL_BLEND, 
				osg::StateAttribute::ON|osg::StateAttribute::PROTECTED|osg::StateAttribute::OVERRIDE);
			scaleHandleNode->getOrCreateStateSet()->setMode(GL_NORMALIZE, 
				osg::StateAttribute::ON|osg::StateAttribute::PROTECTED|osg::StateAttribute::OVERRIDE);
			scaleHandleNode->getOrCreateStateSet()->setRenderBinDetails(50,"DepthSortedBin");
		}
	}
	
}


namespace FeNodeEditor
{
	CFeScaleDragger::CFeScaleDragger()
	{
		m_rpScaleUniform = new CScaleUniformDragger;
		addChild(m_rpScaleUniform);
		addDragger(m_rpScaleUniform);

		m_rpXDragger = new CExScale1DDragger;
		addChild(m_rpXDragger);
		addDragger(m_rpXDragger);

		m_rpYDragger = new CExScale1DDragger;
		addChild(m_rpYDragger);
		addDragger(m_rpYDragger);

		m_rpZDragger = new CExScale1DDragger;
		addChild(m_rpZDragger);
		addDragger(m_rpZDragger);

		setParentDragger(getParentDragger());
	}

	CFeScaleDragger::~CFeScaleDragger()
	{
	}

	void CFeScaleDragger::setupDefaultGeometry()
	{
		m_rpScaleUniform->setupDefaultGeometry();
		m_rpXDragger->setupDefaultGeometry();
		m_rpYDragger->setupDefaultGeometry();
		m_rpZDragger->setupDefaultGeometry();
		
		// Rotate Z-axis dragger appropriately.
		{
			osg::Quat rotation; rotation.makeRotate(osg::Vec3(1.0f, 0.0f, 0.0f), osg::Vec3(0.0f, 0.0f, 1.0f));
			m_rpZDragger->setMatrix(osg::Matrix(rotation));
		}

		// Rotate Y-axis dragger appropriately.
		{
			osg::Quat rotation; rotation.makeRotate(osg::Vec3(1.0f, 0.0f, 0.0f), osg::Vec3(0.0f, 1.0f, 0.0f));
			m_rpYDragger->setMatrix(osg::Matrix(rotation));
		}

		// Send different colors for each dragger.
		m_rpXDragger->setColor(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
		m_rpYDragger->setColor(osg::Vec4(0.0f,1.0f,0.0f,1.0f));
		m_rpZDragger->setColor(osg::Vec4(0.0f,0.0f,1.0f,1.0f));

		m_rpScaleUniform->setColor(osg::Vec4f(0.8, 0.7, 0, 0.6));
		m_rpScaleUniform->setPickColor(osg::Vec4f(0.8, 0.8, 0, 0.8));
	}

}

