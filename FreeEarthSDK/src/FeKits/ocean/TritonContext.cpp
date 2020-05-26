#include <FeKits/ocean/TritonContext.h>
#include <OpenThreads/ScopedLock>
#include <OpenThreads/Thread>
#include <osgEarth/ThreadingUtils>

using namespace FeKit;

TritonContext::TritonContext()
	:m_pOcean(0)
	,m_pEnvironment(NULL)
	,m_pSRS(0)
	,m_pResourceLoader(0)
	,m_bInitFailed(false)
	,m_bInitAttempted(false)
{

}

void TritonContext::Initialize(osg::RenderInfo & renderInfo)
{
	if ( !m_bInitAttempted && !m_bInitFailed )
	{
		osgEarth::Threading::ScopedMutexLock excl(m_InitMutex);
		if ( !m_bInitAttempted && !m_bInitFailed )
		{
			m_bInitAttempted = true;
			
#ifdef _WIN32
			std::string strPath("H:/root/FreeEarth/trunk/resource/triton");
#else
			char * path = getenv("TRITON_PATH");
			std::string strPath(path);
			strPath += "\\Resources";
#endif
			//
			m_pResourceLoader = new ::Triton::ResourceLoader(strPath.c_str());

			m_pEnvironment = new ::Triton::Environment();
            m_pEnvironment->SetLicenseCode("Sundog Software", "1619021d4f224c361f0e1519155212");
			//m_pEnvironment->SetLicenseCode("CC", "CC");

			::Triton::CoordinateSystem cs =
				m_pSRS->isGeographic() ? ::Triton::WGS84_ZUP :
				::Triton::FLAT_ZUP;

			if ( m_pSRS->isGeographic() )
			{
				const osg::EllipsoidModel* ellipsoid = m_pSRS->getEllipsoid();

				std::string eqRadius = osgEarth::Stringify() << ellipsoid->getRadiusEquator();
				std::string poRadius = osgEarth::Stringify() << ellipsoid->getRadiusPolar();

				m_pEnvironment->SetConfigOption( "equatorial-earth-radius-meters", eqRadius.c_str() );
				m_pEnvironment->SetConfigOption( "polar-earth-radius-meters",      poRadius.c_str() );
			}

			::Triton::EnvironmentError err = m_pEnvironment->Initialize(
				cs,
				::Triton::OPENGL_2_0,
				m_pResourceLoader );

			if ( err == ::Triton::SUCCEEDED )
			{
				::Triton::WindFetch wf;
				wf.SetWind( 10.0, 0.0 );
				m_pEnvironment->AddWindFetch( wf );

				m_pOcean = ::Triton::Ocean::Create(
					m_pEnvironment, 
					::Triton::JONSWAP );
			}
			else
			{
				m_bInitFailed = true;
			}
		}
	}
}

void TritonContext::Update( double simTime )
{
	if (m_pOcean != NULL)
	{
		m_pOcean->UpdateSimulation(simTime);
	}
}

void TritonContext::SetSRS(const osgEarth::SpatialReference * srs)
{
	m_pSRS = srs;
}

TritonContext::~TritonContext()
{
	if (m_pOcean != NULL)
	{
		delete m_pOcean;
	}
	
	if (m_pResourceLoader != NULL)
	{
		delete m_pResourceLoader;
	}
	if (m_pEnvironment != NULL)
	{
		delete m_pEnvironment;
	}
}
