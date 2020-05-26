#include "UserMaskSource.h"

CUserMaskSource::CUserMaskSource( const MaskSourceOptions& options )
{

}

void CUserMaskSource::SetBoundaryVertext( osg::Vec3dArray* pArray )
{
	if (pArray)
	{
		m_rpBoundarys = pArray;
	}
}

void CUserMaskSource::SetPosition( osg::Vec3d pos )
{
	m_vecPosition = pos;
}

void CUserMaskSource::initialize( const osgDB::Options* dbOptions )
{
	MaskSource::initialize( dbOptions );
	
	_failed = false;
}

osg::Vec3dArray* CUserMaskSource::createBoundary( const SpatialReference* srs, ProgressCallback* progress )
{
	if ( _failed )
		return 0L;

	if (m_rpBoundarys.valid())
	{
		osg::Matrix matrix;
		srs->getEllipsoid()->computeLocalToWorldTransformFromLatLongHeight(
			osg::DegreesToRadians(m_vecPosition.y()),
			osg::DegreesToRadians(m_vecPosition.x()),
			m_vecPosition.z(),
			matrix);

		osg::ref_ptr<osg::Vec3dArray> rpBoundaryWorld = new osg::Vec3dArray();

		osg::Vec3dArray::iterator it = m_rpBoundarys->begin();
		while (it != m_rpBoundarys->end())
		{
			osg::Vec3d boundXYZ = (*it)*matrix;
			osg::Vec3d boundLLH;
			srs->getEllipsoid()->convertXYZToLatLongHeight(boundXYZ.x(), boundXYZ.y(), boundXYZ.z(), boundLLH.y(), boundLLH.x(), boundLLH.z());
			boundLLH.set(osg::RadiansToDegrees(boundLLH.x()), osg::RadiansToDegrees(boundLLH.y()), boundLLH.z());

			rpBoundaryWorld->push_back(boundLLH);
			it++;
		}

		return rpBoundaryWorld.release();
	}

	return 0L;
}
