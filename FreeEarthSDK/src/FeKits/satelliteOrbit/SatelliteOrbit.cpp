#include <osg/Geode>
#include <osg/Geometry>

#include <FeKits/satelliteOrbit/SatelliteOrbit.h>
#include <FeUtils/UtilityGeom.h>

namespace FeKit
{

    CSynSatelliteOrbit::CSynSatelliteOrbit(FeUtil::CRenderContext* pContext, const osg::Vec4d& vecColor)
        :osg::Group()
        ,CKitsCtrl(pContext)
        ,m_vecColor(vecColor)
        ,m_dOrbitRadius(36000000)
    {
        osg::Node* pNode = FeUtil::CreateParallelSphere(osg::Vec3d(), m_dOrbitRadius, m_vecColor, 1.0, false, m_vecColor);
        
        addChild(pNode);

		m_bShow ? Show() : Hide();
    }

    CSynSatelliteOrbit::~CSynSatelliteOrbit( void )
    {

    }

    void CSynSatelliteOrbit::Show()
    {
        setNodeMask(0xffffffff);
        m_bShow = true;
    }

    void CSynSatelliteOrbit::Hide()
    {
        setNodeMask(0x00000000);
        m_bShow = false;
    }
}
