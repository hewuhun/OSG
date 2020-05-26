#include <FeKits/manipulator/FreeEarthManipulator.h>

#include <osg/TexEnv>
#include <osgViewer/View>
#include <osgViewer/Renderer>

#include <osgEarth/DPLineSegmentIntersector>
#include <osgEarthDrivers/engine_mp/TerrainNode>

#include <FeUtils/PathRegistry.h>
#include <FeUtils/CoordConverter.h>

namespace FeKit
{
	CameraUpdateCallback::CameraUpdateCallback(FreeEarthManipulator* m)
		:m_pEarthManipulator(m)
	{

	}

	void CameraUpdateCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		if(m_pEarthManipulator)
		{
			if(m_pEarthManipulator->m_billboard.valid() && m_pEarthManipulator->m_billboardMT.valid())
			{
				if( m_pEarthManipulator->m_bIsRotating || m_pEarthManipulator->m_bIsZooming)
				{
					m_pEarthManipulator->m_billboardMT->setNodeMask(1);

					osg::Matrixd viewMat = m_pEarthManipulator->getMatrix();
					osg::Vec3d rightVector(viewMat(0,0),viewMat(0,1),viewMat(0,2));
					rightVector.normalize();

					osg::Plane pl(rightVector,osg::Vec3d(0,0,0));

					osg::Vec3d center;
					if(m_pEarthManipulator->m_bIsRotating)
						center = m_pEarthManipulator->m_v3dLastRotateIntersect;
					else
						center = m_pEarthManipulator->m_v3dLastZoomIntersect;

					double disToPL = pl.distance(center);
					osg::Vec3d projCenter = center - pl.getNormal() * disToPL;
					projCenter.normalize();

					osg::Vec3d lookVectorR(viewMat(2,0),viewMat(2,1),viewMat(2,2));
					lookVectorR.normalize();

					double dotLC = lookVectorR * projCenter;
					dotLC = osg::clampBetween(dotLC,-1.0,1.0);
					double angle = acos(dotLC);

					osg::Vec3d crossVec = lookVectorR ^ projCenter;
					crossVec.normalize();
					if(crossVec * rightVector > 0)
						angle *= -1;

					double z = (0.5 * angle) / osg::PI_2 - 0.5;
					z = osg::clampBetween(z,-1.0,0.0);

					if(abs(angle) < osg::PI / 3.0)
					{
						m_pEarthManipulator->m_billboard->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,
							osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE); 

						osg::Vec3d corner(-0.5f,0.0f,z);
						corner *= m_pEarthManipulator->m_billboardScale;
						m_pEarthManipulator->updateBillboard(corner);
					}
					else
					{
						m_pEarthManipulator->m_billboard->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,
							osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

						osg::Vec3d corner(-0.5f,0.0f,z);
						corner *= m_pEarthManipulator->m_billboardScale;
						m_pEarthManipulator->updateBillboard(corner);
					}
				}
				else
				{
					m_pEarthManipulator->m_billboardMT->setNodeMask(0);
				}
			}
		}

		traverse(node,nv);
	}

	void FreeEarthManipTerrainCallback::onTileAdded( const TileKey& key, osg::Node* tile, TerrainCallbackContext& context )
	{
		if(m_pEarthManipulator && m_pEarthManipulator->m_bLastIntersectWithTerrain)
		{
			if(m_pEarthManipulator->m_bIsRotating && m_pEarthManipulator->m_billboardMT.valid() && m_pEarthManipulator->m_billboard.valid())
			{
				osg::Vec3d pRenderContext;
				FeUtil::XYZ2DegreeLLH(m_pEarthManipulator->m_opRenderContext.get(), m_pEarthManipulator->m_v3dLastRotateIntersect,pRenderContext);
				if(key.getExtent().contains(pRenderContext.x(), pRenderContext.y()) )
				{
					osg::Vec3d start,end,curPoint,outNor;
					start.set(0,0,0);
					end = m_pEarthManipulator->m_v3dLastRotateIntersect;
					end.normalize();
					end = end * osg::WGS_84_RADIUS_EQUATOR * 3.0;
					bool curFlag = m_pEarthManipulator->RayToWorld(end,start,curPoint,outNor);
					if(curFlag)
					{
						m_pEarthManipulator->m_v3dLastRotateIntersect = curPoint;
						m_pEarthManipulator->m_billboardMT->setPosition(m_pEarthManipulator->m_v3dLastRotateIntersect);
					}
				}
			}

			if(m_pEarthManipulator->m_bIsZooming && m_pEarthManipulator->m_billboardMT.valid() && m_pEarthManipulator->m_billboard.valid())
			{
				osg::Vec3d pRenderContext;
				FeUtil::XYZ2DegreeLLH(m_pEarthManipulator->m_opRenderContext.get(), m_pEarthManipulator->m_v3dLastZoomIntersect,pRenderContext);
				if(key.getExtent().contains(pRenderContext.x(), pRenderContext.y()) )
				{
					osg::Vec3d start,end,curPoint,outNor;
					start.set(0,0,0);
					end = m_pEarthManipulator->m_v3dLastZoomIntersect;
					end.normalize();
					end = end * osg::WGS_84_RADIUS_EQUATOR * 3.0;
					bool curFlag = m_pEarthManipulator->RayToWorld(end,start,curPoint,outNor);
					if(curFlag)
					{
						m_pEarthManipulator->m_v3dLastZoomIntersect = curPoint;
						m_pEarthManipulator->m_billboardMT->setPosition(m_pEarthManipulator->m_v3dLastZoomIntersect);
					}
				}
			}
		}
	}

}

namespace FeKit
{
	FreeEarthManipulator::FreeEarthManipulator(FeUtil::CRenderContext* pContext)
		:PreEarthManipulator()
		,FreeEarthManipulatorBase(pContext)
		,m_bGoogleModeEnable(true)
		,m_bLastPanIntersected(false)
		,m_bLastZoomIntersected(false)
		,m_bLastRotateIntersected(false)
		,m_fLastX(-1.0)
		,m_fLastY(-1.0)
		,m_uRotateSensitivity(2.0)
		,m_bLastPanSkyIntersected(false)
		,m_dScrollDuration(0.25)
		,m_dSkyPlaneDis(2000000)
		,m_bCurIntersectWithEarth(false)
		,m_bPanRelease(true)
		,m_dLastPanAngle(0.0)
		,m_dMaxZoomDis(400000000)
		,m_dLastZoomDis(0)
		,m_billboardScale(40.0)
		,m_bIsRotating(false)
		,m_bIsZooming(false)
		,m_bEnableThrowing(true)
		,m_bIsRotateChecked(false)
		,m_bLastIntersectWithTerrain(true)
	{
		if(m_bGoogleModeEnable)
		{
			configureDefaultSettings();
			setScrollDuration(m_dScrollDuration);

			osg::ref_ptr<CameraUpdateCallback> m_rpCamUpdateCallback = new CameraUpdateCallback(this);
			
			if(m_opRenderContext.valid() && m_opRenderContext->GetView())
			{
				osg::Camera* cam = m_opRenderContext->GetView()->getCamera();
				if(cam)
				{
					cam->addUpdateCallback(m_rpCamUpdateCallback.get());

					MapNode* mapNode = MapNode::findMapNode( cam, _findNodeTraversalMask );
					if ( mapNode )
					{            
						m_rpTerrainCallback = new FreeEarthManipTerrainCallback( this );
						mapNode->getTerrain()->addTerrainCallback( m_rpTerrainCallback );
					}
				}

				if(m_opRenderContext->GetRoot())
				{
					m_billboardMT = new osg::AutoTransform();
					m_billboard = new osg::Billboard();
					m_billboard->setName("test0");

					m_billboardMT->addChild(m_billboard);

					m_billboard->setMode(osg::Billboard::POINT_ROT_EYE);
					m_billboard->addDrawable(
						createSquare(osg::Vec3(-0.5f,0.0f,-0.5f) * m_billboardScale,osg::Vec3(1.0f,0.0f,0.0f) * m_billboardScale,osg::Vec3(0.0f,0.0f,1.0f) * m_billboardScale,
						osgDB::readImageFile(FeFileReg->GetFullPath("texture/mouse/center.png"))),
						osg::Vec3(0.0f,0.0f,0.0f));

					m_billboardMT->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
					m_billboardMT->setAutoScaleToScreen(true);
					m_billboardMT->setNodeMask(0);

					m_opRenderContext->GetRoot()->addChild(m_billboardMT.get());
				}


			}
		}
	}

	FreeEarthManipulator::~FreeEarthManipulator()
	{
		if(m_opRenderContext.valid() && m_opRenderContext->GetRoot() && m_billboardMT.valid())
		{
			m_opRenderContext->GetRoot()->removeChild(m_billboardMT.get());
		}
	}

	bool FreeEarthManipulator::handlePointAction( const Action &action, float mx, float my, osg::View *view )
	{
		bool bresult = false;
		{
			if(m_bGoogleModeEnable)
			{
				if (action._type == ACTION_NULL)
				{
					return true;
				}				

				osg::Vec3d point;
				bool hitTerrain;
				if ( screenToWorld( mx, my, view, point,hitTerrain))
				{
					switch( action._type )
					{
					case ACTION_GOTO:
						{
							osgEarth::Viewpoint here = getViewpoint();

							if ( !here.focalPoint().get().getSRS() )
								return false;

							double duration_s = action.getDoubleOption(OPTION_DURATION, 1.0);
							double range_factor = action.getDoubleOption(OPTION_GOTO_RANGE_FACTOR, 1.0);

							double radius = point.length();
							updateEarthIntersectPoint(radius);

							const double dMaxDis = 400000000.0; // 测试值，与鼠标滚动缩放限制大小一致

							if(m_bCurIntersectWithEarth)
							{
								bool flag = getOldModeViewPoint(here);
								if(flag)
								{
									osg::Vec3d pointVP;
									here.focalPoint().get().getSRS()->transformFromWorld(point, pointVP);

									osgEarth::GeoPoint gp(here.focalPoint().get().getSRS(),
										pointVP.x(),
										pointVP.y(),
										pointVP.z());
									here.focalPoint() = gp;
									//here.setFocalPoint( pointVP );
									here.setRange(here.getRange() * range_factor);

									// 防止地球无限远
									if(here.getRange() >= dMaxDis)
									{
										here.setRange(dMaxDis);
									}

									setViewpoint( here, duration_s ,true);
								}			
							}
							else
							{
								double alt = here.focalPoint().get().z();
								alt = alt * range_factor;

								// 防止地球无限远
								if(alt >= dMaxDis)
								{
									alt = dMaxDis;
								}

								osg::Vec3d pointVP;
								here.focalPoint().get().getSRS()->transformFromWorld(point, pointVP);

								pointVP.z() = alt;

								osgEarth::GeoPoint gp(here.focalPoint().get().getSRS(),
									pointVP.x(),
									pointVP.y(),
									pointVP.z());
								here.focalPoint() = gp;

								setViewpoint( here, duration_s );
							}							
						}
						break;
					default:
						break;
					}
				}
				return true;
			}
			else
			{
				bresult = PreEarthManipulator::handlePointAction(action, mx, my, view);
			}		
		}
		return bresult;
	}

