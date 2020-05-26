#include <FeAlg/MotionStateInfo.h>
#include <osg/CoordinateSystemNode>

namespace FeAlg
{
	void CMotionStateInfo::interpolate(double radio, const CMotionStateInfo& start, const CMotionStateInfo& end)
	{
		vecLLH = start.vecLLH + (end.vecLLH - start.vecLLH) * radio;
		vecPosture = start.vecPosture + (end.vecPosture - start.vecPosture) * radio;
		double yawRadianStart = start.vecPosture.z();
		double yawRadianEnd = end.vecPosture.z();
		if (yawRadianEnd * yawRadianStart < 0.0 && fabs(yawRadianStart - yawRadianEnd) >= osg::PI)
		{
			if (yawRadianStart > yawRadianEnd)
			{
				vecPosture.z() = yawRadianStart + (osg::PI*2.0 - yawRadianStart + yawRadianEnd) * radio;
			}
			else {
				vecPosture.z() = yawRadianStart + (-osg::PI*2.0 - yawRadianStart + yawRadianEnd) * radio;
			}
		}

		dTime = start.dTime + (end.dTime - start.dTime) * radio;
		dSpeed = start.dSpeed + (end.dSpeed - start.dSpeed) * radio;
	}

	void CMotionStateInfo::getMatrix(osg::Matrixf& matrix) const
	{
		osg::Matrixd localToWorld;
		osg::EllipsoidModel ellip;
		ellip.computeLocalToWorldTransformFromLatLongHeight(
			osg::DegreesToRadians(vecLLH.y()), osg::DegreesToRadians(vecLLH.x()), vecLLH.z(), localToWorld);
		matrix = localToWorld;

		osg::Quat rotation;
		rotation.makeRotate(vecPosture.y(), osg::Y_AXIS,
			vecPosture.x(), osg::X_AXIS,
			vecPosture.z(), osg::Z_AXIS);
		matrix.preMultRotate(rotation);
	}

	void CMotionStateInfo::getMatrix(osg::Matrixd& matrix) const
	{
		osg::EllipsoidModel ellip;
		ellip.computeLocalToWorldTransformFromLatLongHeight(
			osg::DegreesToRadians(vecLLH.y()), osg::DegreesToRadians(vecLLH.x()), vecLLH.z(), matrix);

		osg::Quat rotation;
		rotation.makeRotate(vecPosture.y(), osg::Y_AXIS,
			vecPosture.x(), osg::X_AXIS,
			vecPosture.z(), osg::Z_AXIS);
		matrix.preMultRotate(rotation);
	}

}
