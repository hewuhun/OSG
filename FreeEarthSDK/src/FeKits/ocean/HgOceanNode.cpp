#include <FeKits/ocean/TritonNode.h>

using namespace FeKit;

HgOceanNode::HgOceanNode( osgViewer::View * viewer, const osgEarth::SpatialReference * srs )
    :osg::Group()
    ,CKitsCtrl()
	,m_pSrs(srs)
	,m_fSeaLevel(100)
	,m_pViewer(viewer)
    ,m_dMaxAltitude(20000)
{

}

void HgOceanNode::traverse( osg::NodeVisitor& nv )
{
	if ( nv.getVisitorType() == nv.CULL_VISITOR &&  m_pSrs != NULL)
	{
		osgUtil::CullVisitor* cv = osgEarth::Culling::asCullVisitor(nv);
		if ( cv->getCurrentCamera() )
		{
			osg::Vec3d eye = osg::Vec3d(0,0,0) * cv->getCurrentCamera()->getInverseViewMatrix();
			osg::Vec3d local;
			double altitude;
			m_pSrs->transformFromWorld(eye, local, &altitude);

			if (  altitude > m_dMaxAltitude )
				return;

			altitude -= (double)GetSeaLevel();

			altitude = std::max( ::fabs(altitude), 1.0 );

			osg::CullSettings::ComputeNearFarMode mode = cv->getComputeNearFarMode();

			osg::Group::traverse( nv );

			cv->setComputeNearFarMode( mode );

			double oldNear = cv->getCalculatedNearPlane();

			double newNear = std::min( oldNear, altitude );
			if ( newNear < oldNear )
			{
				cv->setCalculatedNearPlane( newNear );
			}

			return;
		}
	}
	osg::Group::traverse( nv );
}

void HgOceanNode::SetSeaLevel( float value )
{
	m_fSeaLevel = value;	
	OnSeaLevel();
}

float HgOceanNode::GetSeaLevel() const
{
	return m_fSeaLevel;
}

void HgOceanNode::OnSeaLevel()
{
    
}

const osgEarth::SpatialReference * HgOceanNode::GetSRS()
{
	return m_pSrs;
}

void HgOceanNode::Show()
{
    setNodeMask(0x11111111);
    m_bShow = true;
}

void HgOceanNode::Hide()
{
    setNodeMask(0x00000000);
    m_bShow = false;
}

void FeKit::HgOceanNode::SetWakeHeight( float value )
{
    m_fWakeHeight = value; 
}

float FeKit::HgOceanNode::GetWakeHeight() const
{
    return m_fWakeHeight;
}