	void FreeEarthManipulator::updateSetViewpoint()
	{
		if(m_bGoogleModeEnable)
		{
			double t = ( _time_s_now - _time_s_set_viewpoint ) / _set_viewpoint_duration_s;
			double tp = t;

			if ( t >= 1.0 )
			{
				t = tp = 1.0;
				_setting_viewpoint = false;
			}
			else if ( _arc_height > 0.0 )
			{
				if ( tp <= 0.5 )
				{
					double t2 = 2.0*tp;
					t2 = accelerationInterp( t2, _set_viewpoint_accel );
					tp = 0.5*t2;
				}
				else
				{
					double t2 = 2.0*(tp-0.5);
					t2 = accelerationInterp( t2, _set_viewpoint_accel_2 );
					tp = 0.5+(0.5*t2);
				}

				tp = smoothStepInterp( tp );
				tp = smoothStepInterp( tp );
			}
			else if ( t > 0.0 )
			{
				tp = accelerationInterp( tp, _set_viewpoint_accel );
				tp = smoothStepInterp( tp );
			}


			osg::Vec3d svp;
			svp.x() = _start_viewpoint.focalPoint().get().x();
			svp.y() = _start_viewpoint.focalPoint().get().y();
			svp.z() = _start_viewpoint.focalPoint().get().z();
			osgEarth::Viewpoint new_vp(
				"",
				(svp + _delta_focal_point * tp).x(),
				(svp + _delta_focal_point * tp).y(),
				(svp + _delta_focal_point * tp).z(),
				_start_viewpoint.getHeading() + _delta_heading * tp,
				_start_viewpoint.getPitch() + _delta_pitch * tp,
				_start_viewpoint.getRange() + _delta_range * tp,
				_start_viewpoint.focalPoint().get().getSRS() );

			setViewpoint( new_vp );
		}
		else
		{
			PreEarthManipulator::updateSetViewpoint();
		}
	}

	void FreeEarthManipulator::handleMovementAction( const ActionType &type, double dx, double dy, osg::View *view )
	{
		if(m_bGoogleModeEnable)
		{
			switch( type )
			{
			case ACTION_PAN:
				pan( dx, dy );
				break;

			case ACTION_ROTATE:
				if ( _continuous && _settings->getSingleAxisRotation() )
				{
					if ( ::fabs(dx) > ::fabs(dy) )
						dy = 0.0;
					else
						dx = 0.0;
				}
				rotate( dx, dy );
				break;

			case ACTION_ZOOM:
				zoomToPoint( dx, dy );
				break;

			case ACTION_EARTH_DRAG:
				if (_thrown)
					pan(dx*0.5, dy*0.5);  //TODO: create proper drag throwing instead of panning trick
				else
					drag( dx, dy, view );
				break;
			default:break;
			}
		}
		else
			PreEarthManipulator::handleMovementAction(type, dx, dy, view);
	}

	bool FreeEarthManipulator::serviceTask(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		if(m_bGoogleModeEnable)
		{
			if ( _task.valid() && _task->_type != TASK_NONE )
			{
				double dt = _time_s_now - _task->_time_last_service;
				if( dt >= _task->_duration_s)
					dt = _task->_duration_s;

				double zoomFactor = 1.3;

				switch( _task->_type )
				{
				case TASK_ZOOM:					
					if(_task->_dy < 0.0)
						zoomFactor *= -1.0;
					zoom( dt * _task->_dx, dt * zoomFactor,ea,aa );
					break;
				default: break;
				}

				_task->_duration_s -= dt;
				_task->_time_last_service = _time_s_now;

				if ( _task->_duration_s <= 0.0 )
				{
					_task->_type = TASK_NONE;
					m_bIsZooming = false;
				}
			}

			return _task.valid() && _task->_type != TASK_NONE;
		}
		else
		{
			return PreEarthManipulator::serviceTask(ea,aa);
		}
	}

	void FreeEarthManipulator::recalculateCenter( const osg::CoordinateFrame& frame,osg::Node* tile )
	{
		if(m_bGoogleModeEnable)
		{
			if(tile)
			{
				if(!tile->getBound().contains(_center))
					return;

				osg::Camera* cam = m_opViewer->getCamera();
				if(cam == NULL)
					return;
				osg::Matrixd projMat = cam->getProjectionMatrix();

				double fov,aspect,dCurNear,tmpNearFar;
				projMat.getPerspective(fov,aspect,dCurNear,tmpNearFar); 

				osg::Group* gp = dynamic_cast<osg::Group*>(tile);
				if(gp)
				{
					int num = gp->getNumChildren();
					if(num > 0)
					{
						osg::ref_ptr<osg::Group> rGroup = new osg::Group();
						for (int i = 0; i < num; i++)
						{
							osg::Group* g = dynamic_cast<osg::Group*>(gp->getChild(i));
							if(g && g->getNumChildren() > 0)
							{
								osgEarth::Drivers::MPTerrainEngine::TileNode* tn = dynamic_cast<osgEarth::Drivers::MPTerrainEngine::TileNode*>(g->getChild(0));
								if(tn)
								{
									if(tn->getBound().contains(_center))
										rGroup->addChild(tn);
								}
							}
						}

						double radius = dCurNear + 1;			
						osg::Vec3d safePoint;

						bool foundCollision = false;
						foundCollision = GetIntersectPointWithTerrain(radius,_center,rGroup,safePoint);

						if(foundCollision)
						{
							setCenter(safePoint);
						}
					}
				}		
			}
		}
		else
		{
			PreEarthManipulator::recalculateCenter(frame);
		}
	}

	void FreeEarthManipulator::configureDefaultSettings()
	{  
		m_settings = new Settings();

		_continuousPow = -0.1;

		m_settings->setMinMaxPitch(-179.5,179.5);
		m_settings->setMinMaxDistance(0.0,DBL_MAX);
		m_settings->setThrowDecayRate(0.01);

		// install default action bindings:
		ActionOptions options;

		m_settings->bindKey( ACTION_HOME, osgGA::GUIEventAdapter::KEY_Space );

		m_settings->bindMouse( ACTION_PAN, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON );
		//_settings->bindMouse( ACTION_EARTH_DRAG, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON );

		// zoom as you hold the right button:
		options.clear();
		options.add( OPTION_CONTINUOUS, true );

		// zoom with the scroll wheel:
		m_settings->bindScroll( ACTION_ZOOM_IN,  osgGA::GUIEventAdapter::SCROLL_DOWN );
		m_settings->bindScroll( ACTION_ZOOM_OUT, osgGA::GUIEventAdapter::SCROLL_UP );

		m_settings->bindMouse( ACTION_ZOOM, osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON, 0L, options );

		m_settings->bindMouse( ACTION_ROTATE, osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON );
		m_settings->bindMouse( ACTION_ROTATE, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON | osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON );

		// pan around with arrow keys:
		m_settings->bindKey( ACTION_PAN_LEFT,  osgGA::GUIEventAdapter::KEY_Left );
		m_settings->bindKey( ACTION_PAN_RIGHT, osgGA::GUIEventAdapter::KEY_Right );
		m_settings->bindKey( ACTION_PAN_UP,    osgGA::GUIEventAdapter::KEY_Up );
		m_settings->bindKey( ACTION_PAN_DOWN,  osgGA::GUIEventAdapter::KEY_Down );

		// double click the left button to zoom in on a point:
		options.clear();
		options.add( OPTION_GOTO_RANGE_FACTOR, 0.4 );
		m_settings->bindMouseDoubleClick( ACTION_GOTO, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, 0L, options );

		// double click the right button (or CTRL-left button) to zoom out to a point
		options.clear();
		options.add( OPTION_GOTO_RANGE_FACTOR, 2.5 );
		m_settings->bindMouseDoubleClick( ACTION_GOTO, osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON, 0L, options );
		m_settings->bindMouseDoubleClick( ACTION_GOTO, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, osgGA::GUIEventAdapter::MODKEY_CTRL, options );

		// map multi-touch pinch to a discrete zoom
		options.clear();
		m_settings->bindPinch( ACTION_ZOOM, options );

		options.clear();
		m_settings->bindTwist( ACTION_ROTATE, options );
		m_settings->bindMultiDrag( ACTION_ROTATE, options );

		//_settings->setThrowingEnabled( false );
		m_settings->setLockAzimuthWhilePanning( true );

		applySettings(m_settings);
	}

	void FreeEarthManipulator::zoom( double dx, double dy )
	{
		if(m_bGoogleModeEnable && m_opRenderContext.valid())
		{
			osg::Vec3d sp;
			double alt = m_opRenderContext->getCamAltFromTerrain();

			osg::Matrixd viewMat = getMatrix();
			osg::Vec3d lookVector(- viewMat(2,0),-viewMat(2,1),-viewMat(2,2));
			lookVector.normalize();
			osg::Vec3d centerVector = -_center;
			centerVector.normalize();

			double dotLC = centerVector * lookVector;
			dotLC = osg::clampBetween(dotLC,-1.0,1.0);
			double angle = acos(dotLC);
			angle = osg::RadiansToDegrees(angle);
			if(angle > 90)
				angle = 180 - angle;
			if(angle > 60)
				angle = 60;
			angle = osg::DegreesToRadians(angle);

			double altDis = alt * dy;
			double zoomDis = altDis / cos(angle);

			storeParameter();

			osg::Vec3d newCenter = _center + lookVector * zoomDis;
			setCenter(newCenter);

			if(CheckCollision(m_v3dOldCenter,_center,sp,sp) || (_center.length() > m_dMaxZoomDis && dy < 0))
			{
				restoreParameter();
			}
		}
		else
		{
			PreEarthManipulator::zoom(dx,dy);
		}
	}

