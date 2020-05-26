#include <FeAlg/CoordConverter.h>
#include <osg/CoordinateSystemNode>

namespace FeAlg
{
    osg::Vec3d RadianLLH2XYZ(const osg::Vec3d& vecLLH)
    {
		osg::Vec3d vecXYZ;
		osg::EllipsoidModel conver;
		conver.convertLatLongHeightToXYZ(
                vecLLH.y(), vecLLH.x(), vecLLH.z(), vecXYZ.x(), vecXYZ.y(), vecXYZ.z());
        return vecXYZ;
    }

    osg::Vec3d DegreeLLH2XYZ( const osg::Vec3d& vecLLH)
    {
		osg::Vec3d vecRadianLLH(osg::DegreesToRadians(vecLLH.x()), osg::DegreesToRadians(vecLLH.y()), vecLLH.z());
		return RadianLLH2XYZ(vecRadianLLH);
    }

    osg::Vec3d XYZ2RadianLLH(const osg::Vec3d& vecXYZ)
    {
		osg::Vec3d vecLLH;
		osg::EllipsoidModel conver;
        conver.convertXYZToLatLongHeight(
			vecXYZ.x(), vecXYZ.y(), vecXYZ.z(), vecLLH.y(), vecLLH.x(), vecLLH.z());
        return vecLLH;
    }

    osg::Vec3d XYZ2DegreeLLH(const osg::Vec3d& vecXYZ)
    {
		osg::Vec3d vecLLH = XYZ2RadianLLH(vecXYZ);
		vecLLH.x() = osg::RadiansToDegrees(vecLLH.x());
		vecLLH.y() = osg::RadiansToDegrees(vecLLH.y());
		return vecLLH;
    }

	void RadiaLLH2Matrix(const osg::Vec3d& vecLLH, osg::Matrix& matrix )
	{
		osg::EllipsoidModel conver;
		conver.computeLocalToWorldTransformFromLatLongHeight(vecLLH.y(),vecLLH.x(),vecLLH.z(),matrix);
	}

	void DegreeLLH2Matrix(const osg::Vec3d& vecLLH,osg::Matrix& matrix )
	{
		RadiaLLH2Matrix(osg::Vec3d(osg::DegreesToRadians(vecLLH.x()),osg::DegreesToRadians(vecLLH.y()),vecLLH.z()), matrix);
	}

	void XYZ2Matrix(const osg::Vec3d& vecXYZ,osg::Matrix& matrix )
	{
		osg::Vec3d vecLLHRadia = XYZ2RadianLLH(vecXYZ);
		RadiaLLH2Matrix(vecLLHRadia, matrix);
	}

}