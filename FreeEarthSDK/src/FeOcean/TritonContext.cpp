#include <FeOcean/TritonContext.h>
#include <OpenThreads/ScopedLock>
#include <OpenThreads/Thread>
#include <osgEarth/ThreadingUtils>
#include <FeUtils/logger/LoggerDef.h>
#if defined(WIN32)
#include <cstdio>
#else
#include <cstdlib>
#endif
using namespace FeOcean;

TritonContext::TritonContext(const std::string& strTritonPath)
	:m_pOcean(0)
	,m_pEnvironment(NULL)
	,m_pResourceLoader(0)
	,m_strTritonPath(strTritonPath)
{
}

bool TritonContext::InitilizeImpl(unsigned int contextID)
{
			
#ifdef _WIN32
    //环境变量
    //m_strTritonPath = std::string(getenv("TRITON_PATH"));
    //m_strTritonPath += "\\Resources";
#else
    //环境变量
//            m_strTritonPath.clear();
//            char * path = getenv("TRITON_PATH");
//            m_strTritonPath = std::string(path);
//			m_strTritonPath += "/Resources";
#endif
	if(NULL == GetSRS()) 
	{
		return false;
	}

	m_pResourceLoader = new ::Triton::ResourceLoader(m_strTritonPath.c_str());

	m_pEnvironment = new ::Triton::Environment();
    m_pEnvironment->SetLicenseCode("Sundog Software", "1619021d4f224c361f0e1519155212");
	//m_pEnvironment->SetLicenseCode("CC", "CC");
	osg::GLExtensions* et = osg::GLExtensions::Get(contextID,false);
	m_pEnvironment->SetExtension(et);

	::Triton::CoordinateSystem cs =
		GetSRS()->isGeographic() ? ::Triton::WGS84_ZUP :
		::Triton::FLAT_ZUP;

	::Triton::EnvironmentError err = m_pEnvironment->Initialize(
		cs,
		::Triton::OPENGL_2_0,
		m_pResourceLoader );

	if ( GetSRS()->isGeographic() )
	{
		const osg::EllipsoidModel* ellipsoid = GetSRS()->getEllipsoid();
		if(ellipsoid)
		{
			std::stringstream eqs;
			eqs.precision(7);
			eqs << ellipsoid->getRadiusEquator();
			std::stringstream ps;
			ps.precision(7);
			ps << ellipsoid->getRadiusPolar();

			m_pEnvironment->SetConfigOption( "equatorial-earth-radius-meters", eqs.str().c_str() );
			m_pEnvironment->SetConfigOption( "polar-earth-radius-meters",      ps.str().c_str() );
		}		
	}

	if ( err == ::Triton::SUCCEEDED )
	{
		::Triton::WindFetch wf;
        wf.SetWind( 10.0, 0.0 );
		m_pEnvironment->AddWindFetch( wf );

		m_pOcean = ::Triton::Ocean::Create(
			m_pEnvironment, 
			::Triton::JONSWAP,true );

		return (NULL != m_pOcean);
	}
			
	return false;	
}

void TritonContext::UpdateImpl( double simTime )
{
	if (m_pOcean != NULL)
	{
		//m_pOcean->UpdateSimulation(simTime);
		m_pOcean->update(simTime);
	}
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