	void FreeEarthManipulator::zoomToPoint(double dx, double dy)
	{
		if(m_bLastZoomIntersected)
		{
			m_bIsZooming = true;

			storeParameter();

			double curAltDelta = _center.length() - m_v3dLastZoomIntersect.length();
			if(curAltDelta <= 0)
				return;

			osg::Vec3d centerAxis = _center;
			centerAxis.normalize();
			osg::Vec3d tarAxis = m_v3dLastZoomIntersect;
			tarAxis.normalize();
			osg::Vec3d curRotateAxis = centerAxis ^ tarAxis;
			curRotateAxis.normalize();

			double dotLC = centerAxis * tarAxis;
			dotLC = osg::clampBetween(dotLC,-1.0,1.0);
			double curAngleDelta = acos(dotLC);

			double minScale = 1.0;
			double maxScale = 4.0;
			double customDis = 2e6;

			double curScale = (m_dLastZoomDis * (minScale - maxScale))/customDis + maxScale;
			curScale = osg::clampBetween(curScale,minScale,maxScale);

			if((_center - m_v3dLastZoomIntersect).length() > m_dLastZoomDis * curScale && dy < 0)
			{
				osg::Matrixd viewMat = getMatrix();
				osg::Vec3d lookVector(- viewMat(2,0),-viewMat(2,1),-viewMat(2,2));
				lookVector.normalize();

				double zoomDis = curAltDelta * abs(dy);
				osg::Vec3d newCenter = _center - lookVector * zoomDis;
				if(newCenter.length() > m_dMaxZoomDis && dy < 0)
					return;
				setCenter(newCenter);

				osg::Vec3d rightVector = lookVector ^ -tarAxis;
				rightVector.normalize();
				dotLC = lookVector * -tarAxis;
				dotLC = osg::clampBetween(dotLC,-1.0,1.0);
				double lookAngle = acos(dotLC);

				osg::Matrixd transMat;
				transMat.makeIdentity();
				transMat.makeTranslate(-m_v3dLastZoomIntersect);

				lookAngle = abs(dy) * lookAngle;
				osg::Quat q(lookAngle,rightVector);
				osg::Matrixd rotateMat(q);

				newCenter = _center * transMat * rotateMat * transMat.inverse(transMat);
				if(newCenter.length() > m_dMaxZoomDis && dy < 0)
					return;
				setCenter(newCenter);
				osg::Quat curQuat = _rotation * _centerRotation * q;
				_centerRotation = makeCenterRotation(_center);
				_rotation = curQuat * _centerRotation.inverse();
			}
			else
			{
				double zoomDis = curAltDelta * dy;
				osg::Vec3d newCenter = _center - centerAxis * zoomDis;
				if(newCenter.length() > m_dMaxZoomDis && dy < 0)
					return;
				if((newCenter - m_v3dLastZoomIntersect).length() < 10 && dy > 0)
					return;

				setCenter(newCenter);

				double zoomAngle = curAngleDelta * dy;
				osg::Quat q;
				q.makeRotate(zoomAngle,curRotateAxis);
				osg::Matrixd mat(q);

				newCenter = _center * mat;
				if(newCenter.length() > m_dMaxZoomDis && dy < 0)
					return;
				setCenter(newCenter);	
				osg::Quat curQuat = _rotation * _centerRotation * q;
				_centerRotation = makeCenterRotation(newCenter);
				_rotation = curQuat * _centerRotation.inverse();
			}

			osg::Vec3d tmp;
			if(CheckCollision(m_v3dOldCenter,_center,tmp,tmp))
			{
				restoreParameter();
			}
		}
	}

	void FreeEarthManipulator::zoom( double dx, double dy, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		const double epsilon = 0.5;
		const double timeEpsilon = 1e-4;
		if((ea.getX() > m_fLastX + 0.5 || ea.getX() < m_fLastX - 0.5 || ea.getY() > m_fLastY + 0.5 || ea.getY() < m_fLastY - 0.5)
			&& (_task->_duration_s > m_dScrollDuration - timeEpsilon) && (_task->_duration_s < m_dScrollDuration + timeEpsilon))		
		{
			m_bLastZoomIntersected = screenToWorld(ea.getX(),ea.getY(),aa.asView(),m_v3dLastZoomIntersect,m_bLastIntersectWithTerrain);
			if(m_bLastZoomIntersected)
			{
				m_dLastZoomDis = (_center - m_v3dLastZoomIntersect).length();
				if(_center.length() < m_v3dLastZoomIntersect.length())
				{
					m_bLastZoomIntersected = false;
				}	

				if(m_bLastZoomIntersected && m_billboardMT.valid() && m_billboard.valid()) 
				{
					m_billboardMT->setPosition(m_v3dLastZoomIntersect);
					m_bIsZooming = true;
				}
			}

			m_fLastX = ea.getX();
			m_fLastY = ea.getY();
		}
		if(m_bLastZoomIntersected)
		{
			zoomToPoint(dx,dy);
		}
		else
		{
			zoom(dx,dy);
		}
	}

	void FreeEarthManipulator::setByLookAt(const osg::Vec3d& eye, const osg::Vec3d& center, const osg::Vec3d& up)
	{
		if(m_bGoogleModeEnable)
		{	
			setCenter( eye );
			setDistance( 0.0 );

			osg::Matrixd rotation_matrix = osg::Matrixd::lookAt(eye,center,up);

			_centerRotation = getRotation( _center ).getRotate().inverse();
			_rotation = rotation_matrix.getRotate().inverse() * _centerRotation.inverse();	
		}
		else
		{
			PreEarthManipulator::setByLookAt(eye,center,up);
		}
	}

	void FreeEarthManipulator::setByMatrix(const osg::Matrixd& matrix)
	{
		if(m_bGoogleModeEnable)
		{
			osg::Vec3d lookVector(- matrix(2,0),-matrix(2,1),-matrix(2,2));
			osg::Vec3d eye(matrix(3,0),matrix(3,1),matrix(3,2));

			setCenter( eye );
			setDistance( 0.0 );
			_centerRotation = makeCenterRotation(_center);
			_rotation = matrix.getRotate() * _centerRotation.inverse();	
		}
		else
		{
			PreEarthManipulator::setByMatrix(matrix);
		}
	}

	void FreeEarthManipulator::updateEarthIntersectPoint(double radius)
	{
		m_bCurIntersectWithEarth = false;

		osg::Vec3d start,end,lookVec,intersectPoint1,intersectPoint2;
		start = _center;
		end = start;

		osg::Matrixd viewMat = getMatrix();
		lookVec.set(-viewMat(2,0),-viewMat(2,1),-viewMat(2,2));
		lookVec.normalize();

		if(m_opRenderContext.valid())
		{
			osg::Camera* cam = m_opRenderContext->GetCamera();
			if(cam)
			{
				osg::Matrixd projMat = cam->getProjectionMatrix();
				double farClip,temp;
				projMat.getPerspective(temp,temp,temp,farClip);

				end = end + lookVec * farClip;

				m_bCurIntersectWithEarth = FeMath::intersectWithEllipse(radius,radius,radius,
					start,end,intersectPoint1,intersectPoint2);

				if(m_bCurIntersectWithEarth)
				{
					double length1 = (intersectPoint1 - start).length();
					double length2 = (intersectPoint2 - start).length();

					m_v3dCurIntersectWithEarth = length1 < length2 ? intersectPoint1 : intersectPoint2;
				}					
			}
		}		
	}

	bool FreeEarthManipulator::getOldModeViewPoint(osgEarth::Viewpoint& v)
	{
		if(m_bCurIntersectWithEarth)
		{
			double rangeDis = (_center - m_v3dCurIntersectWithEarth).length();
			if(rangeDis < 0)
				return false;

			osg::Quat centerQuat = makeCenterRotation(m_v3dCurIntersectWithEarth);
			osg::Matrixd mat = getMatrix();
			osg::Quat rotateQuat = mat.getRotate() * centerQuat.inverse();

			osg::CoordinateFrame centerLocalToWorld;
			createLocalCoordFrame( m_v3dCurIntersectWithEarth, centerLocalToWorld );

			osg::Matrix m = getMatrix() * osg::Matrixd::inverse(centerLocalToWorld);
			osg::Vec3d look = -getUpVector( m );
			osg::Vec3d up   =  getFrontVector( m );

			look.normalize();
			up.normalize();

			double out_azim,out_pitch;

			if ( look.z() < -0.9 )
				out_azim = atan2( up.x(), up.y() );
			else if ( look.z() > 0.9 )
				out_azim = atan2( -up.x(), -up.y() );
			else
				out_azim = atan2( look.x(), look.y() );

			out_azim = normalizeAzimRad( out_azim );
			out_pitch = asin( look.z() );


			osg::Vec3d focal_point = m_v3dCurIntersectWithEarth;
			if ( getSRS() && _is_geocentric )
			{
				getSRS()->getEllipsoid()->convertXYZToLatLongHeight(
					m_v3dCurIntersectWithEarth.x(), m_v3dCurIntersectWithEarth.y(), m_v3dCurIntersectWithEarth.z(),
					focal_point.y(), focal_point.x(), focal_point.z() );

				focal_point.x() = osg::RadiansToDegrees( focal_point.x() );
				focal_point.y() = osg::RadiansToDegrees( focal_point.y() );
			}

			osgEarth::GeoPoint gp(v.focalPoint().get().getSRS(),
				focal_point.x(),
				focal_point.y(),
				focal_point.z());
			v.focalPoint() = gp;
			//v.setFocalPoint(focal_point);
			v.setHeading(osg::RadiansToDegrees(out_azim));
			v.setPitch(osg::RadiansToDegrees(out_pitch));
			v.setRange(rangeDis);

			return true;
		}
		else
			return false;
	}

	void FreeEarthManipulator::setGoogleModeViewPoint(osgEarth::Viewpoint& vp)
	{
		storeParameter();

		osg::Vec3d new_center;
		new_center.x() = vp.focalPoint().get().x();
		new_center.y() = vp.focalPoint().get().y();
		new_center.z() = vp.focalPoint().get().z();

		if ( getSRS() )
		{
			if ( _is_geocentric )
			{
				osg::Vec3d geocentric;

				getSRS()->getEllipsoid()->convertLatLongHeightToXYZ(
					osg::DegreesToRadians( new_center.y() ),
					osg::DegreesToRadians( new_center.x() ),
					new_center.z(),
					geocentric.x(), geocentric.y(), geocentric.z() );

				new_center = geocentric;            
			}
		}

		double new_pitch = osg::DegreesToRadians(
			osg::clampBetween( vp.getPitch(), _settings->getMinPitch(), _settings->getMaxPitch() ) );

		double new_azim = normalizeAzimRad( osg::DegreesToRadians( vp.getHeading() ) );

		setCenter( new_center );
		setDistance( vp.getRange() );

		_previousUp = getUpVector( _centerLocalToWorld );
		_centerRotation = getRotation( new_center ).getRotate().inverse();

		osg::Quat azim_q( new_azim, osg::Vec3d(0,0,1) );
		osg::Quat pitch_q( -new_pitch -osg::PI_2, osg::Vec3d(1,0,0) );
		osg::Matrix new_rot = osg::Matrixd( azim_q * pitch_q );
		_rotation = osg::Matrixd::inverse(new_rot).getRotate();

		setByMatrix(getMatrix());

		vp = getViewpoint();

		restoreParameter();
	}

