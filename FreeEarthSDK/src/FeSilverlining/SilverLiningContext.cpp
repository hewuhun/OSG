
#include <FeSilverliningLib/SilverLining.h> // SilverLinking SDK
#include <FeSilverlining/SilverLiningContext.h>
#include <osg/Light>
#include <osgEarth/SpatialReference>


#define LC "[FeSilverLiningContext] "

namespace FeSilverLining
{
	FeSilverLiningContext::FeSilverLiningContext(const FeSilverLiningOptions& options,int nCloudType) :
_options              ( options ),
	_initAttempted        ( false ),
	_initFailed           ( false ),
	_maxAmbientLightingAlt( -1.0 ),
	_atmosphere           ( 0L ),
	_clouds               ( 0L ),
	_minAmbient           ( 0,0,0,0 ),
	m_dHight(8000),
	m_dThickness(200),
	m_dLength(100000),
	m_dWidth(100000),
	m_dDensity (0.6),
	m_dAlpha (0.8),
	m_dMetersPerSecond(10.0),
	m_dDegreesFromNorth(45.0),
	m_dWindMinAltitude(100.0),
	m_dWindMaxAltitude(10000.0),
	m_nCloudType(nCloudType)
	//m_eCloudType(CUMULUS_CONGESTUS)
{
	// Create a SL atmosphere (the main SL object).
	// TODO: plug in the username + license key.
	_atmosphere = new ::SilverLining::Atmosphere(
		options.user()->c_str(),
		options.licenseCode()->c_str() );
}

FeSilverLiningContext::~FeSilverLiningContext()
{
	if ( _atmosphere )
		delete _atmosphere;

	OE_INFO << LC << "Destroyed\n";
}

void
	FeSilverLiningContext::setLight(osg::Light* light)
{
	_light = light;
}

void
	FeSilverLiningContext::setSRS(const osgEarth::SpatialReference* srs)
{
	_srs = srs;
}

void
	FeSilverLiningContext::setMinimumAmbient(const osg::Vec4f& value)
{
	_minAmbient = value;
}

void
	FeSilverLiningContext::initialize(osg::RenderInfo& renderInfo)
{
	if ( !_initAttempted && !_initFailed )
	{
		// lock/double-check:
		osgEarth::Threading::ScopedMutexLock excl(_initMutex);
		if ( !_initAttempted && !_initFailed )
		{
			_initAttempted = true;

			// constant random seed ensures consistent clouds across windows
			// TODO: replace this with something else since this is global! -gw
			::srand(1234);

			int result = _atmosphere->Initialize(
				::SilverLining::Atmosphere::OPENGL,
				_options.resourcePath()->c_str(),
				true,
				0 );
			
			if ( result != ::SilverLining::Atmosphere::E_NOERROR )
			{
				_initFailed = true;
				OE_WARN << LC << "SilverLining failed to initialize: " << result << std::endl;
			}
			else
			{
				OE_INFO << LC << "SilverLining initialized OK!" << std::endl;

				// Defaults for a projected terrain. ECEF terrain vectors are set
				// in updateLocation().
				_atmosphere->SetUpVector( 0.0, 0.0, 1.0 );
				_atmosphere->SetRightVector( 1.0, 0.0, 0.0 );

#if 0 // todo: review this
				_maxAmbientLightingAlt = 
					_atmosphere->GetConfigOptionDouble("atmosphere-height");
#endif

				if ( _options.drawClouds() == true )
				{
					OE_INFO << LC << "Initializing clouds\n";

					SetCloudType(m_nCloudType);
				}
			}
		}
	}
}

void
	FeSilverLiningContext::setupClouds()
{
	_clouds = ::SilverLining::CloudLayerFactory::Create( CUMULUS_CONGESTUS);
	_clouds->SetIsInfinite( true );
	_clouds->SetFadeTowardEdges(true);
	_clouds->SetBaseAltitude( m_dHight );
	_clouds->SetThickness( m_dThickness );
	_clouds->SetBaseLength( m_dLength );
	_clouds->SetBaseWidth( m_dWidth );
	_clouds->SetDensity( m_dDensity );
	_clouds->SetAlpha( m_dAlpha );

	_clouds->SeedClouds( *_atmosphere );
	_clouds->GenerateShadowMaps( false );

	_clouds->SetLayerPosition(0, 0);
	
	::SilverLining::AtmosphericConditions *pConditions =  _atmosphere->GetConditions();
	
	int nCloudLayerIndex = pConditions->AddCloudLayer( _clouds );

}

void
	FeSilverLiningContext::updateLight()
{
	if ( !ready() || !_light.valid() || !_srs.valid() )
		return;

	float ra, ga, ba, rd, gd, bd, x, y, z;

	// Clamp the camera's altitude while fetching the colors so the
	// lighting's ambient component doesn't fade to black at high altitude.
	::SilverLining::Location savedLoc = _atmosphere->GetConditions()->GetLocation();
	::SilverLining::Location clampedLoc = savedLoc;
	if ( _maxAmbientLightingAlt > 0.0 )
	{
		clampedLoc.SetAltitude( std::min(clampedLoc.GetAltitude(), _maxAmbientLightingAlt) );
		_atmosphere->GetConditions()->SetLocation( clampedLoc );
	}

	_atmosphere->GetAmbientColor( &ra, &ga, &ba );
	_atmosphere->GetSunColor( &rd, &gd, &bd );

	// Restore the actual altitude.
	if ( _maxAmbientLightingAlt > 0.0 )
	{
		_atmosphere->GetConditions()->SetLocation( savedLoc );
	}

	if ( _srs->isGeographic() )
	{
		_atmosphere->GetSunPositionGeographic( &x, &y, &z );
	}
	else
	{
		_atmosphere->GetSunPosition(&x, &y, &z);
	}

	osg::Vec3 direction(x, y, z);
	direction.normalize();

	osg::Vec4 ambient(
		osg::clampAbove(ra, _minAmbient.r()),
		osg::clampAbove(ba, _minAmbient.g()),
		osg::clampAbove(ga, _minAmbient.b()),
		1.0);

	_light->setAmbient( ambient );
	_light->setDiffuse( osg::Vec4(rd, gd, bd, 1.0f) );
	_light->setPosition( osg::Vec4(direction, 0.0f) ); //w=0 means "at infinity"
}

void
	FeSilverLiningContext::updateLocation()
{
	if ( !ready() || !_srs.valid() )
		return;

	if ( _srs->isGeographic() )
	{
		// Get new local orientation
		osg::Vec3d up = _cameraPos;
		up.normalize();
		osg::Vec3d north = osg::Vec3d(0, 1, 0);
		osg::Vec3d east = north ^ up;

		// Check for edge case of north or south pole
		if (east.length2() == 0)
		{
			east = osg::Vec3d(1, 0, 0);
		}

		east.normalize();

		_atmosphere->SetUpVector(up.x(), up.y(), up.z());
		_atmosphere->SetRightVector(east.x(), east.y(), east.z());

		// Get new lat / lon / altitude
		osg::Vec3d latLonAlt;
		_srs->transformFromWorld(_cameraPos, latLonAlt);

		::SilverLining::Location loc;
		loc.SetAltitude ( latLonAlt.z() );
		loc.SetLongitude( latLonAlt.x() ); //osg::DegreesToRadians(latLonAlt.x()) );
		loc.SetLatitude ( latLonAlt.y() ); //osg::DegreesToRadians(latLonAlt.y()) );

		_atmosphere->GetConditions()->SetLocation( loc );

#if 0
		if ( _clouds )
		{
#if 1 //TODO: figure out why we need to call this a couple times before
			//      it takes effect. -gw
			static int c = 2;
			if ( c > 0 ) {
				--c;
				_clouds->SetLayerPosition(0, 0);
			}
		}
#endif
#endif
	}
}

void FeSilverLiningContext::SetCloudsBaseAltitude( double dHight )
{
	m_dHight = dHight;
	if(_clouds)
	{
		_clouds->SetBaseAltitude(m_dHight);
	}
}

void FeSilverLiningContext::SetCloudsThickness( double dThickness )
{
	m_dThickness = dThickness;
	if(_clouds)
	{
		_clouds->SetThickness(dThickness);
	}
}

void FeSilverLiningContext::SetCloudsBaseLength( double dLength )
{
	m_dLength = dLength;
	if(_clouds)
	{
		_clouds->SetBaseLength(dLength);
	}
}

void FeSilverLiningContext::SetCloudsBaseWidth( double dWidth )
{
	m_dWidth = dWidth;
	if(_clouds)
	{
		_clouds->SetBaseWidth(dWidth);
	}
}

void FeSilverLiningContext::SetCloudsDensity( double dDensity )
{
	m_dDensity = dDensity;
	if(_clouds)
	{
		_clouds->SetDensity(dDensity);
	}
}

void FeSilverLiningContext::SetCloudsAlpha( double dAlpha )
{
	m_dAlpha = dAlpha;
	if(_clouds)
	{
		_clouds->SetAlpha(m_dAlpha);
	}
}

void FeSilverLiningContext::SetCloudType(int nIndexType)
{
	CloudTypes eCloudType(CUMULUS_CONGESTUS);

	switch (nIndexType)
	{
	case 0://CIRROCUMULUS
	{
		eCloudType = CIRROCUMULUS;
		m_dHight = 5000.0;
	}break;
	case 1://CIRRUS_FIBRATUS
	{
		eCloudType = CIRRUS_FIBRATUS;
		m_dHight = 6000.0;
		m_dDensity = 1.0;
		m_dWidth = 10000.0;
	}break;
	case 2: //STRATUS
	{
		eCloudType = STRATUS;
		m_dHight = 1000.0;
		m_dThickness = 1500.0;
		m_dDensity = 1.0;
	}
	break;
	case 3://CUMULUS_MEDIOCRIS
	{
		eCloudType = CUMULUS_MEDIOCRIS;
		m_dHight = 3500.0;
		m_dWidth = 30000.0;
		m_dThickness = 200.0;
	}break;
	case 4://CUMULUS_CONGESTUS
	{
		eCloudType = CUMULUS_CONGESTUS;
		m_dHight = 4000.0;
		m_dWidth = 30000.0;
		m_dThickness = 200.0;
	}break;
	case 5://CUMULUS_CONGESTUS_HI_RES
	{
		eCloudType = CUMULUS_CONGESTUS_HI_RES;
		m_dWidth = 30000.0;
		m_dHight = 4000.0;
		m_dThickness = 200.0;
	}break;
	case 6://CUMULONIMBUS_CAPPILATUS
	{
		eCloudType = CUMULONIMBUS_CAPPILATUS;
		m_dHight = 1000.0;
		m_dWidth = 5000.0;
		m_dLength = 5000.0;
	}break;
	case 7://STRATOCUMULUS
	{
		eCloudType = STRATOCUMULUS;
		m_dHight = 800.0;
		m_dWidth = 20000.0;
		m_dThickness = 3000.0;
	}break;
	case 8://TOWERING_CUMULUS
	{
		eCloudType = TOWERING_CUMULUS;
		m_dHight = 1500.0;
	}break;
	case 9://SANDSTORM
	{
		eCloudType = SANDSTORM;
		m_dHight = 0.0;
		m_dDensity = 1.0;
		m_dWidth = 30000.0;
		//return;
	}break;

	default:
		break;
	}
	_atmosphere->GetConditions()->RemoveAllCloudLayers();
	_clouds = ::SilverLining::CloudLayerFactory::Create(eCloudType);
	_clouds->SetIsInfinite( true );
	_clouds->SetFadeTowardEdges(true);
	_clouds->SetBaseAltitude( m_dHight );
	_clouds->SetThickness( m_dThickness );
	_clouds->SetBaseLength( m_dLength );
	_clouds->SetBaseWidth( m_dWidth );
	_clouds->SetDensity( m_dDensity );
	_clouds->SetAlpha( m_dAlpha );

	_clouds->SeedClouds( *_atmosphere );
	_clouds->GenerateShadowMaps( false );

	_clouds->SetLayerPosition(0, 0);

	::SilverLining::AtmosphericConditions *pConditions =  _atmosphere->GetConditions();

	int nCloudLayerIndex = pConditions->AddCloudLayer( _clouds );

}


double  FeSilverLiningContext::GetCloudsBaseAltitude()
{
	return m_dHight;
}

double FeSilverLiningContext::GetCloudsThickness()
{
	return m_dThickness;
}

double FeSilverLiningContext::GetCloudsDensity()
{
	return m_dDensity;
}

double FeSilverLiningContext::GetCloudsAlpha()
{
	return m_dAlpha;
}

void FeSilverLiningContext::SetWindSpeed( double metersPerSecond )
{
	m_dMetersPerSecond = metersPerSecond;
	ReinstallWind();
}

double FeSilverLiningContext::GetWindSpeed()
{
	return m_dMetersPerSecond;
}

void FeSilverLiningContext::SetWindDirection( double degreesFromNorth )
{
	m_dDegreesFromNorth = degreesFromNorth;
	ReinstallWind();
}

double FeSilverLiningContext::GetWindDirection()
{
	return m_dDegreesFromNorth;
}

void FeSilverLiningContext::GetWindAltitudeRange( double &dWindMinAltitude,double &dWindMaxAltitude )
{
	dWindMinAltitude = m_dWindMinAltitude;
	dWindMaxAltitude = m_dWindMaxAltitude;
}


void FeSilverLiningContext::SetWindAltitudeRange( double dWindMinAltitude,double dWindMaxAltitude )
{

	if(dWindMinAltitude <dWindMaxAltitude)
	{
		m_dWindMinAltitude = dWindMinAltitude;
		m_dWindMaxAltitude = dWindMaxAltitude;

		ReinstallWind();
	}
}

void FeSilverLiningContext::ReinstallWind()
{
	if(_atmosphere)
	{
		::SilverLining::AtmosphericConditions *pConditions =  _atmosphere->GetConditions();
		pConditions->ClearWindVolumes();

		::SilverLining::WindVolume windVolume ;
		windVolume.SetWindSpeed(m_dMetersPerSecond);
		windVolume.SetDirection(m_dDegreesFromNorth);
		windVolume.SetMinAltitude(m_dWindMinAltitude);
		windVolume.SetMaxAltitude(m_dWindMaxAltitude);
		pConditions->SetWind(windVolume);
	}
	
}










}


