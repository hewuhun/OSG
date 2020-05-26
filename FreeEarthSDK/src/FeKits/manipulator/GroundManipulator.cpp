#include <osgUtil/LineSegmentIntersector>
#include <osgViewer/View>

#include <FeUtils/CoordConverter.h>
#include <FeKits/manipulator/GroundManipulator.h>


namespace FeKit
{


    using namespace osg;
    using namespace osgGA;

    CGroundManipulator::CGroundManipulator(void)
    {
        _speed = 10;
    }


    CGroundManipulator::~CGroundManipulator(void)
    {
    }

    bool CGroundManipulator::performMovementLeftMouseButton( const double eventTimeDelta, const double dx, const double dy )
    {
        // world up vector
        CoordinateFrame coordinateFrame = getCoordinateFrame( _eye );
        Vec3d localUp = getUpVector( coordinateFrame );

        osg::Vec3 eye = _eye;
        eye.normalize();

        rotateYawPitch( _rotation, dx, dy, eye);

        return true;
    }

    bool CGroundManipulator::handleMouseWheel( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
    {
        //osgGA::GUIEventAdapter::ScrollingMotion sm = ea.getScrollingMotion();

        //// handle centering
        //if( _flags & SET_CENTER_ON_WHEEL_FORWARD_MOVEMENT )
        //{

        //	if( ((sm == GUIEventAdapter::SCROLL_DOWN) && (_wheelMovement > 0.)) ||
        //		((sm == GUIEventAdapter::SCROLL_UP)   && (_wheelMovement < 0.)) )
        //	{

        //		// stop thrown animation
        //		_thrown = false;

        //		if( getAnimationTime() <= 0. )

        //			// center by mouse intersection (no animation)
        //			setCenterByMousePointerIntersection( ea, us );

        //		else {

        //			// start new animation only if there is no animation in progress
        //			if( !isAnimating() )
        //				startAnimationByMousePointerIntersection( ea, us );

        //		}
        //	}
        //}

        //switch( sm )
        //{

        //	// mouse scroll up event
        //case GUIEventAdapter::SCROLL_UP:
        //	{
        //		// move forward
        //		moveForward( isAnimating() ? dynamic_cast< FirstPersonAnimationData* >( _animationData.get() )->_targetRot : _rotation,
        //			-_wheelMovement * (getRelativeFlag( _wheelMovementFlagIndex ) ? _modelSize : 1. ));
        //		us.requestRedraw();
        //		us.requestContinuousUpdate( isAnimating() || _thrown );
        //		return true;
        //	}

        //	// mouse scroll down event
        //case GUIEventAdapter::SCROLL_DOWN:
        //	{
        //		// move backward
        //		moveForward( _wheelMovement * (getRelativeFlag( _wheelMovementFlagIndex ) ? _modelSize : 1. ));
        //		_thrown = false;
        //		us.requestRedraw();
        //		us.requestContinuousUpdate( isAnimating() || _thrown );
        //		return true;
        //	}

        //	// unhandled mouse scrolling motion
        //default:
        //	return false;
        //}
        return false;
    }

    bool CGroundManipulator::handleKeyDown( const osgGA::GUIEventAdapter & ea, osgGA::GUIActionAdapter & us )
    {
        if( osgGA::FirstPersonManipulator::handleKeyDown( ea, us ) )
            return true;

        osgViewer::View * view = dynamic_cast<osgViewer::View*>(us.asView());

        osg::Vec3d eyePointXYZ=_eye;

        if(ea.getKey() == ea.KEY_Plus)
        {
            _speed += 100;
        }

        if(ea.getKey() == ea.KEY_Minus)
        {
            _speed -= 100;
        }

        if(( ea.getKey() == ea.KEY_Up) || (ea.getKey() == 'w') || (ea.getKey() == 'W'))
        {
            if (isHitByFrontBackLeftRight(Front))
            {
                return false;
            }
            moveForward(_speed);
        }
        if((ea.getKey() == ea.KEY_Down)  || (ea.getKey() == 's') || (ea.getKey() == 'S'))
        {
            if (isHitByFrontBackLeftRight(Back))
            {
                return false;
            }		
            moveForward(-_speed);
        }
        if((ea.getKey() == ea.KEY_Right) || (ea.getKey() == 'd') || (ea.getKey() == 'D'))
        {
            if (isHitByFrontBackLeftRight(Right))
            {
                return false;
            }
            moveRight(_speed);
        }
        if((ea.getKey() == ea.KEY_Left) || (ea.getKey() == 'a') || (ea.getKey() == 'A'))
        {
            if (isHitByFrontBackLeftRight(Left))
            {
                return false;
            }
            moveRight(-_speed);
        }
        osg::Vec3d eyePositionLLH ;
        //COORD_CONV->XYZ2RadianLLH(_eye, eyePositionLLH);
        if (eyePositionLLH.z()<200)
        {
            _eye=eyePointXYZ;
        }
        return false;
    }

    bool CGroundManipulator::isHitByFrontBackLeftRight( int direction )
    {
        // 	//获取航宇新楼的建筑，并加入到NodePath中
        // 	osg::ref_ptr<osg::MatrixTransform> buildMt = new osg::MatrixTransform; 
        // 	/*(COsgViewer::Instance()->GetCHangYuBuild())->GetHangYuBuild();*/
        // 	osg::NodePath np;
        // 	np.push_back(buildMt.get());
        // 
        // 	//获取视点的位置以及视点按指定操作移动后的位置
        // 	osg::Vec3d eyePositionNow;
        // 	osg::Quat rotationQuat;
        // 	osg::Vec3d eyePositionLater;
        // 
        // 	getTransformation(eyePositionNow, rotationQuat);
        // 
        // 	osg::ref_ptr<osgUtil::IntersectorGroup> intersectorGroup = new osgUtil::IntersectorGroup;
        // 
        // 	switch (direction)
        // 	{
        // 	case Front:
        // 		{ 
        // 			eyePositionLater = eyePositionNow + rotationQuat * osg::Vec3d(0, 0, -_speed);
        // 
        // 			osg::ref_ptr<osgUtil::LineSegmentIntersector> lineSegmentIntersector = new osgUtil::LineSegmentIntersector(eyePositionNow, eyePositionLater);
        // 			intersectorGroup->addIntersector(lineSegmentIntersector.get());
        // 			osgUtil::IntersectionVisitor intersectionVisitor(intersectorGroup);
        // 			buildMt->accept(intersectionVisitor);
        // 
        // 			if (intersectorGroup->containsIntersections())
        // 			{
        // 				return true;
        // 			}
        // 			else
        // 			{
        // 				return false;
        // 			}
        // 			break;
        // 		}
        // 	case Back:
        // 		{
        // 			eyePositionLater = eyePositionNow + rotationQuat * osg::Vec3d(0, 0, _speed);
        // 
        // 			osg::ref_ptr<osgUtil::LineSegmentIntersector> lineSegmentIntersector = new osgUtil::LineSegmentIntersector(eyePositionNow, eyePositionLater);
        // 			intersectorGroup->addIntersector(lineSegmentIntersector.get());
        // 			osgUtil::IntersectionVisitor intersectionVisitor(intersectorGroup);
        // 			buildMt->accept(intersectionVisitor);
        // 
        // 			if (intersectorGroup->containsIntersections())
        // 			{
        // 				return true;
        // 			}
        // 			else
        // 			{
        // 				return false;
        // 			}
        // 			break;
        // 		}
        // 	case Left:
        // 		{
        // 			eyePositionLater = eyePositionNow + rotationQuat * osg::Vec3d(-_speed, 0, 0);
        // 
        // 			osg::ref_ptr<osgUtil::LineSegmentIntersector> lineSegmentIntersector = new osgUtil::LineSegmentIntersector(eyePositionNow, eyePositionLater);
        // 			intersectorGroup->addIntersector(lineSegmentIntersector.get());
        // 			osgUtil::IntersectionVisitor intersectionVisitor(intersectorGroup);
        // 			buildMt->accept(intersectionVisitor);
        // 
        // 			if (intersectorGroup->containsIntersections())
        // 			{
        // 				return true;
        // 			}
        // 			else
        // 			{
        // 				return false;
        // 			}
        // 			break;
        // 		}
        // 	case Right:
        // 		{
        // 			eyePositionLater = eyePositionNow + rotationQuat * osg::Vec3d(_speed, 0, 0);
        // 
        // 			osg::ref_ptr<osgUtil::LineSegmentIntersector> lineSegmentIntersector = new osgUtil::LineSegmentIntersector(eyePositionNow, eyePositionLater);
        // 			intersectorGroup->addIntersector(lineSegmentIntersector.get());
        // 			osgUtil::IntersectionVisitor intersectionVisitor(intersectorGroup);
        // 			buildMt->accept(intersectionVisitor);
        // 
        // 			if (intersectorGroup->containsIntersections())
        // 			{
        // 				return true;
        // 			}
        // 			else
        // 			{
        // 				return false;
        // 			}
        // 			break;
        // 		}
        // 
        // 	default:
        // 		return false;
        /*	}*/
        return false;
    }

}