	void FreeEarthManipulator::setViewpoint( const osgEarth::Viewpoint& oldVp, double duration_s ,bool testFlag)
	{
		if(m_bGoogleModeEnable)
		{
			osgEarth::Viewpoint vp = oldVp;
			if(vp.getRange() > 1e-3 && duration_s <= 0.0 && !testFlag)
				setGoogleModeViewPoint(vp);

			if ( duration_s > 0.0 )
			{
				// xform viewpoint into map SRS
				osg::Vec3d vpFocalPoint;
				vpFocalPoint.x() = vp.focalPoint().get().x();
				vpFocalPoint.y() = vp.focalPoint().get().y();
				vpFocalPoint.z() = vp.focalPoint().get().z();

				if ( _cached_srs.valid() && vp.focalPoint().get().getSRS() && !_cached_srs->isEquivalentTo( vp.focalPoint().get().getSRS() ) )
				{
					osg::Vec3d fp;
					fp.x() = vp.focalPoint().get().x();
					fp.y() = vp.focalPoint().get().y();
					fp.z() = vp.focalPoint().get().z();
					vp.focalPoint().get().getSRS()->transform( fp, _cached_srs.get(), vpFocalPoint );
				}

				_start_viewpoint = getViewpoint();

				if(testFlag)
				{
					bool flag = getOldModeViewPoint(_start_viewpoint);
					if(!flag)
						return;
				}

				_delta_heading = vp.getHeading() - _start_viewpoint.getHeading(); //TODO: adjust for crossing -180
				_delta_pitch   = vp.getPitch() - _start_viewpoint.getPitch();

				if(testFlag)
				{					
					_delta_range = vp.getRange() - _start_viewpoint.getRange();
					osg::Vec3d fp;
					fp.x() = _start_viewpoint.focalPoint().get().x();
					fp.y() = _start_viewpoint.focalPoint().get().y();
					fp.z() = _start_viewpoint.focalPoint().get().z();
					_delta_focal_point = vpFocalPoint - fp; // TODO: adjust for lon=180 crossing
				}
				else
				{
					_delta_range = vp.focalPoint().get().z() - _start_viewpoint.focalPoint().get().z();
					osg::Vec3d fp;
					fp.x() = _start_viewpoint.focalPoint().get().x();
					fp.y() = _start_viewpoint.focalPoint().get().y();
					fp.z() = _start_viewpoint.focalPoint().get().z();
					_delta_focal_point = vpFocalPoint - fp; // TODO: adjust for lon=180 crossing
				}

				while( _delta_heading > 180.0 ) _delta_heading -= 360.0;
				while( _delta_heading < -180.0 ) _delta_heading += 360.0;

				// adjust for geocentric date-line crossing
				if ( _is_geocentric )
				{
					while( _delta_focal_point.x() > 180.0 ) _delta_focal_point.x() -= 360.0;
					while( _delta_focal_point.x() < -180.0 ) _delta_focal_point.x() += 360.0;
				}

				// calculate an acceleration factor based on the Z differential
				double h0,h1,dh;
				if(testFlag)
				{
					h0 = _start_viewpoint.getRange() * sin( osg::DegreesToRadians(-_start_viewpoint.getPitch()) );
					h1 = vp.getRange() * sin( osg::DegreesToRadians( -vp.getPitch() ) );
				}
				else
				{
					h0 = _start_viewpoint.focalPoint().get().z();
					h1 = vp.focalPoint().get().z();
				}
				dh = (h1 - h0);

				// calculate the total distance the focal point will travel and derive an arc height:
				double de;
				if ( _is_geocentric && (vp.focalPoint().get().getSRS() == 0L || vp.focalPoint().get().getSRS()->isGeographic()) )
				{
					osg::Vec3d startFP;
					startFP.x() = _start_viewpoint.focalPoint().get().x();
					startFP.y() = _start_viewpoint.focalPoint().get().y();
					startFP.z() = _start_viewpoint.focalPoint().get().z();
					double x0,y0,z0, x1,y1,z1;
					_cached_srs->getEllipsoid()->convertLatLongHeightToXYZ(
						osg::DegreesToRadians( _start_viewpoint.focalPoint().get().y() ), osg::DegreesToRadians( _start_viewpoint.focalPoint().get().x() ), 0.0, x0, y0, z0 );
					_cached_srs->getEllipsoid()->convertLatLongHeightToXYZ(
						osg::DegreesToRadians( vpFocalPoint.y() ), osg::DegreesToRadians( vpFocalPoint.x() ), 0.0, x1, y1, z1 );
					de = (osg::Vec3d(x0,y0,z0) - osg::Vec3d(x1,y1,z1)).length();
				}
				else
				{
					de = _delta_focal_point.length();
				}

				_arc_height = 0.0;
				if ( _settings->getArcViewpointTransitions() )
				{         
					_arc_height = osg::maximum( de - fabs(dh), 0.0 );
				}

				// calculate acceleration coefficients
				if ( _arc_height > 0.0 )
				{
					// if we're arcing, we need seperate coefficients for the up and down stages
					double h_apex = 2.0*(h0+h1) + _arc_height;
					double dh2_up = fabs(h_apex - h0)/100000.0;
					_set_viewpoint_accel = log10( dh2_up );
					double dh2_down = fabs(h_apex - h1)/100000.0;
					_set_viewpoint_accel_2 = -log10( dh2_down );
				}
				else
				{
					// on arc => simple unidirectional acceleration:
					double dh2 = (h1 - h0)/100000.0;
					_set_viewpoint_accel = fabs(dh2) <= 1.0? 0.0 : dh2 > 0.0? log10( dh2 ) : -log10( -dh2 );
					if ( fabs( _set_viewpoint_accel ) < 1.0 ) _set_viewpoint_accel = 0.0;
				}

				if ( _settings->getAutoViewpointDurationEnabled() )
				{
					double maxDistance = _cached_srs->getEllipsoid()->getRadiusEquator();
					double ratio = osg::clampBetween( de/maxDistance, 0.0, 1.0 );
					ratio = accelerationInterp( ratio, -4.5 );
					double minDur, maxDur;
					_settings->getAutoViewpointDurationLimits( minDur, maxDur );
					duration_s = minDur + ratio*(maxDur-minDur);
				}

				// don't use _time_s_now; that's the time of the last event
				_time_s_set_viewpoint = osg::Timer::instance()->time_s();
				_set_viewpoint_duration_s = duration_s;

				_setting_viewpoint = true;

				_thrown = false;
				_task->_type = TASK_NONE;
			}
			else
			{
				osg::Vec3d new_center;
				new_center.x() = vp.focalPoint().get().x();
				new_center.y() = vp.focalPoint().get().y();
				new_center.z() = vp.focalPoint().get().z();

				if ( _is_geocentric )
				{
					osg::Vec3d geocentric;

					getSRS()->getEllipsoid()->convertLatLongHeightToXYZ(
						osg::DegreesToRadians( new_center.y() ),
						osg::DegreesToRadians( new_center.x() ),
						new_center.z(),
						geocentric.x(), geocentric.y(), geocentric.z() );

					new_center = geocentric;            
				}

				double new_pitch = osg::DegreesToRadians(
					osg::clampBetween( vp.getPitch(), _settings->getMinPitch(), _settings->getMaxPitch() ) );

				double new_azim = normalizeAzimRad( osg::DegreesToRadians( vp.getHeading() ) );

				storeParameter();

				setCenter( new_center );
				setDistance( vp.getRange() );

				_previousUp = getUpVector( _centerLocalToWorld );

				_centerRotation = getRotation( new_center ).getRotate().inverse();

				osg::Quat azim_q( new_azim, osg::Vec3d(0,0,1) );
				osg::Quat pitch_q( -new_pitch -osg::PI_2, osg::Vec3d(1,0,0) );

				osg::Matrix new_rot = osg::Matrixd( azim_q * pitch_q );

				_rotation = osg::Matrixd::inverse(new_rot).getRotate();

				osg::Vec3d tp;
				bool flag = CheckCollision(m_v3dOldCenter,_center,tp,tp);
				if(flag)
				{
					restoreParameter();
				}
			}
		}
		else
		{
			PreEarthManipulator::setViewpoint(oldVp,duration_s);
		}		
	}

	bool FreeEarthManipulator::getIntersectPointFromSky(float x,float y,osg::Camera* cam,osg::Vec3d& outPoint)
	{
		if(!cam)
			return false;

		osg::Vec3d start,end;
		osg::ref_ptr<osg::Camera> rpCam = new osg::Camera(*cam);
		bool rayFlag = screenToRay(x,y,rpCam,start,end);
		if(!rayFlag)
			return false;

		osg::Matrixd viewMat = cam->getInverseViewMatrix();
		osg::Vec3d lookVector(-viewMat(2,0),-viewMat(2,1),-viewMat(2,2));
		lookVector.normalize();
		osg::Vec3d planePoint(0,0,0);
		osg::Plane tmpPlane(lookVector,planePoint);

		osg::Vec3d vecCameraPos = cam->getInverseViewMatrix().getTrans();
		double eyeToPlane = -tmpPlane.distance(vecCameraPos);
		m_dSkyPlaneDis = eyeToPlane * 0.1;
		double maxDis = 2000000;
		if(m_dSkyPlaneDis >= maxDis)
			m_dSkyPlaneDis = maxDis;

		planePoint = planePoint - lookVector * m_dSkyPlaneDis;
		tmpPlane.set(lookVector,planePoint);

		vector<osg::Vec3d> vl;
		vl.push_back(start);
		vl.push_back(end);
		int flag = tmpPlane.intersect(vl);
		if(flag == 0)
		{
			osg::Vec3d lineVec = end - start;
			lineVec.normalize();
			double hitLength;
			int hit = FeMath::getLinePanelIntersec(start,lineVec,planePoint,lookVector,hitLength,outPoint);

			if(hit != 0)
			{
				return true;
			}
		}

		return false;
	}

