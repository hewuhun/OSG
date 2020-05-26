#include <osgViewer/View>
#include <FeExtNode/FeCompositeDragger.h>


namespace FeNodeEditor
{
	bool CFeCompositeDragger::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
	{
		if (ea.getHandled()) return false;

		osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
		if (!view) return false;

		bool handled = false;

		bool activationPermitted = true;

		if (activationPermitted || _draggerActive)
		{
			switch (ea.getEventType())
			{
			case osgGA::GUIEventAdapter::PUSH:
				{
					if(0 == (ea.getButton() & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON))
					{
						return false;
					}

					osgUtil::LineSegmentIntersector::Intersections intersections;

					_pointer.reset();

					if (view->computeIntersections(ea ,intersections, _intersectionMask))
					{
						for(osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
							hitr != intersections.end();
							++hitr)
						{
							osg::NodePath nodePath = hitr->nodePath;

							for (osg::NodePath::iterator itr = nodePath.begin();
								itr != nodePath.end();
								++itr)
							{
								osgManipulator::Dragger* dragger = dynamic_cast<osgManipulator::Dragger*>(*itr);
								if (dragger)
								{
									if (dragger==this)
									{
										_pointer.addIntersection(nodePath, hitr->getLocalIntersectPoint());

										osg::Camera *rootCamera = view->getCamera();
										//osg::NodePath nodePath = _pointer._hitList.front().first;
										osg::NodePath::reverse_iterator ritr;
										for(ritr = nodePath.rbegin();
											ritr != nodePath.rend();
											++ritr)
										{
											osg::Camera* camera = dynamic_cast<osg::Camera*>(*ritr);
											if (camera && (camera->getReferenceFrame()!=osg::Transform::RELATIVE_RF || camera->getParents().empty()))
											{
												rootCamera = camera;
												break;
											}
										}

										_pointer.setCamera(rootCamera);
										_pointer.setMousePosition(ea.getX(), ea.getY());

										if(dragger->handle(_pointer, ea, aa))
										{
											dragger->setDraggerActive(true);
											handled = true;
											return handled;
										}
									}
								}
							}	


						}
					}
					break;
				}
			case osgGA::GUIEventAdapter::DRAG:
			case osgGA::GUIEventAdapter::RELEASE:
				{
					if (_draggerActive)
					{
						_pointer._hitIter = _pointer._hitList.begin();
						_pointer.setMousePosition(ea.getX(), ea.getY());

						if(CompositeDragger::handle(_pointer, ea, aa))
						{
							handled = true;
						}

						if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE)
						{
							setDraggerActive(false);
							_pointer.reset();
						}
					}
					break;
				}
			default:
				break;
			}
		}

		return handled;
	}

}