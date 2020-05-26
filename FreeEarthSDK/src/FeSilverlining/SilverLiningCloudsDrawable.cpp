
#include <FeSilverliningLib/SilverLining.h>
#include <FeSilverlining/SilverLiningCloudsDrawable.h>
#include <FeSilverlining/SilverLiningContext.h>
#include <osgEarth/SpatialReference>

namespace FeSilverLining
{
		FeCloudsDrawable::FeCloudsDrawable(FeSilverLiningContext* SL) :
	_SL( SL )
	{
		// call this to ensure draw() gets called every frame.
		setSupportsDisplayList( false );
	
		// not MT-safe (camera updates, etc)
		this->setDataVariance(osg::Object::DYNAMIC);
	}
	
	void
		FeCloudsDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
	{
		if( _SL->ready() )
		{
			const osg::State* state = renderInfo.getState();
	
			osgEarth::NativeProgramAdapterCollection& adapters = _adapters[ state->getContextID() ]; // thread safe.
			if ( adapters.empty() )
			{
				adapters.push_back( new osgEarth::NativeProgramAdapter(state, _SL->getAtmosphere()->GetSkyShader()) );
				adapters.push_back( new osgEarth::NativeProgramAdapter(state, _SL->getAtmosphere()->GetBillboardShader()) );
				adapters.push_back( new osgEarth::NativeProgramAdapter(state, _SL->getAtmosphere()->GetStarShader()) );
				adapters.push_back( new osgEarth::NativeProgramAdapter(state, _SL->getAtmosphere()->GetPrecipitationShader()) );
	
				SL_VECTOR(unsigned) handles = _SL->getAtmosphere()->GetActivePlanarCloudShaders();
				for(int i=0; i<handles.size(); ++i)          
					adapters.push_back( new osgEarth::NativeProgramAdapter(state, handles[i]) );
			}

			adapters.apply( state );
	
			renderInfo.getState()->disableAllVertexArrays();
			_SL->getAtmosphere()->DrawObjects( true, true, true );
			renderInfo.getState()->dirtyAllVertexArrays();
		}
	}
	
	osg::BoundingBox
	#if OSG_VERSION_GREATER_THAN(3,3,1)
		FeCloudsDrawable::computeBoundingBox() const
	#else
		CloudsDrawable::computeBound() const
	#endif
	{
		osg::BoundingBox cloudBoundBox;
		if ( !_SL->ready() )
			return cloudBoundBox;
	
		double minX, minY, minZ, maxX, maxY, maxZ;
		_SL->getAtmosphere()->GetCloudBounds( minX, minY, minZ, maxX, maxY, maxZ );
		cloudBoundBox.set( osg::Vec3d(minX, minY, minZ), osg::Vec3d(maxX, maxY, maxZ) );
		return cloudBoundBox;
	}

}