	bool FreeEarthManipulator::RayToWorld(osg::Vec3d start,osg::Vec3d end,osg::Vec3d& outPoint,osg::Vec3d& hitNormal)
	{	
		if(!_mapNode.valid())
			return false;

		osg::ref_ptr<MapNode> mapNode;
		if ( !_mapNode.lock(mapNode) )
			return false;

		osg::NodePath nodePath;
		nodePath.push_back(mapNode->getTerrain()->getGraph());
		if ( nodePath.empty() )
			return false;

		osg::ref_ptr<osgUtil::LineSegmentIntersector> picker = NULL;

		picker = new osgEarth::DPLineSegmentIntersector(osgUtil::Intersector::MODEL, start, end);	

		osgUtil::IntersectionVisitor iv(picker.get());
		iv.setTraversalMask(_intersectTraversalMask);
		nodePath.back()->accept(iv);

		if ( picker->containsIntersections() )
		{
			osgUtil::LineSegmentIntersector::Intersections& results = picker->getIntersections();
			outPoint = results.begin()->getWorldIntersectPoint();
			hitNormal = results.begin()->getWorldIntersectNormal();
			return true;
		}

		return false;
	}

	void FreeEarthManipulator::storeParameter()
	{
		m_v3dOldCenter = _center;
		m_qOldCenterRotation = _centerRotation;
		m_qOldRotation = _rotation;
		m_dOldDistance = _distance;
	}
	void FreeEarthManipulator::restoreParameter()
	{
		setCenter(m_v3dOldCenter);
		_centerRotation = m_qOldCenterRotation;
		_rotation = m_qOldRotation;
		_distance = m_dOldDistance;
	}

	void FreeEarthManipulator::mouseThrowingEvent(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		if(m_bGoogleModeEnable)
		{
			m_bPanRelease = true;
			m_bLastPanIntersected = false;
			m_bLastZoomIntersected = false;

			if(!m_bEnableThrowing)
			{
				_thrown = false;
				aa.requestContinuousUpdate( false );
			}	
		}
	}

	bool FreeEarthManipulator::screenToWorldHitTerrian(float x, float y, osg::View* view, osg::ref_ptr<osg::Camera>& out_cam, osg::Vec3d& out_coords,bool& hitTerrain) const
	{
		if(m_bGoogleModeEnable)
		{
			hitTerrain = false;

			osg::Vec3d wp;
			bool hitWorld = screenToWorldWithIntersector(x,y,view, out_cam ,wp);

			bool hitNode = false;
			osg::Vec3d op;

			double sqrDis = DBL_MAX;
			osg::Vec3d start,end;
			osg::ref_ptr<osg::Camera> cam = view->getCamera();
			screenToRay(x,y,cam,start,end);

			if(m_opRenderContext.valid() && !m_opRenderContext->GetManipulatorIntersectionList().empty())
			{
				FeUtil::CRenderContext::ManipulatorIntersectionList::iterator it = m_opRenderContext->GetManipulatorIntersectionList().begin();
				for (; it != m_opRenderContext->GetManipulatorIntersectionList().end(); it++)
				{
					if((*it).valid())
					{
						osg::Vec3d hitPoint;
						if((*it)->intersectWithRay(start,end,hitPoint))
						{
							double hitDis = (hitPoint - start).length2();
							if( hitDis < sqrDis)
							{
								hitNode = true;
								op = hitPoint;
								sqrDis = hitDis;
							}
						}
					}
				}
			}

			osg::Vec3d maskHit;
			if(screenToMaskNodeIntersector(x,y,view,maskHit))
			{
				double hitDis = (maskHit - start).length2();
				if( hitDis < sqrDis)
				{
					hitNode = true;
					op = maskHit;
				} 
			}

			if(hitNode && hitWorld)
			{
				double dis1 = (wp - _center).length();
				double dis2 = (op - _center).length();
				if(dis1 < dis2)
				{
					out_coords = wp;
					hitTerrain = true;
				}
				else
					out_coords = op;
			}
			else if(hitWorld)
			{
				out_coords = wp;
				hitTerrain = true;
			}
			else if(hitNode)
			{
				out_coords = op;
			}
			else
			{
				return false;
			}

			return true;
		}
		else
		{
			return screenToWorldWithIntersector(x,y,view, out_cam,out_coords);
		}
	}

	bool FreeEarthManipulator::screenToWorld(float x, float y, osg::View* view, osg::Vec3d& out_coords,bool& hitTerrain) const
	{
		if(m_bGoogleModeEnable)
		{
			hitTerrain = false;

			osg::Vec3d wp;
			bool hitWorld = PreEarthManipulator::screenToWorld(x,y,view,wp);

			bool hitNode = false;
			osg::Vec3d op;

			double sqrDis = DBL_MAX;
			osg::Vec3d start,end;
			osg::ref_ptr<osg::Camera> cam = view->getCamera();
			screenToRay(x,y,cam,start,end);

			if(m_opRenderContext.valid() && !m_opRenderContext->GetManipulatorIntersectionList().empty())
			{
				FeUtil::CRenderContext::ManipulatorIntersectionList::iterator it = m_opRenderContext->GetManipulatorIntersectionList().begin();
				for (; it != m_opRenderContext->GetManipulatorIntersectionList().end(); it++)
				{
					if((*it).valid())
					{
						osg::Vec3d hitPoint;
						if((*it)->intersectWithRay(start,end,hitPoint))
						{
							double hitDis = (hitPoint - start).length2();
							if( hitDis < sqrDis)
							{
								hitNode = true;
								op = hitPoint;
								sqrDis = hitDis;
							}
						}
					}
				}
			}

			osg::Vec3d maskHit;
			if(screenToMaskNodeIntersector(x,y,view,maskHit))
			{
				double hitDis = (maskHit - start).length2();
				if( hitDis < sqrDis)
				{
					hitNode = true;
					op = maskHit;
				} 
			}

			if(hitNode && hitWorld)
			{
				double dis1 = (wp - _center).length();
				double dis2 = (op - _center).length();
				if(dis1 < dis2)
				{
					out_coords = wp;
					hitTerrain = true;
				}
				else
					out_coords = op;
			}
			else if(hitWorld)
			{
				out_coords = wp;
				hitTerrain = true;
			}
			else if(hitNode)
			{
				out_coords = op;
			}
			else
			{
				return false;
			}

			return true;
		}
		else
		{
			return PreEarthManipulator::screenToWorld(x,y,view,out_coords);
		}
	}

	void FreeEarthManipulator::mousePushEvent(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, const Action& action)
	{
		if(m_bGoogleModeEnable && _settings.valid())
		{
			if(action._type == PreEarthManipulator::ACTION_PAN)
			{
				m_v3dLastCenter = _center;
				m_qLastQuat = _rotation * _centerRotation;

				bool hitTerrain; // 接触了地球，即有了碰撞
				m_bLastPanIntersected = screenToWorldHitTerrian(ea.getX(),ea.getY(),aa.asView(),m_rpCameraCopy,m_v3dLastPanIntersect,hitTerrain);

				osg::Vec3d skyPoint;  
				m_bLastPanSkyIntersected = getIntersectPointFromSky(ea.getX(),ea.getY(),m_rpCameraCopy.get(),skyPoint);

				if(m_bLastPanIntersected && m_bLastPanSkyIntersected)
				{
					if(m_rpCameraCopy.valid())
					{
						osg::Vec3d eye = m_rpCameraCopy->getInverseViewMatrix().getTrans();
						double dis1 = (m_v3dLastPanIntersect - eye).length2();
						double dis2 = (skyPoint - eye).length2();

						if(dis2 < dis1)
							m_v3dLastPanIntersect = skyPoint;
					}			
				}
				else if(m_bLastPanSkyIntersected)
				{
					m_v3dLastPanIntersect = skyPoint;
				}

				m_bEnableThrowing = true;
				m_bPanRelease = false;
				m_bIsRotateChecked = false;
			}
			else if(action._type == PreEarthManipulator::ACTION_ROTATE)
			{
				osg::Vec3d curPoint;
				bool curFlag;
				curFlag = screenToWorld(ea.getX(),ea.getY(),aa.asView(),curPoint,m_bLastIntersectWithTerrain);
				if(curFlag)
				{
					osg::Vec3d tp;
					m_v3dLastRotateIntersect = curPoint;
					m_bLastRotateIntersected = true;

					if(m_bLastRotateIntersected && m_billboardMT.valid() && m_billboard.valid()) 
					{
						m_billboardMT->setPosition(m_v3dLastRotateIntersect);
						m_bIsRotating = true;
					}
				}

				m_bLastPanIntersected = false;
				m_bLastPanSkyIntersected = false;
				m_bEnableThrowing = false;
				m_bIsRotateChecked = true;

				if(m_bLastRotateIntersected)
					m_bIsRotating = true;
			}
			else if(action._type == PreEarthManipulator::ACTION_ZOOM)
			{
				m_bLastZoomIntersected = screenToWorld(ea.getX(),ea.getY(),aa.asView(),m_v3dLastZoomIntersect,m_bLastIntersectWithTerrain);
				if(m_bLastZoomIntersected)
				{
					m_dLastZoomDis = (_center - m_v3dLastZoomIntersect).length();
					if(_center.length() < m_v3dLastZoomIntersect.length())
					{
						m_bLastZoomIntersected = false;
					}

					if(m_bLastZoomIntersected && m_billboardMT.valid() && m_billboard.valid()) 
					{
						m_billboardMT->setPosition(m_v3dLastZoomIntersect);
						m_bIsZooming = true;
					}
				}
			}
		}
	}

	void FreeEarthManipulator::mouseReleaseEvent(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		if(m_bGoogleModeEnable)
		{
			m_bIsRotating = false;
			m_bIsZooming = false;
		}
	}

	void FreeEarthManipulator::panThrown(double rate)
	{
		if(!m_bIsRotateChecked)
		{
			storeParameter();

			osg::Quat q;
			q.makeRotate(-m_dLastPanAngle * rate,m_v3dLastPanAxis);
			osg::Matrixd mat(q);

			osg::Vec3d center = _center * mat;

			setCenter(center);
			osg::Quat curQuat = _rotation * _centerRotation * q;
			_centerRotation = makeCenterRotation(_center);
			_rotation = curQuat * _centerRotation.inverse();

			osg::Vec3d safePoint,upPoint;
			bool flag = CheckCollision(m_v3dOldCenter,_center,safePoint,upPoint);
			if(flag)
			{
				setCenter(safePoint);
			}
		}	
	}

