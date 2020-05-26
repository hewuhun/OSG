#include "FeOcean/WakeGeneratorCallback.h"

#include <FeAlg/CoordConverter.h>

using namespace FeOcean;

WakeGeneratorCallback::WakeGeneratorCallback(TritonNode* t, const Triton::WakeGeneratorParameters& parameters)
    : _triton(t)
    , _lastFrameTime(0.0)
    ,_dirty(true)
    ,_wakeGenerator(NULL)
	,_parameters(parameters)
{

}

void WakeGeneratorCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
    bool firstRun = false;
    if ( _dirty )
    {
        if ( _wakeGenerator!=NULL)
        {
            delete _wakeGenerator;
            _wakeGenerator = NULL;
        }

        if ( _triton.valid() && _triton->GetTritonContext()->GetOcean() )
        {
            _wakeGenerator = new Triton::WakeGenerator(_triton->GetTritonContext()->GetOcean(), _parameters );
            _dirty = false;
            firstRun = true;
		}
    }

    if ( _wakeGenerator!=NULL )
    {
        osg::Vec3d dir;
		osg::Vec3d pos = node->getWorldMatrices()[0].getTrans();
		double velocity = 0.0, time = nv->getFrameStamp()->getSimulationTime();

		if(time == 0) return;

		if ( !firstRun )
        {
            dir = pos - _lastPosition;
			velocity = dir.normalize();
			velocity /= (time > _lastFrameTime) ? (time - _lastFrameTime) : 1.0;

// 			static osg::Vec3d dirLast = dir;
// 			double angle = osg::RadiansToDegrees(std::acos(dirLast*dir));
// 			std::cout <<  angle << std::endl;
// 			dirLast = dir;

 			_wakeGenerator->Update( Triton::Vector3(pos[0], pos[1], pos[2]), 
 				Triton::Vector3(dir[0], dir[1], dir[2]), velocity/6, time );
		}

        _lastFrameTime = time;
		_lastPosition = pos;
    }
    traverse( node, nv );
}
