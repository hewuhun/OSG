#ifndef USER_MASK_SOURCE_H
#define USER_MASK_SOURCE_H 1

#include <osgEarth/MaskSource>

using namespace osgEarth;

class CUserMaskSource : public MaskSource
{
public:
	CUserMaskSource( const MaskSourceOptions& options );

public:
	void SetBoundaryVertext(osg::Vec3dArray* pArray);

	void SetPosition(osg::Vec3d pos);

	osg::Vec3dArray* createBoundary(const SpatialReference* srs, ProgressCallback* progress);

protected:
	void initialize(const osgDB::Options* dbOptions);

protected:
	bool _failed;

	osg::ref_ptr<osg::Vec3dArray>		m_rpBoundarys;

	osg::Vec3d							m_vecPosition;
};

#endif 