	void FreeEarthManipulator::panAngleAixs(osg::Vec3d lp, osg::Vec3d cp,bool checkPan)
	{
		lp.normalize();
		cp.normalize();
		double dotLC = lp * cp;
		dotLC = osg::clampBetween(dotLC,-1.0,1.0);
		double angle = acos(dotLC);
		osg::Vec3d axis = lp ^ cp;
		axis.normalize();

		osg::Quat q;
		q.makeRotate(-angle,axis);
		osg::Matrixd mat(q);

		osg::Vec3d center = m_v3dLastCenter * mat;

		setCenter(center);
		osg::Quat curQuat = m_qLastQuat * q;
		_centerRotation = makeCenterRotation(_center);
		_rotation = curQuat * _centerRotation.inverse();

		if(!m_bPanRelease)
		{
			m_dLastPanAngle = angle;
			m_v3dLastPanAxis = axis;
		}
	}

	void FreeEarthManipulator::panPush(float x, float y)
	{
		if(m_bLastPanIntersected || m_bLastPanSkyIntersected)
		{
			osg::Vec3d eye = _center;
			if(eye.length() <= m_v3dLastPanIntersect.length())
				return;

			osg::Vec3d start,end;
			bool rayFlag = screenToRay(x,y,m_rpCameraCopy,start,end);
			if(!rayFlag)
				return;

			osg::Vec3d intersectPoint,intersectPoint1,intersectPoint2;

			double radius = 6378137.0;
			if(m_bLastPanIntersected)
				radius = m_v3dLastPanIntersect.length();

			bool intersectFlag = FeMath::intersectWithEllipse(radius,radius,radius,
				start,end,intersectPoint1,intersectPoint2);

			if(intersectFlag)
			{
				double length1 = (intersectPoint1 - m_v3dLastCenter).length();
				double length2 = (intersectPoint2 - m_v3dLastCenter).length();

				intersectPoint = length1 < length2 ? intersectPoint1 : intersectPoint2;
			}

			osg::Vec3d skyPoint;
			bool skyFlag = getIntersectPointFromSky(x,y,m_rpCameraCopy,skyPoint);

			if(intersectFlag && skyFlag)
			{
				if(m_rpCameraCopy.valid())
				{
					osg::Vec3d eye = m_rpCameraCopy->getInverseViewMatrix().getTrans();
					double dis1 = (intersectPoint - eye).length2();
					double dis2 = (skyPoint - eye).length2();
					if(dis2 < dis1)
						intersectPoint = skyPoint;
				}				
			}
			else if(skyFlag)
			{
				intersectPoint = skyPoint;
			}
			else if(intersectFlag)
			{
			}
			else
			{
				return;
			}

			storeParameter();

			panAngleAixs(m_v3dLastPanIntersect,intersectPoint);

			osg::Vec3d safePoint,upPoint;
			bool flag = CheckCollision(m_v3dOldCenter,_center,safePoint,upPoint);
			if(flag)
			{
				setCenter(safePoint);
			}
		}	

	}

	void FreeEarthManipulator::pan( double dx, double dy )
	{
		if(m_bGoogleModeEnable)
		{
			if(m_bPanRelease)
			{
				double rate = pow(dx * dx + dy * dy,0.5);
				if(rate > 0.05)
					rate = 0.05;
				panThrown(rate);
			}				
			else
			{
				if(_ga_t0.valid())
					panPush(_ga_t0->getX(),_ga_t0->getY());
			}	    
		}
		else
		{
			PreEarthManipulator::pan(dx,dy);
		}
	}

	void FreeEarthManipulator::getPanAxis(osg::Vec3d& upAxis,osg::Vec3d& northAxis,osg::Vec3d& rightAxis)
	{
		osg::CoordinateFrame cf;
		createLocalCoordFrame( _center, cf );

		osg::Vec3d lookVector = getUpVector(cf);

		osg::Vec3d side;

		osg::Vec3d worldUp;
		osg::Matrixd viewMat = getMatrix();
		worldUp.set(-viewMat(2,0),-viewMat(2,1),-viewMat(2,2));

		double dot = osg::absolute(worldUp * lookVector);
		if (osg::equivalent(dot, 1.0) || osg::equivalent(dot,-1.0))
		{
			worldUp.set(viewMat(1,0),viewMat(1,1),viewMat(1,2));
		}

		side = lookVector ^ worldUp;
		osg::Vec3d up = side ^ lookVector;
		up.normalize();

		upAxis = -lookVector;
		upAxis.normalize();
		northAxis = up;
		northAxis.normalize();
		rightAxis = side;
		rightAxis.normalize();
	}

	void FreeEarthManipulator::rotate( double dx, double dy ,const osg::Vec3d& rotateCenter)
	{
		m_bIsRotating = true;

		storeParameter();

		osg::Matrixd viewMat = getMatrix();
		osg::Vec3d headAxis = rotateCenter;
		headAxis.normalize();
		double headAngle = -dx * m_uRotateSensitivity;
		osg::Quat headQuat(headAngle,headAxis);
		osg::Matrixd headMat(headQuat);
		osg::Vec3d headCenter = _center * headMat;
		setCenter(headCenter);

		osg::Vec3d safePoint;

		osg::Quat curRotate = viewMat.getRotate();
		curRotate = curRotate * headQuat;
		_centerRotation = makeCenterRotation(_center);
		_rotation = curRotate * _centerRotation.inverse();

		osg::Vec3d upPoint;
		double cd = 5.0 + (_center - m_v3dOldCenter).length() * 0.01;

		bool flag = CheckCollision(m_v3dOldCenter,_center,safePoint,upPoint,cd);
		//bool flag = CheckCollision(_center,upPoint);
		if(flag)
		{
			setCenter(safePoint);
			//restoreParameter();
		}

		storeParameter();

		viewMat = getMatrix();
		osg::Vec3d rightVector(viewMat(0,0),viewMat(0,1),viewMat(0,2));
		rightVector.normalize();

		osg::Matrixd transMat;
		transMat.makeIdentity();
		transMat.makeTranslate(-rotateCenter);

		double tileAngle = -dy * m_uRotateSensitivity;
		osg::Quat q(tileAngle,rightVector);
		osg::Matrixd rotateMat(q);

		osg::Vec3d center = _center * transMat * rotateMat * transMat.inverse(transMat);
		setCenter(center);
		curRotate = viewMat.getRotate();
		curRotate = curRotate * q;
		_centerRotation = makeCenterRotation(_center);
		_rotation = curRotate * _centerRotation.inverse();

		double checkDis = 5.0;
		if(m_bLastRotateIntersected)
			checkDis += (_center - m_v3dLastRotateIntersect).length() * 0.001;

		flag = CheckCollision(m_v3dOldCenter,_center,safePoint,upPoint,checkDis);
		if(flag)
		{
			restoreParameter();
		}
	}

	void FreeEarthManipulator::rotate( double dx, double dy )
	{
		if(m_bGoogleModeEnable)
		{
			if(m_bLastRotateIntersected)
			{	
				rotate(dx,dy,m_v3dLastRotateIntersect);
			}
		}
		else
		{
			PreEarthManipulator::rotate(dx,dy);
		}
	}

	void FreeEarthManipulator::rotateWithCamCenter( double dx, double dy )
	{
		storeParameter();

		bool headFlag = abs(dx) > abs(dy) ? true : false;
		if(headFlag)
		{
			double addHead = 1.0;
			if(dx > 0)
				addHead *= -1.0;

			osgEarth::Viewpoint p = getViewpoint();
			p.setHeading(p.getHeading() + addHead);
			setViewpoint(p);
		}
		else
		{			
			double addPitch = 0.25;
			if(dy < 0)
				addPitch *= -1.0;

			osgEarth::Viewpoint p = getViewpoint();

			double tarPitch = p.getPitch() + addPitch;
			if(tarPitch < -90 || tarPitch > 90)
				return;

			p.setPitch(tarPitch);
			setViewpoint(p);
		}

		osg::Vec3d tmp;
		if(CheckCollision(m_v3dOldCenter,_center,tmp,tmp))
		{
			restoreParameter();
		}
	}

	void FreeEarthManipulator::panThroughCtrl(double dx, double dy)
	{
		if(!m_opRenderContext.valid())
			return;

		bool panFlag = abs(dx) > abs(dy) ? true : false;

		//////////////////////////////////////////////////////////////////////////
		///c00005 
		///根据视距调整每次旋转的角度值,视距从大变小的过程中，旋转的角度值逐渐变小
		///但角度始终保持在0.000001和0.02之间，经验值
		double alt = m_opRenderContext->getCamAltFromTerrain();
		double angle = 0.02 * alt / 30000000.0;
		if (angle > 0.02)
		{
			angle = 0.02;
		}
		if (angle <= 0.0)
		{
			angle = 0.000001;
		}
		//////////////////////////////////////////////////////////////////////////

		osg::Vec3d axis,tmp;
		if(panFlag)
		{
			if(dx > 0)
				angle *= -1.0;

			getPanAxis(tmp,axis,tmp);
		}
		else
		{
			if(dy > 0)
				angle *= -1.0;

			getPanAxis(tmp,tmp,axis);
		}

		osg::Quat q;
		q.makeRotate(angle,axis);
		osg::Matrixd mat(q);

		osg::Vec3d center = _center * mat;

		storeParameter();

		setCenter(center);
		osg::Quat curQuat = _rotation * _centerRotation * q;
		_centerRotation = makeCenterRotation(_center);
		_rotation = curQuat * _centerRotation.inverse();

		if(CheckCollision(m_v3dOldCenter,_center,tmp,tmp))
		{
			restoreParameter();
		}
	}

	void FreeEarthManipulator::updateBillboard(osg::Vec3d corner)
	{
		if(m_billboard.valid() && m_billboard->getNumDrawables() > 0)
		{
			osg::Geometry* gem = dynamic_cast<osg::Geometry*>(m_billboard->getDrawable(0));
			if(gem)
			{
				osg::Vec3dArray* vertexArr = dynamic_cast<osg::Vec3dArray*>(gem->getVertexArray());
				if(vertexArr)
				{
					osg::Vec3d v0 = (*vertexArr)[0];
					osg::Vec3d v1 = (*vertexArr)[1];
					osg::Vec3d v3 = (*vertexArr)[3];

					osg::Vec3d width = v1 - v0;
					osg::Vec3d height = v3 - v0;

					(*vertexArr)[0] = corner;
					(*vertexArr)[1] = corner + width;
					(*vertexArr)[2] = corner + width + height;
					(*vertexArr)[3] = corner + height;

					gem->setVertexArray(vertexArr);
				}
			}
		}
	}

