
#include <FeSilverliningLib/SilverLining.h>
#include <FeSilverlining/SilverLiningSkyDrawable.h>
#include <FeSilverlining/SilverLiningContext.h>
#include <osgEarth/SpatialReference>

#include <osgEarth/CullingUtils>

#include <iostream>//2016-10-19

#define LC "[SilverLining:FeSkyDrawable] "

namespace FeSilverLining
{
	FeSkyDrawable::FeSkyDrawable(FeSilverLiningContext* SL) :
_SL( SL )
{
	// call this to ensure draw() gets called every frame.
	setSupportsDisplayList( false );

	// not MT-safe (camera updates, etc)
	this->setDataVariance( osg::Object::DYNAMIC );
}

void
	FeSkyDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{
	//osg::Camera* camera = renderInfo.getCurrentCamera();//h00021 2016-10-20
	//h00021 2016-10-20
	osg::Camera* camera = _SL->getCamera();
	if (!camera)
	{
		camera = renderInfo.getCurrentCamera();
		if (!camera)
		{
			return ;
		}
	}//end h00021 2016-10-20

	if ( camera )
	{
		renderInfo.getState()->disableAllVertexArrays();
		_SL->initialize( renderInfo );

		double fovy, ar, znear, zfar;
		//_SL->setCamera(camera);//h00021 2016-10-20

		//renderInfo.getCurrentCamera()->setNearFarRatio(.00000001);

		camera->getProjectionMatrixAsPerspective(fovy, ar, znear, zfar);
		_SL->setSkyBoxSize( zfar < 100000.0 ? zfar : 10000.0 );

		osg::State* st = renderInfo.getState();
		if(st)
		{
			double mv[16];
			int i = 0;
			for (int row = 0; row < 4; row++) 
			{
				for (int col = 0; col < 4; col++)
				{
					mv[i++] = (float)(st->getModelViewMatrix()(row,col));
				}
			}
			_SL->getAtmosphere()->SetCameraMatrix(mv);

			double mp[16];
			i = 0;
			for (int row = 0; row < 4; row++) 
			{
				for (int col = 0; col < 4; col++)
				{
					mp[i++] = (float)(st->getProjectionMatrix()(row,col));
				}
			}
			_SL->getAtmosphere()->SetProjectionMatrix(mp);
		}

		_SL->getAtmosphere()->DrawSky(
			false, 
			_SL->getSRS()->isGeographic(),
			_SL->getSkyBoxSize(),
			false,
			false,
			false);

		renderInfo.getState()->dirtyAllVertexArrays();
	}
}

osg::BoundingBox
#if OSG_VERSION_GREATER_THAN(3,3,1)
	FeSkyDrawable::computeBoundingBox() const
#else
	FeSkyDrawable::computeBound() const
#endif
{
	osg::BoundingBox skyBoundBox;
	if ( !_SL->ready() )
		return skyBoundBox;

	::SilverLining::Atmosphere* atmosphere = _SL->getAtmosphere();
	double skyboxSize = _SL->getSkyBoxSize();
	if ( skyboxSize == 0.0 )
		skyboxSize = 1000.0;

	osg::Vec3d radiusVec = osg::Vec3d(skyboxSize, skyboxSize, skyboxSize) * 0.5;
	osg::Vec3d camPos = _SL->getCameraPosition();
	if (_SL->getCamera())
	{
		osg::Vec3f eye, center, up;
		_SL->getCamera()->getViewMatrixAsLookAt(eye, center, up);
		camPos = osg::Vec3d(eye.x(), eye.y(), eye.z());
	}

	skyBoundBox.set( camPos-radiusVec, camPos+radiusVec );

	 //this enables the "blue ring" around the earth when viewing from space.
	bool hasLimb = atmosphere->GetConfigOptionBoolean("enable-atmosphere-from-space");
	if ( hasLimb )
	{
		//// Compute bounds of atmospheric limb centered at (0,0,0)
		double earthRadius = atmosphere->GetConfigOptionDouble("earth-radius-meters");
		earthRadius = _SL->getSRS()->getEllipsoid()->getRadiusEquator();
		double atmosphereHeight = earthRadius + 8435.0/*atmosphere->GetConfigOptionDouble("atmosphere-height")*/;
		double atmosphereThickness = /*atmosphere->GetConfigOptionDouble("atmosphere-scale-height-meters")*/10000.0 + earthRadius;

		osg::BoundingBox atmosphereBox;
		osg::Vec3d atmMin(-atmosphereHeight, -atmosphereHeight, -atmosphereHeight);
		osg::Vec3d atmMax(atmosphereThickness, atmosphereThickness, atmosphereThickness);
		atmosphereBox.set( atmMin, atmMax );
		skyBoundBox.expandBy( atmosphereBox );
	}
	return skyBoundBox;
}

void FeSkyDrawable::accept( osg::NodeVisitor& nv )
{
	if ( nv.getVisitorType() == nv.CULL_VISITOR )
	{
		osgUtil::CullVisitor* cv = osgEarth::Culling::asCullVisitor(nv);
		if(cv)
		{
			osg::CullSettings::ComputeNearFarMode oldCullMode;
			oldCullMode = cv->getComputeNearFarMode();
			//cv->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

			osg::Drawable::accept(nv);

			cv->setComputeNearFarMode(oldCullMode);
		}
	}
}



}

