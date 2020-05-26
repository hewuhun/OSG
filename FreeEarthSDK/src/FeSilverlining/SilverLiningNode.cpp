
#include <FeSilverliningLib/SilverLining.h>

#include <FeSilverlining/SilverLiningNode.h>
#include <FeSilverlining/SilverLiningContext.h>
#include <FeSilverlining/SilverLiningSkyDrawable.h>
#include <FeSilverlining/SilverLiningCloudsDrawable.h>

#include <osg/Light>
#include <osg/LightSource>
#include <osgEarth/CullingUtils>

#undef  LC
#define LC "[FeSilverLiningNode] "

namespace FeSilverLining
{
#if 1
FeSilverLiningNode::FeSilverLiningNode(const osgEarth::Map*       map,
                                   const FeSilverLiningOptions& options) :
osgEarth::Util::SkyNode(),
_options     (options),
_lastAltitude(DBL_MAX),
m_blSkyShow(true),
m_blCloudsShow(true),
m_nCloudType(2), 
m_AutoTime(true),
_light(NULL),
m_rpCamera(NULL),
m_rpAtmosphere(NULL)
{
	m_opMap = const_cast<osgEarth::Map*>(map);

	/////////注掉 h00021 2016-10-14 解决光照冲突问题/////
	//// Create a new Light for the Sun.
	//_light = new osg::Light();
	//_light->setLightNum( 0 );
	//_light->setDiffuse( osg::Vec4(1,1,1,1) );
	//_light->setAmbient( osg::Vec4(0.2f, 0.2f, 0.2f, 1) );
	//_light->setPosition( osg::Vec4(1, 0, 0, 0) ); // w=0 means infinity
	//_light->setDirection( osg::Vec3(-1,0,0) );
	//
 //   _lightSource = new osg::LightSource();
 //   _lightSource->setLight( _light.get() );
 //   _lightSource->setReferenceFrame(osg::LightSource::RELATIVE_RF);

 //   // The main silver lining data:
 //   _SL = new FeSilverLiningContext( options );
 //   _SL->setLight( _light.get() );
 //   _SL->setSRS  ( map->getSRS() );

 //   // Geode to hold each of the SL drawables:
 //   _geode = new osg::Geode();
 //   _geode->setCullingActive( false );

 //   // Draws the sky before everything else
	//_skyDrawable = new FeSkyDrawable( _SL.get() );
	//_skyDrawable->getOrCreateStateSet()->setRenderBinDetails( -99, "RenderBin" );
	//_geode->addDrawable( _skyDrawable );

 //   // Clouds draw after everything else
	//_cloudsDrawable = new FeCloudsDrawable( _SL.get() );
	//_cloudsDrawable->getOrCreateStateSet()->setRenderBinDetails( 99, "DepthSortedBin" );
	//_geode->addDrawable( _cloudsDrawable.get() );

 //   // scene lighting
 //   osg::StateSet* stateset = this->getOrCreateStateSet();
 //   _lighting = new osgEarth::PhongLightingEffect();
 //   _lighting->setCreateLightingUniform( false );
 //   _lighting->attach( stateset );

 //   // ensure it's depth sorted and draws after the terrain
 //   //stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
 //   //getOrCreateStateSet()->setRenderBinDetails( 100, "DepthSortedBin" );

 //   // SL requires an update pass.
 //   ADJUST_UPDATE_TRAV_COUNT(this, +1);

 //   // initialize date/time
 //   onSetDateTime();
	////////////end 注掉 h00021 2016-10-14 解决光照冲突问题/////////
}


FeSilverLiningNode::~FeSilverLiningNode()
{
    if ( _lighting.valid() )
        _lighting->detach();
}

void
FeSilverLiningNode::attach(osg::View* view, int lightNum)
{
    _light->setLightNum( lightNum );
    view->setLight( _light.get() );
    view->setLightingMode( osg::View::SKY_LIGHT );
}

void
FeSilverLiningNode::onSetDateTime()
{
	if (m_AutoTime)
	{
		// set the SL local time to UTC/epoch.
		::SilverLining::LocalTime utcTime;
		utcTime.SetFromEpochSeconds( getDateTime().asTimeStamp() );
		_SL->getAtmosphere()->GetConditions()->SetTime( utcTime );
	}
	else
		SetLocalDateTime(m_DateTime);
}

void
FeSilverLiningNode::onSetMinimumAmbient()
{
    _SL->setMinimumAmbient( getMinimumAmbient() );
}

void
FeSilverLiningNode::traverse(osg::NodeVisitor& nv)
{
    if ( _SL && _SL->ready() )
    {
        if ( nv.getVisitorType() == nv.UPDATE_VISITOR )
        {
			int frameNumber = nv.getFrameStamp()->getFrameNumber();
            _skyDrawable->dirtyBound(); 

            if( _cloudsDrawable )
            {
                if ( _lastAltitude <= *_options.cloudsMaxAltitude() )
                {
                    if ( _cloudsDrawable->getNumParents() == 0 )
                        _geode->addDrawable( _cloudsDrawable.get() );
		
                    _cloudsDrawable->dirtyBound();
                }
                else
                {
                    if ( _cloudsDrawable->getNumParents() > 0 )
                        _geode->removeDrawable( _cloudsDrawable.get() );
                }
            }
        }

        else if ( nv.getVisitorType() == nv.CULL_VISITOR )
        {

            // TODO: make this multi-camera safe
			_SL->setCameraPosition( nv.getEyePoint() );
			osgUtil::CullVisitor* cv = osgEarth::Culling::asCullVisitor(nv);
			_SL->getAtmosphere()->SetCameraMatrix( cv->getModelViewMatrix()->ptr() );
			_SL->getAtmosphere()->SetProjectionMatrix( cv->getProjectionMatrix()->ptr() );

			_lastAltitude = _SL->getSRS()->isGeographic() ?
				cv->getEyePoint().length() - _SL->getSRS()->getEllipsoid()->getRadiusEquator() :
			cv->getEyePoint().z();

			_SL->updateLocation();

			//屏蔽 h00021 2016-10-17 解决光照冲突问题
			//_SL->updateLight();

			_SL->getAtmosphere()->UpdateSkyAndClouds();
			_SL->getAtmosphere()->CullObjects();
        }
    }

    osgEarth::Util::SkyNode::traverse( nv );

	if ( _geode.valid() )
	{
		_geode->accept(nv);
	}

    if ( _lightSource.valid() )
    {
        _lightSource->accept(nv);
    }
}

void FeSilverLiningNode::SetSkyShow( bool blShow )
{
	m_blSkyShow = blShow;
	SetDrawableShow(_skyDrawable,blShow);
}

bool FeSilverLiningNode::IsSkyShow()
{
	return m_blSkyShow;
}

void FeSilverLiningNode::SetCloudsShow( bool blShow )
{
	m_blCloudsShow = blShow;
	SetDrawableShow(_cloudsDrawable,blShow);
}

bool FeSilverLiningNode::IsCloudsShow()
{
	return m_blCloudsShow;
}

void FeSilverLiningNode::SetDrawableShow( osg::Drawable *pDrawable,bool blShow )
{
	if(!pDrawable)
	{
		return;
	}
	if(blShow)
	{
		pDrawable->setNodeMask(~0x00);
	}
	else
	{
		pDrawable->setNodeMask(0x00);
	}
}

FeSilverLiningContext * FeSilverLiningNode::GetSilverLiningContext()
{
	return _SL.get();
}

void FeSilverLiningNode::setSunLight(osg::Light* pLight)
{
	if(!pLight)
	{
		return;
	}

	if (_light)
	{
		_light = NULL;
	}
	_light = pLight;
}

osg::Light* FeSilverLiningNode::getSunLight()
{
	return _light.get(); 
}

void FeSilverLiningNode::setCamera( osg::Camera* pCamera )
{
	if (!pCamera)
	{
		return;
	}

	if (m_rpCamera.valid())
	{
		m_rpCamera = NULL;
	}

	m_rpCamera = pCamera;
}

osg::Camera* FeSilverLiningNode::getCamera()
{
	return m_rpCamera.get();
}

void FeSilverLiningNode::SetCloudsAltitude(double dAltitude)
{
	if(_SL.valid())
	{
		_SL->SetCloudsBaseAltitude(dAltitude);
	}
}

double FeSilverLiningNode::GetCloudsAltitude()
{
	double  dAltitude = 100;
	if(_SL.valid())
	{
		dAltitude =  _SL->GetCloudsBaseAltitude();
	}

	return dAltitude;
}

void FeSilverLiningNode::SetCloudsThickness( double dThickness )
{
	if(_SL.valid())
	{
		_SL->SetCloudsThickness(dThickness);
	}
}

double FeSilverLiningNode::GetCloudsThickness()
{
	double  dThickness = 100;
	if(_SL.valid())
	{
		dThickness =  _SL->GetCloudsThickness();
	}

	return dThickness;
}

void FeSilverLiningNode::SetCloudsDensity( double dDensity )
{
	if(_SL.valid())
	{
		_SL->SetCloudsDensity(dDensity);
	}
}

double FeSilverLiningNode::GetCloudsDensity()
{
	double  dDensity = 100;
	if(_SL.valid())
	{
		dDensity =  _SL->GetCloudsDensity();
	}

	return dDensity;
}

void FeSilverLiningNode::SetCloudsAlpha( double dAlpha )
{
	if(_SL.valid())
	{
		_SL->SetCloudsAlpha(dAlpha);
	}
}

double FeSilverLiningNode::GetCloudsAlpha()
{
	double  dAlpha = 100;
	if(_SL.valid())
	{
		dAlpha =  _SL->GetCloudsAlpha();
	}

	return dAlpha;
}

void FeSilverLiningNode::SetWindSpeed( double metersPerSecond )
{
	if(_SL.valid())
	{
		_SL->SetWindSpeed(metersPerSecond);
	}
}

double FeSilverLiningNode::GetWindSpeed()
{
	double  dWindSpeed = 100;
	if(_SL.valid())
	{
		dWindSpeed =  _SL->GetWindSpeed();
	}

	return dWindSpeed;
}

void FeSilverLiningNode::SetWindDirection( double degreesFromNorth )
{
	if(_SL.valid())
	{
		_SL->SetWindDirection(degreesFromNorth);
	}
}

double FeSilverLiningNode::GetWindDirection()
{
	double  dDirection = 100;
	if(_SL.valid())
	{
		dDirection =  _SL->GetWindDirection();
	}

	return dDirection;
}

void FeSilverLiningNode::GetWindAltitudeRange( double &dWindMinAltitude,double &dWindMaxAltitude )
{
	if(_SL.valid())
	{
		_SL->GetWindAltitudeRange(dWindMinAltitude,dWindMaxAltitude);
	}
}


void FeSilverLiningNode::SetWindAltitudeRange( double dWindMinAltitude,double dWindMaxAltitude )
{
	if(_SL.valid())
	{
		_SL->SetWindAltitudeRange(dWindMinAltitude,dWindMaxAltitude);
	}
}

void FeSilverLiningNode::ReSet()
{
	_SL =NULL;

	_SL = new FeSilverLiningContext( _options,m_nCloudType );
	_SL->setLight( _light.get() );
	_SL->setSRS  ( m_opMap->getSRS() );
	_SL->setCamera(m_rpCamera.get());//h00021 2016-10-14

	_geode->removeDrawables(0,_geode->getNumDrawables());
	_skyDrawable = NULL;
	_cloudsDrawable = NULL;

	// Draws the sky before everything else
	_skyDrawable = new FeSkyDrawable( _SL.get() );
	_skyDrawable->getOrCreateStateSet()->setRenderBinDetails( -99, "RenderBin" );
	_geode->addDrawable( _skyDrawable );

	// Clouds draw after everything else
	_cloudsDrawable = new FeCloudsDrawable( _SL.get() );
	_cloudsDrawable->getOrCreateStateSet()->setRenderBinDetails( 99, "DepthSortedBin" );
	_geode->addDrawable( _cloudsDrawable.get() );

	onSetDateTime();
}

void FeSilverLiningNode::SetCloudType( int nCloudType )
{
	m_nCloudType = nCloudType;
	ReSet();
}

int FeSilverLiningNode::GetCloudType()
{
	return m_nCloudType;
}

void FeSilverLiningNode::SetLocalDateTime( osgEarth::DateTime dateTime )
{
	if (m_AutoTime)
		m_AutoTime = false;
	{
		// set the SL local time to UTC/epoch.
		::SilverLining::LocalTime utcTime;
		utcTime.SetFromEpochSeconds( dateTime.asTimeStamp() );
		_SL->getAtmosphere()->GetConditions()->SetTime( utcTime );
	}
}

void FeSilverLiningNode::Init()
{
	// Create a new Light for the Sun.
	if (!_light)
	{
		_light = new osg::Light();
		_light->setLightNum( 0 );
		_light->setDiffuse( osg::Vec4(1,1,1,1) );
		_light->setAmbient( osg::Vec4(0.2f, 0.2f, 0.2f, 1) );
		_light->setPosition( osg::Vec4(1, 0, 0, 0) ); // w=0 means infinity
		_light->setDirection( osg::Vec3(-1,0,0) );
	}

	_lightSource = new osg::LightSource();
	_lightSource->setLight( _light.get() );
	_lightSource->setReferenceFrame(osg::LightSource::RELATIVE_RF);

	// The main silver lining data:
	_SL = new FeSilverLiningContext( _options );
	_SL->setLight( _light.get() );
	_SL->setSRS  ( m_opMap->getSRS() );
	_SL->setCamera(m_rpCamera.get());//h00021 2016-10-14

	// Geode to hold each of the SL drawables:
	_geode = new osg::Geode();
	_geode->setCullingActive( false );

	// Draws the sky before everything else
	_skyDrawable = new FeSkyDrawable( _SL.get() );
	_skyDrawable->getOrCreateStateSet()->setRenderBinDetails( -99, "RenderBin" );
	_geode->addDrawable( _skyDrawable );

	// Clouds draw after everything else
	_cloudsDrawable = new FeCloudsDrawable( _SL.get() );
	_cloudsDrawable->getOrCreateStateSet()->setRenderBinDetails( 99, "DepthSortedBin" );
	_geode->addDrawable( _cloudsDrawable.get() );

	// scene lighting
	osg::StateSet* stateset = this->getOrCreateStateSet();
	_lighting = new osgEarth::PhongLightingEffect();
	_lighting->setCreateLightingUniform( false );
	_lighting->attach( stateset );

	// ensure it's depth sorted and draws after the terrain
	//stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	//getOrCreateStateSet()->setRenderBinDetails( 1002, "DepthSortedBin" );

	// SL requires an update pass.
	ADJUST_UPDATE_TRAV_COUNT(this, +1);

	// initialize date/time
	onSetDateTime();
}

#endif
}