	osg::Drawable* FreeEarthManipulator::createSquare(const osg::Vec3d& corner,const osg::Vec3d& width,const osg::Vec3d& height, osg::Image* image)
	{
		osg::Geometry* geom = new osg::Geometry;

		osg::Vec3dArray* coords = new osg::Vec3dArray(4);
		(*coords)[0] = corner;
		(*coords)[1] = corner + width;
		(*coords)[2] = corner + width+height;
		(*coords)[3] = corner + height;

		geom->setVertexArray(coords);

		osg::Vec3dArray* norms = new osg::Vec3dArray(1);
		(*norms)[0] = width ^ height;
		(*norms)[0].normalize();

		geom->setNormalArray(norms, osg::Array::BIND_PER_VERTEX);

		osg::Vec2dArray* tcoords = new osg::Vec2dArray(4);
		(*tcoords)[0].set(0.0f,0.0f);
		(*tcoords)[1].set(1.0f,0.0f);
		(*tcoords)[2].set(1.0f,1.0f);
		(*tcoords)[3].set(0.0f,1.0f);
		geom->setTexCoordArray(0,tcoords);

		geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));

		if (image)
		{
			osg::StateSet* stateset = new osg::StateSet;
			osg::Texture2D* texture = new osg::Texture2D;
			texture->setImage(image);
			stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);

			osg::ref_ptr<osg::TexEnv> texEnv = new osg::TexEnv();
			texEnv->setMode(osg::TexEnv::REPLACE);
			stateset->setTextureAttribute(0,texEnv.get());  

			geom->setStateSet(stateset);
		}

		geom->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
		geom->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);  
		//geom->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN ); 
		geom->getOrCreateStateSet()->setRenderBinDetails(1000,"DepthSortedBin");

		geom->setEnableReflection(false);

		return geom;
	}

	bool FreeEarthManipulator::screenToWorldWithIntersector(float x, float y, osg::View* theView,osg::ref_ptr<osg::Camera>& out_cam, osg::Vec3d& out_coords) const
	{
		if(!_mapNode.valid())
			return false;

		osg::ref_ptr<MapNode> mapNode;
		if ( !_mapNode.lock(mapNode) )
			return false;

		osgViewer::View* view2 = dynamic_cast<osgViewer::View*>(theView);
		if ( !view2 || !mapNode->getTerrain()->getGraph())
			return false;

		osgUtil::LineSegmentIntersector::Intersections intersections;

		osg::NodePath nodePath;
		nodePath.push_back( mapNode->getTerrain()->getGraph());

		// New code, uses the code from osg::View::computeIntersections but uses our DPLineSegmentIntersector instead to get around floating point precision issues.
		float local_x, local_y = 0.0;
		const osg::Camera* camera = view2->getCameraContainingPosition(x, y, local_x, local_y);
		if (!camera) camera = view2->getCamera();

		out_cam = new osg::Camera(*camera);

		osg::Matrixd matrix;
		if (nodePath.size()>1)
		{
			osg::NodePath prunedNodePath(nodePath.begin(),nodePath.end()-1);
			matrix = osg::computeLocalToWorld(prunedNodePath);
		}

		matrix.postMult(camera->getViewMatrix());
		matrix.postMult(camera->getProjectionMatrix());

		double zNear = -1.0;
		double zFar = 1.0;
		if (camera->getViewport())
		{
			matrix.postMult(camera->getViewport()->computeWindowMatrix());
			zNear = 0.0;
			zFar = 1.0;
		}

		osg::Matrixd inverse;
		inverse.invert(matrix);

		osg::Vec3d startVertex = osg::Vec3d(local_x,local_y,zNear) * inverse;
		osg::Vec3d endVertex = osg::Vec3d(local_x,local_y,zFar) * inverse;

		// Use a double precision line segment intersector
		//osg::ref_ptr< osgUtil::LineSegmentIntersector > picker = new osgUtil::LineSegmentIntersector(osgUtil::Intersector::MODEL, startVertex, endVertex);
		osg::ref_ptr< DPLineSegmentIntersector > picker = new DPLineSegmentIntersector(osgUtil::Intersector::MODEL, startVertex, endVertex);

		// Limit it to one intersection, we only care about the first
		picker->setIntersectionLimit( osgUtil::Intersector::LIMIT_NEAREST );

		osgUtil::IntersectionVisitor iv(picker.get());
		nodePath.back()->accept(iv);

		if (picker->containsIntersections())
		{        
			intersections = picker->getIntersections();
			// find the first hit under the mouse:
			osgUtil::LineSegmentIntersector::Intersection first = *(intersections.begin());
			out_coords = first.getWorldIntersectPoint();        
			return true;       
		}
		return false;    
	}

	bool FreeEarthManipulator::screenToRay(float x, float y, osg::ref_ptr<osg::Camera>& cam, osg::Vec3d& start,osg::Vec3d& end) const
	{
		if(!cam.valid() || !_mapNode.valid())
			return false;

		osg::Viewport* viewport = cam->getViewport();

		double local_x,local_y;
		if ( viewport )
		{
			double new_x = x;
			double new_y = y;

			const double epsilon = 0.5;

			if (
				new_x >= (viewport->x()-epsilon) && new_y >= (viewport->y()-epsilon) &&
				new_x < (viewport->x()+viewport->width()-1.0+epsilon) && new_y <= (viewport->y()+viewport->height()-1.0+epsilon) )
			{
				local_x = new_x;
				local_y = new_y;
			}
			else
				return false;
		}
		else
			return false;

		osg::ref_ptr<MapNode> mapNode;
		if ( !_mapNode.lock(mapNode) )
			return false;

		osg::NodePath nodePath;
		nodePath.push_back(mapNode->getTerrain()->getGraph());

		osg::Matrixd matrix;
		if (nodePath.size()>1)
		{
			osg::NodePath prunedNodePath(nodePath.begin(),nodePath.end()-1);
			matrix = osg::computeLocalToWorld(prunedNodePath);
		}

		matrix.postMult(cam->getViewMatrix());
		matrix.postMult(cam->getProjectionMatrix());

		double zNear = -1.0;
		double zFar = 1.0;
		if (cam->getViewport())
		{
			matrix.postMult(cam->getViewport()->computeWindowMatrix());
			zNear = 0.0;
			zFar = 1.0;
		}

		osg::Matrixd inverse;
		inverse.invert(matrix);

		start = osg::Vec3d(local_x,local_y,zNear) * inverse;
		end   = osg::Vec3d(local_x,local_y,zFar) * inverse;

		return true;
	}

#define VP_MIN_DURATION      2.0     // minimum fly time.
#define VP_METERS_PER_SECOND 2500.0  // fly speed
#define VP_MAX_DURATION      5.0 //8.0     // maximum fly time
#include <algorithm>

	osg::Vec3d getFrontVector(const osg::CoordinateFrame& cf) { return osg::Vec3d(cf(1,0),cf(1,1),cf(1,2)); }
	osg::Vec3d getUpVector(const osg::CoordinateFrame& cf) { return osg::Vec3d(cf(2,0),cf(2,1),cf(2,2)); }

	// Extracts azim and pitch from a quaternion that does not contain any roll.
	void
		getEulerAngles(const osg::Quat& q, double* out_azim, double* out_pitch) 
	{
		osg::Matrix m( q );

		osg::Vec3d look = -getUpVector( m );
		osg::Vec3d up   =  getFrontVector( m );

		look.normalize();
		up.normalize();

		if ( out_azim )
		{
			if ( look.z() < -0.9 )
				*out_azim = atan2( up.x(), up.y() );
			else if ( look.z() > 0.9 )
				*out_azim = atan2( -up.x(), -up.y() );
			else
				*out_azim = atan2( look.x(), look.y() );

			*out_azim = normalizeAzimRad( *out_azim );
		}

		if ( out_pitch )
		{
			*out_pitch = asin( look.z() );
		}
	}

	// normalized linear intep
	osg::Vec3d nlerp(const osg::Vec3d& a, const osg::Vec3d& b, double t) {
		double am = a.length(), bm = b.length();
		osg::Vec3d c = a*(1.0-t) + b*t;
		c.normalize();
		c *= (1.0-t)*am + t*bm;
		return c;
	}

	// linear interp
	osg::Vec3d lerp(const osg::Vec3d& a, const osg::Vec3d& b, double t) {
		return a*(1.0-t) + b*t;
	}

	osg::Matrix computeLocalToWorld(osg::Node* node) {
		osg::Matrix m;
		if ( node ) {
			osg::NodePathList nodePaths = node->getParentalNodePaths();
			if ( nodePaths.size() > 0 ) {
				m = osg::computeLocalToWorld( nodePaths[0] );
			}
			else {
				osg::Transform* t = dynamic_cast<osg::Transform*>(node);
				if ( t ) {
					t->computeLocalToWorldMatrix( m, 0L );
				}
			}
		}
		return m;
	}

	osg::Vec3d computeWorld(osg::Node* node) {
		return node ? osg::Vec3d(0,0,0) * computeLocalToWorld(node) : osg::Vec3d(0,0,0);
	}


	void FreeEarthManipulator::flyToViewpoint(const Viewpoint& vp, double dTime)
	{
		Viewpoint newVp = vp;

		if(vp.getRange() > 1e-3)
			setGoogleModeViewPoint(newVp);

		Viewpoint currentVP = getViewpoint();
		//if (dTime < 2.0) { dTime = 2.0; }
		{
			// starting viewpoint; all fields will be set:
			_setVP0 =  currentVP; //getViewpoint();

			// ending viewpoint
			_setVP1 = newVp;

			// Fill in any missing end-point data with defaults matching the current camera setup.
			// Then all fields are guaranteed to contain usable data during transition.
			double defPitch, defAzim;
			getEulerAngles( _rotation, &defAzim, &defPitch );

			if ( !_setVP1->heading().isSet() )
				_setVP1->heading() = Angle(defAzim, Units::RADIANS);

			if ( !_setVP1->pitch().isSet() )
				_setVP1->pitch() = Angle(defPitch, Units::RADIANS);

			if ( !_setVP1->range().isSet() )
				_setVP1->range() = Distance(_distance, Units::METERS);

			if ( !_setVP1->nodeIsSet() && !_setVP1->focalPoint().isSet() )
			{
				osg::ref_ptr<osg::Node> safeNode;
				if ( _setVP0->getNode( safeNode ) )
					_setVP1->setNode( safeNode.get() );
				else
					_setVP1->focalPoint() = _setVP0->focalPoint().get();
			}

			if (dTime < 0.0) { dTime = 0.0; }
			_setVPDuration.set( dTime, Units::SECONDS );

			// access the new tether node if it exists:
			osg::ref_ptr<osg::Node> endNode;
			_setVP1->getNode(endNode);

			// Timed transition, we need to calculate some things:
			if ( dTime > 0.0 )
			{
				// Start point is the current manipulator center:
				osg::Vec3d startWorld;
				osg::ref_ptr<osg::Node> startNode;
				startWorld = _setVP0->getNode(startNode) ? computeWorld(startNode.get()) : _center;

				_setVPStartTime.unset();

				// End point is the world coordinates of the target viewpoint:
				osg::Vec3d endWorld;
				if ( endNode.valid() )
					endWorld = computeWorld(endNode.get());
				else
					_setVP1->focalPoint()->transform( getSRS() ).toWorld(endWorld);

				// calculate an acceleration factor based on the Z differential.
				_setVPArcHeight = 0.0;
				double range0 = _setVP0->range()->as(Units::METERS);
				double range1 = _setVP1->range()->as(Units::METERS);

				double pitch0 = _setVP0->pitch()->as(Units::RADIANS);
				double pitch1 = _setVP1->pitch()->as(Units::RADIANS);

				double h0 = range0 * sin( -pitch0 );
				double h1 = range1 * sin( -pitch1 );
				double dh = (h1 - h0);

				// calculate the total distance the focal point will travel and derive an arc height:
				double de = (endWorld - startWorld).length();

				// maximum height during viewpoint transition
				if ( m_settings->getArcViewpointTransitions() )
				{         
					_setVPArcHeight = osg::maximum( de - fabs(dh), 0.0 );
				}

				// calculate acceleration coefficients
				if ( _setVPArcHeight > 0.0 )
				{
					// if we're arcing, we need seperate coefficients for the up and down stages
					double h_apex = 2.0*(h0+h1) + _setVPArcHeight;
					double dh2_up = fabs(h_apex - h0)/100000.0;
					_setVPAccel = log10( dh2_up );
					double dh2_down = fabs(h_apex - h1)/100000.0;
					_setVPAccel2 = -log10( dh2_down );
				}
				else
				{
					// on arc => simple unidirectional acceleration:
					double dh2 = (h1 - h0)/100000.0;
					_setVPAccel = fabs(dh2) <= 1.0? 0.0 : dh2 > 0.0? log10( dh2 ) : -log10( -dh2 );
					if ( fabs( _setVPAccel ) < 1.0 ) _setVPAccel = 0.0;
				}
			}
			else
			{
				// Immediate transition? Just do it now.
				_setVPStartTime->set( _time_s_now, Units::SECONDS );
				setViewpointFrame( _time_s_now );
			}
		}

		// reset other global state flags.
		_thrown      = false;
		_task->_type = TASK_NONE;
	}

	double
		FreeEarthManipulator::setViewpointFrame(double time_s)
	{
		if ( !_setVPStartTime.isSet() )
		{
			_setVPStartTime->set( time_s, Units::SECONDS );
			return 0.0;
		}
		else
		{
			// Start point is the current manipulator center:
			osg::Vec3d startWorld;
			osg::ref_ptr<osg::Node> startNode;
			if ( _setVP0->getNode(startNode) )
				startWorld = computeWorld(startNode);
			else
				_setVP0->focalPoint()->transform( getSRS() ).toWorld(startWorld);

			// End point is the world coordinates of the target viewpoint:
			osg::Vec3d endWorld;
			osg::ref_ptr<osg::Node> endNode;
			if ( _setVP1->getNode(endNode) )
				endWorld = computeWorld(endNode);
			else
				_setVP1->focalPoint()->transform( getSRS() ).toWorld(endWorld);

			// Remaining time is the full duration minus the time since initiation:
			double elapsed = time_s - _setVPStartTime->as(Units::SECONDS);
			//double t = std::min(1.0, elapsed / _setVPDuration.as(Units::SECONDS));
			double t = 1.0;
			if (t > elapsed / _setVPDuration.as(Units::SECONDS))
			{
				t = elapsed / _setVPDuration.as(Units::SECONDS);
			}

			double tp = t;

			if ( _setVPArcHeight > 0.0 )
			{
				if ( tp <= 0.5 )
				{
					double t2 = 2.0*tp;
					tp = 0.5*t2;
				}
				else
				{
					double t2 = 2.0*(tp-0.5);
					tp = 0.5+(0.5*t2);
				}

				// the more smoothsteps you do, the more pronounced the fade-in/out effect        
				tp = smoothStepInterp( tp );
			}
			else if ( t > 0.0 )
			{
				tp = smoothStepInterp( tp );
			}

			osg::Vec3d newCenter =
				getSRS()->isGeographic() ? nlerp(startWorld, endWorld, tp) : lerp(startWorld, endWorld, tp);

			// Calculate the delta-heading, and make sure we are going in the shortest direction:
			Angle d_azim = _setVP1->heading().get() - _setVP0->heading().get();
			if ( d_azim.as(Units::RADIANS) > osg::PI )
				d_azim = d_azim - Angle(2.0*osg::PI, Units::RADIANS);
			else if ( d_azim.as(Units::RADIANS) < -osg::PI )
				d_azim = d_azim + Angle(2.0*osg::PI, Units::RADIANS);
			double newAzim = _setVP0->heading()->as(Units::RADIANS) + tp*d_azim.as(Units::RADIANS);

			// Calculate the new pitch:
			Angle d_pitch = _setVP1->pitch().get() - _setVP0->pitch().get();
			double newPitch = _setVP0->pitch()->as(Units::RADIANS) + tp*d_pitch.as(Units::RADIANS);

			// Calculate the new range:
			Distance d_range = _setVP1->range().get() - _setVP0->range().get();
			double newRange =
				_setVP0->range()->as(Units::METERS) +
				d_range.as(Units::METERS)*tp + sin(osg::PI*tp)*_setVPArcHeight;

			// Calculate the offsets
			osg::Vec3d offset0 = _setVP0->positionOffset().getOrUse(osg::Vec3d(0,0,0));
			osg::Vec3d offset1 = _setVP1->positionOffset().getOrUse(osg::Vec3d(0,0,0));
			osg::Vec3d newOffset = offset0 + (offset1-offset0)*tp;

			newAzim = normalizeAzimRad( newAzim );

			osg::Vec3d new_center = newCenter;

			double new_pitch = osg::clampBetween( newPitch, _settings->getMinPitch(), _settings->getMaxPitch() );

			double new_azim = newAzim;

			//storeParameter();

			setCenter( new_center );
			setDistance( newRange );

			_previousUp = getUpVector( _centerLocalToWorld );

			_centerRotation = getRotation( new_center ).getRotate().inverse();

			osg::Quat azim_q( new_azim, osg::Vec3d(0,0,1) );
			osg::Quat pitch_q( -new_pitch -osg::PI_2, osg::Vec3d(1,0,0) );

			osg::Matrix new_rot = osg::Matrixd( azim_q * pitch_q );

			_rotation = osg::Matrixd::inverse(new_rot).getRotate();

			if ( t >= 1.0 )
			{            
				_setVP0.unset();

				// If this was a transition into a tether, keep the endpoint around so we can
				// continue tracking it.
				if ( !(_setVP1.isSet() && _setVP1->nodeIsSet()) )
				{
					_setVP1.unset();
				}
			}

			return tp;
		}
	}

	bool FreeEarthManipulator::screenToMaskNodeIntersector( float x, float y, osg::View* theView,osg::Vec3d& out_coords ) const
	{
		if(!m_rpMaskNodeGroup.valid())
			return false;

		osgViewer::View* view2 = dynamic_cast<osgViewer::View*>(theView);
		if ( !view2 )
			return false;

		osgUtil::LineSegmentIntersector::Intersections intersections;

		osg::NodePath nodePath;
		nodePath.push_back(m_rpMaskNodeGroup);

		// New code, uses the code from osg::View::computeIntersections but uses our DPLineSegmentIntersector instead to get around floating point precision issues.
		float local_x, local_y = 0.0;
		const osg::Camera* camera = view2->getCameraContainingPosition(x, y, local_x, local_y);
		if (!camera) camera = view2->getCamera();

		osg::Matrixd matrix;
		matrix.makeIdentity();
		matrix.postMult(camera->getViewMatrix());
		matrix.postMult(camera->getProjectionMatrix());

		double zNear = -1.0;
		double zFar = 1.0;
		if (camera->getViewport())
		{
			matrix.postMult(camera->getViewport()->computeWindowMatrix());
			zNear = 0.0;
			zFar = 1.0;
		}

		osg::Matrixd inverse;
		inverse.invert(matrix);

		osg::Vec3d startVertex = osg::Vec3d(local_x,local_y,zNear) * inverse;
		osg::Vec3d endVertex = osg::Vec3d(local_x,local_y,zFar) * inverse;

		// Use a double precision line segment intersector
		//osg::ref_ptr< osgUtil::LineSegmentIntersector > picker = new osgUtil::LineSegmentIntersector(osgUtil::Intersector::MODEL, startVertex, endVertex);
		osg::ref_ptr< DPLineSegmentIntersector > picker = new DPLineSegmentIntersector(osgUtil::Intersector::MODEL, startVertex, endVertex);

		// Limit it to one intersection, we only care about the first
		picker->setIntersectionLimit( osgUtil::Intersector::LIMIT_NEAREST );

		osgUtil::IntersectionVisitor iv(picker.get());
		nodePath.back()->accept(iv);

		if (picker->containsIntersections())
		{        
			intersections = picker->getIntersections();
			// find the first hit under the mouse:
			osgUtil::LineSegmentIntersector::Intersection first = *(intersections.begin());
			out_coords = first.getWorldIntersectPoint();        
			return true;       
		}
		return false; 
	}

	void FreeEarthManipulator::modifyFirstPersonNearFar()
	{
		if(m_opRenderContext.valid())
		{
			osg::Camera* cam = m_opRenderContext->GetCamera();
			if(cam)
			{
				FeKit::CEarthClampProjectionMatrixCallback* cb = dynamic_cast<FeKit::CEarthClampProjectionMatrixCallback*>(cam->getClampProjectionMatrixCallback());
				if(cb)
				{
					cb->SetUseCustomNearFar(false);
				}
			}
		}
	}

}
